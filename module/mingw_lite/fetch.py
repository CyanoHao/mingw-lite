from hashlib import sha256
import logging
from pathlib import Path
import shutil
import subprocess
import time
from typing import Optional, Sequence
from urllib.error import URLError
from urllib.request import urlopen

from .checksum import CHECKSUMS

def validate_and_download(path: Path, urls: Sequence[str]):
  MAX_RETRY = 5
  checksum = CHECKSUMS[path.name]
  if path.exists():
    with open(path, 'rb') as f:
      body = f.read()
      if checksum != sha256(body).hexdigest():
        message = 'Validate fail: %s exists but checksum mismatch' % path.name
        logging.critical(message)
        logging.info('Please delete %s and try again' % path.name)
        raise Exception(message)
  else:
    logging.info('Downloading %s' % path.name)
    retry_count = 0
    last_error: Optional[URLError] = None
    while True:
      retry_count += 1
      for source_index, url in enumerate(urls):
        try:
          logging.info('Trying source %d/%d for %s' % (source_index + 1, len(urls), path.name))
          response = urlopen(url)
          body = response.read()
          if checksum != sha256(body).hexdigest():
            message = 'Download fail: checksum mismatch for %s' % path.name
            logging.critical(message)
            raise Exception(message)
          with open(path, "wb") as f:
            f.write(body)
            return
        except URLError as e:
          last_error = e
          message = f'Download fail: {e.reason} (source {source_index + 1}/{len(urls)}, retry {retry_count}/{MAX_RETRY})'
          if retry_count < MAX_RETRY:
            logging.warning(message)
          else:
            logging.critical(message)
      if retry_count < MAX_RETRY:
        wait_time = 2 ** retry_count
        logging.warning(f'Retry in {wait_time} seconds...')
        time.sleep(wait_time)
      else:
        assert last_error is not None
        raise last_error

def check_and_extract(path: Path, arx: Path):
  # check if already extracted
  if path.exists():
    mark = path / '.patched'
    if mark.exists():
      return False
    else:
      message = f'Extract fail: {path.name} exists but not marked as fully patched'
      logging.critical(message)
      logging.info(f'Please delete {path.name} and try again')
      raise Exception(message)

  # extract
  res = subprocess.run([
    'bsdtar',
    '-xf',
    arx,
    '--no-same-owner',
  ], cwd = path.parent)
  if res.returncode != 0:
    message = 'Extract fail: bsdtar returned %d extracting %s' % (res.returncode, arx.name)
    logging.critical(message)
    raise Exception(message)

  return True

def check_and_sync(dest: Path, src: Path):
  if dest.exists():
    mark = dest / '.patched'
    if mark.exists():
      return False
    else:
      message = f'Sync fail: {dest.name} exists but not marked as fully patched'
      logging.critical(message)
      logging.info(f'Please delete {dest.name} and try again')
      raise Exception(message)

  # sync
  shutil.copytree(src, dest, ignore = shutil.ignore_patterns('.patched'))

  return True

def patch(path: Path, patch: Path):
  subprocess.run(
    ['patch', '-Np1', '-i', patch],
    cwd = path,
    check = True,
  )

def patch_done(path: Path):
  mark = path / '.patched'
  mark.touch()
