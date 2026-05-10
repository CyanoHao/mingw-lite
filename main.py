#!/usr/bin/python3

import argparse
import logging
import os
from pathlib import Path
import shutil
import subprocess
from subprocess import PIPE, Popen
from tempfile import NamedTemporaryFile
from typing import Dict, List, Tuple

from module.args import parse_args
from module.path import ProjectPaths
from module.prepare_source import prepare_source
from module.profile import BranchProfile, resolve_profile
from module.util import ensure, tmpfs

# A = x86_64-linux-gnu or x86_64-linux-musl
# B = {i686,x86_64}-w64-mingw32
# XYZ: build = X, host = Y, target = Z
from module.AAA import build_AAA_library, build_AAA_tool
from module.AAB import build_AAB_compiler, build_AAB_library
from module.ABB import build_ABB_library, build_ABB_test_driver, build_ABB_toolchain

def clean(config: argparse.Namespace, paths: ProjectPaths):
  if paths.build_dir.exists():
    shutil.rmtree(paths.build_dir)
  if not config.no_cross and paths.layer_AAA.prefix.exists():
    shutil.rmtree(paths.layer_AAA.prefix)
  if not config.no_cross and paths.layer_AAB.prefix.exists():
    shutil.rmtree(paths.layer_AAB.prefix)
  if paths.layer_ABB.prefix.exists():
    shutil.rmtree(paths.layer_ABB.prefix)

def prepare_dirs(paths: ProjectPaths):
  paths.assets_dir.mkdir(parents = True, exist_ok = True)
  paths.build_dir.mkdir(parents = True, exist_ok = True)
  paths.dist_dir.mkdir(parents = True, exist_ok = True)

def _sort_tarball(prefix: str, layer: Path):
  files: Dict[Path, List[str]] = {}
  for file in layer.glob('**/*'):
    if not file.is_dir():
      dn = file.relative_to(layer).parent
      fn = file.name
      if dn not in files:
        files[dn] = []
      files[dn].append(fn)

  result = []
  for dn in sorted(files.keys()):
    result.append(f'{prefix}/{dn}/' if prefix else f'{dn}/')
    for fn in sorted(files[dn]):
      result.append(f'{prefix}/{dn}/{fn}' if prefix else f'{dn}/{fn}')
  return result

def _package(root: Path, files: List[str], dst: Path):
  with NamedTemporaryFile(delete = False) as listfile:
    listname = listfile.name
    for fn in files:
      listfile.write(f'{fn}\n'.encode())

  tar = Popen([
    'bsdtar', '-c',
    '-f', '-',
    '-C', root,
    '-T', listname, '-n',
    '--numeric-owner',
  ], stdout = PIPE)
  zstd = Popen([
    'zstd', '-f',
    '--zstd=strat=5,wlog=27,hlog=25,slog=6,ovlog=9',
    '-o', dst,
  ], stdin = tar.stdout)
  tar.stdout.close()
  zstd.communicate()
  tar.wait()
  if tar.returncode != 0 or zstd.returncode != 0:
    raise Exception('bsdtar | zstd failed')

  os.unlink(listname)

def package_layers(root: Path, layers: List[Tuple[str, Path]], dst: Path):
  with tmpfs(root):
    files: List[str] = []
    file_map: Dict[str, str] = {}
    for prefix, layer in layers:
      sorted_part = _sort_tarball(prefix, layer)
      files.extend(sorted_part)
      for fn in sorted_part:
        if fn.endswith('/'):
          continue
        if fn in file_map:
          raise Exception(f'file collision: {fn} in {layer.name} and {file_map[fn]}')
        file_map[fn] = layer.name
      shutil.copytree(layer, root / prefix if prefix else root, dirs_exist_ok = True)
    _package(root, files, dst)

def package_cross(paths: ProjectPaths):
  layers = [
    (paths.abi_name, paths.layer_dir),
  ]
  package_layers(paths.pkg_dir, layers, paths.cross_pkg)

def package_test_driver(ver: BranchProfile, paths: ProjectPaths):
  layers = [('', paths.layer_ABB.test_driver)]
  if not ver.utf8_user_crt:
    layers.append((paths.abi_name, paths.layer_ABB.nowide))
  package_layers(paths.pkg_dir, layers, paths.test_driver_pkg)

def package_mingw(ver: BranchProfile, paths: ProjectPaths):
  layers = [
    (paths.abi_name, paths.layer_ABB.binutils),
    (paths.abi_name, paths.layer_ABB.crt),
    (paths.abi_name, paths.layer_ABB.gcc),
    (paths.abi_name, paths.layer_ABB.gcc_lib),
    (paths.abi_name, paths.layer_ABB.gdb),
    (paths.abi_name, paths.layer_ABB.headers),
    (paths.abi_name, paths.layer_ABB.make),
    (paths.abi_name, paths.layer_ABB.mcfgthread),
    (paths.abi_name, paths.layer_ABB.pkgconf),
    (paths.abi_name, paths.layer_ABB.winpthreads),
  ]
  if ver.utf8_user_crt:
    layers.append((paths.abi_name, paths.layer_ABB.nowide))
  package_layers(paths.pkg_dir, layers, paths.mingw_pkg)

def package_xmake(paths: ProjectPaths):
  package_layers(paths.pkg_dir, [(paths.abi_name, paths.layer_ABB.xmake)], paths.xmake_pkg)

def main():
  config = parse_args(require_build_compiler = True)

  if config.verbose >= 2:
    logging.basicConfig(level = logging.DEBUG)
  elif config.verbose >= 1:
    logging.basicConfig(level = logging.INFO)
  else:
    logging.basicConfig(level = logging.ERROR)

  logging.info("building GCC %s for %s", config.branch, config.profile)

  ver = resolve_profile(config)
  paths = ProjectPaths(config, ver)

  if config.clean:
    clean(config, paths)

  prepare_dirs(paths)

  prepare_source(ver, paths, config.download_only)

  if config.download_only:
    return

  os.environ['XMAKE_ROOT'] = 'y'

  if not config.no_cross:
    build_AAA_library(ver, paths, config)
    build_AAA_tool(ver, paths, config)
    build_AAB_compiler(ver, paths, config)
    build_AAB_library(ver, paths, config)
    package_cross(paths)

  build_ABB_library(ver, paths, config)
  build_ABB_test_driver(ver, paths, config)
  package_test_driver(ver, paths)
  build_ABB_toolchain(ver, paths, config)
  package_mingw(ver, paths)
  package_xmake(paths)

if __name__ == '__main__':
  main()
