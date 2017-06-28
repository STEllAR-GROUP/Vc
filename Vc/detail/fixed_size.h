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

#ifndef VC_DATAPAR_FIXED_SIZE_H_
#define VC_DATAPAR_FIXED_SIZE_H_

#include "datapar.h"
#include "detail.h"
#include <array>

/**
 * The fixed_size ABI gives the following guarantees:
 *  - datapar objects are passed via the stack
 *  - memory layout of `datapar<T, N>` is equivalent to `std::array<T, N>`
 *  - alignment of `datapar<T, N>` is `N * sizeof(T)` if N is a power-of-2 value,
 *    otherwise `next_power_of_2(N * sizeof(T))` (Note: if the alignment were to
 *    exceed the system/compiler maximum, it is bounded to that maximum)
 *  - mask objects are passed like std::bitset<N>
 *  - memory layout of `mask<T, N>` is equivalent to `std::bitset<N>`
 *  - alignment of `mask<T, N>` is equal to the alignment of `std::bitset<N>`
 */

Vc_VERSIONED_NAMESPACE_BEGIN
namespace detail {
// select_best_vector_type_t<T, N>{{{1
/**
 * \internal
 * Selects the best SIMD type out of a typelist to store N scalar values.
 */
struct dummy : public size_constant<~size_t()> {
};

template <class T, int N, class A, class... More>
struct select_best_vector_type {
    using V = std::conditional_t<std::is_destructible<datapar<T, A>>::value,
                                 datapar_size<T, A>, dummy>;
    using type =
        std::conditional_t<(N >= V::value), datapar<T, A>,
                           typename select_best_vector_type<T, N, More...>::type>;
};
template <class T, int N, class A> struct select_best_vector_type<T, N, A> {
    using type = datapar<T, A>;
};
template <class T, int N>
using select_best_vector_type_t = typename select_best_vector_type<T, N,
      datapar_abi::avx512,
      datapar_abi::avx,
      datapar_abi::neon,
      datapar_abi::sse,
      datapar_abi::scalar
      >::type;

// fixed_size_storage<T, N>{{{1
template <class T, int N, class Tuple, class Next = select_best_vector_type_t<T, N>,
          int Remain = N - int(Next::size())>
struct fixed_size_storage_builder;

template <class T, int N, class... As, class Next>
struct fixed_size_storage_builder<T, N, datapar_tuple<T, As...>, Next, 0> {
    using type = datapar_tuple<T, As..., typename Next::abi_type>;
};

template <class T, int N, class... As, class Next, int Remain>
struct fixed_size_storage_builder<T, N, datapar_tuple<T, As...>, Next, Remain> {
    using type = typename fixed_size_storage_builder<
        T, Remain, datapar_tuple<T, As..., typename Next::abi_type>>::type;
};

template <class T, int N>
using fixed_size_storage = typename fixed_size_storage_builder<T, N, datapar_tuple<T>>::type;

namespace tests {
using datapar_abi::scalar;
using datapar_abi::sse;
using datapar_abi::avx;
using datapar_abi::avx512;
static_assert(
    std::is_same<fixed_size_storage<float, 1>, datapar_tuple<float, scalar>>::value,
    "fixed_size_storage failure");
static_assert(std::is_same<fixed_size_storage<float, 2>,
                           datapar_tuple<float, scalar, scalar>>::value,
              "fixed_size_storage failure");
static_assert(std::is_same<fixed_size_storage<float, 3>,
                           datapar_tuple<float, scalar, scalar, scalar>>::value,
              "fixed_size_storage failure");
static_assert(
    std::is_same<fixed_size_storage<float, 4>, datapar_tuple<float, sse>>::value,
    "fixed_size_storage failure");
static_assert(
    std::is_same<fixed_size_storage<float, 5>, datapar_tuple<float, sse, scalar>>::value,
    "fixed_size_storage failure");
#ifdef Vc_HAVE_AVX_ABI
static_assert(
    std::is_same<fixed_size_storage<float, 8>, datapar_tuple<float, avx>>::value,
    "fixed_size_storage failure");
static_assert(
    std::is_same<fixed_size_storage<float, 12>, datapar_tuple<float, avx, sse>>::value,
    "fixed_size_storage failure");
static_assert(std::is_same<fixed_size_storage<float, 13>,
                           datapar_tuple<float, avx, sse, scalar>>::value,
              "fixed_size_storage failure");
#endif
}  // namespace tests

// n_abis_in_tuple {{{1
template <class T> struct seq_op;
template <size_t I0, size_t... Is> struct seq_op<std::index_sequence<I0, Is...>> {
    using first_plus_one = std::index_sequence<I0 + 1, Is...>;
    using notfirst_plus_one = std::index_sequence<I0, (Is + 1)...>;
    template <size_t First, size_t Add>
    using prepend = std::index_sequence<First, I0 + Add, (Is + Add)...>;
};

template <class T> struct n_abis_in_tuple;
template <class T> struct n_abis_in_tuple<datapar_tuple<T>> {
    using counts = std::index_sequence<0>;
    using begins = std::index_sequence<0>;
};
template <class T, class A> struct n_abis_in_tuple<datapar_tuple<T, A>> {
    using counts = std::index_sequence<1>;
    using begins = std::index_sequence<0>;
};
template <class T, class A0, class... As>
struct n_abis_in_tuple<datapar_tuple<T, A0, A0, As...>> {
    using counts = typename seq_op<
        typename n_abis_in_tuple<datapar_tuple<T, A0, As...>>::counts>::first_plus_one;
    using begins = typename seq_op<typename n_abis_in_tuple<
        datapar_tuple<T, A0, As...>>::begins>::notfirst_plus_one;
};
template <class T, class A0, class A1, class... As>
struct n_abis_in_tuple<datapar_tuple<T, A0, A1, As...>> {
    using counts = typename seq_op<typename n_abis_in_tuple<
        datapar_tuple<T, A1, As...>>::counts>::template prepend<1, 0>;
    using begins = typename seq_op<typename n_abis_in_tuple<
        datapar_tuple<T, A1, As...>>::begins>::template prepend<0, 1>;
};

namespace tests
{
static_assert(
    std::is_same<n_abis_in_tuple<datapar_tuple<int, datapar_abi::sse, datapar_abi::sse,
                                                datapar_abi::scalar, datapar_abi::scalar,
                                                datapar_abi::scalar>>::counts,
                 std::index_sequence<2, 3>>::value,
    "");
static_assert(
    std::is_same<n_abis_in_tuple<datapar_tuple<int, datapar_abi::sse, datapar_abi::sse,
                                                datapar_abi::scalar, datapar_abi::scalar,
                                                datapar_abi::scalar>>::begins,
                 std::index_sequence<0, 2>>::value,
    "");
}  // namespace tests

// tree_reduction {{{1
template <size_t Count, size_t Begin> struct tree_reduction {
    static_assert(Count > 0,
                  "tree_reduction requires at least one datapar object to work with");
    template <class T, class... As, class BinaryOperation>
    auto operator()(const datapar_tuple<T, As...> &tup,
                    const BinaryOperation &binary_op) const noexcept
    {
        constexpr size_t left = next_power_of_2(Count) / 2;
        constexpr size_t right = Count - left;
        return binary_op(tree_reduction<left, Begin>()(tup, binary_op),
                         tree_reduction<right, Begin + left>()(tup, binary_op));
    }
};
template <size_t Begin> struct tree_reduction<1, Begin> {
    template <class T, class... As, class BinaryOperation>
    auto operator()(const datapar_tuple<T, As...> &tup, const BinaryOperation &) const
        noexcept
    {
        return detail::get<Begin>(tup);
    }
};
template <size_t Begin> struct tree_reduction<2, Begin> {
    template <class T, class... As, class BinaryOperation>
    auto operator()(const datapar_tuple<T, As...> &tup,
                    const BinaryOperation &binary_op) const noexcept
    {
        return binary_op(detail::get<Begin>(tup), detail::get<Begin + 1>(tup));
    }
};

// partial_bitset_to_member_type {{{1
template <class V, size_t N>
Vc_INTRINSIC auto partial_bitset_to_member_type(std::bitset<N> shifted_bits)
{
    static_assert(V::size() <= N, "");
    using M = typename V::mask_type;
    using T = typename V::value_type;
    constexpr T *type_tag = nullptr;
    return detail::get_impl_t<M>::from_bitset(
        std::bitset<V::size()>(shifted_bits.to_ullong()), type_tag);
}

// datapar impl {{{1
template <int N> struct fixed_size_datapar_impl {
    // member types {{{2
    using mask_member_type = std::bitset<N>;
    template <class T> using datapar_member_type = fixed_size_storage<T, N>;
    template <class T>
    static constexpr std::size_t tuple_size = datapar_member_type<T>::tuple_size;
    template <class T>
    static constexpr std::make_index_sequence<tuple_size<T>> index_seq = {};
    template <class T> using datapar = Vc::datapar<T, datapar_abi::fixed_size<N>>;
    template <class T> using mask = Vc::mask<T, datapar_abi::fixed_size<N>>;
    using size_tag = size_constant<N>;
    template <class T> using type_tag = T *;

    // broadcast {{{2
    template <class T, size_t... I>
    static Vc_INTRINSIC datapar_member_type<T> broadcast_impl(
        T x, std::index_sequence<I...>) noexcept
    {
        return {((void)I, x)...};
    }
    template <class T>
    static inline datapar_member_type<T> broadcast(T x, size_tag) noexcept
    {
        return broadcast_impl(x, index_seq<T>);
    }

    // generator {{{2
    template <class F, class T>
    static Vc_INTRINSIC datapar_member_type<T> generator(F &&gen, type_tag<T>, size_tag)
    {
        return datapar_member_type<T>::generate([&gen](auto native, auto offset_) {
            return decltype(native)([&](auto i_) {
                return gen(
                    size_constant<decltype(offset_)::value + decltype(i_)::value>());
            });
        });
    }

    // load {{{2
    template <class T, class U, class F>
    static inline datapar_member_type<T> load(const U *mem, F f,
                                              type_tag<T>) Vc_NOEXCEPT_OR_IN_TEST
    {
        return datapar_member_type<T>::generate([&](auto native, auto offset) {
                                                return decltype(native){&mem[offset], f};
                                                });
    }

    // masked load {{{2
    template <class T, class... As, class U, class F>
    static inline void masked_load(datapar_tuple<T, As...> &merge,
                                   const mask_member_type bits, const U *mem,
                                   F f) Vc_NOEXCEPT_OR_IN_TEST
    {
        detail::for_each(merge, [&](auto &native, auto offset) {
            using V = std::decay_t<decltype(native)>;
            detail::get_impl_t<V>::masked_load(
                detail::data(native), partial_bitset_to_member_type<V>(bits >> offset),
                &mem[offset], f);
        });
    }

    // store {{{2
    template <class T, class U, class F>
    static inline void store(const datapar_member_type<T> v, U *mem, F f,
                             type_tag<T>) Vc_NOEXCEPT_OR_IN_TEST
    {
        detail::for_each(
            v, [&](auto native, auto offset) { native.memstore(&mem[offset], f); });
    }

    // masked store {{{2
    template <class T, class... As, class U, class F>
    static inline void masked_store(const datapar_tuple<T, As...> v, U *mem, F f,
                                    const mask_member_type bits) Vc_NOEXCEPT_OR_IN_TEST
    {
        detail::for_each(v, [&](auto native, auto offset) {
            detail::get_impl_t<decltype(native)>::masked_store(
                detail::data(native), &mem[offset], f,
                partial_bitset_to_member_type<decltype(native)>(bits >> offset));
        });
    }

    // negation {{{2
    template <class T, class... As>
    static inline mask_member_type negate(datapar_tuple<T, As...> x) noexcept
    {
        mask_member_type bits = 0;
        for_each(x, [&bits](auto native, auto offset) {
            bits |= mask_member_type((!native).to_bitset().to_ullong()) << offset;
        });
        return bits;
    }

    // reductions {{{2
private:
    template <class T, class... As, class BinaryOperation, size_t... Counts,
              size_t... Begins>
    static inline T reduce(const datapar_tuple<T, As...> tup,
                           const BinaryOperation &binary_op,
                           std::index_sequence<Counts...>, std::index_sequence<Begins...>)
    {
        // TODO: E.g. <AVX, SSE, Scalar> should not reduce as
        // reduce(reduce(AVX), reduce(SSE), Scalar) but rather as
        // reduce(reduce(lo(AVX), hi(AVX), SSE), Scalar)
        // If multiple AVX objects are present, they should reduce to a single AVX object
        // first
        const auto scalars =
            detail::make_tuple(Vc::datapar<T, datapar_abi::scalar>(Vc::reduce(
                detail::tree_reduction<Counts, Begins>()(tup, binary_op), binary_op))...);
        return detail::data(
            detail::tree_reduction<scalars.tuple_size, 0>()(scalars, binary_op));
    }

public:
    template <class T, class BinaryOperation>
    static inline T reduce(size_tag, const datapar<T> x, const BinaryOperation &binary_op)
    {
        using ranges = n_abis_in_tuple<datapar_member_type<T>>;
        return fixed_size_datapar_impl::reduce(x.d, binary_op, typename ranges::counts(),
                                               typename ranges::begins());
    }

    // min, max, clamp {{{2
    template <class T>
    static inline datapar<T> min(const datapar<T> a, const datapar<T> b)
    {
        return {private_init,
                apply([](auto aa, auto bb) { return Vc::min(aa, bb); }, a.d, b.d)};
    }

    template <class T>
    static inline datapar<T> max(const datapar<T> a, const datapar<T> b)
    {
        return {private_init,
                apply([](auto aa, auto bb) { return Vc::max(aa, bb); }, a.d, b.d)};
    }

    // complement {{{2
    template <class T, class... As>
    static inline datapar_tuple<T, As...> complement(datapar_tuple<T, As...> x) noexcept
    {
        return apply([](auto xx) { return ~xx; }, x);
    }

    // unary minus {{{2
    template <class T, class... As>
    static inline datapar_tuple<T, As...> unary_minus(datapar_tuple<T, As...> x) noexcept
    {
        return apply([](auto xx) { return -xx; }, x);
    }

    // arithmetic operators {{{2

#define Vc_FIXED_OP(name_, op_)                                                          \
    template <class T, class... As>                                                      \
    static inline datapar_tuple<T, As...> name_(datapar_tuple<T, As...> x,               \
                                                datapar_tuple<T, As...> y)               \
    {                                                                                    \
        return apply([](auto xx, auto yy) { return xx op_ yy; }, x, y);                  \
    }                                                                                    \
    Vc_NOTHING_EXPECTING_SEMICOLON

    Vc_FIXED_OP(plus, +);
    Vc_FIXED_OP(minus, -);
    Vc_FIXED_OP(multiplies, *);
    Vc_FIXED_OP(divides, /);
    Vc_FIXED_OP(modulus, %);
    Vc_FIXED_OP(bit_and, &);
    Vc_FIXED_OP(bit_or, |);
    Vc_FIXED_OP(bit_xor, ^);
    Vc_FIXED_OP(bit_shift_left, <<);
    Vc_FIXED_OP(bit_shift_right, >>);
#undef Vc_FIXED_OP

    template <class T, class... As>
    static inline datapar_tuple<T, As...> bit_shift_left(datapar_tuple<T, As...> x, int y)
    {
        return apply([y](auto xx) { return xx << y; }, x);
    }

    template <class T, class... As>
    static inline datapar_tuple<T, As...> bit_shift_right(datapar_tuple<T, As...> x,
                                                          int y)
    {
        return apply([y](auto xx) { return xx >> y; }, x);
    }

    // sqrt {{{2
    template <class T, class... As>
    static inline datapar_tuple<T, As...> sqrt(datapar_tuple<T, As...> x) noexcept
    {
        return apply([](auto xx) { return Vc::sqrt(xx); }, x);
    }

    // abs {{{2
    template <class T, class... As>
    static inline datapar_tuple<T, As...> abs(datapar_tuple<T, As...> x) noexcept
    {
        return apply([](auto xx) { return Vc::abs(xx); }, x);
    }

    // increment & decrement{{{2
    template <class... Ts> static inline void increment(datapar_tuple<Ts...> &x)
    {
        for_each(x, [](auto &native, int) { ++native; });
    }

    template <class... Ts> static inline void decrement(datapar_tuple<Ts...> &x)
    {
        for_each(x, [](auto &native, int) { --native; });
    }

    // compares {{{2
#define Vc_CMP_OPERATIONS(cmp_)                                                          \
    template <class T, class... As>                                                      \
    static inline mask_member_type cmp_(datapar_tuple<T, As...> x,                       \
                                        datapar_tuple<T, As...> y)                       \
    {                                                                                    \
        mask_member_type bits = 0;                                                       \
        detail::for_each(x, y, [&bits](auto native_x, auto native_y, auto offset) {      \
            bits |= mask_member_type(                                                    \
                        std::cmp_<>()(native_x, native_y).to_bitset().to_ullong())       \
                    << offset;                                                           \
        });                                                                              \
        return bits;                                                                     \
    }                                                                                    \
    Vc_NOTHING_EXPECTING_SEMICOLON
    Vc_CMP_OPERATIONS(equal_to);
    Vc_CMP_OPERATIONS(not_equal_to);
    Vc_CMP_OPERATIONS(less);
    Vc_CMP_OPERATIONS(greater);
    Vc_CMP_OPERATIONS(less_equal);
    Vc_CMP_OPERATIONS(greater_equal);
#undef Vc_CMP_OPERATIONS

    // smart_reference access {{{2
    template <class T, class... As>
    static Vc_INTRINSIC T get(const datapar_tuple<T, As...> &v, int i) noexcept
    {
        return v[i];
    }
    template <class T, class... As, class U>
    static Vc_INTRINSIC void set(datapar_tuple<T, As...> &v, int i, U &&x) noexcept
    {
        v.set(i, std::forward<U>(x));
    }

    // masked_assign {{{2
    template <typename T, class... As>
    static Vc_INTRINSIC void masked_assign(
        const mask_member_type bits, detail::datapar_tuple<T, As...> &lhs,
        const detail::id<detail::datapar_tuple<T, As...>> rhs)
    {
        detail::for_each(lhs, rhs, [&](auto &native_lhs, auto native_rhs, auto offset) {
            using V = std::decay_t<decltype(native_lhs)>;
            detail::get_impl_t<V>::masked_assign(
                partial_bitset_to_member_type<V>(bits >> offset),
                detail::data(native_lhs), detail::data(native_rhs));
        });
    }

    // Optimization for the case where the RHS is a scalar. No need to broadcast the
    // scalar to a datapar first.
    template <typename T, class... As>
    static Vc_INTRINSIC void masked_assign(const mask_member_type bits,
                                           detail::datapar_tuple<T, As...> &lhs,
                                           const detail::id<T> rhs)
    {
        detail::for_each(lhs, [&](auto &native_lhs, auto offset) {
            using V = std::decay_t<decltype(native_lhs)>;
            detail::get_impl_t<V>::masked_assign(
                partial_bitset_to_member_type<V>(bits >> offset),
                detail::data(native_lhs), rhs);
        });
    }

    // masked_cassign {{{2
    template <template <typename> class Op, typename T, class... As>
    static inline void masked_cassign(const mask_member_type bits,
                                      detail::datapar_tuple<T, As...> &lhs,
                                      const detail::datapar_tuple<T, As...> rhs)
    {
        detail::for_each(lhs, rhs,
                         [&](auto &native_lhs, auto native_rhs, auto offset) {
                             using V = decltype(native_rhs);
                             detail::get_impl_t<V>::template masked_cassign<Op>(
                                 partial_bitset_to_member_type<V>(bits >> offset),
                                 detail::data(native_lhs), detail::data(native_rhs));
                         });
    }

    // Optimization for the case where the RHS is a scalar. No need to broadcast the
    // scalar to a datapar
    // first.
    template <template <typename> class Op, typename T, class... As>
    static inline void masked_cassign(const mask_member_type bits,
                                      detail::datapar_tuple<T, As...> &lhs, const T rhs)
    {
        detail::for_each(lhs, [&](auto &native_lhs, auto offset) {
            using V = std::decay_t<decltype(native_lhs)>;
            detail::get_impl_t<V>::template masked_cassign<Op>(
                partial_bitset_to_member_type<V>(bits >> offset),
                detail::data(native_lhs), rhs);
        });
    }

    // masked_unary {{{2
    template <template <typename> class Op, class T, class... As>
    static inline detail::datapar_tuple<T, As...> masked_unary(
        const mask_member_type bits, const detail::datapar_tuple<T, As...> v)
    {
        detail::datapar_tuple<T, As...> r;
        detail::for_each(r, v, [&](auto &native_r, auto native_v, auto offset) {
            using V = decltype(native_v);
            detail::data(native_r) = detail::get_impl_t<V>::template masked_unary<Op>(
                partial_bitset_to_member_type<V>(bits >> offset), detail::data(native_v));
        });
        return r;
    }

    // }}}2
};

// mask impl {{{1
template <int N> struct fixed_size_mask_impl {
    static_assert(sizeof(ullong) * CHAR_BIT >= N,
                  "The fixed_size implementation relies on one "
                  "ullong being able to store all boolean "
                  "elements.");  // required in load & store

    // member types {{{2
    static constexpr std::make_index_sequence<N> index_seq = {};
    using mask_member_type = std::bitset<N>;
    template <class T> using mask = Vc::mask<T, datapar_abi::fixed_size<N>>;
    using size_tag = size_constant<N>;
    template <class T> using type_tag = T *;

    // to_bitset {{{2
    static Vc_INTRINSIC mask_member_type to_bitset(const mask_member_type &bs) noexcept
    {
        return bs;
    }

    // from_bitset {{{2
    template <class T>
    static Vc_INTRINSIC mask_member_type from_bitset(const mask_member_type &bs,
                                                     type_tag<T>) noexcept
    {
        return bs;
    }

    // broadcast {{{2
    template <class T>
    static Vc_INTRINSIC mask_member_type broadcast(bool x, type_tag<T>) noexcept
    {
        return ullong(x) * ((1llu << N) - 1llu);
    }

    // load {{{2
    template <class F>
    static inline mask_member_type load(const bool *mem, F f, size_tag) noexcept
    {
        // TODO: uchar is not necessarily the best type to use here. For smaller N ushort,
        // uint, ullong, float, and double can be more efficient.
        ullong r = 0;
        using Vs = fixed_size_storage<uchar, N>;
        detail::for_each(Vs{}, [&](auto v, auto i) {
            typename decltype(v)::mask_type k(&mem[i], f);
            r |= k.to_bitset().to_ullong() << i;
        });
        return r;
    }

    // masked load {{{2
    template <class F>
    static inline void masked_load(mask_member_type &merge, mask_member_type mask,
                                   const bool *mem, F) noexcept
    {
        execute_n_times<N>([&](auto i) {
            if (mask[i]) {
                merge[i] = mem[i];
            }
        });
    }

    // store {{{2
    template <class F>
    static inline void store(mask_member_type bs, bool *mem, F f, size_tag) noexcept
    {
#ifdef Vc_HAVE_AVX512BW
        const __m512i bool64 =
            and_(_mm512_movm_epi8(bs.to_ullong()), x86::one64(uchar()));
        detail::x86::store_n_bytes(size_constant<N>(), bool64, mem, f);
#elif defined Vc_HAVE_BMI2
#ifdef Vc_IS_AMD64
        unused(f);
        execute_n_times<N / 8>([&](auto i) {
            constexpr size_t offset = i * 8;
            const ullong bool8 =
                _pdep_u64(bs.to_ullong() >> offset, 0x0101010101010101ULL);
            std::memcpy(&mem[offset], &bool8, 8);
        });
        if (N % 8 > 0) {
            constexpr size_t offset = (N / 8) * 8;
            const ullong bool8 =
                _pdep_u64(bs.to_ullong() >> offset, 0x0101010101010101ULL);
            std::memcpy(&mem[offset], &bool8, N % 8);
        }
#else   // Vc_IS_AMD64
        unused(f);
        execute_n_times<N / 4>([&](auto i) {
            constexpr size_t offset = i * 4;
            const ullong bool4 =
                _pdep_u32(bs.to_ullong() >> offset, 0x01010101U);
            std::memcpy(&mem[offset], &bool4, 4);
        });
        if (N % 4 > 0) {
            constexpr size_t offset = (N / 4) * 4;
            const ullong bool4 =
                _pdep_u32(bs.to_ullong() >> offset, 0x01010101U);
            std::memcpy(&mem[offset], &bool4, N % 4);
        }
#endif  // Vc_IS_AMD64
#elif defined Vc_HAVE_SSE2   // !AVX512BW && !BMI2
        using V = datapar<uchar, datapar_abi::sse>;
        ullong bits = bs.to_ullong();
        execute_n_times<(N + 15) / 16>([&](auto i) {
            constexpr size_t offset = i * 16;
            constexpr size_t remaining = N - offset;
            if (remaining == 1) {
                mem[offset] = static_cast<bool>(bits >> offset);
            } else if (remaining <= 4) {
                const uint bool4 = ((bits >> offset) * 0x00204081U) & 0x01010101U;
                std::memcpy(&mem[offset], &bool4, remaining);
            } else if (remaining <= 7) {
                const ullong bool8 =
                    ((bits >> offset) * 0x40810204081ULL) & 0x0101010101010101ULL;
                std::memcpy(&mem[offset], &bool8, remaining);
            } else {
                auto tmp = _mm_cvtsi32_si128(bits >> offset);
                tmp = _mm_unpacklo_epi8(tmp, tmp);
                tmp = _mm_unpacklo_epi16(tmp, tmp);
                tmp = _mm_unpacklo_epi32(tmp, tmp);
                V tmp2(tmp);
                tmp2 &= V([](auto j) {
                    return static_cast<uchar>(1 << (j % CHAR_BIT));
                });  // mask bit index
                const __m128i bool16 =
                    _mm_add_epi8(data(tmp2 == V()),
                                 x86::one16(uchar()));  // 0xff -> 0x00 | 0x00 -> 0x01
                if (remaining >= 16) {
                    x86::store16(bool16, &mem[offset], f);
                } else if (remaining & 3) {
                    constexpr int to_shift = 16 - int(remaining);
                    _mm_maskmoveu_si128(bool16,
                                        _mm_srli_si128(allone<__m128i>(), to_shift),
                                        reinterpret_cast<char *>(&mem[offset]));
                } else  // at this point: 8 < remaining < 16
                    if (remaining >= 8) {
                    x86::store8(bool16, &mem[offset], f);
                    if (remaining == 12) {
                        x86::store4(_mm_unpackhi_epi64(bool16, bool16), &mem[offset + 8],
                                    f);
                    }
                }
            }
        });
#else
        // TODO: uchar is not necessarily the best type to use here. For smaller N ushort,
        // uint, ullong, float, and double can be more efficient.
        using Vs = fixed_size_storage<uchar, N>;
        detail::for_each(Vs{}, [&](auto v, auto i) {
            using M = typename decltype(v)::mask_type;
            M::from_bitset(bs.to_ullong() >> i).memstore(&mem[i], f);
        });
//#else
        //execute_n_times<N>([&](auto i) { mem[i] = bs[i]; });
#endif  // Vc_HAVE_BMI2
    }

    // masked store {{{2
    template <class F>
    static inline void masked_store(const mask_member_type v, bool *mem, F,
                                    const mask_member_type k) noexcept
    {
        execute_n_times<N>([&](auto i) {
            if (k[i]) {
                mem[i] = v[i];
            }
        });
    }

    // negation {{{2
    static Vc_INTRINSIC mask_member_type negate(const mask_member_type &x,
                                                size_tag) noexcept
    {
        return ~x;
    }

    // logical and bitwise operators {{{2
    template <class T>
    static Vc_INTRINSIC mask<T> logical_and(const mask<T> &x, const mask<T> &y) noexcept
    {
        return {bitset_init, x.d & y.d};
    }

    template <class T>
    static Vc_INTRINSIC mask<T> logical_or(const mask<T> &x, const mask<T> &y) noexcept
    {
        return {bitset_init, x.d | y.d};
    }

    template <class T>
    static Vc_INTRINSIC mask<T> bit_and(const mask<T> &x, const mask<T> &y) noexcept
    {
        return {bitset_init, x.d & y.d};
    }

    template <class T>
    static Vc_INTRINSIC mask<T> bit_or(const mask<T> &x, const mask<T> &y) noexcept
    {
        return {bitset_init, x.d | y.d};
    }

    template <class T>
    static Vc_INTRINSIC mask<T> bit_xor(const mask<T> &x, const mask<T> &y) noexcept
    {
        return {bitset_init, x.d ^ y.d};
    }

    // smart_reference access {{{2
    static Vc_INTRINSIC bool get(const mask_member_type k, int i) noexcept
    {
        return k[i];
    }
    static Vc_INTRINSIC void set(mask_member_type &k, int i, bool x) noexcept
    {
        k.set(i, x);
    }

    // masked_assign {{{2
    static Vc_INTRINSIC void masked_assign(const mask_member_type k,
                                           mask_member_type &lhs,
                                           const mask_member_type rhs)
    {
        lhs = (lhs & ~k) | (rhs & k);
    }

    // Optimization for the case where the RHS is a scalar.
    static Vc_INTRINSIC void masked_assign(const mask_member_type k,
                                           mask_member_type &lhs, const bool rhs)
    {
        if (rhs) {
            lhs |= k;
        } else {
            lhs &= ~k;
        }
    }

    // }}}2
};

// traits {{{1
template <class T, int N, bool = ((N <= 32 && N >= 0) || N == 64)>
struct fixed_size_traits {
    static constexpr size_t size() noexcept { return N; }

    using datapar_impl_type = fixed_size_datapar_impl<N>;
    using datapar_member_type = fixed_size_storage<T, N>;
    static constexpr size_t datapar_member_alignment =
#ifdef Vc_GCC
        std::min(size_t(
#ifdef __AVX__
                     256
#else
                     128
#endif
                     ),
#else
        (
#endif
                 next_power_of_2(N * sizeof(T)));
    struct datapar_cast_type {
        datapar_cast_type(const std::array<T, N> &);
        datapar_cast_type(datapar_member_type dd) : d(dd) {}
        explicit operator datapar_member_type() const { return d; }

    private:
        datapar_member_type d;
    };
    struct datapar_base {
        datapar_base() = default;
        Vc_INTRINSIC datapar_base(const datapar_base &) {}

        explicit operator const datapar_member_type &() const
        {
            return data(*static_cast<const fixed_size_datapar<T, N> *>(this));
        }
        explicit operator std::array<T, N>() const
        {
            std::array<T, N> r;
            // datapar_member_type can be larger because of higher alignment
            static_assert(sizeof(r) <= sizeof(datapar_member_type), "");
            std::memcpy(r.data(), &static_cast<const datapar_member_type &>(*this),
                        sizeof(r));
            return r;
        }
    };

    using mask_impl_type = fixed_size_mask_impl<N>;
    using mask_member_type = std::bitset<N>;
    static constexpr size_t mask_member_alignment = alignof(mask_member_type);
    class mask_cast_type
    {
        mask_cast_type() = delete;
    };
    struct mask_base {
        //explicit operator std::bitset<size()>() const { return impl::to_bitset(d); }
        // empty. The std::bitset interface suffices
    };

};
template <class T, int N>
struct fixed_size_traits<T, N, false> : public traits<void, void> {
};
template <int N> struct traits<long double, datapar_abi::fixed_size<N>> : public fixed_size_traits<long double, N> {};
template <int N> struct traits<double, datapar_abi::fixed_size<N>> : public fixed_size_traits<double, N> {};
template <int N> struct traits< float, datapar_abi::fixed_size<N>> : public fixed_size_traits< float, N> {};
template <int N> struct traits<ullong, datapar_abi::fixed_size<N>> : public fixed_size_traits<ullong, N> {};
template <int N> struct traits< llong, datapar_abi::fixed_size<N>> : public fixed_size_traits< llong, N> {};
template <int N> struct traits< ulong, datapar_abi::fixed_size<N>> : public fixed_size_traits< ulong, N> {};
template <int N> struct traits<  long, datapar_abi::fixed_size<N>> : public fixed_size_traits<  long, N> {};
template <int N> struct traits<  uint, datapar_abi::fixed_size<N>> : public fixed_size_traits<  uint, N> {};
template <int N> struct traits<   int, datapar_abi::fixed_size<N>> : public fixed_size_traits<   int, N> {};
template <int N> struct traits<ushort, datapar_abi::fixed_size<N>> : public fixed_size_traits<ushort, N> {};
template <int N> struct traits< short, datapar_abi::fixed_size<N>> : public fixed_size_traits< short, N> {};
template <int N> struct traits< uchar, datapar_abi::fixed_size<N>> : public fixed_size_traits< uchar, N> {};
template <int N> struct traits< schar, datapar_abi::fixed_size<N>> : public fixed_size_traits< schar, N> {};
template <int N> struct traits<  char, datapar_abi::fixed_size<N>> : public fixed_size_traits<  char, N> {};

// }}}1
}  // namespace detail

// [mask.reductions] {{{1
template <class T, int N> inline bool all_of(const fixed_size_mask<T, N> &k)
{
    return data(k).all();
}

template <class T, int N> inline bool any_of(const fixed_size_mask<T, N> &k)
{
    return data(k).any();
}

template <class T, int N> inline bool none_of(const fixed_size_mask<T, N> &k)
{
    return data(k).none();
}

template <class T, int N> inline bool some_of(const fixed_size_mask<T, N> &k)
{
    for (int i = 1; i < N; ++i) {
        if (k[i] != k[i - 1]) {
            return true;
        }
    }
    return false;
}

template <class T, int N> inline int popcount(const fixed_size_mask<T, N> &k)
{
    return data(k).count();
}

template <class T, int N> inline int find_first_set(const fixed_size_mask<T, N> &k)
{
    for (int i = 0; i < N; ++i) {
        if (k[i]) {
            return i;
        }
    }
    return -1;
}

template <class T, int N> inline int find_last_set(const fixed_size_mask<T, N> &k)
{
    for (int i = N - 1; i >= 0; --i) {
        if (k[i]) {
            return i;
        }
    }
    return -1;
}

// }}}1
Vc_VERSIONED_NAMESPACE_END

namespace std
{
// mask operators {{{1
template <class T, int N>
struct equal_to<Vc::mask<T, Vc::datapar_abi::fixed_size<N>>> {
private:
    using M = Vc::mask<T, Vc::datapar_abi::fixed_size<N>>;

public:
    bool operator()(const M &x, const M &y) const
    {
        bool r = x[0] == y[0];
        for (int i = 1; i < N; ++i) {
            r = r && x[i] == y[i];
        }
        return r;
    }
};
// }}}1
}  // namespace std

#endif  // VC_DATAPAR_FIXED_SIZE_H_

// vim: foldmethod=marker
