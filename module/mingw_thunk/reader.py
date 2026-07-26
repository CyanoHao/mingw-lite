import base64
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Set, Tuple, TypedDict

from pe_coff import (
  CoffSymbol,
  ExportEntry,
  ImportDll,
  PeMachine,
  read_import_library,
)

# ``ImportSymbol`` is the TypedDict (dict) shape of an import descriptor — an
# alias of pe_coff's canonical ``ImportDll`` so the shape is defined in a
# single place. ``pe_coff.read_import_library`` already returns dicts of this
# shape, so every mingw_thunk consumer uses dict-style access directly.
ImportSymbol = ImportDll


_ENTRY_KEY = Tuple[str, Optional[str], Optional[int], str, bool]


def _entry_key(e: ExportEntry) -> _ENTRY_KEY:
  return (e['name'], e.get('target'), e.get('ordinal'),
          e['import_type'], e['private'])


def _add_unique(dst: List[ExportEntry], entry: ExportEntry) -> None:
  key = _entry_key(entry)
  for existing in dst:
    if _entry_key(existing) == key:
      return
  dst.append(entry)


def extend_unique(dst: List[ExportEntry], src: Iterable[ExportEntry]) -> None:
  for e in src:
    _add_unique(dst, e)


class ThunkObject(TypedDict):
  name: str
  b64data: str
  symbol: List[str]


class MixedLibrary(TypedDict):
  import_: List[ImportSymbol]
  thunk: List[ThunkObject]


class AliasLibrary(TypedDict):
  import_: List[ImportSymbol]
  revert: List[str]


def thunk_data(obj: ThunkObject) -> bytes:
  """Decode a ``ThunkObject``'s ``b64data`` back to raw COFF bytes."""
  return base64.b64decode(obj['b64data'])


def _norm_name(name: str, leading_underscore: bool) -> str:
  if leading_underscore and name and name.startswith('_'):
    return name[1:]
  return name


def read_mixed_library(path: Path, arch: PeMachine) -> MixedLibrary:
  """
  Read a base import library as dynamic imports plus the static (thunk)
  objects it carries. Parsing of short imports, long-form ``.idata$``
  descriptors and per-arch name normalization is delegated to
  ``pe_coff.read_import_library``; this wrapper classifies which surviving
  objects are real thunks via ``filter_static_objects``.
  """
  lib = read_import_library(path, arch)
  candidates: Dict[str, Tuple[str, List[CoffSymbol]]] = {
    o['name']: (o['b64data'], o['symbols']) for o in lib['objects']
  }
  import_ = list(lib['imports'])
  thunks = filter_static_objects(
    candidates, import_, arch.leading_underscore)
  return MixedLibrary(import_=import_, thunk=thunks)


def filter_static_objects(
  candidates: Dict[str, Tuple[str, List[CoffSymbol]]],
  dynamic: Sequence[ImportSymbol],
  leading_underscore: bool = False,
) -> List[ThunkObject]:
  """
  argument
    candidates = {
      'thunk.o':   ('<b64>', [CoffSymbol(name='printf', type_letter='T', ...)]),
      'wrapper.o': ('<b64>', [CoffSymbol(name='fprintf', type_letter='T', ...),
                              CoffSymbol(name='__imp___stdio_common_vfprintf', type_letter='U', ...)]),
    }
    dynamic = [ImportSymbol(
                 dll='api-ms-win-crt-stdio-l1-1-0.dll',
                 exports=[export_entry('printf')])]

  return value
    [ThunkObject(name='wrapper.o', b64data='<b64>',
                 symbol=['__imp___stdio_common_vfprintf'])]
    # 'thunk.o' dropped: every defined symbol is an import name
  """
  import_names: Set[str] = set()
  for syms in dynamic:
    for e in syms['exports']:
      import_names.add(e['name'])

  result: List[ThunkObject] = []
  for mname, (b64, typed) in candidates.items():

    defined_names: List[str] = []
    all_imp: List[str] = []
    for sym in typed:
      name = sym['name']
      if (name.startswith('__force_override_mingw_emu__imp_')
          or name.startswith('__imp_')) and sym['type_letter'] != 'U':
        all_imp.append(name)
      if sym['type_letter'] != 'U':
        defined_names.append(name)

    real_defined_raw = [
      n for n in defined_names
      if not n.startswith('__imp_')
      and not n.startswith('__force_override_mingw_emu__imp_')
    ]
    if (real_defined_raw
        and all(n in import_names
                for n in (_norm_name(x, leading_underscore)
                          for x in real_defined_raw))
        and all('__imp_' + n in all_imp for n in real_defined_raw)):
      continue

    result.append(ThunkObject(name=mname, b64data=b64, symbol=list(dict.fromkeys(all_imp))))

  return result


def read_alias_dynamic(
  path: Path,
  arch: PeMachine,
  revert_prefix: str = '__ms_',
) -> AliasLibrary:
  """
  argument
    path = Path('~thunk/libalias-short-kernel32.a')
    arch = PeMachine.AMD64
    revert_prefix = '__ms_'

  return value
    AliasLibrary(
      import_=[ImportSymbol(
                 dll='kernel32.dll',
                 exports=[export_entry('__ms_ExitProcess')])],
      revert=['ExitProcess'])     # revert_prefix stripped

  All parsing (short imports, long-form descriptors, weak-external alias
  promotion and per-arch normalization) is delegated to
  ``pe_coff.read_import_library``; this wrapper only derives the revert set
  by stripping ``revert_prefix`` from every emitted entry name.
  """
  lib = read_import_library(path, arch)
  import_ = list(lib['imports'])
  return AliasLibrary(
    import_=import_,
    revert=_extract_revert_from_dynamic(import_, revert_prefix),
  )


def _extract_revert_from_dynamic(
  dynamic: Sequence[ImportSymbol],
  revert_prefix: str = '__ms_',
) -> List[str]:
  """
  argument
    dynamic = [ImportSymbol(
                 dll='kernel32.dll',
                 exports=[export_entry('__ms_ExitProcess'),
                          export_entry('__ms_foo', target='bar')])]
    revert_prefix = '__ms_'

  return value
    ['ExitProcess', 'foo']   # revert_prefix stripped
  """
  plen = len(revert_prefix)
  revert: Set[str] = set()
  for syms in dynamic:
    for entry in syms['exports']:
      if entry['name'].startswith(revert_prefix):
        revert.add(entry['name'][plen:])
  return list(revert)
