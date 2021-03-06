/*! @file correlate.cc
 *  @brief New file description.
 *  @author Markovtsev Vadim <v.markovtsev@samsung.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright © 2013 Samsung R&D Institute Russia
 *
 *  @section License
 *  Licensed to the Apache Software Foundation (ASF) under one
 *  or more contributor license agreements.  See the NOTICE file
 *  distributed with this work for additional information
 *  regarding copyright ownership.  The ASF licenses this file
 *  to you under the Apache License, Version 2.0 (the
 *  "License"); you may not use this file except in compliance
 *  with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing,
 *  software distributed under the License is distributed on an
 *  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied.  See the License for the
 *  specific language governing permissions and limitations
 *  under the License.
 */

#include <gtest/gtest.h>
#ifndef NO_FFTF
#include <cmath>
#include <simd/correlate.h>
#include <simd/memory.h>
#include <simd/arithmetic.h>
#include <fftf/api.h>

void cross_correlate_reference(const float *__restrict x, size_t xLength,
                         const float *__restrict h, size_t hLength,
                         float *__restrict result) {
  cross_correlate_simd(0, x, xLength, h, hLength, result);
}

void DebugPrintConvolution(const char* name, const float* vec) {
  printf("%s\t", name);
  for (int i = 0; i < 40; i++) {
    printf("%f  ", vec[i]);
  }
  printf("\n");
}

TEST(correlate, cross_correlate_reference) {
  float x[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  float y[] = { 10, 9, 8, 7 };
  float z[11];
  cross_correlate_reference(x, sizeof(x)/sizeof(float),
                            y, sizeof(y)/sizeof(float),
                            z);
  ASSERT_NEAR(z[0], 7, 0.0001f);
  ASSERT_NEAR(z[1], 22, 0.0001f);
  ASSERT_NEAR(z[2], 46, 0.0001f);
  ASSERT_NEAR(z[3], 80, 0.0001f);
  ASSERT_NEAR(z[4], 114, 0.0001f);
  ASSERT_NEAR(z[5], 148, 0.0001f);
  ASSERT_NEAR(z[6], 182, 0.0001f);
  ASSERT_NEAR(z[7], 216, 0.0001f);
  ASSERT_NEAR(z[8], 187, 0.0001f);
  ASSERT_NEAR(z[9], 142, 0.0001f);
  ASSERT_NEAR(z[10], 80, 0.0001f);
}

TEST(correlate, cross_correlate_fft) {
  const int xlen = 1020;
  const int hlen = 50;

  float x[xlen];
  for (int i = 0; i < xlen; i++) {
    x[i] = sinf(i) * 100;
  }
  float h[hlen];
  for (int i = 0; i < hlen; i++) {
    h[i] = i / (hlen - 1.0f);
  }

  float res[xlen + hlen - 1];
  auto handle = cross_correlate_fft_initialize(xlen, hlen);
  cross_correlate_fft(handle, x, h, res);
  cross_correlate_fft_finalize(handle);

  float verif[xlen + hlen - 1];
  cross_correlate_reference(x, xlen, h, hlen, verif);

  DebugPrintConvolution("REFERENCE", verif);
  DebugPrintConvolution("FFT\t", res);

  for (int i = 0; i < xlen + hlen - 1; i++) {
    ASSERT_NEAR(res[i], verif[i], 1E-3) << i;
  }
}

TEST(correlate, cross_correlate_overlap_save) {
  const int xlen = 1021;
  const int hlen = 50;

  float x[xlen];
  for (int i = 0; i < xlen; i++) {
    x[i] = sinf(i) * 100;
  }
  float h[hlen];
  for (int i = 0; i < hlen; i++) {
    h[i] = i / (hlen- 1.0f);
  }

  float verif[xlen + hlen - 1];
  cross_correlate_reference(x, xlen, h, hlen, verif);
  DebugPrintConvolution("REFERENCE", verif);

  float res[xlen + hlen - 1];
  auto handle = cross_correlate_overlap_save_initialize(xlen, hlen);
  cross_correlate_overlap_save(handle, x, h, res);
  cross_correlate_overlap_save_finalize(handle);
  DebugPrintConvolution("OVERLAP-SAVE", res);

  for (int i = 0; i < xlen + hlen - 1; i++) {
    ASSERT_NEAR(res[i], verif[i], 1E-3) << i;
  }
}

TEST(correlate, cross_correlate_simd) {
  const int xlen = 1024;
  const int hlen = 50;

  float x[xlen];
  for (int i = 0; i < xlen; i++) {
    x[i] = sinf(i) * 100;
  }
  float h[hlen];
  for (int i = 0; i < hlen; i++) {
    h[i] = i / (hlen - 1.0f);
  }

  float verif[xlen + hlen - 1];
  cross_correlate_reference(x, xlen, h, hlen, verif);

  float res[xlen + hlen - 1];
  cross_correlate_simd(true, x, xlen, h, hlen, res);

  for (int i = 0; i < xlen + hlen - 1; i++) {
    ASSERT_NEAR(res[i], verif[i], 1E-3) << i;
  }
}

#endif

#include "tests/google/src/gtest_main.cc"
