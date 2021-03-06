/*  This file is part of the Vc library. {{{
Copyright © 2016-2017 Matthias Kretz <kretz@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of contributing organizations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}}}*/

#ifndef TESTS_TESTTYPES_H_
#define TESTS_TESTTYPES_H_

#define WITH_DATAPAR 1
#include "unittest.h"
#include <Vc/datapar>

#ifndef TESTTYPES
#error "Please define TESTTYPES to the list of fundamental types to be tested with datapar/mask."
#endif

using schar = signed char;
using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;
using llong = long long;
using ullong = unsigned long long;
using ldouble = long double;

using testtypes_wo_ldouble = typename filter_list<long double, Typelist<TESTTYPES>>::type;
using testtypes_64_32 = typename filter_list<Typelist<ushort, short, uchar, schar>,
                                            testtypes_wo_ldouble>::type;
using testtypes_fp = typename filter_list<Typelist<ullong, llong, ulong, long, uint, int>,
                                          testtypes_64_32>::type;
using testtypes_float = typename filter_list<double, testtypes_fp>::type;
static_assert(list_size<testtypes_fp>::value <= 2, "filtering the list failed");
static_assert(list_size<testtypes_float>::value <= 1, "filtering the list failed");

// vT {{{1
using vschar = Vc::native_datapar<schar>;
using vuchar = Vc::native_datapar<uchar>;
using vshort = Vc::native_datapar<short>;
using vushort = Vc::native_datapar<ushort>;
using vint = Vc::native_datapar<int>;
using vuint = Vc::native_datapar<uint>;
using vlong = Vc::native_datapar<long>;
using vulong = Vc::native_datapar<ulong>;
using vllong = Vc::native_datapar<llong>;
using vullong = Vc::native_datapar<ullong>;
using vfloat = Vc::native_datapar<float>;
using vdouble = Vc::native_datapar<double>;
using vldouble = Vc::native_datapar<long double>;

// viN/vfN {{{1
template <typename T> using vi8  = Vc::fixed_size_datapar<T, vschar::size()>;
template <typename T> using vi16 = Vc::fixed_size_datapar<T, vshort::size()>;
template <typename T> using vf32 = Vc::fixed_size_datapar<T, vfloat::size()>;
template <typename T> using vi32 = Vc::fixed_size_datapar<T, vint::size()>;
template <typename T> using vf64 = Vc::fixed_size_datapar<T, vdouble::size()>;
template <typename T> using vi64 = Vc::fixed_size_datapar<T, vllong::size()>;
template <typename T>
using vl = typename std::conditional<sizeof(long) == sizeof(llong), vi64<T>, vi32<T>>::type;

// current_native_test_types {{{1
using current_native_test_types =
    expand_one<Template1<Vc::native_datapar>, Typelist<TESTTYPES>>;
using current_native_mask_test_types =
    expand_one<Template1<Vc::native_mask>, Typelist<TESTTYPES>>;

// native_test_types {{{1
typedef concat<
#if defined Vc_HAVE_AVX512_ABI && !defined Vc_HAVE_FULL_AVX512_ABI
    expand_one<Template<base_template, Vc::datapar_abi::avx512>, testtypes_64_32>,
#endif
#if defined Vc_HAVE_AVX_ABI && !defined Vc_HAVE_FULL_AVX_ABI
    expand_one<Template<base_template, Vc::datapar_abi::avx>, testtypes_fp>,
#endif
#if defined Vc_HAVE_SSE_ABI && !defined Vc_HAVE_FULL_SSE_ABI
    expand_one<Template<base_template, Vc::datapar_abi::sse>, testtypes_float>,
#endif
    expand_list<concat<
#ifdef Vc_HAVE_FULL_AVX512_ABI
                    Template<base_template, Vc::datapar_abi::avx512>,
#endif
#ifdef Vc_HAVE_FULL_AVX_ABI
                    Template<base_template, Vc::datapar_abi::avx>,
#endif
#ifdef Vc_HAVE_FULL_SSE_ABI
                    Template<base_template, Vc::datapar_abi::sse>,
#endif
                    Typelist<>>,
                testtypes_wo_ldouble>> native_test_types;

// all_test_types / ALL_TYPES {{{1
typedef concat<
    native_test_types,
    expand_list<Typelist<Template<base_template, Vc::datapar_abi::scalar>,
                         // Template<base_template, Vc::datapar_abi::fixed_size<2>>,
                         Template<base_template, Vc::datapar_abi::fixed_size<3>>,
                         // Template<base_template, Vc::datapar_abi::fixed_size<4>>,
                         // Template<base_template, Vc::datapar_abi::fixed_size<8>>,
                         Template<base_template, Vc::datapar_abi::fixed_size<12>>,
                         // Template<base_template, Vc::datapar_abi::fixed_size<16>>,
                         Template<base_template, Vc::datapar_abi::fixed_size<
                                                     Vc::datapar_abi::max_fixed_size>>>,
                Typelist<TESTTYPES>>> all_test_types;

#define ALL_TYPES (all_test_types)

// reduced_test_types {{{1
typedef concat<native_test_types,
               expand_list<Typelist<Template<base_template, Vc::datapar_abi::scalar>>,
                           Typelist<TESTTYPES>>> reduced_test_types;

//}}}1
#endif  // TESTS_TESTTYPES_H_
