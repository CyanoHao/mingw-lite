import base64
import re
from pathlib import Path
from typing import Dict, List, Optional, Set, Tuple, TypedDict

from .archive import read_archive_members
from .coff import CoffSymbol, is_external, is_weak, parse_coff_object
from .import_desc import ExportEntry, ImportType, export_entry, parse_short_import
from .machine import PeMachine


class ImportDll(TypedDict):
  dll: str
  exports: List[ExportEntry]


class ObjectMember(TypedDict):
  name: str
  b64data: str
  symbols: List[CoffSymbol]


class ImportLibrary(TypedDict):
  imports: List[ImportDll]
  objects: List[ObjectMember]


_ENTRY_KEY = Tuple[str, str, int, str, bool]


def _entry_key(e: ExportEntry) -> _ENTRY_KEY:
  return (e['name'], e.get('target') or '',
          -1 if e['ordinal'] is None else e['ordinal'],
          e['import_type'], e['private'])


def _norm_name(name: str, leading_underscore: bool) -> str:
  if leading_underscore and name and name.startswith('_'):
    return name[1:]
  return name


def _norm_target(target: Optional[str], leading_underscore: bool) -> Optional[str]:
  if not target or not leading_underscore:
    return target
  if target.startswith('_'):
    target = target[1:]
  return re.sub(r'@\d+$', '', target)


def _norm_entry(entry: ExportEntry, leading_underscore: bool) -> ExportEntry:
  if not leading_underscore:
    return entry
  return export_entry(
    name=_norm_name(entry['name'], True),
    target=_norm_target(entry.get('target'), True),
    ordinal=entry['ordinal'],
    import_type=entry['import_type'],
    private=entry['private'],
  )


def _ensure_dll(by_dll: Dict[str, ImportDll], dll_lower: str) -> ImportDll:
  if dll_lower not in by_dll:
    by_dll[dll_lower] = ImportDll(dll=dll_lower, exports=[])
  return by_dll[dll_lower]


def _flush_pending(
    pending: List[Tuple[Optional[str], str, bytes]],
    by_dll: Dict[str, ImportDll],
    dll_lower: str,
    leading_underscore: bool,
) -> None:
  if not pending:
    return
  syms = _ensure_dll(by_dll, dll_lower)
  for func_sym, imp_sym, idata6 in pending:
    # CODE: func_sym defined (e.g. "_Func"), imp_sym="Func"
    #   -> {name:"Func", target:null, import_type:CODE}
    # DATA: func_sym=None (only "__imp_Data" defined), imp_sym="Data"
    #   -> {name:"Data", target:null, import_type:DATA}
    if func_sym is not None:
      name = func_sym
      import_type = ImportType.CODE.name
    else:
      name = imp_sym
      import_type = ImportType.DATA.name

    # .idata$6 = b"\x00\x00Func\x00" -> import_name="Func".
    # target is set only when import_name != symbol name.
    import_name = (idata6[2:].rstrip(b'\x00').decode('ascii', errors='replace')
                   if idata6 and len(idata6) > 2 else '')
    if import_name:
      target = (import_name
                if import_name != _norm_name(name, leading_underscore)
                else None)
    elif func_sym is not None and func_sym != imp_sym:
      target = imp_sym
    else:
      target = None

    entry = _norm_entry(
      export_entry(name=name, target=target, import_type=import_type),
      leading_underscore,
    )
    syms['exports'].append(entry)
  pending.clear()


def read_import_library(path: Path, arch: PeMachine) -> ImportLibrary:
  """
  Parse *path* (a static archive / import library) into a canonical,
  deterministically-sorted ``ImportLibrary`` dict::

    {
      "imports": [ImportDll(dll, exports=[...])],   # sorted by dll
      "objects": [ObjectMember(name, b64data, symbols)],
    }

  Short-import headers and long-form ``.idata$`` members feed ``imports``
  (e.g. libcdecl.a yields dll "cdecl.dll" with CODE/DATA/alias exports);
  remaining COFF members are returned verbatim as ``objects`` (e.g.
  libcdecl_mixed.a carries cdecl_thunk_1.o with symbols
  __imp__cdecl_thunk_1 and _cdecl_thunk_1).

  i386 names are normalized: leading-underscore stripping (_cdecl_function_1
  -> cdecl_function_1) and @N stdcall decoration removal.  The result is sorted and
  de-duplicated for straight-to-JSON serialization.
  """
  leading_underscore = arch.leading_underscore
  members = read_archive_members(path)
  by_dll: Dict[str, ImportDll] = {}
  objects: List[ObjectMember] = []

  pending: List[Tuple[Optional[str], str, bytes]] = []
  current_idata_dll: Optional[str] = None

  # libcdecl.a: weak __imp__cdecl_data_alias_1 -> __imp__cdecl_data_1 is
  # deferred here, later emitted as {name:"cdecl_data_alias_1",
  # target:"cdecl_data_1", import_type:DATA}.
  data_alias_pending: Dict[str, Set[Tuple[str, str]]] = {}

  for member in members:
    parsed = parse_short_import(member.data)
    if parsed is not None:
      syms = _ensure_dll(by_dll, parsed.dll_name)
      syms['exports'].append(_norm_entry(parsed.entry, leading_underscore))
      continue

    obj = parse_coff_object(member.data)
    if obj is None:
      continue

    sections = obj['sections']
    symbols = obj['symbols']

    has_idata = any(s.startswith('.idata$') for s in sections)

    if has_idata:
      idata7 = sections.get('.idata$7')
      if idata7 is not None:
        dll_name = idata7.rstrip(b'\x00').decode('ascii', errors='replace').strip()
        if dll_name and dll_name.endswith('.dll'):
          if current_idata_dll is not None:
            _flush_pending(pending, by_dll, current_idata_dll, leading_underscore)
          current_idata_dll = dll_name.lower()
          continue

      imp_sym: Optional[str] = None
      func_sym: Optional[str] = None
      for sym in symbols:
        if not is_external(sym):
          continue
        name = sym['name']
        if name.startswith('__imp_'):
          imp_sym = name[6:]
        elif sym['type_letter'] == 'U':
          continue
        elif not name.startswith('_head_') and not name.startswith('.') and not name.startswith('__nm_'):
          func_sym = name

      # DATA import: only "__imp_Data" is defined (no "_Data" thunk), so
      # func_sym stays None -> DATA in flush.
      if imp_sym is not None:
        idata6 = sections.get('.idata$6', b'')
        pending.append((func_sym, imp_sym, idata6))
      continue

    # libcdecl.a member "cdecl.dll" (no .idata$ sections) — weak externals:
    #   CODE alias:  _cdecl_alias_1 -> _cdecl_function_1
    #                -> {name:"cdecl_alias_1", target:"cdecl_function_1", CODE}
    #   redundant:   __imp__cdecl_alias_1 -> __imp__cdecl_function_1  (skip;
    #                dlltool regenerates the __imp_ slot from the code alias)
    #   DATA alias:  __imp__cdecl_data_alias_1 -> __imp__cdecl_data_1
    #                -> deferred (see data_alias_pending above)
    name_lower = member.name.lower()
    if name_lower.endswith('.dll'):
      for sym in symbols:
        if not is_weak(sym):
          continue
        weak_name = sym['name']
        target = sym['alias_target']
        if target is None:
          continue
        if weak_name.startswith('__imp_') and target.startswith('__imp_'):
          # e.g. __imp__cdecl_data_alias_1 -> __imp__cdecl_data_1 — defer.
          data_alias_pending.setdefault(name_lower, set()).add(
            (weak_name[6:], target[6:]))
          continue
        if weak_name.startswith('__imp_') or target.startswith('__imp_'):
          continue
        syms = _ensure_dll(by_dll, name_lower)
        syms['exports'].append(
          _norm_entry(export_entry(name=weak_name, target=target),
                      leading_underscore))
      continue

    typed: List[CoffSymbol] = []
    for sym in symbols:
      if not (is_external(sym) or is_weak(sym)):
        continue
      typed.append(sym)
    objects.append(ObjectMember(
      name=member.name,
      b64data=base64.b64encode(member.data).decode('ascii'),
      symbols=typed,
    ))

  if current_idata_dll is not None:
    _flush_pending(pending, by_dll, current_idata_dll, leading_underscore)

  for dll_lower, pairs in data_alias_pending.items():
    syms = _ensure_dll(by_dll, dll_lower)
    existing_names = {e['name'] for e in syms['exports']}
    for x_name, y_target in sorted(pairs):
      entry = _norm_entry(
        export_entry(name=x_name, target=y_target,
                     import_type=ImportType.DATA.name),
        leading_underscore)
      if entry['name'] in existing_names:
        continue
      existing_names.add(entry['name'])
      syms['exports'].append(entry)

  imports_sorted: List[ImportDll] = []
  for imp in sorted(by_dll.values(), key=lambda i: i['dll']):
    seen_keys: Set[_ENTRY_KEY] = set()
    exports: List[ExportEntry] = []
    for e in imp['exports']:
      key = _entry_key(e)
      if key in seen_keys:
        continue
      seen_keys.add(key)
      exports.append(e)
    imports_sorted.append(ImportDll(
      dll=imp['dll'],
      exports=sorted(exports, key=_entry_key),
    ))

  objects_sorted: List[ObjectMember] = []
  for o in sorted(objects, key=lambda m: m['name']):
    seen: Set[Tuple[str, str, Optional[str]]] = set()
    deduped: List[CoffSymbol] = []
    for s in o['symbols']:
      key = (s['name'], s['type_letter'], s['alias_target'])
      if key in seen:
        continue
      seen.add(key)
      deduped.append(s)
    objects_sorted.append(ObjectMember(
      name=o['name'],
      b64data=o['b64data'],
      symbols=sorted(
        deduped,
        key=lambda s: (s['name'], s['type_letter'], s['alias_target'] or ''),
      ),
    ))

  return ImportLibrary(imports=imports_sorted, objects=objects_sorted)
