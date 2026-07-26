import enum
import struct
from dataclasses import dataclass
from typing import Optional, TypedDict


IMPORT_HDR_FMT = struct.Struct('<HHHHIIHH')
IMPORT_HDR_SIZE = IMPORT_HDR_FMT.size  # 20

SHORT_IMPORT_SIG = (0x0000, 0xFFFF)


class ImportType(enum.Enum):
  CODE = 0
  DATA = 1
  CONSTANT = 2


class ExportEntry(TypedDict):
  name: str
  target: Optional[str]
  ordinal: Optional[int]
  import_type: str
  private: bool


def export_entry(
    name: str,
    target: Optional[str] = None,
    ordinal: Optional[int] = None,
    import_type: str = ImportType.CODE.name,
    private: bool = False,
) -> ExportEntry:
  return {
    'name': name,
    'target': target,
    'ordinal': ordinal,
    'import_type': import_type,
    'private': private,
  }


@dataclass
class ParsedImport:
  dll_name: str
  entry: ExportEntry
  name_type: int


def _import_type_from_flags(flags: int) -> ImportType:
  typ = flags & 0x3
  if typ == 1:
    return ImportType.DATA
  if typ == 2:
    return ImportType.CONSTANT
  return ImportType.CODE


def parse_short_import(data: bytes) -> Optional[ParsedImport]:
  if len(data) < IMPORT_HDR_SIZE:
    return None
  sig1, sig2, _version, _machine, _timestamp, size_of_data, hint, flags = IMPORT_HDR_FMT.unpack_from(data, 0)
  if sig1 != SHORT_IMPORT_SIG[0] or sig2 != SHORT_IMPORT_SIG[1]:
    return None

  name_type = (flags >> 2) & 0x7   # IMPORT_OBJECT_NAME: 0=ordinal 1=name 2=noprefix 3=undecorate

  raw = data[IMPORT_HDR_SIZE:IMPORT_HDR_SIZE + size_of_data]
  strings = [s.decode('ascii', errors='replace') for s in raw.split(b'\x00') if s]

  if name_type == 0:
    # Ordinal import. GNU dlltool emits both the import name and the DLL
    # name (two strings); the hint field carries the ordinal.  A name-less
    # ordinal (DLL string only) is not useful for import-library regeneration
    # and is treated as unparseable (None).
    if len(strings) >= 2:
      entry = export_entry(
        name = strings[0],
        ordinal = hint,
        import_type = _import_type_from_flags(flags).name,
      )
      return ParsedImport(dll_name = strings[1].lower(), entry = entry,
                          name_type = name_type)
    return None

  # name_type 1 (NAME), 2 (NOPREFIX), 3 (UNDECORATE) all carry an import
  # name string and are handled uniformly below.
  if len(strings) < 2:
    return None

  import_name = strings[0]
  dll_name = strings[1]
  export_name = strings[2] if len(strings) >= 3 else None

  is_alias = export_name is not None and export_name != import_name
  entry = export_entry(
    name = import_name,
    target = export_name if is_alias else None,
    import_type = _import_type_from_flags(flags).name,
  )
  return ParsedImport(dll_name = dll_name.lower(), entry = entry,
                      name_type = name_type)
