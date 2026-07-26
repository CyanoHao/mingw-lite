import os
import sys

def add_extra_path():
  _extra_path = f'{os.getcwd()}/module'
  if _extra_path not in sys.path:
    sys.path.append(_extra_path)
