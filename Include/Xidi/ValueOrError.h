/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file ValueOrError.h
 *   Variant type that holds either a value or an error of some kind.
 *****************************************************************************/

#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <variant>


namespace Xidi
{
    /// Template for variants that can hold either values or errors.
    /// Useful as a return value from functions that can either produce a result or indicate an error.
    /// @tparam ValueType Type used to represent values.
    /// @tparam ErrorType Type used to represent errors.
    template <typename ValueType, typename ErrorType> class ValueOrError : private std::variant<ValueType, ErrorType>
    {
        static_assert(false == std::is_same_v<ValueType, ErrorType>, "Value and error types cannot be the same.");


    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Default constructor.
        /// Should never be invoked because all objects must have either a value or an error.
        ValueOrError(void) = delete;

        /// Conversion constructor.
        /// Delegates to the underlying data structure.
        template <typename... Args> constexpr ValueOrError(Args&&... args) : std::variant<ValueType, ErrorType>(std::forward<Args>(args)...)
        {
            // Nothing to do here.
        }

        /// Disambiguating constructor for null pointers.
        /// If the value type is constructible from a null pointer,  then a null pointer argument favors construction of a value rather than an error.
        /// Useful when both value and error types can be constructed using a null pointer argument.
        template <typename = std::enable_if_t<std::is_constructible_v<ValueType, std::nullptr_t>>> constexpr ValueOrError(std::nullptr_t) : std::variant<ValueType, ErrorType>(ValueType(nullptr))
        {
            // Nothing to do here.
        }


        // -------- OPERATORS ---------------------------------------------- //

        /// Equality operator.
        constexpr inline bool operator==(const ValueOrError& other) const = default;


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Retrieves a read-only reference to the error held by this object.
        /// @return Error by reference.
        constexpr inline const ErrorType& Error(void) const
        {
            return std::get<ErrorType>(*this);
        }

        /// Retrieves a mutable reference to the error held by this object.
        /// @return Error by reference.
        constexpr inline ErrorType& Error(void)
        {
            return std::get<ErrorType>(*this);
        }

        /// Specifies if this object holds an error, as opposed to a value.
        /// @return `true` if so, `false` otherwise.
        constexpr inline bool HasError(void) const
        {
            return std::holds_alternative<ErrorType>(*this);
        }

        /// Specifies if this object holds a value, as opposed to an error.
        /// @return `true` if so, `false` otherwise.
        constexpr inline bool HasValue(void) const
        {
            return std::holds_alternative<ValueType>(*this);
        }

        /// Retrieves a read-only reference to the value held by this object.
        /// @return Value by reference.
        constexpr inline const ValueType& Value(void) const
        {
            return std::get<ValueType>(*this);
        }

        /// Retrieves a mutable reference to the value held by this object.
        /// @return Value by reference.
        constexpr inline ValueType& Value(void)
        {
            return std::get<ValueType>(*this);
        }

        /// Retrieves a copy of the value held by this object, if this object holds a value, or a copy of the specified default value otherwise.
        /// @tparam DefaultValueType Type to use for the default value, which must be convertible to the value type held by this object.
        /// @param [in] defaultValue Default value to use in the absence of a value.
        /// @return Value held by this object or default value, depending on the state of this object.
        template <typename DefaultValueType> constexpr inline ValueType ValueOr(DefaultValueType&& defaultValue) const&
        {
            if (true == HasValue())
                return Value();

            return std::forward<DefaultValueType>(defaultValue);
        }

        /// Moves and returns the value held by this object, if this object holds a value, or a copy of the specified default value otherwise.
        /// @tparam DefaultValueType Type to use for the default value, which must be convertible to the value type held by this object.
        /// @param [in] defaultValue Default value to use in the absence of a value.
        /// @return Value held by this object or default value, depending on the state of this object.
        template <typename DefaultValueType> constexpr inline ValueType ValueOr(DefaultValueType&& defaultValue) &&
        {
            if (true == HasValue())
                return std::move(Value());

            return std::forward<DefaultValueType>(defaultValue);
        }
    };
}
