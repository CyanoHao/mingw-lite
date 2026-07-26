from enum import Enum


class PeMachine(Enum):
  I386 = 'i386'
  AMD64 = 'amd64'
  ARM64 = 'arm64'

  @property
  def leading_underscore(self) -> bool:
    return self is PeMachine.I386
