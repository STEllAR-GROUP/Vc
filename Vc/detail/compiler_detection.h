/*  This file is part of the Vc library. {{{
Copyright © 2009-2017 Matthias Kretz <kretz@kde.org>

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

#ifndef VC_COMPILER_DETECTION_H_
#define VC_COMPILER_DETECTION_H_

#ifdef DOXYGEN

/**
 * \name Compiler Identification Macros
 * \ingroup Utilities
 */
//@{
/**
 * \ingroup Utilities
 * This macro is defined to a number identifying the ICC version if the current
 * translation unit is compiled with the Intel compiler.
 *
 * For any other compiler this macro is not defined.
 */
#define Vc_ICC __INTEL_COMPILER_BUILD_DATE
#undef Vc_ICC
/**
 * \ingroup Utilities
 * This macro is defined to a number identifying the Clang version if the current
 * translation unit is compiled with the Clang compiler.
 *
 * For any other compiler this macro is not defined.
 */
#define Vc_CLANG (__clang_major__ * 0x10000 + __clang_minor__ * 0x100 + __clang_patchlevel__)
#undef Vc_CLANG
/**
 * \ingroup Utilities
 * This macro is defined to a number identifying the Apple Clang version if the current
 * translation unit is compiled with the Apple Clang compiler.
 *
 * For any other compiler this macro is not defined.
 */
#define Vc_APPLECLANG (__clang_major__ * 0x10000 + __clang_minor__ * 0x100 + __clang_patchlevel__)
#undef Vc_APPLECLANG
/**
 * \ingroup Utilities
 * This macro is defined to a number identifying the GCC version if the current
 * translation unit is compiled with the GCC compiler.
 *
 * For any other compiler this macro is not defined.
 */
#define Vc_GCC (__GNUC__ * 0x10000 + __GNUC_MINOR__ * 0x100 + __GNUC_PATCHLEVEL__)
/**
 * \ingroup Utilities
 * This macro is defined to a number identifying the Microsoft Visual C++ version if
 * the current translation unit is compiled with the Visual C++ (MSVC) compiler.
 *
 * For any other compiler this macro is not defined.
 */
#define Vc_MSVC _MSC_FULL_VER
#undef Vc_MSVC
//@}

#else  // DOXYGEN

// Compiler defines
#ifdef __INTEL_COMPILER
#define Vc_ICC __INTEL_COMPILER_BUILD_DATE
#elif defined(__OPENCC__)
#define Vc_OPEN64 1
#elif defined(__clang__) && defined(__APPLE__) && __clang_major__ >= 6
// this is going to break :-(
#define Vc_APPLECLANG (__clang_major__ * 0x10000 + __clang_minor__ * 0x100 + __clang_patchlevel__)
#elif defined(__clang__)
#define Vc_CLANG (__clang_major__ * 0x10000 + __clang_minor__ * 0x100 + __clang_patchlevel__)
#elif defined(__GNUC__)
#define Vc_GCC (__GNUC__ * 0x10000 + __GNUC_MINOR__ * 0x100 + __GNUC_PATCHLEVEL__)
#elif defined(_MSC_VER)
#define Vc_MSVC _MSC_FULL_VER
#else
#define Vc_UNSUPPORTED_COMPILER 1
#endif

#if !(defined Vc_ICC || (defined Vc_MSVC && Vc_MSVC >= 191025017) || __cplusplus >= 201402L)
#error "Vc requires support for C++14."
#endif

#define Vc_CXX14 1
#if __cplusplus > 201700L
#  define Vc_CXX17 1
#endif

#if defined(__GNUC__) && !defined(Vc_NO_INLINE_ASM)
#define Vc_GNU_ASM 1
#endif

#ifdef Vc_GCC
#  define Vc_HAVE_MAX_ALIGN_T 1
#elif !defined(Vc_CLANG) && !defined(Vc_ICC)
//   Clang/ICC don't provide max_align_t at all
#  define Vc_HAVE_STD_MAX_ALIGN_T 1
#endif

#if defined(Vc_GCC) || defined(Vc_CLANG) || defined Vc_APPLECLANG
#define Vc_USE_BUILTIN_VECTOR_TYPES 1
#endif

#ifdef Vc_MSVC
#  define Vc_CDECL __cdecl
#  define Vc_VDECL __vectorcall
#else
#  define Vc_CDECL
#  define Vc_VDECL
#endif

/* Define the following strings to a unique integer, which is the only type the preprocessor can
 * compare. This allows to use -DVc_IMPL=SSE3. The preprocessor will then consider Vc_IMPL and SSE3
 * to be equal. Of course, it is important to undefine the strings later on!
 */
#define NoSIMD 0x00100000
#define SSE    0x00200000
#define SSE2   0x00300000
#define SSE3   0x00400000
#define SSSE3  0x00500000
#define SSE4_1 0x00600000
#define SSE4_2 0x00700000
#define AVX    0x00800000
#define AVX2   0x00900000
#define MIC    0x00A00000
#define NEON   0x00B00000

#define XOP    0x00000001
#define FMA4   0x00000002
#define F16C   0x00000004
#define POPCNT 0x00000008
#define SSE4a  0x00000010
#define FMA    0x00000020
#define BMI2   0x00000040

#define IMPL_MASK 0xFFF00000
#define EXT_MASK  0x000FFFFF

#ifdef Vc_MSVC
# ifdef _M_IX86_FP
#  if _M_IX86_FP >= 1
#   ifndef __SSE__
#    define __SSE__ 1
#   endif
#  endif
#  if _M_IX86_FP >= 2
#   ifndef __SSE2__
#    define __SSE2__ 1
#   endif
#  endif
# elif defined(_M_AMD64)
// If the target is x86_64 then SSE2 is guaranteed
#  ifndef __SSE__
#   define __SSE__ 1
#  endif
#  ifndef __SSE2__
#   define __SSE2__ 1
#  endif
# endif
#endif

#if defined Vc_ICC && !defined __POPCNT__
# if defined __SSE4_2__ || defined __SSE4A__
#  define __POPCNT__ 1
# endif
#endif

#ifdef VC_IMPL
#error "You are using the old VC_IMPL macro. Since Vc 1.0 all Vc macros start with Vc_, i.e. a lower-case 'c'"
#endif

#ifndef Vc_IMPL

#  if defined(__MIC__)
#    define Vc_IMPL_MIC 1
#  elif defined(__ARM_NEON)
#    define Vc_IMPL_NEON 1
#  elif defined(__AVX2__)
#    define Vc_IMPL_AVX2 1
#    define Vc_IMPL_AVX 1
#  elif defined(__AVX__)
#    define Vc_IMPL_AVX 1
#  else
#    if defined(__SSE4_2__)
#      define Vc_IMPL_SSE 1
#      define Vc_IMPL_SSE4_2 1
#    endif
#    if defined(__SSE4_1__)
#      define Vc_IMPL_SSE 1
#      define Vc_IMPL_SSE4_1 1
#    endif
#    if defined(__SSE3__)
#      define Vc_IMPL_SSE 1
#      define Vc_IMPL_SSE3 1
#    endif
#    if defined(__SSSE3__)
#      define Vc_IMPL_SSE 1
#      define Vc_IMPL_SSSE3 1
#    endif
#    if defined(__SSE2__)
#      define Vc_IMPL_SSE 1
#      define Vc_IMPL_SSE2 1
#    endif

#    if defined(Vc_IMPL_SSE)
       // nothing
#    else
#      define Vc_IMPL_Scalar 1
#    endif
#  endif
#  if !defined(Vc_IMPL_Scalar)
#    ifdef __FMA4__
#      define Vc_IMPL_FMA4 1
#    endif
#    ifdef __XOP__
#      define Vc_IMPL_XOP 1
#    endif
#    ifdef __F16C__
#      define Vc_IMPL_F16C 1
#    endif
#    ifdef __POPCNT__
#      define Vc_IMPL_POPCNT 1
#    endif
#    ifdef __SSE4A__
#      define Vc_IMPL_SSE4a 1
#    endif
#    ifdef __FMA__
#      define Vc_IMPL_FMA 1
#    endif
#    ifdef __BMI2__
#      define Vc_IMPL_BMI2 1
#    endif
#  endif

#else // Vc_IMPL

#  if (Vc_IMPL & IMPL_MASK) == MIC // MIC supersedes everything else
#    define Vc_IMPL_MIC 1
#    ifdef __POPCNT__
#      define Vc_IMPL_POPCNT 1
#    endif
#  elif (Vc_IMPL & IMPL_MASK) == NEON
#    define Vc_IMPL_NEON 1
#  elif (Vc_IMPL & IMPL_MASK) == AVX2 // AVX2 supersedes SSE
#    define Vc_IMPL_AVX2 1
#    define Vc_IMPL_AVX 1
#  elif (Vc_IMPL & IMPL_MASK) == AVX // AVX supersedes SSE
#    define Vc_IMPL_AVX 1
#  elif (Vc_IMPL & IMPL_MASK) == NoSIMD
#    define Vc_IMPL_Scalar 1
#  elif (Vc_IMPL & IMPL_MASK) == SSE4_2
#    define Vc_IMPL_SSE4_2 1
#    define Vc_IMPL_SSE4_1 1
#    define Vc_IMPL_SSSE3 1
#    define Vc_IMPL_SSE3 1
#    define Vc_IMPL_SSE2 1
#    define Vc_IMPL_SSE 1
#  elif (Vc_IMPL & IMPL_MASK) == SSE4_1
#    define Vc_IMPL_SSE4_1 1
#    define Vc_IMPL_SSSE3 1
#    define Vc_IMPL_SSE3 1
#    define Vc_IMPL_SSE2 1
#    define Vc_IMPL_SSE 1
#  elif (Vc_IMPL & IMPL_MASK) == SSSE3
#    define Vc_IMPL_SSSE3 1
#    define Vc_IMPL_SSE3 1
#    define Vc_IMPL_SSE2 1
#    define Vc_IMPL_SSE 1
#  elif (Vc_IMPL & IMPL_MASK) == SSE3
#    define Vc_IMPL_SSE3 1
#    define Vc_IMPL_SSE2 1
#    define Vc_IMPL_SSE 1
#  elif (Vc_IMPL & IMPL_MASK) == SSE2
#    define Vc_IMPL_SSE2 1
#    define Vc_IMPL_SSE 1
#  elif (Vc_IMPL & IMPL_MASK) == SSE
#    define Vc_IMPL_SSE 1
#    if defined(__SSE4_2__)
#      define Vc_IMPL_SSE4_2 1
#    endif
#    if defined(__SSE4_1__)
#      define Vc_IMPL_SSE4_1 1
#    endif
#    if defined(__SSE3__)
#      define Vc_IMPL_SSE3 1
#    endif
#    if defined(__SSSE3__)
#      define Vc_IMPL_SSSE3 1
#    endif
#    if defined(__SSE2__)
#      define Vc_IMPL_SSE2 1
#    endif
#  elif (Vc_IMPL & IMPL_MASK) == 0 && (Vc_IMPL & SSE4a)
     // this is for backward compatibility only where SSE4a was included in the main
     // line of available SIMD instruction sets
#    define Vc_IMPL_SSE3 1
#    define Vc_IMPL_SSE2 1
#    define Vc_IMPL_SSE 1
#  endif
#  if (Vc_IMPL & XOP)
#    define Vc_IMPL_XOP 1
#  endif
#  if (Vc_IMPL & FMA4)
#    define Vc_IMPL_FMA4 1
#  endif
#  if (Vc_IMPL & F16C)
#    define Vc_IMPL_F16C 1
#  endif
#  if (!defined(Vc_IMPL_Scalar) && defined(__POPCNT__)) || (Vc_IMPL & POPCNT)
#    define Vc_IMPL_POPCNT 1
#  endif
#  if (Vc_IMPL & SSE4a)
#    define Vc_IMPL_SSE4a 1
#  endif
#  if (Vc_IMPL & FMA)
#    define Vc_IMPL_FMA 1
#  endif
#  if (Vc_IMPL & BMI2)
#    define Vc_IMPL_BMI2 1
#  endif
#  undef Vc_IMPL

#endif // Vc_IMPL

// If AVX is enabled in the compiler it will use VEX coding for the SIMD instructions.
#ifdef __AVX__
#  define Vc_USE_VEX_CODING 1
#endif

#ifdef Vc_IMPL_AVX
// if we have AVX then we also have all SSE intrinsics
#    define Vc_IMPL_SSE4_2 1
#    define Vc_IMPL_SSE4_1 1
#    define Vc_IMPL_SSSE3 1
#    define Vc_IMPL_SSE3 1
#    define Vc_IMPL_SSE2 1
#    define Vc_IMPL_SSE 1
#endif

#if defined(Vc_CLANG) && Vc_CLANG >= 0x30600 && Vc_CLANG < 0x30700
#    if defined(Vc_IMPL_AVX)
#        warning "clang 3.6.x miscompiles AVX code, frequently losing 50% of the data. Vc will fall back to SSE4 instead."
#        undef Vc_IMPL_AVX
#        if defined(Vc_IMPL_AVX2)
#            undef Vc_IMPL_AVX2
#        endif
#    endif
#endif

# if !defined(Vc_IMPL_Scalar) && !defined(Vc_IMPL_SSE) && !defined(Vc_IMPL_AVX) && !defined(Vc_IMPL_MIC) && !defined(Vc_IMPL_NEON)
#  error "No suitable Vc implementation was selected! Probably Vc_IMPL was set to an invalid value."
# elif defined(Vc_IMPL_SSE) && !defined(Vc_IMPL_SSE2)
#  error "SSE requested but no SSE2 support. Vc needs at least SSE2!"
# endif

#undef NoSIMD
#undef SSE
#undef SSE2
#undef SSE3
#undef SSSE3
#undef SSE4_1
#undef SSE4_2
#undef AVX
#undef AVX2
#undef MIC
#undef NEON

#undef XOP
#undef FMA4
#undef F16C
#undef POPCNT
#undef SSE4a
#undef FMA
#undef BMI2

#undef IMPL_MASK
#undef EXT_MASK

#ifdef Vc_IMPL_MIC
#define Vc_DEFAULT_IMPL_MIC
#elif defined Vc_IMPL_NEON
#define Vc_DEFAULT_IMPL_NEON
#elif defined Vc_IMPL_AVX2
#define Vc_DEFAULT_IMPL_AVX2
#elif defined Vc_IMPL_AVX
#define Vc_DEFAULT_IMPL_AVX
#elif defined Vc_IMPL_SSE
#define Vc_DEFAULT_IMPL_SSE
#elif defined Vc_IMPL_Scalar
#define Vc_DEFAULT_IMPL_Scalar
#else
#error "Preprocessor logic broken. Please report a bug."
#endif

#endif // DOXYGEN

#endif // VC_COMPILER_DETECTION_H_

