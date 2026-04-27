#pragma once

#include <iconv.h>

#include <stdint.h>

extern "C" struct iconv_descriptor
{
  unsigned int from_cp;
  unsigned int to_cp;
  bool discard_ilseq;
  bool translit;

  uint8_t from_pending[4];
  uint8_t from_pending_len;

  uint16_t pending_high_surrogate;
};
