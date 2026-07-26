from concurrent.futures import Future, ThreadPoolExecutor
import json
from pathlib import Path
import shutil
from typing import Dict, List, Optional

from mingw_thunk import PeMachine, apply_thunk

from .profile import BranchProfile
from .util import ensure

ThunkMap = Dict[str, Dict[str, str]]

_ARCH_TO_PE = {
  '32': PeMachine.I386,
  '64': PeMachine.AMD64,
  'arm64': PeMachine.ARM64,
}

def postprocess_crt_import_libraries(
  ver: BranchProfile,
  thunk_lib_dir: Path,
  crt0_lib_dir: Path,
  crt_lib_dir: Path,
  assert_thunk_free: bool,
  assert_thunk_revertible: bool,
  jobs: int,
) -> ThunkMap:
  """
  argument
    thunk_lib_dir = Path('~layer/ABB/thunk')
    crt0_lib_dir = Path('~layer/ABB/crt0')
    crt_lib_dir = Path('~layer/ABB/crt')

  return value
    {
      'kernel32': {
        'GetDynamicTimeZoneInformation': '__ms_GetDynamicTimeZoneInformation',
      },
    }
  """
  ensure(crt_lib_dir)

  thunk_map: ThunkMap = {}

  with ThreadPoolExecutor(max_workers = jobs) as executor:
    futures: List[Future[Dict[str, str]]] = []
    lib_names: List[str] = []

    for imp0 in crt0_lib_dir.glob('*'):
      assert(imp0.is_file())
      file_name = imp0.name

      if file_name.startswith('lib') and file_name.endswith('.a'):
        if file_name == 'libmsvcrt.a':
          continue
        lib_name = file_name[3:-2]

        overlay_file_name = f'liboverlay-{lib_name}.a'
        overlay_ar: Optional[Path] = thunk_lib_dir / overlay_file_name
        alias_ar: Optional[Path] = None
        if not overlay_ar.exists():
          overlay_ar = None
        else:
          alias_file_name = f'libalias-short-{lib_name}.a'
          alias_candidate = thunk_lib_dir / alias_file_name
          if alias_candidate.exists():
            alias_ar = alias_candidate

        imp = crt_lib_dir / file_name
        futures.append(executor.submit(
          apply_thunk,
          _ARCH_TO_PE[ver.arch],
          imp0,
          overlay_ar,
          alias_ar,
          imp,
          ver.short_import,
          assert_thunk_free,
          assert_thunk_revertible,
          '__ms_',
          False,
          '__force_override_mingw_emu__imp_',
        ))
        lib_names.append(lib_name)
      else:
        shutil.copy2(imp0, crt_lib_dir / file_name)

    for lib_name, future in zip(lib_names, futures):
      result = future.result()
      if result:
        thunk_map[lib_name] = result

  return thunk_map

def generate_thunk_revert_map(thunk_map: ThunkMap, output_path: Path):
  if not thunk_map:
    return
  ensure(output_path.parent)
  with open(output_path, 'w') as f:
    json.dump(thunk_map, f, indent=2, sort_keys=True)
    f.write('\n')
