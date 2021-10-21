# Rebooting the `std::bitset` franchise

[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/c%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-Boost-blue.svg)](https://opensource.org/licenses/BSL-1.0)
[![Lines of Code](https://tokei.rs/b1/github/rhalbersma/bit_set?category=code)](https://github.com/rhalbersma/bit_set)

`xstd::bit_set<N>` is a modern and opinionated reimagining of `std::bitset<N>`, keeping what time has proven to be effective, and throwing out what is not. `xstd::bit_set` is a **fixed-size ordered set of integers** that is compact and fast. It does less work than `std::bitset` (e.g. no bounds-checking and no throwing of `out_of_range` exceptions) yet offers more (e.g. fulll `constexpr`-ness and bidirectional iterators over individual 1-bits). This enables **fixed-size bit-twiddling with set-like syntax** (identical to `std::set<int>`), typically leading to cleaner, more expressive code that seamlessly interacts with the rest of the Standard Library.

## Design choices for a `bitset` data structure

> "A `bitset` can be seen as either an array of bits or a set of integers. [...]  
> Common usage suggests that dynamic-length `bitsets` are seldom needed."
>  
> Chuck Allison, [ISO/WG21/N0075](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1991/WG21%201991/X3J16_91-0142%20WG21_N0075.pdf), November 25, 1991

The above quote is from the first C++ Standard Committee proposal on what would eventually become `std::bitset<N>`. The quote highlights two design choices to be made for a `bitset` data structure:

1. a sequence of `bool` versus an ordered set of `int`;
2. fixed-size versus variable-size storage.

A `bitset` should also optimize for both space (using contiguous storage) and time (using CPU-intrinsics for data-parallelism) wherever possible.

## The current `bitset` landscape

The C++ Standard Library and Boost provide the following optimized data structures in the landscape spanned by the aforementioned design decisions and optimization directives, as shown in the table below.

|                          | fixed-size        | variable-size |
| :--------------------    | :---------        | :------------ |
| **sequence of `bool`**   | `std::bitset<N>`  | `std::vector<bool, Allocator>` <br> `boost::dynamic_bitset<Block, Allocator>` |
| **ordered set of `int`** | `std::bitset<N>`  | `boost::dynamic_bitset<Block, Allocator>` (dense) <br> `boost::container::flat_set<int, Compare, Allocator>` (sparse) |

Notes:

1. Both `std::bitset` and `boost::dynamic_bitset` are not clear about the interface they provide. E.g. both offer a hybrid of sequence-like random element access (through `operator[]`) as well as primitives for set-like bidirectional iteration (using non-Standard GCC extensions `_Find_first` and `_Find_next` in the case of `std::bitset`).
2. It [has been known](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2130.html#96) for over two decades that providing a variable-size sequence of `bool` through specializing `std::vector<bool>` was an unfortunate design choice.
3. For ordered sets, there is a further design choice whether to optimize for **dense** sets or for **sparse** sets. Dense sets require a single bit per **potential** element, whereas sparse sets require a single `int` per **actual** element. For 32-bit integers, if less (more) than 1 in 32 elements (3.125%) are actually present in a set, a dense representation will be less (more) compact than a sparse representation.
4. Only `boost::dynamic_bitset` allows storage configuration through its `Block` template parameter (defaulted to `unsigned long`).

## A reimagined `bitset` landscape

The aforementioned issues with the current `bitset` landscape can be resolved by implementing a single-purpose container for each of the four quadrants in the design space.

|                          | fixed-size                                   | variable-size                             |
| :--------------------    | :---------                                   | :------------                             |
| **sequence of `bool`**   | `xstd::bit_array<N, Block>`                  | `xstd::bit_vector<Block, Allocator>`      |
| **ordered set of `int`** | `xstd::bit_set<N, Block>` (**this library**) | `xstd::dynamic_bit_set<Block, Allocator>` |

Notes:

1. Each data structure is clear about the interface it provides: sequences are random access containers and ordered sets are bidirectional containers.
2. The variable-size sequence of `bool` is named `xstd::bit_vector` and decoupled from the general `std::vector` class template.
3. All containers use a dense (single bit per element) representation. Variable-size sparse sets of `int` can continue to be provided by `boost::container::flat_set` or by a possible future C++ Standard Library addition (as proposed in [p1222r2](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1222r2.pdf)).
4. All containers allow storage configuration through their `Block` template parameter (defaulted to `std::size_t`).

This library provides one of the four outlined quadrants: `xstd::bit_set<N>` as a **fixed-size ordered set of `int`**. The other three quadrants are not implemented.

### Hello World

The code below demonstrates how `xstd::bit_set<N>` implements the [Sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes) algorithm to generate all prime numbers below a compile time number `N`.

[![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://wandbox.org/permlink/Z7N8Jh7ELmDQoCek)

```cpp
#include "xstd/bit_set.hpp"
#define FMT_HEADER_ONLY
#include "fmt/core.h"
#include "fmt/ranges.h"

constexpr auto N = 100;
using set_type = xstd::bit_set<N>;

int main()
{
    // initialize all numbers from [2, N)
    set_type primes;
    primes.fill();                              // renamed set() member from std::bitset<N>
    primes.erase(0);
    primes.erase(1);

    // find all primes below N
    for (auto const& p : primes) {              // range-for finds begin() / end() iterators
        auto const p_squared = p * p;
        if (p_squared >= N) break;
        for (auto n = p_squared; n < N; n += p) {
            primes.erase(n);
        }
    }

    // find all twin primes below N
    auto const twins = primes & primes >> 2;    // bitwise operators from std::bitset<N>

    // pretty-print solution
    fmt::print("{}\n", primes);
    fmt::print("{}\n", twins);
}
```

which has as output:
<pre>
{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}
{3, 5, 11, 17, 29, 41, 59, 71}
</pre>

### Sequence of bits

How would the Sieve of Eratosthenes code look when using a sequence of bits? The links in the table below provide the full code examples for `std::bitset<N>` and `boost::dynamic_bitset<>`.

| Library                   | Try it online |
| :------                   | :------------ |
| `std::bitset<N>`          | [![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://wandbox.org/permlink/rMAIOogh4iVt82qu) |
| `boost::dynamic_bitset<>` | [![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://wandbox.org/permlink/1Gyca2dxluXCO8Kz) |

The essential difference (apart from differently named member functions) is the lack of proxy iterators. The GCC Standard Library `libstdc++` provides member functions `_Find_first` and `_Find_next` for `std::bitset<N>` as **non-standard extensions**. For `boost::dynamic_bitset<>`, similarly named member functions `find_first` and `find_next` exist. For `boost::dynammic_bitset<>`, these can be retro-fitted into forward proxy iterators `begin` and `end`, but for `std::bitset<N>` the required user-defined specializations of `begin` and `end` inside `namespace std` entail **undefined behavior**, preventing range-`for` support for `std::bitset<N>`. The best one can do is a manual loop like below (or wrapped in a `for_each` non-member function)

```cpp
// find all primes below N
for (auto p = primes._Find_first(); p < N; p = primes._Find_next(p)) {
    // ...
}
```

The output of these `bitset` implementations gives a bitstring, to be read from right-to-left:
<pre>
0010000000100000100010000010100010000010100000100000100010100010000010100000100010100010100010101100
0000000000000000000000000000100000000000100000000000000000100000000000100000000000100000100000101000
</pre>

Printing the actual bit indices requires a manual loop using the `_Find_first` and `_Find_next` extensions mentioned above.

### Ordered set of integers

How would the Sieve of Eratosthenes code look when using an ordered set of integers? The links in the table below provide the full code examples for `std::set<int>` and `boost::container::flat_set<int>`. By design, `xstd::bit_set<N>` is an almost **drop-in replacement** for either of these `set` implementations.

| Library                           | Try it online |
| :------                           | :------------ |
| `std::set<int>`                   | [![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://wandbox.org/permlink/pPAOfQqbEIB1A4us) |
| `boost::container::flat_set<int>` | [![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://wandbox.org/permlink/52na7rrJVKD8As0Q) |
| `xstd::bit_set<N>`                | [![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://wandbox.org/permlink/anakzkISkkxxJ8xO) |

The essential difference is that `std::set<int>` and `boost::flat_set<int>` lack the bitwise operators `&` and `>>` to efficiently find twin primes. Instead, one has to iterate over the ordered set of primes using `std::adjacent_find` and write these one-by-one into a new `set`. This style of programming is also supported by `xstd::bit_set` and its proxy iterators seamlessly interact with the `std::adjacent_find` algorithm.

```cpp
// find all twin primes below N
set_type twins;
for (auto it = primes.begin(); it != primes.end() && std::next(it) != primes.end();) {
    it = std::adjacent_find(it, primes.end(), [](auto p, auto q) {
        return q - p == 2;
    });
    if (it != primes.end()) {
        twins.insert(*it++);
    }
}
```

## Documentation

The interface for the class template `xstd::bit_set<N>` is the coherent union of the following building blocks:

1. An almost **drop-in** implementation of the full interface of `std::set<int>`.
2. An almost complete **translation** of the [`std::bitset<N>`](http://en.cppreference.com/w/cpp/utility/bitset) member functions to the [`std::set<int>`](http://en.cppreference.com/w/cpp/container/set) naming convention.
3. The single-pass and short-circuiting **set predicates** from [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/1_73_0/libs/dynamic_bitset/dynamic_bitset.html).
4. The bitwise operators from [`std::bitset<N>`](http://en.cppreference.com/w/cpp/utility/bitset) and [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/1_73_0/libs/dynamic_bitset/dynamic_bitset.html) reimagined as composable and data-parallel **set algorithms**.

The **full** interface of `xstd::bit_set` is `constexpr`, made possible by the great advancements of C++20 in this area (in particular `constexpr` standard algorithms).

### 1 An almost drop-in replacement for `std::set<int>`

`xstd::bit_set<N>` is a fixed-size ordered set of integers, providing conceptually the same functionality as `std::set<int, std::less<int>, Allocator>`, where `Allocator` statically allocates memory to store `N` integers. In particular, `xstd::bit_set<N>` has:

- **No customized key comparison**: `xstd::bit_set` uses `std::less<int>` as its fixed comparator (accessible through its nested types `key_compare` and `value_compare`). In particular, the `xstd::bit_set` constructors do not take a comparator argument.
- **No allocators**: `xstd::bit_set` is a fixed-size set of non-negative integers and does not dynamically allocate memory. In particular, `xstd::bit_set` does **not provide** a `get_allocator()` member function and its constructors do not take an allocator argument.
- **No splicing**: `xstd::bit_set` is **not a node-based container**, and does not provide the splicing operations as defined in [p0083r3](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0083r3.pdf). In particular, `xstd::bit_set` does **not provide** the nested types `node_type` and `insert_return_type`, the `extract()` or `merge()` member functions, or the `insert()` overloads taking a node handle.

Minor **semantic differences** between common functionality in `xstd::bit_set<N>` and `std::set<int>` are:

- the `xstd::bit_set` member fucntion `max_size` is a `static` member function (because `N` is a constant expression).
- the `xstd::bit_set` member function `clear` returns `*this` instead of `void` as for `std::set`, to allow better chaining of member functions (consisent with `std::bitset::reset`).
- the `xstd::bit_set` iterators are **proxy iterators**, and taking their address yields **proxy references**. The difference should be undetectable. See the FAQ at the end of this document.
- the `xstd::bit_set` members `fill`, `complement`, `replace` and `full` do not exist for `std::set`.

With these caveats in mind, all fixed-size, defaulted comparing, non-allocating, non-splicing `std::set<int>` code in the wild should continue to work out-of-the-box with `xstd::bit_set<N>`.

### 2 An almost complete translation of `std::bitset<N>`

Almost all existing `std::bitset<N>` code has **a direct translation** (i.e. achievable through search-and-replace) to an equivalent `xstd::bit_set<N>` expression, with the same and familiar semantics as `std::set<int>` or `boost::flat_set<int>`.

| `xstd::bit_set<N>`                     | `std::bitset<N>`   | Notes                                           |
| :-----------------                     | :---------------   | :----                                           |
| `bs.fill()`                            | `bs.set()`         | not a member of `std::set<int>`                 |
| `bs.insert(pos)`                       | `bs.set(pos)`      | no bounds-checking or `out_of_range` exceptions |
| `val ? bs.insert(pos) : bs.erase(pos)` | `bs.set(pos, val)` | no bounds-checking or `out_of_range` exceptions |
| `bs.clear()`                           | `bs.reset()`       | returns `*this`, not `void` as `std::set<int>`  |
| `bs.erase(pos)`                        | `bs.reset(pos)`    | no bounds-checking or `out_of_range` exceptions |
| `bs.complement()`                      | `bs.flip()`        | not a member of `std::set<int>`                 |
| `bs.replace(pos)`                      | `bs.flip(pos)`     | no bounds-checking or `out_of_range` exceptions <br> not a member of `std::set<int>` |
| `bs.size()`                            | `bs.count()`       | |
| `bs.max_size()`                        | `bs.size()`        | is a `static` member                            |
| `bs.contains(pos)`                     | `bs.test(pos)`     | no bounds-checking or `out_of_range` exceptions |
| `bs.full()`                            | `bs.all()`         | not a member of `std::set<int>`                 |
| `!bs.empty()`                          | `bs.any()`         | |
| `bs.empty()`                           | `bs.none()`        | |
| `bs.contains(pos)`                     | `bs[pos]`          | |
| `val ? bs.insert(pos) : bs.erase(pos)` | `bs[pos] = val`    | |

The semantic differences between `xstd::bit_set<N>` and `std::bitset<N>` are:

- the **full** interface of `xstd::bit_set` is `constexpr`, in contrast to `std::bitset`;
- `xstd::bit_set` has a `static` member function `max_size()`;
- `xstd::bit_set` does not do bounds-checking for its members `insert`, `erase`, `replace` and `contains`. Instead of throwing an `out_of_range` exception for argument values outside the range `[0, N)`, this **behavior is undefined**. This gives `xstd::bit_set<N>` a small performance benefit over `std::bitset<N>`.

Functionality from `std::bitset<N>` that is not in `xstd::bit_set<N>`:

- **No integer or string constructors**: `xstd::bit_set` cannot be constructed from `unsigned long long`, `std::string` or `char const*`.
- **No integer or string conversion operators**: `xstd::bit_set` does not convert to `unsigned long`, `unsigned long long` or `std::string`.
- **No I/O streaming operators**: `xstd::bit_set` does not provide overloaded I/O streaming `operator<<` and `operator>>`.
- **No hashing**: `xstd::bit_set` does not provide a specialization for `std::hash<>`.

I/O functionality can be obtained through third-party libraries such as [{fmt}](https://fmt.dev/latest/), which has generic support for ranges such as `xstd::bit_set`. Similarly, hashing functionality can be obtained through third-party libraries such as [N3980](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html) by streaming the `xstd::bit_set<N>` elements and size through an overloaded `hash_append` function.

### 3 Set predicates from `boost::dynamic_bitset`

The set predicates `is_subset`, `is_proper_subset` and `intersects` from `boost::dynamic_bitset` are present in `xstd::bit_set` with **identical syntax** and **identical semantics**. Note that these set predicates are not present in `std::bitset`. Efficient emulation of these set predicates for `std::bitset` is not possible using **single-pass** and **short-circuiting** semantics. In addition, `xstd::bit_set` comes with convenient set predicates `is_superset_of`, `is_proper_superset_of` and `disjoint`.

| `xstd::bit_set<N>`           | `boost::dynamic_bitset<>`  | `std::bitset<N>`            |
| :-----------------           | :------------------------  | :---------------            |
| `a.is_subset_of(b)`          | `a.is_subset_of(b)`        | `(a & ~b).none()`           |
| `a.is_superset_of(b)`        | `b.is_subset_of(a)`        | `(b & ~a).none()`           |
| `a.is_proper_subset_of(b)`   | `a.is_proper_subset_of(b)` | `(a & ~b).none() && a != b` |
| `a.is_proper_superset_of(b)` | `b.is_proper_subset_of(a)` | `(b & ~a).none() && b != a` |
| `a.intersects(b)`            | `a.intersects(b)`          | `(a & b).any()`             |
| `is_disjoint(a, b)`          | `!a.intersects(b)`         | `(a & b).none()`            |

### 4 The bitwise operators from `std::bitset` and `boost::dynamic_bitset` reimagined as set algorithms

The bitwise operators (`&=`, `|=`, `^=`, `-=`, `~`, `&`, `|`, `^`, `-`) from `std::bitset` and `boost::dynamic_bitset` are present in `xstd::bit_set` with **identical syntax** and **identical semantics**. Note that the bitwise difference operators (`-=` and `-`) from `boost::dynamic_bitset` are not present in `std::bitset`. The `operator-` can be emulated for `std::bitset` using the identity `a - b == a & ~b`.

The bitwise-shift operators (`<<=`, `>>=`, `<<`, `>>`) from `std::bitset` and `boost::dynamic_bitset` are present in `xstd::bit_set` with **identical syntax**, but with the **semantic difference** that `xstd::bit_set<N>` does not support bit-shifting for lengths `>= N`. Instead of calling `clear()` for argument values outside the range `[0, N)`, this **behavior is undefined**. Note that these semantics for `xstd::bit_set<N>` are identical to bit-shifting on native unsigned integers. This gives `xstd::bit_set<N>` a small performance benefit over `std::bitset<N>`.

With the exception of `operator~`, the non-member bitwise operators can be reimagined as **composable** and **data-parallel** versions of the set algorithms on sorted ranges from the C++ Standard Library header `<algorithm>`.

| `xstd::bit_set<N>`                |  `std::set<int>` and constrained algorithms |
| :----------------                 | :------------------------------------------ |
| `a.is_subset_of(b)`               | `includes(a, b)`                                                          |
| `auto c = a & b;`                 | `set<int> c;` <br> `set_intersection(a, b, inserter(c, end(c)));`         |
| <code>auto c = a &#124; b;</code> | `set<int> c;` <br> `set_union(a, b, inserter(c, end(c)));`                |
| `auto c = a ^ b;`                 | `set<int> c;` <br> `set_symmetric_difference(a, b, inserter(c, end(c)));` |
| `auto c = a - b;`                 | `set<int> c;` <br> `set_difference(a, b, inserter(c, end(c)));`           |

The bitwise shift operators of `xstd::bit_set<N>` can be reimagined as set **transformations** that add or subtract a non-negative constant to all set elements, followed by **filtering** out elements that would fall outside the range `[0, N)`. Using the C++20 constrained algorithms and range adaptors, including the proposed but not yet standardized `std::ranges::to`, this can also be formulated in a composable way for `std::set<int>`, albeit without the data-parallelism that `xstd::bit_set<N>` provides.

<table>
<tr>
    <th>
        xstd::bit_set&ltN&gt
    </th>
    <th>
        std::set&ltint&gt and constrained algorithms with <a href=http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1206r1.pdf>P1206</a>
    </th>
</tr>
<tr>
    <td>
        <pre lang="cpp">
auto b = a << n;
        </pre>
    </td>
    <td>
        <pre lang="cpp">
auto b = a
    | transform([=](auto x) { return x + n; })
    | filter([](auto x) { return x < N;  })
    | to&ltstd::set&gt
;
        </pre>
    </td>
</tr>
<tr>
    <td>
        <pre lang="cpp">
auto b = a >> n;
        </pre>
    </td>
    <td>
        <pre lang="cpp">
auto b = a
    | transform([=](auto x) { return x - n; })
    | filter([](auto x) { return 0 <= x;  })
    | to&ltstd::set&gt
;
        </pre>
    </td>
</tr>
</table>

## Frequently Asked Questions

### Iterators

**Q**: How can you iterate over individual bits? I thought a byte was the unit of addressing?  
**A**: Using proxy iterators, which hold a pointer and an offset.

**Q**: What happens if you dereference a proxy iterator?  
**A**: You get a proxy reference: `ref == *it`.

**Q**: What happens if you take the address of a proxy reference?  
**A**: You get a proxy iterator: `it == &ref`.

**Q**: How do you get any value out of a proxy reference?  
**A**: They implicitly convert to `int`.

**Q**: How can proxy references work if C++ does not allow overloading of `operator.`?  
**A**: Indeed, proxy references break the equivalence between functions calls like `ref.mem_fn()` and `it->mem_fn()`.

**Q**: How do you work around this?  
**A**: `int` is not a class-type and does not have member functions, so this situation never occurs.

**Q**: Aren't there too many implicit conversions when assigning a proxy reference to an implicitly `int`-constructible class?  
**A**: No, proxy references also implicity convert to any class type that is implicitly constructible from an `int`.

**Q**: So iterating over an `xstd::bit_set` is really fool-proof?  
**A**: Yes, `xstd::bit_set` iterators are [easy to use correctly and hard to use incorrectly](http://www.aristeia.com/Papers/IEEE_Software_JulAug_2004_revised.htm).

### Bit-layout

**Q**: How is `xstd::bit_set` implemented?  
**A**: It uses a stack-allocated array of unsigned integers as bit storage.

**Q**: How is the set ordering mapped to the array's bit layout?  
**A**: The most significant bit of the last array word maps onto set value `0`.  
**A**: The least significant bit of the first array word maps onto set value `N - 1`.  

**Q**: I'm visually oriented, can you draw a diagram?  
**A**: Sure, it looks like this for `bit_set<16, uint8_t>`:

|value |01234567|89ABCDEF|
|:---- |-------:|-------:|
|word  |       1|       0|
|offset|76543210|76543210|

**Q**: Why is the set order reversely mapped onto the array's bit-layout?  
**A**: To be able to use **data-parallelism** for `(a < b) == std::ranges::lexicographical_compare(a, b)`.

**Q**: How is efficient set comparison connected to the bit-ordering within words?  
**A**: Take `bit_set<8, uint8_t>` and consider when `sL < sR` for ordered sets of integers `sL` and `sR`.

**Q**: Ah, lexicographical set comparison corresponds to bit comparison from most to least significant.  
**A**: Indeed, and this is equivalent to doing the integer comparison `wL > wR` on the underlying words `wL` and `wR`.  

**Q**: So the set ordering a `bit_set` is equivalent to its representation as a bitstring?  
**A**: Yes, indeed, and this property has been known for several decades:

> "bit-0 is the leftmost, just like char-0 is the leftmost in character strings. [...]  
> This makes converting from and to unsigned integers a little counter-intuitive,  
> but the string-ness (or "array-ness") is the foundation of this abstraction.
>  
> Chuck Allison, [ISO/WG21/N0128](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1992/WG21%201992/X3J16_92-0051%20WG21_N0128.pdf), May 26, 1992

### Storage type

**Q**: What storage type does `xstd::bit_set` use?  
**A**: By default, `xstd::bit_set` uses an array of `std::size_t` integers.

**Q**: Can I customize the storage type?  
**A**: Yes, the full class template signature is `template<std::size_t N, std::unsigned_integral Block = std::size_t> xstd::bit_set`.

**Q**: What other storage types can be used as template argument for `Block`?  
**A**: Any type modelling the Standard Library `unsigned_integral` concept, which includes (for GCC and Clang) the non-Standard `__uint128_t`.

**Q**: Does the `xstd::bit_set` implementation optimize for the case of a small number of words of storage?  
**A**: Yes, there are three special cases for 0, 1 and 2 words of storage, as well as the general case of 3 or more words.

## Requirements

This single-header library has no other dependencies than the C++ Standard Library and is continuously being tested with the following conforming [C++20](http://www.open-std.org/jtc1/sc22/wg21/prot/14882fdis/n4860.pdf) compilers:

| Platform | Compiler | Versions | Build |
| :------- | :------- | :------- | :---- |
| Linux    | GCC | 10, 11-SVN | [![codecov](https://codecov.io/gh/rhalbersma/bit_set/branch/master/graph/badge.svg)](https://codecov.io/gh/rhalbersma/bit_set) <br> [![Build Status](https://travis-ci.org/rhalbersma/bit_set.svg)](https://travis-ci.org/rhalbersma/bit_set) |

Note that this library makes liberal use of C++20 features, in particular Concepts, Ranges, `constexpr` algorithms and the `<=>` operator for comparisons. Only GCC >= 10 is supported at the moment. Clang and Visual Studio are catching up fast, and will be added as soon as possible.

## License

Copyright Rein Halbersma 2014-2021.  
Distributed under the [Boost Software License, Version 1.0](http://www.boost.org/users/license.html).  
(See accompanying file LICENSE_1_0.txt or copy at [http://www.boost.org/LICENSE_1_0.txt](http://www.boost.org/LICENSE_1_0.txt))
