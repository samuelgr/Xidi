/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ApiBitSet.h
 *   Type aliases for bit set objects that provide improved readability.
 **************************************************************************************************/

#pragma once

#include <cstddef>
#include <type_traits>

#include <xstd/bit_set.hpp>

namespace Xidi
{
  /// Type alias for most bit set objects that operate on plain integers.
  /// @tparam kNumBits Number of bits to be represented by the underlying bit set object.
  template <size_t kNumBits> using BitSet = xstd::bit_set<kNumBits>;

  /// Type alias for bit set objects that operate on enumerations.
  /// @tparam EnumType Enumeration to be represented by the bit set object.
  /// @tparam kNumBits Number of bits to be represented by the underlying bit set object, which
  /// defaults to a `Count` member of the enumeration.
  template <
      typename EnumType,
      EnumType kNumBits = EnumType::Count,
      typename = std::enable_if_t<std::is_enum_v<EnumType>>>
  using BitSetEnum = xstd::bit_set<static_cast<size_t>(kNumBits)>;
} // namespace Xidi
