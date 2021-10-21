#pragma once

//          Copyright Rein Halbersma 2014-2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>            // all_of, copy_backward, copy_n, equal, fill_n, for_each, lexicographical_compare_three_way, max, none_of, swap_ranges
#include <bit>                  // countl_zero, countr_zero, popcount
#include <cassert>              // assert
#include <compare>              // strong_ordering
#include <concepts>             // constructible_from, unsigned_integral
#include <cstddef>              // ptrdiff_t, size_t
#include <functional>           // less
#include <initializer_list>     // initializer_list
#include <iterator>             // bidirectional_iterator_tag, begin, end, next, prev, rbegin, rend, reverse_iterator
#include <limits>               // digits
#include <numeric>              // accumulate
#include <ranges>               // drop, take
#include <tuple>                // tie
#include <type_traits>          // common_type_t, is_class_v, make_signed_t
#include <utility>              // forward, pair, swap

#if defined(_MSC_VER)

#define PRAGMA_VC_WARNING_PUSH_DISABLE(X)       \
        __pragma(warning(push))                 \
        __pragma(warning(disable: X))

#define PRAGMA_VC_WARNING_POP                   \
        __pragma(warning(pop))

#else

#define PRAGMA_VC_WARNING_PUSH_DISABLE(X)
#define PRAGMA_VC_WARNING_POP

#endif

namespace xstd {

template<std::size_t N, std::unsigned_integral Block = std::size_t>
class bit_set
{
        static_assert(N <= std::numeric_limits<int>::max());

        static constexpr auto M = static_cast<int>(N);  // keep size_t from spilling all over the code base
        static constexpr auto block_size = std::numeric_limits<Block>::digits;
        static constexpr auto num_logical_blocks = (M - 1 + block_size) / block_size;
        static constexpr auto num_storage_blocks = std::max(num_logical_blocks, 1);
        static constexpr auto num_bits = num_logical_blocks * block_size;
        static constexpr auto num_excess_bits = num_bits - M;

        static_assert(0 <= num_excess_bits && num_excess_bits < block_size);

        class proxy_reference;
        class proxy_iterator;

        Block m_data[num_storage_blocks]{};     // zero-initialization
public:
        using key_type               = int;
        using key_compare            = std::less<key_type>;
        using value_type             = int;
        using value_compare          = std::less<value_type>;
        using pointer                = proxy_iterator;
        using const_pointer          = proxy_iterator;
        using reference              = proxy_reference;
        using const_reference        = proxy_reference;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using iterator               = proxy_iterator;
        using const_iterator         = proxy_iterator;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using block_type             = Block;

        bit_set() = default;                    // zero-initialization

        template<class InputIterator>
        [[nodiscard]] constexpr bit_set(InputIterator first, InputIterator last) // Throws: Nothing.
        {
                insert(first, last);
        }

        [[nodiscard]] constexpr bit_set(std::initializer_list<value_type> ilist) // Throws: Nothing.
        :
                bit_set(ilist.begin(), ilist.end())
        {}

        constexpr auto& operator=(std::initializer_list<value_type> ilist) // Throws: Nothing.
        {
                clear();
                insert(ilist.begin(), ilist.end());
                return *this;
        }

        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=94924
        // bool operator==(bit_set const&) const = default;

        [[nodiscard]] constexpr auto operator==(bit_set const& other [[maybe_unused]]) const noexcept
                -> bool
        {
                if constexpr (num_logical_blocks == 0) {
                        return true;
                } else if constexpr (num_logical_blocks <= 1) {
                        return m_data[0] == other.m_data[0];
                } else if constexpr (num_logical_blocks == 2) {
                        constexpr auto tied = [](auto const& bs) {
                                return std::tie(bs.m_data[0], bs.m_data[1]);
                        };
                        return tied(*this) == tied(other);
                } else if constexpr (num_logical_blocks >= 3) {
                        return std::ranges::equal(m_data, other.m_data);
                }
        }

        [[nodiscard]] constexpr auto begin()         noexcept { return       iterator(this, find_first()); }
        [[nodiscard]] constexpr auto begin()   const noexcept { return const_iterator(this, find_first()); }
        [[nodiscard]] constexpr auto end()           noexcept { return       iterator(this, M); }
        [[nodiscard]] constexpr auto end()     const noexcept { return const_iterator(this, M); }

        [[nodiscard]] constexpr auto rbegin()        noexcept { return       reverse_iterator(end()); }
        [[nodiscard]] constexpr auto rbegin()  const noexcept { return const_reverse_iterator(end()); }
        [[nodiscard]] constexpr auto rend()          noexcept { return       reverse_iterator(begin()); }
        [[nodiscard]] constexpr auto rend()    const noexcept { return const_reverse_iterator(begin()); }

        [[nodiscard]] constexpr auto cbegin()  const noexcept { return const_iterator(begin()); }
        [[nodiscard]] constexpr auto cend()    const noexcept { return const_iterator(end());   }
        [[nodiscard]] constexpr auto crbegin() const noexcept { return const_reverse_iterator(rbegin()); }
        [[nodiscard]] constexpr auto crend()   const noexcept { return const_reverse_iterator(rend());   }

        [[nodiscard]] constexpr auto front() const // Throws: Nothing.
                -> const_reference
        {
                assert(!empty());
                return { *this, find_front() };
        }

        [[nodiscard]] constexpr auto back() const // Throws: Nothing.
                -> const_reference
        {
                assert(!empty());
                return { *this, find_back() };
        }

        [[nodiscard]] constexpr auto empty() const noexcept
                -> bool
        {
                if constexpr (num_logical_blocks == 0) {
                        return true;
                } else if constexpr (num_logical_blocks == 1) {
                        return !m_data[0];
                } else if constexpr (num_logical_blocks == 2) {
                        return !(
                                m_data[0] ||
                                m_data[1]
                        );
                } else if constexpr (num_logical_blocks >= 3) {
                        return std::ranges::none_of(m_data, [](auto block) -> bool {
                                return block;
                        });
                }
        }

        [[nodiscard]] constexpr auto full() const noexcept
        {
                if constexpr (num_excess_bits == 0) {
                        if constexpr (num_logical_blocks == 0) {
                                return true;
                        } else if constexpr (num_logical_blocks == 1) {
                                return m_data[0] == ones;
                        } else if constexpr (num_logical_blocks == 2) {
                                return
                                        m_data[0] == ones &&
                                        m_data[1] == ones
                                ;
                        } else if constexpr (num_logical_blocks >= 3) {
                                return std::ranges::all_of(m_data, [](auto block) {
                                        return block == ones;
                                });
                        }
                } else {
                        static_assert(num_logical_blocks >= 1);
                        if constexpr (num_logical_blocks == 1) {
                                return m_data[0] == no_excess_bits;
                        } else if constexpr (num_logical_blocks == 2) {
                                return
                                        m_data[0] == no_excess_bits &&
                                        m_data[1] == ones
                                ;
                        } else if constexpr (num_logical_blocks >= 3) {
                                return
                                        m_data[0] == no_excess_bits &&
                                        std::ranges::all_of(std::ranges::views::drop(m_data, 1), [](auto block) {
                                                return block == ones;
                                        })
                                ;
                        }
                }
        }

        [[nodiscard]] constexpr auto ssize() const noexcept
        {
                if constexpr (num_logical_blocks == 0) {
                        return 0;
                } else if constexpr (num_logical_blocks == 1) {
                        return std::popcount(m_data[0]);
                } else if constexpr (num_logical_blocks == 2) {
                        return
                                std::popcount(m_data[0]) +
                                std::popcount(m_data[1])
                        ;
                } else if constexpr (num_logical_blocks >= 3) {
                        // Range version pending: http://open-std.org/JTC1/SC22/WG21/docs/papers/2019/p1813r0.pdf
                        return std::accumulate(std::begin(m_data), std::end(m_data), 0, [](auto sum, auto block) {
                                return sum + std::popcount(block);
                        });
                }
        }

        [[nodiscard]] constexpr auto size() const noexcept
        {
                return static_cast<size_type>(ssize());
        }

        [[nodiscard]] static constexpr auto max_size() noexcept
        {
                return N;
        }

        [[nodiscard]] static constexpr auto capacity() noexcept
                -> size_type
        {
                return num_bits;
        }

        template<class... Args>
        constexpr auto emplace(Args&&... args) // Throws: Nothing.
        {
                static_assert(sizeof...(Args) == 1);
                return insert(value_type(std::forward<Args>(args)...));
        }

        template<class... Args>
        constexpr auto emplace_hint(const_iterator hint, Args&&... args) // Throws: Nothing.
        {
                static_assert(sizeof...(Args) == 1);
                return insert(hint, value_type(std::forward<Args>(args)...));
        }

        constexpr auto insert(value_type const& x) // Throws: Nothing.
                -> std::pair<iterator, bool>
        {
                assert(is_valid_reference(x));
                if constexpr (num_logical_blocks >= 1) {
                        m_data[which(x)] |= single_bit_mask(where(x));
                }
                assert(contains(x));
                return { { this, x }, true };
        }

        constexpr auto insert(const_iterator /* hint */, value_type const& x) // Throws: Nothing.
                -> iterator
        {
                assert(is_valid_reference(x));
                insert(x);
                return { this, x };
        }

        template<class InputIterator>
        constexpr auto insert(InputIterator first, InputIterator last) // Throws: Nothing.
        {
                std::for_each(first, last, [&](auto const& x) {
                        insert(x);
                });
        }

        constexpr auto insert(std::initializer_list<value_type> ilist) // Throws: Nothing.
        {
                insert(ilist.begin(), ilist.end());
        }

        constexpr auto& fill() noexcept
        {
                if constexpr (num_excess_bits == 0) {
                        if constexpr (num_logical_blocks == 1) {
                                m_data[0] = ones;
                        } else if constexpr (num_logical_blocks == 2) {
                                m_data[0] = ones;
                                m_data[1] = ones;
                        } else if constexpr (num_logical_blocks >= 3) {
                                std::fill_n(std::begin(m_data), num_logical_blocks, ones);
                        }
                } else {
                        if constexpr (num_logical_blocks == 1) {
                                m_data[0] = no_excess_bits;
                        } else if constexpr (num_logical_blocks == 2) {
                                m_data[0] = no_excess_bits;
                                m_data[1] = ones;
                        } else if constexpr (num_logical_blocks >= 3) {
                                m_data[0] = no_excess_bits;
                                std::fill_n(std::next(std::begin(m_data)), num_logical_blocks - 1, ones);
                        }
                }
                assert(full());
                return *this;
        }

        constexpr auto erase(key_type const& x) // Throws: Nothing.
                -> size_type
        {
                assert(is_valid_reference(x));
                if constexpr (num_logical_blocks >= 1) {
                        // static_cast to guard against integral promotions of block_type smaller than int.
                        m_data[which(x)] &= static_cast<block_type>(~single_bit_mask(where(x)));
                }
                assert(!contains(x));
                return 1;
        }

        constexpr auto erase(const_iterator pos) // Throws: Nothing.
        {
                assert(pos != end());
                erase(*pos++);
                return pos;
        }

        constexpr auto erase(const_iterator first, const_iterator last) // Throws: Nothing.
        {
                std::for_each(first, last, [&](auto const& x) {
                        erase(x);
                });
                return last;
        }

        constexpr auto swap(bit_set& other [[maybe_unused]]) noexcept
        {
                if constexpr (num_logical_blocks == 1) {
                        std::swap(m_data[0], other.m_data[0]);
                } else if constexpr (num_logical_blocks == 2) {
                        std::swap(m_data[0], other.m_data[0]);
                        std::swap(m_data[1], other.m_data[1]);
                } else if constexpr (num_logical_blocks >= 3) {
                        std::ranges::swap_ranges(m_data, other.m_data);
                }
        }

        constexpr auto& clear() noexcept
        {
                if constexpr (num_logical_blocks == 1) {
                        m_data[0] = zero;
                } else if constexpr (num_logical_blocks == 2) {
                        m_data[0] = zero;
                        m_data[1] = zero;
                } else if constexpr (num_logical_blocks >= 3) {
                        std::fill_n(std::begin(m_data), num_logical_blocks, zero);
                }
                assert(empty());
                return *this;
        }

        constexpr auto& replace(value_type const& x [[maybe_unused]]) // Throws: Nothing.
        {
                assert(is_valid_reference(x));
                if constexpr (num_logical_blocks >= 1) {
                        m_data[which(x)] ^= single_bit_mask(where(x));
                }
                return *this;
        }

        [[nodiscard]] constexpr auto find(key_type const& x) // Throws: Nothing.
        {
                assert(is_valid_reference(x));
                return contains(x) ? iterator(this, x) : end();
        }

        [[nodiscard]] constexpr auto find(key_type const& x) const // Throws: Nothing.
        {
                assert(is_valid_reference(x));
                return contains(x) ? const_iterator(this, x) : cend();
        }

        [[nodiscard]] constexpr auto count(key_type const& x) const // Throws: Nothing.
                -> size_type
        {
                assert(is_valid_reference(x));
                return contains(x);
        }

        [[nodiscard]] constexpr auto contains(key_type const& x) const // Throws: Nothing.
        {
                assert(is_valid_reference(x));
                if constexpr (num_logical_blocks >= 1) {
                        if (m_data[which(x)] & single_bit_mask(where(x))) {
                                return true;
                        }
                }
                return false;
        }

        [[nodiscard]] constexpr auto lower_bound(key_type const& x) // Throws: Nothing.
                -> iterator
        {
                assert(is_valid_reference(x));
                return { this, find_next(x) };
        }

        [[nodiscard]] constexpr auto lower_bound(key_type const& x) const // Throws: Nothing.
                -> const_iterator
        {
                assert(is_valid_reference(x));
                return { this, find_next(x) };
        }

        [[nodiscard]] constexpr auto upper_bound(key_type const& x) // Throws: Nothing.
                -> iterator
        {
                assert(is_valid_reference(x));
                return { this, find_next(x + 1) };
        }

        [[nodiscard]] constexpr auto upper_bound(key_type const& x) const // Throws: Nothing.
                -> const_iterator
        {
                assert(is_valid_reference(x));
                return { this, find_next(x + 1) };
        }

        [[nodiscard]] constexpr auto equal_range(key_type const& x) // Throws: Nothing.
                -> std::pair<iterator, iterator>
        {
                assert(is_valid_reference(x));
                return { lower_bound(x), upper_bound(x) };
        }

        [[nodiscard]] constexpr auto equal_range(key_type const& x) const // Throws: Nothing.
                -> std::pair<const_iterator, const_iterator>
        {
                assert(is_valid_reference(x));
                return { lower_bound(x), upper_bound(x) };
        }

        constexpr auto& complement() noexcept
        {
                // static_cast to guard against integral promotions of block_type smaller than int.
                if constexpr (num_logical_blocks == 1) {
                        m_data[0] = static_cast<block_type>(~m_data[0]);
                } else if constexpr (num_logical_blocks == 2) {
                        m_data[0] = static_cast<block_type>(~m_data[0]);
                        m_data[1] = static_cast<block_type>(~m_data[1]);
                } else if constexpr (num_logical_blocks >= 3) {
                        for (auto& block : m_data) {
                                block = static_cast<block_type>(~block);
                        }
                }
                clear_excess_bits();
                return *this;
        }

        constexpr auto& operator&=(bit_set const& other [[maybe_unused]]) noexcept
        {
                if constexpr (num_logical_blocks == 1) {
                        m_data[0] &= other.m_data[0];
                } else if constexpr (num_logical_blocks == 2) {
                        m_data[0] &= other.m_data[0];
                        m_data[1] &= other.m_data[1];
                } else if constexpr (num_logical_blocks >= 3) {
                        for (auto i = 0; i < num_logical_blocks; ++i) {
                                m_data[i] &= other.m_data[i];
                        }
                }
                return *this;
        }

        constexpr auto& operator|=(bit_set const& other [[maybe_unused]]) noexcept
        {
                if constexpr (num_logical_blocks == 1) {
                        m_data[0] |= other.m_data[0];
                } else if constexpr (num_logical_blocks == 2) {
                        m_data[0] |= other.m_data[0];
                        m_data[1] |= other.m_data[1];
                } else if constexpr (num_logical_blocks >= 3) {
                        for (auto i = 0; i < num_logical_blocks; ++i) {
                                m_data[i] |= other.m_data[i];
                        }
                }
                return *this;
        }

        constexpr auto& operator^=(bit_set const& other [[maybe_unused]]) noexcept
        {
                if constexpr (num_logical_blocks == 1) {
                        m_data[0] ^= other.m_data[0];
                } else if constexpr (num_logical_blocks == 2) {
                        m_data[0] ^= other.m_data[0];
                        m_data[1] ^= other.m_data[1];
                } else if constexpr (num_logical_blocks >= 3) {
                        for (auto i = 0; i < num_logical_blocks; ++i) {
                                m_data[i] ^= other.m_data[i];
                        }
                }
                return *this;
        }

        constexpr auto& operator-=(bit_set const& other [[maybe_unused]]) noexcept
        {
                // static_cast to guard against integral promotions of block_type smaller than int.
                if constexpr (num_logical_blocks == 1) {
                        m_data[0] &= static_cast<block_type>(~other.m_data[0]);
                } else if constexpr (num_logical_blocks == 2) {
                        m_data[0] &= static_cast<block_type>(~other.m_data[0]);
                        m_data[1] &= static_cast<block_type>(~other.m_data[1]);
                } else if constexpr (num_logical_blocks >= 3) {
                        for (auto i = 0; i < num_logical_blocks; ++i) {
                                m_data[i] &= static_cast<block_type>(~other.m_data[i]);
                        }
                }
                return *this;
        }

        constexpr auto& operator<<=(value_type n [[maybe_unused]]) // Throws: Nothing.
        {
                assert(is_valid_reference(n));
                if constexpr (num_logical_blocks == 1) {
                        m_data[0] >>= n;
                } else if constexpr (num_logical_blocks >= 2) {
                        if (n == 0) {
                                return *this;
                        }

                        auto const n_block = n / block_size;
                        auto const R_shift = n % block_size;

                        if (R_shift == 0) {
                                std::copy_n(std::next(std::begin(m_data), n_block), num_logical_blocks - n_block, std::begin(m_data));
                        } else {
                                auto const L_shift = block_size - R_shift;

                                // static_cast to guard against integral promotions of block_type smaller than int.
                                for (auto i = 0; i < num_logical_blocks - 1 - n_block; ++i) {
                                        m_data[i] =
                                                static_cast<block_type>(m_data[i + n_block    ] >> R_shift) |
                                                static_cast<block_type>(m_data[i + n_block + 1] << L_shift)
                                        ;
                                }
                                m_data[num_logical_blocks - 1 - n_block] = static_cast<block_type>(m_data[num_logical_blocks - 1] >> R_shift);
                        }
                        std::fill_n(std::prev(std::end(m_data), n_block), n_block, zero);
                }
                clear_excess_bits();
                return *this;
        }

        constexpr auto& operator>>=(value_type n [[maybe_unused]]) // Throws: Nothing.
        {
                assert(is_valid_reference(n));
                if constexpr (num_logical_blocks == 1) {
                        m_data[0] <<= n;
                } else if constexpr (num_logical_blocks >= 2) {
                        if (n == 0) {
                                return *this;
                        }

                        auto const n_block = n / block_size;
                        auto const L_shift = n % block_size;

                        if (L_shift == 0) {
                                std::ranges::copy_backward(std::ranges::views::take(m_data, num_logical_blocks - n_block), std::end(m_data));
                        } else {
                                auto const R_shift = block_size - L_shift;

                                // static_cast to guard against integral promotions of block_type smaller than int.
                                for (auto i = num_logical_blocks - 1; i > n_block; --i) {
                                        m_data[i] =
                                                static_cast<block_type>(m_data[i - n_block    ] << L_shift) |
                                                static_cast<block_type>(m_data[i - n_block - 1] >> R_shift)
                                        ;
                                }
                                m_data[n_block] = static_cast<block_type>(m_data[0] << L_shift);
                        }
                        std::fill_n(std::begin(m_data), n_block, zero);
                }
                return *this;
        }

        [[nodiscard]] constexpr auto operator<=>(bit_set const& other [[maybe_unused]]) const noexcept
                -> std::strong_ordering
        {
                if constexpr (num_logical_blocks == 0) {
                        return std::strong_ordering::equal;
                } else if constexpr (num_logical_blocks <= 1) {
                        return other.m_data[0] <=> m_data[0];
                } else if constexpr (num_logical_blocks == 2) {
                        constexpr auto tied = [](auto const& bs) {
                                return std::tie(bs.m_data[1], bs.m_data[0]);
                        };
                        return tied(other) <=> tied(*this);
                } else if constexpr (num_logical_blocks >= 3) {
                        return std::lexicographical_compare_three_way(
                                std::rbegin(other.m_data), std::rend(other.m_data),
                                std::rbegin(m_data), std::rend(m_data)
                        );
                }
        }

        [[nodiscard]] constexpr auto is_subset_of(bit_set const& other [[maybe_unused]]) const noexcept
                -> bool
        {
                if constexpr (num_logical_blocks == 0) {
                        return true;
                } else if constexpr (num_logical_blocks == 1) {
                        return !(m_data[0] & ~other.m_data[0]);
                } else if constexpr (num_logical_blocks == 2) {
                        return
                                !(m_data[0] & ~other.m_data[0]) &&
                                !(m_data[1] & ~other.m_data[1])
                        ;
                } else if constexpr (num_logical_blocks >= 3) {
                        return std::ranges::equal(m_data, other.m_data, [](auto wL, auto wR) -> bool {
                                return !(wL & ~wR);
                        });
                }
        }

        [[nodiscard]] constexpr auto is_superset_of(bit_set const& other) const noexcept
        {
                return other.is_subset_of(*this);
        }

        [[nodiscard]] constexpr auto is_proper_subset_of(bit_set const& other [[maybe_unused]]) const noexcept
                -> bool
        {
                if constexpr (num_logical_blocks < 3) {
                        return is_subset_of(other) && !other.is_subset_of(*this);
                } else {
                        auto i = 0;
                        for (/* init-statement before loop */; i < num_logical_blocks; ++i) {
                                if (~m_data[i] & other.m_data[i]) {
                                        break;
                                }
                                if (m_data[i] & ~other.m_data[i]) {
                                        return false;
                                }
                        }
                        return (i == num_logical_blocks) ? false : std::ranges::equal(
                                std::ranges::views::drop(m_data, i),
                                std::ranges::views::drop(other.m_data, i),
                                [](auto wL, auto wR) -> bool {
                                        return !(wL & ~wR);
                                }
                        );
                }
        }

        [[nodiscard]] constexpr auto is_proper_superset_of(bit_set const& other) const noexcept
        {
                return other.is_proper_subset_of(*this);
        }

        [[nodiscard]] constexpr auto intersects(bit_set const& other [[maybe_unused]]) const noexcept
                -> bool
        {
                if constexpr (num_logical_blocks == 0) {
                        return false;
                } else if constexpr (num_logical_blocks == 1) {
                        return m_data[0] & other.m_data[0];
                } else if constexpr (num_logical_blocks == 2) {
                        return
                                (m_data[0] & other.m_data[0]) ||
                                (m_data[1] & other.m_data[1])
                        ;
                } else if constexpr (num_logical_blocks >= 3) {
                        return !std::ranges::equal(m_data, other.m_data, [](auto wL, auto wR) -> bool {
                                return !(wL & wR);
                        });
                }
        }

private:
        // static_cast to guard against integral promotions of block_type smaller than int.
        static constexpr auto zero = static_cast<block_type>( 0);
        static constexpr auto ones = static_cast<block_type>(-1);

        PRAGMA_VC_WARNING_PUSH_DISABLE(4309)
        static constexpr auto no_excess_bits = static_cast<block_type>(ones << num_excess_bits);
        PRAGMA_VC_WARNING_POP

        static_assert(num_excess_bits ^ (ones == no_excess_bits));

        // The inner static_cast guards against integer overflow of block_type larger than int.
        // The outer static_cast guards against integral promotions of block_type smaller than int.
        static constexpr auto unit = static_cast<block_type>(static_cast<block_type>(1) << (block_size - 1));

        [[nodiscard]] static constexpr auto single_bit_mask(value_type n) // Throws: Nothing.
                -> block_type
        {
                static_assert(num_logical_blocks >= 1);
                assert(0 <= n && n < block_size);
                return unit >> n;
        }

        [[nodiscard]] static constexpr auto is_valid_reference(value_type n) noexcept
                -> bool
        {
                return 0 <= n && n < M;
        }

        [[nodiscard]] static constexpr auto is_valid_iterator(value_type n) noexcept
                -> bool
        {
                return 0 <= n && n <= M;
        }

        [[nodiscard]] static constexpr auto which(value_type n [[maybe_unused]]) // Throws: Nothing.
                -> value_type
        {
                static_assert(num_logical_blocks >= 1);
                assert(is_valid_reference(n));
                if constexpr (num_logical_blocks == 1) {
                        return 0;
                } else {
                        return num_logical_blocks - 1 - n / block_size;
                }
        }

        [[nodiscard]] static constexpr auto where(value_type n) // Throws: Nothing.
                -> value_type
        {
                static_assert(num_logical_blocks >= 1);
                assert(is_valid_reference(n));
                if constexpr (num_logical_blocks == 1) {
                        return n;
                } else {
                        return n % block_size;
                }
        }

        constexpr auto clear_excess_bits() noexcept
        {
                if constexpr (num_excess_bits != 0) {
                        static_assert(num_logical_blocks >= 1);
                        m_data[0] &= no_excess_bits;
                }
        }

        [[nodiscard]] constexpr auto find_front() const // Throws: Nothing.
        {
                assert(!empty());
                if constexpr (num_logical_blocks == 1) {
                        return std::countl_zero(m_data[0]);
                } else if constexpr (num_logical_blocks == 2) {
                        return
                                m_data[1] ?
                                std::countl_zero(m_data[1]) :
                                std::countl_zero(m_data[0]) + block_size
                        ;
                } else {
                        auto n = 0;
                        for (auto i = num_storage_blocks - 1; i > 0; --i, n += block_size) {
                                if (auto const block = m_data[i]; block) {
                                        return n + std::countl_zero(block);
                                }
                        }
                        return n + std::countl_zero(m_data[0]);
                }
        }

        [[nodiscard]] constexpr auto find_back() const // Throws: Nothing.
        {
                assert(!empty());
                if constexpr (num_logical_blocks == 1) {
                        return num_bits - 1 - std::countr_zero(m_data[0]);
                } else if constexpr (num_logical_blocks == 2) {
                        return
                                m_data[0] ?
                                num_bits - 1 - std::countr_zero(m_data[0]) :
                                block_size - 1 - std::countr_zero(m_data[1])
                        ;
                } else {
                        auto n = num_bits - 1;
                        for (auto i = 0; i < num_storage_blocks - 1; ++i, n -= block_size) {
                                if (auto const block = m_data[i]; block) {
                                        return n - std::countr_zero(block);
                                }
                        }
                        return n - std::countr_zero(m_data[num_storage_blocks - 1]);
                }
        }

        [[nodiscard]] constexpr auto find_first() const noexcept
        {
                if constexpr (num_logical_blocks == 1) {
                        if (m_data[0]) {
                                return std::countl_zero(m_data[0]);
                        }
                } else if constexpr (num_logical_blocks >= 2) {
                        auto n = 0;
                        for (auto i = num_logical_blocks - 1; i >= 0; --i, n += block_size) {
                                if (auto const block = m_data[i]; block) {
                                        return n + std::countl_zero(block);
                                }
                        }
                }
                return M;
        }

        [[nodiscard]] constexpr auto find_next(int n) const // Throws: Nothing.
        {
                assert(is_valid_iterator(n));
                if (n == M) {
                        return M;
                }
                // static_cast to guard against integral promotions of block_type smaller than int.
                if constexpr (num_logical_blocks == 1) {
                        if (auto const block = static_cast<block_type>(m_data[0] << n); block) {
                                return n + std::countl_zero(block);
                        }
                } else if constexpr (num_logical_blocks >= 2) {
                        auto i = which(n);
                        if (auto const offset = where(n); offset) {
                                if (auto const block = static_cast<block_type>(m_data[i] << offset); block) {
                                        return n + std::countl_zero(block);
                                }
                                --i;
                                n += block_size - offset;
                        }
                        for (/* init-statement before loop */; i >= 0; --i, n += block_size) {
                                if (auto const block = m_data[i]; block) {
                                        return n + std::countl_zero(block);
                                }
                        }
                }
                return M;
        }

        [[nodiscard]] constexpr auto find_prev(int n) const // Throws: Nothing.
        {
                assert(is_valid_reference(n));
                // static_cast to guard against integral promotions of block_type smaller than int.
                if constexpr (num_logical_blocks == 1) {
                        return n - std::countr_zero(static_cast<block_type>(m_data[0] >> (block_size - 1 - n)));
                } else {
                        if constexpr (num_logical_blocks >= 2) {
                                auto i = which(n);
                                if (auto const offset = block_size - 1 - where(n); offset) {
                                        if (auto const block = static_cast<block_type>(m_data[i] >> offset); block) {
                                                return n - std::countr_zero(block);
                                        }
                                        ++i;
                                        n -= block_size - offset;
                                }
                                for (/* init-statement before loop */; i < num_storage_blocks - 1; ++i, n -= block_size) {
                                        if (auto const block = m_data[i]; block) {
                                                return n - std::countr_zero(block);
                                        }
                                }
                        }
                        return n - std::countr_zero(m_data[num_storage_blocks - 1]);
                }
        }

        class proxy_reference
        {
                bit_set const& m_bs;
                value_type const m_value;
        public:
                ~proxy_reference() = default;
                proxy_reference(proxy_reference const&) = default;
                proxy_reference(proxy_reference&&) = default;
                proxy_reference& operator=(proxy_reference const&) = delete;
                proxy_reference& operator=(proxy_reference&&) = delete;

                proxy_reference() = delete;

                [[nodiscard]] constexpr proxy_reference(bit_set const& bs, value_type const& v) noexcept
                :
                        m_bs(bs),
                        m_value(v)
                {
                        assert(is_valid_reference(m_value));
                }

                [[nodiscard]] constexpr auto operator&() const noexcept
                        -> proxy_iterator
                {
                        return { &m_bs, m_value };
                }

                [[nodiscard]] explicit(false) constexpr operator value_type() const noexcept
                {
                        return m_value;
                }

                template<class T>
                        requires std::is_class_v<T> && std::constructible_from<T, value_type>
                [[nodiscard]] explicit(false) constexpr operator T() const noexcept(noexcept(T(m_value)))
                {
                        return m_value;
                }
        };

        class proxy_iterator
        {
        public:
                using difference_type   = typename bit_set::difference_type;
                using value_type        = typename bit_set::value_type;
                using pointer           = proxy_iterator;
                using reference         = proxy_reference;
                using iterator_category = std::bidirectional_iterator_tag;

        private:
                bit_set const* m_bs;
                value_type m_value;

        public:
                proxy_iterator() = default;

                [[nodiscard]] constexpr proxy_iterator(bit_set const* bs, value_type const& v) // Throws: Nothing.
                :
                        m_bs(bs),
                        m_value(v)
                {
                        assert(is_valid_iterator(m_value));
                }

                [[nodiscard]] constexpr auto operator==(proxy_iterator const& other) const noexcept
                        -> bool
                {
                        assert(m_bs == other.m_bs);
                        return m_value == other.m_value;
                }

                [[nodiscard]] constexpr auto operator*() const // Throws: Nothing.
                        -> proxy_reference
                {
                        assert(is_valid_reference(m_value));
                        return { *m_bs, m_value };
                }

                constexpr auto& operator++() // Throws: Nothing.
                {
                        assert(is_valid_reference(m_value));
                        m_value = m_bs->find_next(m_value + 1);
                        assert(is_valid_reference(m_value - 1));
                        return *this;
                }

                constexpr auto operator++(int) // Throws: Nothing.
                {
                        auto nrv = *this; ++*this; return nrv;
                }

                constexpr auto& operator--() // Throws: Nothing.
                {
                        assert(is_valid_reference(m_value - 1));
                        m_value = m_bs->find_prev(m_value - 1);
                        assert(is_valid_reference(m_value));
                        return *this;
                }

                constexpr auto operator--(int) // Throws: Nothing.
                {
                        auto nrv = *this; --*this; return nrv;
                }
        };
};

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto operator~(bit_set<N, Block> const& lhs) noexcept
{
        auto nrv = lhs; nrv.complement(); return nrv;
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto operator&(bit_set<N, Block> const& lhs, bit_set<N, Block> const& rhs) noexcept
{
        auto nrv = lhs; nrv &= rhs; return nrv;
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto operator|(bit_set<N, Block> const& lhs, bit_set<N, Block> const& rhs) noexcept
{
        auto nrv = lhs; nrv |= rhs; return nrv;
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto operator^(bit_set<N, Block> const& lhs, bit_set<N, Block> const& rhs) noexcept
{
        auto nrv = lhs; nrv ^= rhs; return nrv;
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto operator-(bit_set<N, Block> const& lhs, bit_set<N, Block> const& rhs) noexcept
{
        auto nrv = lhs; nrv -= rhs; return nrv;
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto operator<<(bit_set<N, Block> const& lhs, int n) // Throws: Nothing.
{
        assert(0 <= n && n < static_cast<int>(N));
        auto nrv = lhs; nrv <<= n; return nrv;
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto operator>>(bit_set<N, Block> const& lhs, int n) // Throws: Nothing.
{
        assert(0 <= n && n < static_cast<int>(N));
        auto nrv = lhs; nrv >>= n; return nrv;
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto is_disjoint(bit_set<N, Block> const& lhs, bit_set<N, Block> const& rhs) noexcept
{
        return !lhs.intersects(rhs);
}

template<std::size_t N, std::unsigned_integral Block>
constexpr auto swap(bit_set<N, Block>& lhs, bit_set<N, Block>& rhs) noexcept
{
        lhs.swap(rhs);
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto begin(bit_set<N, Block>& bs) noexcept
{
        return bs.begin();
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto begin(bit_set<N, Block> const& bs) noexcept
{
        return bs.begin();
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto end(bit_set<N, Block>& bs) noexcept
{
        return bs.end();
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto end(bit_set<N, Block> const& bs) noexcept
{
        return bs.end();
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto rbegin(bit_set<N, Block>& bs) noexcept
{
        return bs.rbegin();
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto rbegin(bit_set<N, Block> const& bs) noexcept
{
        return bs.rbegin();
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto rend(bit_set<N, Block>& bs) noexcept
{
        return bs.rend();
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto rend(bit_set<N, Block> const& bs) noexcept
{
        return bs.rend();
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto cbegin(bit_set<N, Block> const& bs) noexcept
{
        return xstd::begin(bs);
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto cend(bit_set<N, Block> const& bs) noexcept
{
        return xstd::end(bs);
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto crbegin(bit_set<N, Block> const& bs) noexcept
{
        return xstd::rbegin(bs);
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto crend(bit_set<N, Block> const& bs) noexcept
{
        return xstd::rend(bs);
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto size(bit_set<N, Block> const& bs) noexcept
{
        return bs.size();
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto ssize(bit_set<N, Block> const& bs) noexcept
{
        using R = std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(bs.size())>>;
        return static_cast<R>(bs.size());
}

template<std::size_t N, std::unsigned_integral Block>
[[nodiscard]] constexpr auto empty(bit_set<N, Block> const& bs) noexcept
{
        return bs.empty();
}

}       // namespace xstd
