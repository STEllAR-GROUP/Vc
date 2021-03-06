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

#ifndef VC_DATAPAR_TYPE_TRAITS_H_
#define VC_DATAPAR_TYPE_TRAITS_H_

#include "../traits/type_traits.h"

Vc_VERSIONED_NAMESPACE_BEGIN

#ifdef Vc_CXX17
using std::conjunction;
using std::disjunction;
using std::negation;

#else   // Vc_CXX17

// conjunction
template <class... Ts>
struct conjunction : std::true_type {};

template <class A> struct conjunction<A> : public A {};

template <class A, class... Ts>
struct conjunction<A, Ts...>
    : public std::conditional<A::value, conjunction<Ts...>, A>::type {
};

// disjunction
template <class... Ts>
struct disjunction : std::false_type {};

template <class A> struct disjunction<A> : public A {};

template <class A, class... Ts>
struct disjunction<A, Ts...>
    : public std::conditional<A::value, A, disjunction<Ts...>>::type {
};

// negation
template <class T> struct negation : public std::integral_constant<bool, !T::value> {
};

#endif  // Vc_CXX17

template <class... Ts> constexpr bool conjunction_v = conjunction<Ts...>::value;
template <class... Ts> constexpr bool disjunction_v = disjunction<Ts...>::value;
template <class T> constexpr bool negation_v = negation<T>::value;

Vc_VERSIONED_NAMESPACE_END

#endif  // VC_DATAPAR_TYPE_TRAITS_H_
