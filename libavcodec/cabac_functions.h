/*
 * H.26L/H.264/AVC/JVT/14496-10/... encoder/decoder
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Context Adaptive Binary Arithmetic Coder inline functions
 */

#ifndef AVCODEC_CABAC_FUNCTIONS_H
#define AVCODEC_CABAC_FUNCTIONS_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "cabac.h"
#include "config.h"

#ifndef UNCHECKED_BITSTREAM_READER
#define UNCHECKED_BITSTREAM_READER !CONFIG_SAFE_BITSTREAM_READER
#endif

#if ARCH_AARCH64
#include "aarch64/cabac.h"
#endif
#if ARCH_ARM
#include "arm/cabac.h"
#endif
#if ARCH_X86
#include "x86/cabac.h"
#endif
#if ARCH_MIPS
#include "mips/cabac.h"
#endif

static const uint8_t *const ff_h264_norm_shift =
    ff_h264_cabac_tables + H264_NORM_SHIFT_OFFSET;
static const uint8_t *const rangeTabLPS =
    ff_h264_cabac_tables + H264_LPS_RANGE_OFFSET;
static const uint8_t *const ff_h264_mlps_state =
    ff_h264_cabac_tables + H264_MLPS_STATE_OFFSET;
static const uint8_t *const ff_h264_last_coeff_flag_offset_8x8 =
    ff_h264_cabac_tables + H264_LAST_COEFF_FLAG_OFFSET_8x8_OFFSET;

#if !defined(get_cabac_bypass) || !defined(get_cabac_terminate)
static void refill(CABACContext *c) {
#if CABAC_BITS == 16
  c->ivlOffset += (c->bytestream[0] << 9) + (c->bytestream[1] << 1);
#else
  c->ivlOffset += c->bytestream[0] << 1;
#endif
  c->ivlOffset -= CABAC_MASK;
#if !UNCHECKED_BITSTREAM_READER
  if (c->bytestream < c->bytestream_end)
#endif
    c->bytestream += CABAC_BITS / 8;
}
#endif

#ifndef get_cabac_terminate
static inline void renorm_cabac_decoder_once(CABACContext *c) {
  int shift = (uint32_t)(c->ivlCurrRange - 0x100) >> 31;
  c->ivlCurrRange <<= shift;
  c->ivlOffset <<= shift;
  if (!(c->ivlOffset & CABAC_MASK)) refill(c);
}
#endif

#ifndef get_cabac_inline
static void refill2(CABACContext *c) {
  int i;
  unsigned x;
#if !HAVE_FAST_CLZ
  x = c->ivlOffset ^ (c->ivlOffset - 1);
  i = 7 - ff_h264_norm_shift[x >> (CABAC_BITS - 1)];
#else
  exit(0);
  i = ff_ctz(c->ivlOffset) - CABAC_BITS;
#endif

  x = -CABAC_MASK;

#if CABAC_BITS == 16
  x += (c->bytestream[0] << 9) + (c->bytestream[1] << 1);
#else
  x += c->bytestream[0] << 1;
#endif

  c->ivlOffset += x << i;
#if !UNCHECKED_BITSTREAM_READER
  if (c->bytestream < c->bytestream_end)
#endif
    c->bytestream += CABAC_BITS / 8;
}
#endif

#ifndef get_cabac_inline
static int get_cabac_inline(CABACContext *c, uint8_t *const state) {
  int pStateIdx = *state;
  int codIRangeLPS = rangeTabLPS[2 * (c->ivlCurrRange & 0xC0) + pStateIdx];
  int bit, lps_mask;

  static int yangjing = 0;
  yangjing++;
  printf("yangjing:%d\n", yangjing);
  if (yangjing == 20) {
    int a;
  }

  c->ivlCurrRange -= codIRangeLPS;
  lps_mask = ((c->ivlCurrRange << (CABAC_BITS + 1)) - c->ivlOffset) >> 31;

  //printf("ivlOffset:%d\n", c->ivlOffset );
  c->ivlOffset -= (c->ivlCurrRange << (CABAC_BITS + 1)) & lps_mask;
  c->ivlCurrRange += (codIRangeLPS - c->ivlCurrRange) & lps_mask;

  pStateIdx ^= lps_mask;
  *state = (ff_h264_mlps_state + 128)[pStateIdx];
  bit = pStateIdx & 1;

  lps_mask = ff_h264_norm_shift[c->ivlCurrRange];
  c->ivlCurrRange <<= lps_mask;
  c->ivlOffset <<= lps_mask;
  if (!(c->ivlOffset & CABAC_MASK)) refill2(c);

  //printf("pStateIdx:%d\n", pStateIdx );
  //printf("codIRangeLPS:%d\n", codIRangeLPS );
  //printf("ivlOffset:%d\n", c->ivlOffset );
  //printf("ivlCurrRange:%d\n", c->ivlCurrRange );
  return bit;
}
#endif

static int av_noinline av_unused get_cabac_noinline(CABACContext *c,
                                                    uint8_t *const state) {
  return get_cabac_inline(c, state);
}

// get_cabac_inline()
static int av_unused get_cabac(CABACContext *c, uint8_t *const state) {
  return get_cabac_inline(c, state);
}

#ifndef get_cabac_bypass
static int av_unused get_cabac_bypass(CABACContext *c) {
  int range;
  c->ivlOffset += c->ivlOffset;

  if (!(c->ivlOffset & CABAC_MASK))
    refill(
        c); // Call libavcodec/cabac_functions.h: static void refill(CABACContext *c)

  range = c->ivlCurrRange << (CABAC_BITS + 1);
  if (c->ivlOffset < range) {
    return 0;
  } else {
    c->ivlOffset -= range;
    return 1;
  }
}
#endif

#ifndef get_cabac_bypass_sign
static av_always_inline int get_cabac_bypass_sign(CABACContext *c, int val) {
  int range, mask;
  c->ivlOffset += c->ivlOffset;

  if (!(c->ivlOffset & CABAC_MASK)) refill(c);

  range = c->ivlCurrRange << (CABAC_BITS + 1);
  c->ivlOffset -= range;
  mask = c->ivlOffset >> 31;
  range &= mask;
  c->ivlOffset += range;
  return (val ^ mask) - mask;
}
#endif

/**
 * @return the number of bytes read or 0 if no end
 */
#ifndef get_cabac_terminate
static int av_unused get_cabac_terminate(CABACContext *c) {
  c->ivlCurrRange -= 2;
  if (c->ivlOffset < c->ivlCurrRange << (CABAC_BITS + 1)) {
    renorm_cabac_decoder_once(c);
    return 0;
  } else {
    return c->bytestream - c->bytestream_start;
  }
}
#endif

/**
 * Skip @p n bytes and reset the decoder.
 * @return the address of the first skipped byte or NULL if there's less than @p n bytes left
 */
#ifndef skip_bytes
static av_unused const uint8_t *skip_bytes(CABACContext *c, int n) {
  const uint8_t *ptr = c->bytestream;

  if (c->ivlOffset & 0x1) ptr--;
#if CABAC_BITS == 16
  if (c->ivlOffset & 0x1FF) ptr--;
#endif
  if ((int)(c->bytestream_end - ptr) < n) return NULL;
  if (initialization_decoding_engine(c, ptr + n, c->bytestream_end - ptr - n) <
      0)
    return NULL;

  return ptr;
}
#endif

#endif /* AVCODEC_CABAC_FUNCTIONS_H */
