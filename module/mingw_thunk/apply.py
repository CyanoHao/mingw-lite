from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
from typing import Dict, List, Optional, Sequence, Set, TypedDict

from pe_coff import PeMachine, read_archive_symbols

from .def_file import build_def_file
from .reader import (
  ImportSymbol,
  ThunkObject,
  extend_unique,
  read_alias_dynamic,
  read_mixed_library,
  thunk_data,
)
from .toolchain import dlltool_arch, triplet


class OverlaySymbol(TypedDict):
  symbol: Set[str]
  force_override: Set[str]


def extract_overlay_symbols(
  overlay_ar: Path,
  arch: PeMachine,
  force_override_prefix: str,
) -> OverlaySymbol:
  """
  argument
    overlay_ar = Path('~thunk/liboverlay-kernel32.a')
    arch = PeMachine.AMD64
    force_override_prefix = '__force_override_mingw_emu__imp_'

  return value
    OverlaySymbol(
      symbol = {'GetDynamicTimeZoneInformation'},
      force_override = {},
    )

  ---

  argument
    overlay_ar = Path('~thunk/liboverlay-msvcrt-os.a')
    arch = PeMachine.AMD64
    force_override_prefix = '__force_override_mingw_emu__imp_'

  return value
    OverlaySymbol(
      symbol = {'_findfirst32', '_wfindfirst32'},
      force_override = {'_findfirst32'},
    )
  """
  nm_members = read_archive_symbols(overlay_ar)

  symbol: Set[str] = set()
  force_override: Set[str] = set()

  def _strip(name: str) -> str:
    if arch.leading_underscore and name and name[0] == '_':
      name = name[1:]
    return name

  for syms in nm_members.values():
    for sym in syms:
      if sym['type_letter'] == 'U':
        continue
      name = sym['name']
      if name.startswith('__imp_'):
        symbol.add(_strip(name[6:]))
      elif name.startswith(force_override_prefix):
        force_override.add(_strip(name[len(force_override_prefix):]))

  return OverlaySymbol(
    symbol = symbol,
    force_override = force_override,
  )


def check_static_objects(
  base_ar: Path,
  static_objects: Sequence[ThunkObject],
  overlay_symbols: Set[str],
  overlay_force_override: Set[str],
  arch: PeMachine,
  allow_override_non_import: bool,
  force_override_prefix: str,
) -> Set[str]:
  """
  argument
    base_ar = Path('lib.a')
    static_objects = [ThunkObject(name='misc.o', b64data='...', symbol=['__imp_xyz'])]
    overlay_symbols = {'printf'}
    overlay_force_override = {'sqrt'}
    arch = PeMachine.AMD64
    allow_override_non_import = False
    force_override_prefix = '__force_override_mingw_emu__imp_'

  return value
    {'misc.o'}

  input
    static_objects = [ThunkObject(name='math.o', b64data='...', symbol=['__imp_sqrt'])]  # otherwise same
  output
    set()                                         # 'sqrt' force-override -> blocked

  input
    static_objects = [ThunkObject(name='stdio.o', b64data='...', symbol=['__imp_printf'])]  # otherwise same
  side effect
    raises Exception                              # 'printf' overlay, not forced

  ---

  input
    allow_override_non_import = True              # with the 'stdio.o' input above
  output
    set()                                         # conflicting object deleted, no raise
  """
  if not static_objects:
    return set()

  def _strip_imp(raw_sym: str) -> str:
    name = raw_sym[6:]
    if arch.leading_underscore and name and name[0] == '_':
      name = name[1:]
    return name

  result: Set[str] = set()
  for obj in static_objects:
    blocked = False
    conflict = False
    has_force = False
    for raw_sym in obj['symbol']:
      if raw_sym.startswith(force_override_prefix):
        has_force = True
        continue
      name = _strip_imp(raw_sym)
      if name in overlay_force_override:
        blocked = True
      elif name in overlay_symbols:
        conflict = True
    # An object is never partially overridden: it is either kept whole,
    # dropped whole, or causes a raise.
    if conflict:
      if not has_force and not allow_override_non_import:
        raise Exception(f'Unexpected override of object {obj["name"]} ({base_ar})')
      continue
    if blocked:
      continue
    result.add(obj['name'])
  return result


def apply_thunk(
  arch: PeMachine,
  base_ar: Path,
  overlay_ar: Optional[Path],
  alias_ar: Optional[Path],
  output_ar: Path,
  short_import: bool,
  assert_thunk_free: bool,
  assert_thunk_revertible: bool,
  thunk_revert_prefix: str,
  allow_override_non_import: bool,
  force_override_prefix: str,
) -> Dict[str, str]:
  """
  argument
    arch = PeMachine.AMD64
    base_ar = Path('~crt0/libkernel32.a')
    overlay_ar = Path('~thunk/liboverlay-kernel32.a')
    alias_ar = Path('~thunk/libalias-short-kernel32.a')
    output_ar = Path('~crt/libkernel32.a')
    short_import = True
    assert_thunk_free = False
    assert_thunk_revertible = True
    thunk_revert_prefix = '__ms_'
    allow_override_non_import = False
    force_override_prefix = '__force_override_mingw_emu__imp_'

  return value
    {
      'GetDynamicTimeZoneInformation': '__ms_GetDynamicTimeZoneInformation',
    }
  """
  overlay_symbol: Set[str] = set()
  overlay_force_override: Set[str] = set()
  revert_symbols: Set[str] = set()
  dynamic: Dict[str, ImportSymbol] = {}
  result_names: Set[str] = set()

  has_overlay = overlay_ar is not None

  if has_overlay:
    assert overlay_ar is not None
    symbols = extract_overlay_symbols(overlay_ar, arch, force_override_prefix)
    overlay_symbol, overlay_force_override = symbols['symbol'], symbols['force_override']

    if alias_ar is not None:
      alias = read_alias_dynamic(alias_ar, arch, thunk_revert_prefix)
      revert_symbols = set(alias['revert'])
      for imp in alias['import_']:
        dll_name = imp['dll']
        if dll_name not in dynamic:
          dynamic[dll_name] = ImportSymbol(dll=dll_name, exports=[])
        extend_unique(dynamic[dll_name]['exports'],
          (e for e in imp['exports'] if e['name'] not in overlay_symbol))

    if assert_thunk_free:
      if overlay_symbol:
        raise AssertionError(f'ABI stability broken: {overlay_symbol}')

    if assert_thunk_revertible:
      non_revertible = overlay_symbol - revert_symbols
      if non_revertible:
        raise AssertionError(f'Non-revertible thunks: {non_revertible}')

    if not overlay_symbol:
      has_overlay = False
      dynamic = {}

  # --- debug instrumentation ---
  # Capture the exact state of the overlay/alias inputs so a stale or
  # mismatched overlay library can be diagnosed after the fact.
  _dbg_overlay = {
    'overlay_ar': str(overlay_ar) if overlay_ar else None,
    'overlay_exists': overlay_ar.exists() if overlay_ar is not None else False,
    'overlay_size': overlay_ar.stat().st_size if overlay_ar is not None and overlay_ar.exists() else None,
    'overlay_mtime': overlay_ar.stat().st_mtime if overlay_ar is not None and overlay_ar.exists() else None,
    'alias_ar': str(alias_ar) if alias_ar else None,
    'has_overlay': has_overlay,
    'overlay_symbol_count': len(overlay_symbol),
    'overlay_has_CreateFileA': 'CreateFileA' in overlay_symbol,
    'overlay_symbol_sample': sorted(overlay_symbol)[:40],
    'revert_symbol_count': len(revert_symbols),
    'revert_has_CreateFileA': 'CreateFileA' in revert_symbols,
    'assert_thunk_free': assert_thunk_free,
    'assert_thunk_revertible': assert_thunk_revertible,
  }
  # --- /debug instrumentation ---

  mixed = read_mixed_library(base_ar, arch)
  static_objects = mixed['thunk']
  for imp in mixed['import_']:
    dll_name = imp['dll']
    if dll_name not in dynamic:
      dynamic[dll_name] = ImportSymbol(dll=dll_name, exports=[])
    extend_unique(dynamic[dll_name]['exports'],
      (e for e in imp['exports'] if e['name'] not in overlay_symbol))

  if has_overlay and static_objects and overlay_symbol:
    result_names = check_static_objects(
      base_ar, static_objects, overlay_symbol, overlay_force_override,
      arch, allow_override_non_import, force_override_prefix)
  else:
    if static_objects:
      result_names = {t['name'] for t in static_objects}

  with TemporaryDirectory() as tmpdir:
    mri_content: List[str] = [f'create {output_ar}\n']

    for dll_name, symbols in sorted(dynamic.items()):
      if not symbols['exports']:
        continue
      mri_content.append(f'addlib {dll_name}.a\n')

      if dll_name == 'kernel32.dll':
        dbg = dict(_dbg_overlay)
        dbg['base_ar'] = str(base_ar)
        dbg['output_ar'] = str(output_ar)
        final_names = [e['name'] for e in symbols['exports']]
        dbg['final_exports_count'] = len(final_names)
        dbg['final_has_CreateFileA'] = 'CreateFileA' in final_names

      with open(f'{tmpdir}/{dll_name}.def', 'w') as f:
        f.write(build_def_file(symbols))
      if short_import:
        dlltool = 'llvm-dlltool'
      else:
        dlltool = f'{triplet(arch)}-dlltool'
      subprocess.run([
        dlltool,
        '-m', dlltool_arch(arch),
        '-D', dll_name,
        '-k',
        '-d', f'{dll_name}.def',
        '-l', f'{dll_name}.a',
      ], check = True, cwd = tmpdir)

    if has_overlay:
      mri_content.append(f'addlib {overlay_ar}\n')

    if result_names:
      member_data = {t['name']: thunk_data(t) for t in static_objects}
      for obj in sorted(result_names):
        data = member_data.get(obj)
        if data is not None:
          with open(f'{tmpdir}/{obj}', 'wb') as f:
            f.write(data)
          mri_content.append(f'addmod {obj}\n')

    mri_content.append('save\n')
    mri_content.append('end\n')
    with open(f'{tmpdir}/mri', 'w') as f:
      f.write(''.join(mri_content))
    with open(f'{tmpdir}/mri') as f:
      subprocess.run(
        ['llvm-ar', '-M'],
        cwd = tmpdir,
        check = True,
        stdin = f,
      )

  return {name: thunk_revert_prefix + name for name in sorted(overlay_symbol)}
