/***************************************************************************************************
* Copyright (c) 2018, NVIDIA CORPORATION.  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted
* provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of
*       conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of
*       conditions and the following disclaimer in the documentation and/or other materials
*       provided with the distribution.
*     * Neither the name of the NVIDIA CORPORATION nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written
*       permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
* FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TOR (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************************************/

#pragma once

#include "cutlass/cutlass.h"
#include "tools/test/unit/gemm/gemm_testbed.h"
template <typename GemmTraits_>
static void run_gemm(
    int m,
    int n,
    int k,
    int lda,
    int ldb,
    int ldc,
    typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type alpha =
        typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type(1),
    typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type beta =
        typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type(0)) {

  typedef typename GemmTraits_::KernelClass Gemm;
  typename Gemm::Params params;

  test::GemmTestbed<
      typename test::GemmTestbedTraits<
          typename GemmTraits_::GemmConfig::ScalarA>::host_type,  // AType
      typename test::GemmTestbedTraits<
          typename GemmTraits_::GemmConfig::ScalarB>::host_type,  // BType
      typename test::GemmTestbedTraits<
          typename GemmTraits_::Epilogue::ScalarC>::host_type,  // CType
      typename test::GemmTestbedTraits<
          typename GemmTraits_::Epilogue::Accumulators::Element>::host_type,  // Accumulator
      typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type  // Scalar
      >
      testbed(m,
              n,
              k,
              lda,
              ldb,
              ldc,
              test::convert(GemmTraits_::kLayoutA),
              test::convert(GemmTraits_::kLayoutB),
              alpha,
              beta);

  testbed.initialize();

  if (testbed.has_cublas_support()) {
    EXPECT_TRUE(testbed.verify_host_with_cublas());
  }

  params.initialize(testbed.M(),
                    testbed.N(),
                    testbed.K(),
                    testbed.alpha,
                    testbed.ptr_A(),
                    testbed.lda(),
                    testbed.ptr_B(),
                    testbed.ldb(),
                    testbed.beta,
                    testbed.ptr_C_initial(),
                    testbed.ldc(),
                    testbed.ptr_computed(),
                    testbed.ldc());

  Gemm::launch(params);

  cudaError_t result = cudaDeviceSynchronize();
  ASSERT_EQ(result, cudaSuccess) << "\nCUDA kernel launch error: " << cudaGetErrorString(result)
                                 << "\n";

  if (testbed.has_cublas_support()) {
    ASSERT_TRUE(testbed.verify_with_cublas());
  } else {
    ASSERT_TRUE(testbed.verify_with_host());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename GemmTraits_>
static void run_gemm(
    int m,
    int n,
    int k,
    typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type alpha =
        typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type(1.0f),
    typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type beta =
        typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type(0.0f)) {
  //typedef cutlass::gemm::Gemm<GemmTraits_> Gemm;
  typedef typename GemmTraits_::KernelClass Gemm;
  typename Gemm::Params params;

  typedef test::GemmTestbed<
      typename test::GemmTestbedTraits<
          typename GemmTraits_::GemmConfig::ScalarA>::host_type,  // AType
      typename test::GemmTestbedTraits<
          typename GemmTraits_::GemmConfig::ScalarB>::host_type,  // BType
      typename test::GemmTestbedTraits<
          typename GemmTraits_::Epilogue::ScalarC>::host_type,  // CType
      typename test::GemmTestbedTraits<
          typename GemmTraits_::Epilogue::Accumulators::Element>::host_type,  // Accumulator
      typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type  // Scalar
      > GemmTestbed;

  GemmTestbed testbed(m,
              n,
              k,
              test::convert(GemmTraits_::kLayoutA),
              test::convert(GemmTraits_::kLayoutB),
              alpha,
              beta);

  testbed.initialize();

  if (testbed.has_cublas_support()) {
    EXPECT_TRUE(testbed.verify_host_with_cublas());
  }

  params.initialize(testbed.M(),
                    testbed.N(),
                    testbed.K(),
                    testbed.alpha,
                    testbed.ptr_A(),
                    testbed.lda(),
                    testbed.ptr_B(),
                    testbed.ldb(),
                    testbed.beta,
                    testbed.ptr_C_initial(),
                    testbed.ldc(),
                    testbed.ptr_computed(),
                    testbed.ldc());

  Gemm::launch(params);

  cudaError_t result = cudaDeviceSynchronize();
  ASSERT_EQ(result, cudaSuccess) << "\nCUDA kernel launch error: " << cudaGetErrorString(result)
                                 << "\n";

  if (testbed.has_cublas_support()) {
    ASSERT_TRUE(testbed.verify_with_cublas());
  } else {
    ASSERT_TRUE(testbed.verify_with_host());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename GemmTraits_>
static void run_batched_strided_gemm(
    int m,
    int n,
    int k,
    int batch_count,
    typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type alpha =
        typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type(1),
    typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type beta =
        typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type(0)) {
  //typedef cutlass::gemm::Gemm<GemmTraits_> Gemm;
  typedef typename GemmTraits_::KernelClass Gemm;
  typename Gemm::Params params;
  test::GemmTestbed<
      typename test::GemmTestbedTraits<
          typename GemmTraits_::GemmConfig::ScalarA>::host_type,  // AType
      typename test::GemmTestbedTraits<
          typename GemmTraits_::GemmConfig::ScalarB>::host_type,  // BType
      typename test::GemmTestbedTraits<
          typename GemmTraits_::Epilogue::ScalarC>::host_type,  // CType
      typename test::GemmTestbedTraits<
          typename GemmTraits_::Epilogue::Accumulators::Element>::host_type,  // Accumulator
      typename test::GemmTestbedTraits<typename GemmTraits_::Epilogue::Scalar>::host_type  // Scalar
      >
      testbed(m,
              n,
              k,
              batch_count,
              test::convert(GemmTraits_::kLayoutA),
              test::convert(GemmTraits_::kLayoutB),
              alpha,
              beta);

  testbed.initialize();

  // host support is not implemented for strided batched gemm
  // if (testbed.has_cublas_support()) {
  //  EXPECT_TRUE(testbed.verify_host_with_cublas());
  //}

  params.initialize(testbed.M(),
                    testbed.N(),
                    testbed.K(),
                    testbed.alpha,
                    testbed.ptr_A(),
                    testbed.lda(),
                    testbed.get_batch_stride_A(),
                    testbed.ptr_B(),
                    testbed.ldb(),
                    testbed.get_batch_stride_B(),
                    testbed.beta,
                    testbed.ptr_C_initial(),
                    testbed.ldc(),
                    testbed.get_batch_stride_C(),
                    testbed.ptr_computed(),
                    testbed.ldc(),
                    testbed.get_batch_stride_C(),
                    testbed.get_batch_count());

  Gemm::launch(params);

  cudaError_t result = cudaDeviceSynchronize();
  ASSERT_EQ(result, cudaSuccess) << "\nCUDA kernel launch error: " << cudaGetErrorString(result)
                                 << "\n";

  if (testbed.has_cublas_support()) {
    ASSERT_TRUE(testbed.verify_with_cublas());
  } else {
    // ASSERT_TRUE(testbed.verify_with_host());
    ASSERT_TRUE(false) << "host support is not implemented for strided batched gemm" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
