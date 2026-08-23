import json
import logging
from pathlib import Path
from typing import Any, Callable, Dict, List, Set, cast

from pe_coff import read_pe_imports

from .path import ProjectPaths

TOOLCHAIN_LAYERS = (
  'binutils',
  'gcc',
  'gdb',
  'make',
  'pkgconf',
  'test-driver',
  'xmake',
)

# {dll: [api]}
Baseline = Dict[str, List[str]]
# {dll: {api: [binary]}}
FlatImports = Dict[str, Dict[str, List[str]]]
# {binary (relative to layer/ABB, posix): {dll: [api]}}
ToolchainImports = Dict[str, Dict[str, List[str]]]

def baseline_path(paths: ProjectPaths) -> Path:
  return paths.root_dir / 'support/u8crt_verify/toolchain.json'

def collect_toolchain_imports(layer_dir: Path) -> ToolchainImports:
  """
  argument
    layer_dir = Path('~layer')

  return value
    {
      'gcc/bin/gcc.exe': {
        'kernel32.dll': ['CreateFileW'],
      },
    }
  """
  layer_abb_dir = layer_dir / 'ABB'
  result: ToolchainImports = {}

  for layer_name in TOOLCHAIN_LAYERS:
    tool_dir = layer_abb_dir / layer_name
    if not tool_dir.is_dir():
      logging.info('u8crt verify: layer not found, skipped: %s', tool_dir)
      continue
    for image in sorted(tool_dir.rglob('*')):
      if image.suffix.lower() not in ('.exe', '.dll') or not image.is_file():
        continue
      rel = image.relative_to(layer_abb_dir).as_posix()
      result[rel] = read_pe_imports(image)

  if not result:
    raise Exception(f'u8crt verify: no toolchain PE images found under {layer_abb_dir}')
  return result

def _flatten(collected: ToolchainImports) -> FlatImports:
  """
  return value
    {
      'kernel32.dll': {
        'CreateFileW': ['gcc/bin/gcc.exe'],
      },
    }
  """
  flat: FlatImports = {}
  for binary, dlls in sorted(collected.items()):
    for dll, apis in dlls.items():
      by_api = flat.setdefault(dll, {})
      for api in apis:
        by_api.setdefault(api, []).append(binary)
  return flat

def diff_imports(baseline: Baseline, collected: ToolchainImports) -> FlatImports:
  """
  argument
    baseline = {
      'kernel32.dll': ['CreateFileW'],
    }

  return value
    {
      'kernel32.dll': {
        'CreateFileA': ['gcc/bin/gcc.exe'],
      },
    }
  """
  new: FlatImports = {}
  for dll, by_api in _flatten(collected).items():
    known = set(baseline.get(dll, ()))
    for api, binaries in by_api.items():
      if api not in known:
        new.setdefault(dll, {})[api] = binaries
  return new

def load_baseline(path: Path) -> Baseline:
  try:
    text = path.read_text(encoding = 'utf-8')
  except FileNotFoundError:
    raise Exception(
      f'u8crt verify: baseline not found: {path}\n'
      'run `./u8crt_verify.py --u8crt-verify-update` to create it, review it, and commit') from None

  try:
    raw = json.loads(text)
  except json.JSONDecodeError as e:
    raise Exception(f'u8crt verify: invalid baseline JSON in {path}: {e}') from None

  if not isinstance(raw, dict):
    raise Exception(f'u8crt verify: invalid baseline JSON in {path} (expected an object)')

  data = cast(Dict[str, Any], raw)
  baseline: Baseline = {}
  for dll, apis in data.items():
    if not isinstance(apis, list):
      raise Exception(
        f'u8crt verify: invalid baseline entry {dll!r} in {path} (expected a list of API names)')
    api_list: List[str] = []
    for api in cast(List[Any], apis):
      if not isinstance(api, str):
        raise Exception(
          f'u8crt verify: invalid baseline entry {dll!r} in {path} (expected a list of API names)')
      api_list.append(api)
    baseline[dll] = api_list
  return baseline

def save_baseline(path: Path, dlls: Dict[str, Set[str]]) -> None:
  data = {dll: sorted(apis) for dll, apis in sorted(dlls.items())}
  with open(path, 'w', encoding = 'utf-8') as f:
    json.dump(data, f, indent = 2, sort_keys = True)
    f.write('\n')

def verify(baseline: Path, layer_dir: Path) -> None:
  """
  argument
    baseline = Path('support/u8crt_verify/toolchain.json')
    layer_dir = Path('~layer')
  """
  base = load_baseline(baseline)
  collected = collect_toolchain_imports(layer_dir)

  new = diff_imports(base, collected)
  if new:
    _log_new_imports(new, logging.error)
    raise Exception(
      f'u8crt verify failed: {sum(len(by_api) for by_api in new.values())} new import(s) '
      f'not covered by {baseline}\n'
      'review them (an ANSI API without an UTF-8 thunk breaks the toolchain '
      'on non-UTF-8 systems),\n'
      'then run `./u8crt_verify.py --u8crt-verify-update` and commit the baseline')

  flat = _flatten(collected)
  logging.info(
    'u8crt verify: passed (%d binaries, %d dlls, %d apis)',
    len(collected), len(flat), sum(len(by_api) for by_api in flat.values()))

def update(baseline: Path, layer_dir: Path) -> None:
  """
  argument
    baseline = Path('support/u8crt_verify/toolchain.json')
    layer_dir = Path('~layer')
  """
  collected = collect_toolchain_imports(layer_dir)

  if baseline.exists():
    old = load_baseline(baseline)
  else:
    old = {}
    logging.warning('u8crt verify: baseline not found, creating %s', baseline)

  merged: Dict[str, Set[str]] = {dll: set(apis) for dll, apis in old.items()}
  for dlls in collected.values():
    for dll, apis in dlls.items():
      merged.setdefault(dll, set()).update(apis)

  save_baseline(baseline, merged)

  new = diff_imports(old, collected)
  if new:
    _log_new_imports(new, logging.warning)
    logging.warning(
      'u8crt verify: %d new import(s) merged into %s, review and commit them',
      sum(len(by_api) for by_api in new.values()), baseline)
  else:
    logging.info('u8crt verify: baseline unchanged')

def _log_new_imports(new: FlatImports, log: Callable[..., None]) -> None:
  for dll, by_api in sorted(new.items()):
    for api, binaries in sorted(by_api.items()):
      if len(binaries) > 4:
        shown = f'{", ".join(binaries[:4])}, ... ({len(binaries)} binaries)'
      else:
        shown = ', '.join(binaries)
      log('u8crt verify: new import %s!%s (%s)', dll, api, shown)
