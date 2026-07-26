import struct
from typing import Dict, List, Optional, Tuple, TypedDict

COFF_HDR_FMT = struct.Struct('<HHI II HH')
SEC_HDR_FMT = struct.Struct('<8sIIIIIIHHI')
SYM_FMT = struct.Struct('<8sIHHBB')

SYM_EXTERNAL = 2
SYM_WEAK_EXTERNAL = 105

_KNOWN_MACHINES = frozenset({
  0x014c,  # IMAGE_FILE_MACHINE_I386
  0x8664,  # IMAGE_FILE_MACHINE_AMD64
  0xaa64,  # IMAGE_FILE_MACHINE_ARM64
})

IMAGE_SCN_CNT_CODE = 0x00000020
IMAGE_SCN_CNT_INITIALIZED_DATA = 0x00000040
IMAGE_SCN_CNT_UNINITIALIZED_DATA = 0x00000080


class CoffSymbol(TypedDict):
  name: str
  storage_class: int
  section_number: int
  type_letter: str
  alias_target: Optional[str]


class CoffObject(TypedDict):
  machine: int
  sections: Dict[str, bytes]
  symbols: List[CoffSymbol]


def is_external(sym: CoffSymbol) -> bool:
  return sym['storage_class'] == SYM_EXTERNAL


def is_weak(sym: CoffSymbol) -> bool:
  return sym['storage_class'] == SYM_WEAK_EXTERNAL


def _read_section_headers(data: bytes, nsections: int) -> List[Tuple[str, int, int, int]]:
  headers: List[Tuple[str, int, int, int]] = []
  secoff = COFF_HDR_FMT.size
  for i in range(nsections):
    start = secoff + i * SEC_HDR_FMT.size
    if start + SEC_HDR_FMT.size > len(data):
      break
    vals = SEC_HDR_FMT.unpack_from(data, start)
    name = vals[0].rstrip(b'\x00').decode('ascii', errors='replace')
    raw_size, raw_ptr, characteristics = vals[3], vals[4], vals[9]
    headers.append((name, characteristics, raw_size, raw_ptr))
  return headers


def _classify_section(characteristics: int) -> str:
  if characteristics & IMAGE_SCN_CNT_CODE:
    return 'T'
  if characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA:
    return 'D'
  if characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA:
    return 'B'
  return 'T'


def _read_symbol_name(data: bytes, name_raw: bytes, strtab_off: int) -> str:
  if name_raw[:4] != b'\x00\x00\x00\x00':
    return name_raw.rstrip(b'\x00').decode('ascii', errors='replace')
  str_off = struct.unpack_from('<I', name_raw, 4)[0]
  start = strtab_off + str_off
  if start >= len(data):
    return ''
  rest = data[start:]
  end = rest.find(b'\x00')
  return (rest if end < 0 else rest[:end]).decode('ascii', errors='replace')


def parse_coff_symbols(
  data: bytes, symtab_off: int, nsyms: int, sec_chars: Dict[int, int],
) -> List[CoffSymbol]:
  symbols: List[CoffSymbol] = []
  if symtab_off == 0 or nsyms == 0:
    return symbols

  strtab_off = symtab_off + nsyms * SYM_FMT.size

  names_by_idx: List[Optional[str]] = []
  off = symtab_off
  idx = 0
  while idx < nsyms:
    if off + SYM_FMT.size > len(data):
      break
    name_raw, _value, _sec, _typ, _storage, aux = SYM_FMT.unpack_from(data, off)
    names_by_idx.append(_read_symbol_name(data, name_raw, strtab_off))
    for _ in range(aux):
      names_by_idx.append(None)
    consumed = 1 + aux
    off += SYM_FMT.size * consumed
    idx += consumed

  off = symtab_off
  idx = 0
  while idx < nsyms:
    if off + SYM_FMT.size > len(data):
      break
    name_raw, _value, sec_num, _typ, storage, aux = SYM_FMT.unpack_from(data, off)
    name = _read_symbol_name(data, name_raw, strtab_off)

    alias_target = None
    if storage == SYM_WEAK_EXTERNAL and aux >= 1:
      aux_off = off + SYM_FMT.size
      if aux_off + 4 <= len(data):
        tag_idx: int = struct.unpack_from('<I', data, aux_off)[0]
        if 0 <= tag_idx < len(names_by_idx):
          alias_target = names_by_idx[tag_idx]

    if storage == SYM_WEAK_EXTERNAL:
      letter = 'W'
    elif sec_num == 0:
      letter = 'U'
    else:
      letter = _classify_section(sec_chars.get(sec_num, 0))

    symbols.append(CoffSymbol(
      name=name,
      storage_class=storage,
      section_number=sec_num,
      type_letter=letter,
      alias_target=alias_target,
    ))
    consumed = 1 + aux
    off += SYM_FMT.size * consumed
    idx += consumed

  return symbols


def parse_coff_object(data: bytes) -> Optional[CoffObject]:
  """Parse a COFF object blob into machine, sections, and symbols.

  Returns ``None`` if *data* is too small or has an implausible header.
  Acts as a probe-style parser: callers use ``None`` to determine that
  an archive member is not a COFF object.
  """
  if len(data) < COFF_HDR_FMT.size:
    return None
  machine, nsections, _ts, symtab_off, nsyms, _opt, _char = COFF_HDR_FMT.unpack_from(data, 0)
  if machine not in _KNOWN_MACHINES or nsections == 0:
    return None

  sec_chars: Dict[int, int] = {}
  sections: Dict[str, bytes] = {}
  for i, (sec_name, characteristics, raw_size, raw_ptr) in enumerate(
    _read_section_headers(data, nsections),
  ):
    sec_chars[i + 1] = characteristics
    if raw_size > 0 and raw_ptr + raw_size <= len(data):
      sections[sec_name] = data[raw_ptr:raw_ptr + raw_size]

  symbols = parse_coff_symbols(data, symtab_off, nsyms, sec_chars)
  return CoffObject(machine=machine, sections=sections, symbols=symbols)
