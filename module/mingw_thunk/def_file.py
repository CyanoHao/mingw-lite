from typing import List, Optional

from pe_coff import ExportEntry, ImportType

from .reader import ImportSymbol


def build_def_line(entry: ExportEntry) -> str:
  base = entry['name']
  if entry['ordinal'] is not None:
    base = f'{base} @{entry["ordinal"]}'
  qualifiers: List[str] = []
  if entry['import_type'] == ImportType.CONSTANT.name:
    qualifiers.append('CONSTANT')
  if entry['import_type'] == ImportType.DATA.name:
    qualifiers.append('DATA')
  if entry['private']:
    qualifiers.append('PRIVATE')
  if qualifiers:
    base = base + ' ' + ' '.join(qualifiers)
  if entry['target'] is not None:
    base = f'{base} == {entry["target"]}'
  return base


def build_def_file(symbols: ImportSymbol, dllname: Optional[str] = None) -> str:
  lines: List[str] = []
  if dllname is not None:
    lines.append(f'LIBRARY {dllname}\n')
  lines.append('EXPORTS\n')
  for entry in symbols['exports']:
    lines.append(f'{build_def_line(entry)}\n')
  return ''.join(lines)
