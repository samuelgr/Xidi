/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file TemporaryBuffer.h
 *   Declaration of temporary buffer management functionality.
 *****************************************************************************/

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>


namespace Xidi
{
    /// Manages a global set of temporary buffers.
    /// These can be used for any purpose and are intended to replace large stack-allocated or heap-allocated buffers.
    /// Instead, memory is allocated statically at load-time and divided up as needed to various parts of the application.
    /// If too many buffers are allocated such that the available static buffers are exhausted, additional objects will allocate heap memory.
    /// All temporary buffer functionality is concurrency-safe and available as early as dynamic initialization.
    /// Do not instantiate this class directly; instead, instantiate the template class below.
    class TemporaryBufferBase
    {
    public:
        // -------- CONSTANTS -------------------------------------------------- //

        /// Specifies the total size of all temporary buffers, in bytes.
        static constexpr unsigned int kBuffersTotalNumBytes = 1 * 1024 * 1024;

        /// Specifies the number of temporary buffers to create statically.
        /// Even once this limit is reached buffers can be allocated but they are dynamically heap-allocated.
        static constexpr unsigned int kBuffersCount = 8;

        /// Specifies the size of each temporary buffer in bytes.
        static constexpr unsigned int kBytesPerBuffer = kBuffersTotalNumBytes / kBuffersCount;


    private:
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// Pointer to the buffer space.
        uint8_t* buffer;

        /// Specifies if the buffer space is heap-allocated.
        bool isHeapAllocated;


    protected:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Default constructor.
        TemporaryBufferBase(void);

        /// Default destructor.
        ~TemporaryBufferBase(void);

        /// Move constructor.
        inline TemporaryBufferBase(TemporaryBufferBase&& other) : buffer(nullptr), isHeapAllocated(false)
        {
            *this = std::move(other);
        }


        // -------- OPERATORS ---------------------------------------------- //

        /// Move assignment operator.
        inline TemporaryBufferBase& operator=(TemporaryBufferBase&& other)
        {
            std::swap(buffer, other.buffer);
            std::swap(isHeapAllocated, other.isHeapAllocated);
            return *this;
        }


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Retrieves the buffer pointer.
        /// @return Buffer pointer.
        inline uint8_t* Buffer(void) const
        {
            return buffer;
        }
    };

    /// Implements type-specific temporary buffer functionality.
    template <typename T> class TemporaryBuffer : public TemporaryBufferBase
    {
    public:
        // -------- CONSTANTS ---------------------------------------------- //

        /// Specifies the size of each temporary buffer in number of elements.
        static constexpr unsigned int kNumElementsPerBuffer = kBytesPerBuffer / sizeof(T);


        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Default constructor.
        inline TemporaryBuffer(void) : TemporaryBufferBase()
        {
            // Nothing to do here.
        }

        /// Move constructor.
        inline TemporaryBuffer(TemporaryBuffer&& other) : TemporaryBufferBase(std::move(other))
        {
            // Nothing to do here.
        }


        // -------- OPERATORS ---------------------------------------------- //

        /// Move assignment operator.
        inline TemporaryBuffer& operator=(TemporaryBuffer&& other)
        {
            TemporaryBufferBase::operator=(std::move(other));
            return *this;
        }

        /// Array indexing operator, constant version.
        /// In debug builds this will check that the index is within bounds of the buffer capacity.
        inline const T& operator[](size_t index) const
        {
            assert(index < Capacity());
            return Data()[index];
        }

        /// Array indexing operator, mutable version.
        /// In debug builds this will check that the index is within bounds of the buffer capacity.
        inline T& operator[](size_t index)
        {
            assert(index < Capacity());
            return Data()[index];
        }


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Retrieves the size of the buffer space, in number of elements of type T.
        /// @return Size of the buffer, in T-sized elements.
        constexpr inline unsigned int Capacity(void) const
        {
            return kNumElementsPerBuffer;
        }

        /// Retrieves a properly-typed pointer to the buffer itself, constant version.
        /// @return Typed pointer to the buffer.
        inline const T* Data(void) const
        {
            return (T*)Buffer();
        }

        /// Retrieves a properly-typed pointer to the buffer itself, mutable version.
        /// @return Typed pointer to the buffer.
        inline T* Data(void)
        {
            return (T*)Buffer();
        }

        /// Retrieves the size of the buffer space, in bytes.
        /// @return Size of the buffer, in bytes.
        constexpr inline unsigned int CapacityBytes(void) const
        {
            return kBytesPerBuffer;
        }
    };

    /// Implements a vector-like container backed by a temporary buffer.
    /// Optimized for efficiency. Performs no boundary checks.
    template <typename T> class TemporaryVector : public TemporaryBuffer<T>
    {
    public:
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Iterator type used to denote a position within a temporary vector object.
        template <typename DataType> class Iterator
        {
        private:
            /// Pointer directly to the temporary vector's underlying data.
            /// This implementation takes advantage of the fact that temporary vectors do not dynamically resize and reallocate.
            DataType* buffer;

            /// Index within the temporary vector data buffer.
            int index;

        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Initialization constructor.
            /// Requires a buffer and an index to initialize this iterator.
            constexpr inline Iterator(DataType* buffer, unsigned int index) : buffer(buffer), index(index)
            {
                // Nothing to do here.
            }


            // -------- OPERATORS ------------------------------------------ //

            /// Dereferencing operator.
            /// Allows the underlying data to be accessed directly via the iterator.
            constexpr inline DataType& operator*(void) const
            {
                return buffer[index];
            }

            /// Pre-increment operator.
            constexpr inline Iterator& operator++(void)
            {
                index += 1;
                return *this;
            }

            /// Post-increment operator.
            constexpr inline Iterator operator++(void) const
            {
                Iterator orig = *this;
                index += 1;
                return orig;
            }

            /// Pre-decrement operator.
            constexpr inline Iterator& operator--(void)
            {
                index -= 1;
                return *this;
            }

            /// Post-increment operator.
            constexpr inline Iterator operator--(void) const
            {
                Iterator orig = *this;
                index -= 1;
                return orig;
            }

            /// Equality comparison operator.
            /// In debug builds this will check that the two iterators reference the same object.
            constexpr inline bool operator==(const Iterator& other) const
            {
                assert(buffer == other.buffer);
                return (index == other.index);
            }

            /// Less-than comparison operator.
            /// In debug builds this will check that the two iterators reference the same object.
            constexpr inline bool operator<(const Iterator& rhs) const
            {
                assert(buffer == rhs.buffer);
                return (index < rhs.index);
            }

            /// Less-or-equal comparison operator.
            /// In debug builds this will check that the two iterators reference the same object.
            constexpr inline bool operator<=(const Iterator& rhs) const
            {
                assert(buffer == rhs.buffer);
                return (index <= rhs.index);
            }

            /// Greater-than comparison operator.
            /// In debug builds this will check that the two iterators reference the same object.
            constexpr inline bool operator>(const Iterator& rhs) const
            {
                assert(buffer == rhs.buffer);
                return (index > rhs.index);
            }

            /// Greater-or-equal comparison operator.
            /// In debug builds this will check that the two iterators reference the same object.
            constexpr inline bool operator>=(const Iterator& rhs) const
            {
                assert(buffer == rhs.buffer);
                return (index >= rhs.index);
            }

            /// Addition operator.
            /// Allows arbitrary addition to the index but no changes to the buffer pointer.
            constexpr inline Iterator operator+(int indexIncrement) const
            {
                return Iterator(buffer, index + indexIncrement);
            }

            /// Addition-assignment operator.
            /// Allows arbitrary addition to the index but no changes to the buffer pointer.
            constexpr inline Iterator& operator+=(int indexIncrement)
            {
                index += indexIncrement;
                return *this;
            }

            /// Subtraction operator.
            /// Allows arbitrary subtraction from the index but no changes to the buffer pointer.
            constexpr inline Iterator operator-(int indexIncrement) const
            {
                return Iterator(buffer, index - indexIncrement);
            }

            /// Subtraction-assignment operator.
            /// Allows arbitrary subtraction from the index but no changes to the buffer pointer.
            constexpr inline Iterator& operator-=(int indexIncrement)
            {
                index -= indexIncrement;
                return *this;
            }
        };

        typedef Iterator<T> TIterator;
        typedef Iterator<const T> TConstIterator;


    protected:
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// Number of elements held by this container.
        int size;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Default constructor.
        inline TemporaryVector(void) : TemporaryBuffer<T>(), size(0)
        {
            // Nothing to do here.
        }

        /// Initializer list constructor.
        inline TemporaryVector(std::initializer_list<T> initializers) : TemporaryVector()
        {
            *this = initializers;
        }

        /// Copy constructor.
        inline TemporaryVector(const TemporaryVector& other) : TemporaryVector()
        {
            *this = other;
        }

        /// Move constructor.
        inline TemporaryVector(TemporaryVector&& other) : TemporaryBuffer<T>(std::move(other))
        {
            std::swap(size, other.size);
        }

        /// Default destructor.
        inline ~TemporaryVector(void)
        {
            Clear();
        }


        // -------- OPERATORS ---------------------------------------------- //

        /// Copy assignment operator.
        inline TemporaryVector& operator=(const TemporaryVector& other)
        {
            Clear();

            for (int i = 0; i < other.size; ++i)
                EmplaceBack(other[i]);

            return *this;
        }

        /// Move assignment operator.
        inline TemporaryVector& operator=(TemporaryVector&& other)
        {
            TemporaryBuffer<T>::operator=(std::move(other));
            std::swap(size, other.size);
            return *this;
        }

        /// Initializer list assignment operator.
        inline TemporaryVector& operator=(std::initializer_list<T> initializers)
        {
            Clear();

            for (auto init = initializers.begin(); init != initializers.end(); ++init)
                PushBack(std::move(*init));

            return *this;
        }

        /// Equality check.
        inline bool operator==(const TemporaryVector& other) const
        {
            if (other.size != size)
                return false;

            for (int i = 0; i < size; ++i)
            {
                if (other[i] != (*this)[i])
                    return false;
            }

            return true;
        }

        // -------- ITERATORS ---------------------------------------------- //

        /// Explicit constant-typed beginning iterator.
        inline TConstIterator cbegin(void) const
        {
            return TConstIterator(this->Data(), 0);
        }

        /// Explicit constant-typed one-past-the-end iterator.
        inline TConstIterator cend(void) const
        {
            return TConstIterator(this->Data(), size);
        }

        /// Implicit constant-typed beginning iterator.
        inline TConstIterator begin(void) const
        {
            return cbegin();
        }

        /// Implicit constant-typed one-past-the-end iterator.
        inline TConstIterator end(void) const
        {
            return cend();
        }

        /// Implicit mutable-typed beginning iterator.
        inline TIterator begin(void)
        {
            return TIterator(this->Data(), 0);
        }

        /// Implicit mutable-typed one-past-the-end iterator.
        inline TIterator end(void)
        {
            return TIterator(this->Data(), size);
        }


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Removes all elements from this container, destroying each in sequence.
        inline void Clear(void)
        {
            if constexpr (true == std::is_trivially_destructible_v<T>)
            {
                size = 0;
            }
            else
            {
                while (0 != size)
                    PopBack();
            }
        }

        /// Constructs a new element using the specified arguments at the end of this container.
        /// @return Reference to the constructed and inserted element.
        template <typename... Args> inline T& EmplaceBack(Args&&... args)
        {
            new (&((*this)[size])) T(std::forward<Args>(args)...);
            return (*this)[size++];
        }

        /// Specifies if this container contains no elements.
        /// @return `true` if this is container is empty, `false` otherwise.
        inline bool Empty(void) const
        {
            return (0 == Size());
        }

        /// Removes the last element from this container and destroys it.
        inline void PopBack(void)
        {
            (*this)[size--].~T();
        }

        /// Appends the specified element to the end of this container using copy semantics.
        /// @param [in] value Value to be appended.
        inline void PushBack(const T& value)
        {
            (*this)[size++] = value;
        }

        /// Appends the specified element to the end of this container using move semantics.
        /// @param [in] value Value to be appended.
        inline void PushBack(T&& value)
        {
            (*this)[size++] = std::move(value);
        }

        /// Retrieves the number of elements held in this container.
        /// @return Number of elements in the container.
        inline unsigned int Size(void) const
        {
            return size;
        }
    };

    /// Implements a string-like object backed by a temporary buffer.
    /// All strings represented by this object are null-terminated.
    /// Optimized for efficiency. Performs no boundary checks.
    class TemporaryString : public TemporaryVector<wchar_t>
    {
    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Default constructor.
        inline TemporaryString(void) : TemporaryVector<wchar_t>()
        {
            // Nothing to do here.
        }

        /// Conversion constructor from a C string.
        inline TemporaryString(const wchar_t* str) : TemporaryVector<wchar_t>()
        {
            *this = std::wstring_view(str);
        }

        /// Conversion constructor from a string.
        inline TemporaryString(const std::wstring& str) : TemporaryVector<wchar_t>()
        {
            *this = std::wstring_view(str);
        }

        /// Conversion constructor from a string view.
        inline TemporaryString(std::wstring_view str) : TemporaryVector<wchar_t>()
        {
            *this = str;
        }

        /// Copy constructor.
        inline TemporaryString(const TemporaryString& other) : TemporaryVector<wchar_t>(other)
        {
            (*this)[size] = L'\0';
        }

        /// Move constructor.
        inline TemporaryString(TemporaryString&& other) : TemporaryVector<wchar_t>(std::move(other))
        {
            // Nothing to do here.
        }


        // -------- OPERATORS ---------------------------------------------- //

        /// Copy assignment operator.
        inline TemporaryString& operator=(const TemporaryString& other)
        {
            TemporaryVector<wchar_t>::operator=(other);
            (*this)[size] = L'\0';
            return *this;
        }

        /// Move assignment operator.
        inline TemporaryString& operator=(TemporaryString&& other)
        {
            TemporaryVector<wchar_t>::operator=(std::move(other));
            return *this;
        }

        /// Conversion assignment operator from a C string.
        inline TemporaryString& operator=(const wchar_t* str)
        {
            return (*this = std::wstring_view(str));
        }

        /// Conversion assignment operator from a string.
        inline TemporaryString& operator=(const std::wstring& str)
        {
            return (*this = std::wstring_view(str));
        }

        /// Conversion assignment operator from a string view.
        inline TemporaryString& operator=(std::wstring_view str)
        {
            Clear();
            *this += str;
            return *this;
        }

        /// Concatenation operator with a C string.
        inline TemporaryString& operator+=(const wchar_t* str)
        {
            return (*this += std::wstring_view(str));
        }

        /// Concatenation operator with a string.
        inline TemporaryString& operator+=(const std::wstring& str)
        {
            return (*this += std::wstring_view(str));
        }

        /// Concatenation operator with a string view.
        inline TemporaryString& operator+=(std::wstring_view str)
        {
            for (unsigned int i = 0; (i < str.size()) && (Size() < (Capacity() - 1)); ++i)
                TemporaryVector<wchar_t>::PushBack(str[i]);

            (*this)[size] = L'\0';
            return *this;
        }

        /// Concatenation operator with another temporary string.
        inline TemporaryString& operator+=(const TemporaryString& str)
        {
            return (*this += str.AsStringView());
        }

        /// Concatenation operator with a single character.
        inline TemporaryString& operator+=(wchar_t c)
        {
            if (Size() < (Capacity() - 1))
            {
                TemporaryVector<wchar_t>::PushBack(c);
                (*this)[size] = L'\0';
            }

            return *this;
        }

        /// Concatenation with a Boolean value.
        inline TemporaryString& operator+=(bool b)
        {
            if (true == b)
                return (*this += L"true");
            else
                return (*this += L"false");
        }

        /// Concatenation with an integer value.
        template <typename IntegerType, typename = std::enable_if_t<std::is_integral_v<IntegerType>>> TemporaryString& operator+=(IntegerType i)
        {
            constexpr unsigned int kNumDigitsMax = 1 + std::numeric_limits<IntegerType>::digits10;

            wchar_t digits[1 + kNumDigitsMax];
            digits[kNumDigitsMax] = L'\0';

            if constexpr (true == std::is_signed_v<IntegerType>)
            {
                if (i < 0)
                {
                    *this += L'-';
                    i *= -1;
                }
            }

            if (0 == i)
            {
                return (*this += L'0');
            }
            else
            {
                unsigned int charIndex = kNumDigitsMax;
                while (i > 0)
                {
                    const IntegerType quotient = (i / 10);
                    const IntegerType remainder = (i % 10);

                    i = quotient;
                    digits[--charIndex] = (L'0' + (wchar_t)remainder);
                }

                return (*this += &digits[charIndex]);
            }
        }

        /// Stream concatenation operator for all types that have a suitable concatenation operator defined.
        template <typename T> inline TemporaryString& operator<<(T t)
        {
            return (*this += t);
        }

        /// Equality check with other temporary strings.
        inline bool operator==(const TemporaryString& other) const
        {
            return (other.AsStringView() == AsStringView());
        }

        /// Equality check with C strings.
        inline bool operator==(const wchar_t* other) const
        {
            return (std::wstring_view(other) == AsStringView());
        }

        /// Equality check with strings.
        inline bool operator==(const std::wstring& other) const
        {
            return (other == AsStringView());
        }

        /// Equality check with string views.
        inline bool operator==(std::wstring_view other) const
        {
            return (other == AsStringView());
        }

        /// Implicit conversion to a string view.
        inline operator std::wstring_view(void) const
        {
            return AsStringView();
        }


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Represents this object as a C string.
        /// @return C string representation.
        inline const wchar_t* AsCString(void) const
        {
            return Data();
        }

        /// Represents this object as a string view.
        /// @return String view representation.
        inline std::wstring_view AsStringView(void) const
        {
            return std::wstring_view(Data(), Size());
        }

        /// Determines if the contents of this string have been truncated due to a buffer overflow condition.
        /// @return `true` if so, `false` otherwise.
        inline bool Overflow(void) const
        {
            return (Size() >= (Capacity() - 1));
        }

        /// Replaces the end of the string with the specified replacement string.
        /// If the length of the replacement string exceeds the length of the existing string then the entire string is replaced.
        /// @param [in] replacementSuffix String with which to replace the end of the current string.
        inline void ReplaceSuffix(std::wstring_view replacementSuffix)
        {
            if (replacementSuffix.size() >= Size())
                Clear();
            else
                size -= (unsigned int)replacementSuffix.size();

            *this += replacementSuffix;
        }
        
        /// Removes the specified number of characters from the end of the string.
        /// If the specified count is at least the entire length of the string then the string is cleared.
        /// @param [in] count Number of characters to remove.
        inline void RemoveSuffix(unsigned int count)
        {
            if (count >= Size())
            {
                Clear();
            }
            else
            {
                size -= count;
                (*this)[size] = L'\0';
            }
        }

        /// Changes this object's knowledge of its own size.
        /// This is generally an unsafe operation but is intended to be used after the underlying buffer is manipulated by functions that operate on C strings.
        /// @param [in] newsize New size to use.
        inline void UnsafeSetSize(unsigned int newsize)
        {
            size = newsize;
        }


        // -------- DELETED INSTANCE METHODS ------------------------------- //

        // These methods are unsafe in the context of null-terminated string manipulation.
        // Equivalent operators and methods are supplied in this class.
        template <typename... Args> wchar_t& EmplaceBack(Args&&... args) = delete;
        void PopBack(void) = delete;
        void PushBack(const wchar_t& value) = delete;
        void PushBack(wchar_t&& value) = delete;
    };
}
