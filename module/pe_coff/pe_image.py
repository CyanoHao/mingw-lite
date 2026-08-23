import struct
from pathlib import Path
from typing import Dict, List, Tuple

DOS_MAGIC = b'MZ'
PE_SIGNATURE = b'PE\x00\x00'

PE32_MAGIC = 0x10b
PE32_PLUS_MAGIC = 0x20b

IMPORT_DIRECTORY = 1  # IMAGE_DIRECTORY_ENTRY_IMPORT
DELAY_IMPORT_DIRECTORY = 13  # IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT

IMPORT_DESC_FMT = struct.Struct('<IIIII')  # IMAGE_IMPORT_DESCRIPTOR
DELAY_DESC_FMT = struct.Struct('<IIIIIIII')  # ImgDelayDescr

# (virt_addr, virt_size, raw_size, raw_off)
Section = Tuple[int, int, int, int]


def _parse_sections(data: bytes, pe_off: int) -> List[Section]:
  num_sections = struct.unpack_from('<H', data, pe_off + 6)[0]
  opt_size = struct.unpack_from('<H', data, pe_off + 20)[0]
  sect_off = pe_off + 24 + opt_size
  sections: List[Section] = []
  for i in range(num_sections):
    s = sect_off + i * 40
    if s + 40 > len(data):
      break
    virt_size = struct.unpack_from('<I', data, s + 8)[0]
    virt_addr = struct.unpack_from('<I', data, s + 12)[0]
    raw_size = struct.unpack_from('<I', data, s + 16)[0]
    raw_off = struct.unpack_from('<I', data, s + 20)[0]
    sections.append((virt_addr, virt_size, raw_size, raw_off))
  return sections


def _rva_to_offset(sections: List[Section], rva: int) -> int:
  for virt_addr, virt_size, raw_size, raw_off in sections:
    if virt_addr <= rva < virt_addr + max(virt_size, raw_size):
      return raw_off + (rva - virt_addr)
  raise ValueError(f'RVA {rva:#x} not in any section')


def pe_rva_to_offset(data: bytes, rva: int) -> int:
  pe_off = struct.unpack_from('<I', data, 0x3C)[0]
  return _rva_to_offset(_parse_sections(data, pe_off), rva)


def _read_asciz(data: bytes, off: int) -> str:
  end = data.find(b'\x00', off)
  if end < 0:
    end = len(data)
  return data[off:end].decode('ascii', errors='replace')


def _read_thunks(
    data: bytes, sections: List[Section], rva: int,
    thunk_fmt: struct.Struct, names: List[str],
):
  off = _rva_to_offset(sections, rva)
  ordinal_flag = 1 << (thunk_fmt.size * 8 - 1)
  while True:
    if off + thunk_fmt.size > len(data):
      raise ValueError(f'unterminated thunk table at file offset {off:#x}')
    (thunk,) = thunk_fmt.unpack_from(data, off)
    off += thunk_fmt.size
    if thunk == 0:
      return
    if thunk & ordinal_flag:
      names.append(f'#{thunk & 0xffff}')
    else:
      name_off = _rva_to_offset(sections, thunk)
      names.append(_read_asciz(data, name_off + 2))  # skip Hint/Name hint word


def parse_pe_imports(data: bytes) -> Dict[str, List[str]]:
  """Parse import tables (standard and delay-load) of a PE image blob.

  Returns a mapping from lowercased DLL name to the sorted list of imported
  symbol names; ordinal imports are encoded as ``#<ordinal>``.

  Raises ``ValueError`` if *data* is not a parseable PE32/PE32+ image.
  """
  if len(data) < 0x40 or data[:2] != DOS_MAGIC:
    raise ValueError('missing MZ signature')
  pe_off = struct.unpack_from('<I', data, 0x3C)[0]
  if pe_off + 4 > len(data) or data[pe_off:pe_off + 4] != PE_SIGNATURE:
    raise ValueError('missing PE signature')
  if pe_off + 26 > len(data):
    raise ValueError('truncated PE header')

  magic = struct.unpack_from('<H', data, pe_off + 24)[0]
  if magic == PE32_MAGIC:
    dd_off = pe_off + 24 + 96
    thunk_fmt = struct.Struct('<I')
  elif magic == PE32_PLUS_MAGIC:
    dd_off = pe_off + 24 + 112
    thunk_fmt = struct.Struct('<Q')
  else:
    raise ValueError(f'unknown optional header magic {magic:#x}')

  num_rvas = struct.unpack_from('<I', data, dd_off - 4)[0]
  sections = _parse_sections(data, pe_off)

  def data_dir(index: int) -> int:
    if index >= num_rvas:
      return 0
    return struct.unpack_from('<I', data, dd_off + index * 8)[0]

  result: Dict[str, List[str]] = {}

  def add_dll(name_rva: int, int_rva: int, iat_rva: int):
    name = _read_asciz(data, _rva_to_offset(sections, name_rva)).lower()
    names = result.setdefault(name, [])
    rva = int_rva if int_rva != 0 else iat_rva
    if rva != 0:
      _read_thunks(data, sections, rva, thunk_fmt, names)

  rva = data_dir(IMPORT_DIRECTORY)
  if rva != 0:
    off = _rva_to_offset(sections, rva)
    while True:
      if off + IMPORT_DESC_FMT.size > len(data):
        raise ValueError(f'unterminated import table at file offset {off:#x}')
      int_rva, _ts, _fwd, name_rva, iat_rva = IMPORT_DESC_FMT.unpack_from(data, off)
      off += IMPORT_DESC_FMT.size
      if int_rva == 0 and name_rva == 0 and iat_rva == 0:
        break
      if name_rva == 0:
        raise ValueError('import descriptor without a name')
      add_dll(name_rva, int_rva, iat_rva)

  rva = data_dir(DELAY_IMPORT_DIRECTORY)
  if rva != 0:
    off = _rva_to_offset(sections, rva)
    while True:
      if off + DELAY_DESC_FMT.size > len(data):
        raise ValueError(f'unterminated delay import table at file offset {off:#x}')
      gr_attrs, name_rva, _phmod, iat_rva, int_rva, _bound, _unload, _ts = \
        DELAY_DESC_FMT.unpack_from(data, off)
      off += DELAY_DESC_FMT.size
      if gr_attrs == 0 and name_rva == 0 and iat_rva == 0 and int_rva == 0:
        break
      if gr_attrs & 1 == 0:
        raise ValueError('delay import descriptor is not RVA-form (dlattrRva not set)')
      if name_rva == 0:
        raise ValueError('delay import descriptor without a name')
      add_dll(name_rva, int_rva, iat_rva)

  return {dll: sorted(set(names)) for dll, names in sorted(result.items())}


def read_pe_imports(path: Path) -> Dict[str, List[str]]:
  return parse_pe_imports(path.read_bytes())
