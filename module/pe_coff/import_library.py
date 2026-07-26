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


def _norm_linker_symbol(name: str, leading_underscore: bool) -> str:
  #   i386: __stricmp -> _stricmp,  __Func@4 -> Func
  if leading_underscore and name.startswith('_'):
    name = name[1:]
  return re.sub(r'@\d+$', '', name)


def _norm_entry(entry: ExportEntry, leading_underscore: bool) -> ExportEntry:
  if not leading_underscore:
    return entry
  return export_entry(
    name=_norm_name(entry['name'], True),
    target=entry.get('target'),
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
    # CODE: func_sym="_access"  -> {name:"access", target:null, CODE}
    # DATA: func_sym=None       -> {name:"environ", target:null, DATA}
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
  Parse an import library into a deterministic, sorted ``ImportLibrary``.

  Short-import headers and long-form ``.idata$`` COFF members → ``imports``;
  non-import COFF objects (thunk .o files) → ``objects``.

  i386: leading ``_`` stripped (__access → _access), ``@N`` removed.
  """
  leading_underscore = arch.leading_underscore
  members = read_archive_members(path)
  by_dll: Dict[str, ImportDll] = {}
  objects: List[ObjectMember] = []

  pending: List[Tuple[Optional[str], str, bytes]] = []
  current_idata_dll: Optional[str] = None

  # Weak-external aliases deferred for two-pass resolution:
  #   __strcmpi -> __stricmp              (CODE)
  #   __imp___strcmpi -> __imp___stricmp  (DATA)
  code_alias_pending: Dict[str, Set[Tuple[str, str]]] = {}
  data_alias_pending: Dict[str, Set[Tuple[str, str]]] = {}

  for member in members:
    parsed = parse_short_import(member.data)
    if parsed is not None:
      syms = _ensure_dll(by_dll, parsed.dll_name)
      entry = parsed.entry
      # name_type 1 on i386: DLL export retains leading underscore.
      #   _strnicmp -> name="strnicmp", target="_strnicmp"
      if leading_underscore and parsed.name_type == 1 \
          and entry['name'].startswith('_'):
        entry = export_entry(
          name=_norm_name(entry['name'], True),
          target=entry['name'],
          ordinal=entry['ordinal'],
          import_type=entry['import_type'],
          private=entry['private'],
        )
      else:
        entry = _norm_entry(entry, leading_underscore)
      syms['exports'].append(entry)
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

    # .dll member — weak externals:
    #   __strcmpi -> __stricmp              (CODE alias, defer)
    #   __imp___strcmpi -> __imp___stricmp  (DATA alias, defer)
    #   __imp__redundant -> __imp__orig     (skip; regenerated from CODE alias)
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
        # CODE alias: _cdecl_alias_1 -> _cdecl_function_1 — defer.
        code_alias_pending.setdefault(name_lower, set()).add(
          (weak_name, target))
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

  # Resolve deferred aliases by looking up the normalised target in
  # already-parsed exports to recover the DLL export name.
  def _resolve_dll_export(syms: ImportDll, linker_sym: str) -> Optional[str]:
    norm = _norm_linker_symbol(linker_sym, leading_underscore)
    for e in syms['exports']:
      if e['name'] == norm:
        return e.get('target') or e['name']
    return norm

  for dll_lower, pairs in code_alias_pending.items():
    syms = _ensure_dll(by_dll, dll_lower)
    existing_names = {e['name'] for e in syms['exports']}
    for weak_name, target in sorted(pairs):
      dll_export = _resolve_dll_export(syms, target)
      entry = _norm_entry(
        export_entry(name=weak_name, target=dll_export),
        leading_underscore)
      if entry['name'] in existing_names:
        continue
      existing_names.add(entry['name'])
      syms['exports'].append(entry)

  for dll_lower, pairs in data_alias_pending.items():
    syms = _ensure_dll(by_dll, dll_lower)
    existing_names = {e['name'] for e in syms['exports']}
    for x_name, y_target in sorted(pairs):
      dll_export = _resolve_dll_export(syms, y_target)
      entry = _norm_entry(
        export_entry(name=x_name, target=dll_export,
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
