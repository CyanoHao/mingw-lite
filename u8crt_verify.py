#!/usr/bin/python3

import logging
from pathlib import Path

from _python_path import add_extra_path
add_extra_path()

from mingw_lite import u8crt
from mingw_lite.args import parse_args
from mingw_lite.path import ProjectPaths
from mingw_lite.profile import resolve_profile

def main():
  config = parse_args()

  if config.verbose >= 1:
    logging.basicConfig(level = logging.DEBUG)
  else:
    logging.basicConfig(level = logging.INFO)

  ver = resolve_profile(config)
  paths = ProjectPaths(config, ver)

  if not ver.utf8_thunk:
    logging.info('branch %s does not use UTF-8 thunks, nothing to check', config.branch)
    return

  layer_dir = Path(config.u8crt_verify_layer_dir) if config.u8crt_verify_layer_dir else paths.layer_dir
  baseline = u8crt.baseline_path(paths)

  if config.u8crt_verify_update:
    u8crt.update(baseline, layer_dir)
  else:
    u8crt.verify(baseline, layer_dir)

if __name__ == '__main__':
  main()
