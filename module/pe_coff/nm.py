from pathlib import Path
from typing import Dict, List

from .archive import read_archive_members
from .coff import CoffSymbol, is_external, is_weak, parse_coff_object


def read_archive_symbols(path: Path) -> Dict[str, List[CoffSymbol]]:
  """Return {member_name: [CoffSymbol, ...]} for all external/weak symbols.

  Replaces ``llvm-nm -g`` + ``llvm-ar t``.  Import libraries legitimately
  contain several COFF members that share a name (e.g. multiple
  ``foo.dll`` members); their symbols are merged into a single list under
  that name.
  """
  result: Dict[str, List[CoffSymbol]] = {}
  for member in read_archive_members(path):
    obj = parse_coff_object(member.data)
    if obj is None:
      continue
    typed: List[CoffSymbol] = []
    for sym in obj['symbols']:
      if not (is_external(sym) or is_weak(sym)):
        continue
      typed.append(sym)
    if typed:
      result.setdefault(member.name, []).extend(typed)
  return result
