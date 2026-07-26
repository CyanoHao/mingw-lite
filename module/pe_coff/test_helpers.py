import json

from . import ImportLibrary


def dump_expected(obj: ImportLibrary) -> str:
  return json.dumps(obj, indent=2, sort_keys=True) + '\n'
