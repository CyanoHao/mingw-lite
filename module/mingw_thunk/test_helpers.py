import json
from typing import Dict, List, Tuple, TypedDict

from pe_coff import ExportEntry, ImportDll
from .reader import MixedLibrary


class ThunkEntryDict(TypedDict):
  name: str
  symbol: List[str]


MixedLibraryDict = TypedDict('MixedLibraryDict', {
  'import': List[ImportDll],
  'thunk': List[ThunkEntryDict],
})


class ApplyThunkResultDict(TypedDict):
  result: Dict[str, str]
  output: MixedLibraryDict


def _entry_sort_key(d: ExportEntry) -> Tuple[str, str]:
  return (d['name'] or '', d.get('target') or '')


def serialize_mixed_library(mixed: MixedLibrary) -> MixedLibraryDict:
  return {
    'import': [
      {
        'dll': imp['dll'],
        'exports': sorted(imp['exports'], key = _entry_sort_key),
      }
      for imp in sorted(mixed['import_'], key = lambda i: i['dll'])
    ],
    'thunk': [
      {'name': t['name'], 'symbol': sorted(t['symbol'])}
      for t in sorted(mixed['thunk'], key = lambda t: t['name'])
    ],
  }


def dump_expected(data: MixedLibraryDict) -> str:
  return json.dumps(data, indent = 2, sort_keys = True) + '\n'


def serialize_apply_thunk_result(
  result: Dict[str, str],
  mixed: MixedLibrary,
) -> ApplyThunkResultDict:
  return {
    'result': dict(sorted(result.items())),
    'output': serialize_mixed_library(mixed),
  }
