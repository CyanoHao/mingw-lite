#pragma once

#include <minwindef.h>

typedef struct _TEB
{
  BYTE Reserved1[1952];
  void *Reserved2[412];
  void *TlsSlots[64];
  BYTE Reserved3[8];
  void *Reserved4[26];
  void *ReservedForOle;
  void *Reserved5[4];
  void *TlsExpansionSlots;
} TEB;
