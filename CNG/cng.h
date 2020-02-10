/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef MODULES_AUDIO_CODING_CODECS_CNG_WEBRTC_CNG_H_
#define MODULES_AUDIO_CODING_CODECS_CNG_WEBRTC_CNG_H_

#include <stdlib.h>
#include <iostream>
#include <assert.h>

#define WEBRTC_SPL_WORD16_MAX       32767
#ifdef __cplusplus
extern "C" {
#endif
static void rtc_FatalMessage(const char *file, int line, const char *msg) {
    printf("file:%s line %d: %s", file, line, msg);
}
#ifdef __cplusplus
}  // extern "C"
#endif
#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
#define RTC_DCHECK_IS_ON 1
#else
#define RTC_DCHECK_IS_ON 0
#endif
#define RTC_CHECK(condition)                                             \
  do {                                                                   \
    if (!(condition)) {                                                  \
      rtc_FatalMessage(__FILE__, __LINE__, "CHECK failed: " #condition); \
    }                                                                    \
  } while (0)

#define RTC_DCHECK(condition)                                             \
  do {                                                                    \
    if (RTC_DCHECK_IS_ON && !(condition)) {                               \
      rtc_FatalMessage(__FILE__, __LINE__, "DCHECK failed: " #condition); \
    }                                                                     \
  } while (0)
#define RTC_DCHECK_LT(a, b) RTC_DCHECK((a) < (b))
#define RTC_CHECK_EQ(a, b) RTC_CHECK((a) == (b))
#define RTC_DCHECK_EQ(v1, v2) RTC_CHECK_EQ(v1, v2)
#define RTC_CHECK_LE(a, b) RTC_CHECK((a) <= (b))
#define RTC_DCHECK_LE(a, b) RTC_DCHECK((a) <= (b))
#define RTC_CHECK_GT(a, b) RTC_CHECK((a) > (b))
#define RTC_DCHECK_GT(v1, v2) RTC_CHECK_GT(v1, v2)

#include <cstddef>

#include <stdint.h>  // NOLINT(build/include)

#define WEBRTC_CNG_MAX_LPC_ORDER 12
#ifndef API_ARRAY_VIEW_H_
#define API_ARRAY_VIEW_H_

#include <algorithm>
#include <type_traits>


#ifndef RTC_BASE_TYPE_TRAITS_H_
#define RTC_BASE_TYPE_TRAITS_H_

#include <cstddef>
#include <type_traits>


// Determines if the given class has zero-argument .data() and .size() methods
// whose return values are convertible to T* and size_t, respectively.
template<typename DS, typename T>
class HasDataAndSize {
private:
    template<
            typename C,
            typename std::enable_if<
                    std::is_convertible<decltype(std::declval<C>().data()), T *>::value &&
                    std::is_convertible<decltype(std::declval<C>().size()),
                            std::size_t>::value>::type * = nullptr>
    static int Test(int);

    template<typename>
    static char Test(...);

public:
    static constexpr bool value = std::is_same<decltype(Test<DS>(0)), int>::value;
};

namespace test_has_data_and_size {

    template<typename DR, typename SR>
    struct Test1 {
        DR data();

        SR size();
    };

    static_assert(HasDataAndSize<Test1<int *, int>, int>::value, "");
    static_assert(HasDataAndSize<Test1<int *, int>, const int>::value, "");
    static_assert(HasDataAndSize<Test1<const int *, int>, const int>::value, "");
    static_assert(!HasDataAndSize<Test1<const int *, int>, int>::value,
                  "implicit cast of const int* to int*");
    static_assert(!HasDataAndSize<Test1<char *, size_t>, int>::value,
                  "implicit cast of char* to int*");

    struct Test2 {
        int *data;
        size_t size;
    };
    static_assert(!HasDataAndSize<Test2, int>::value,
                  ".data and .size aren't functions");

    struct Test3 {
        int *data();
    };

    static_assert(!HasDataAndSize<Test3, int>::value, ".size() is missing");

    class Test4 {
        int *data();

        size_t size();
    };

    static_assert(!HasDataAndSize<Test4, int>::value,
                  ".data() and .size() are private");

}  // namespace test_has_data_and_size

namespace type_traits_impl {

// Determines if the given type is an enum that converts implicitly to
// an integral type.
    template<typename T>
    struct IsIntEnum {
    private:
        // This overload is used if the type is an enum, and unary plus
        // compiles and turns it into an integral type.
        template<typename X,
                typename std::enable_if<
                        std::is_enum<X>::value &&
                        std::is_integral<decltype(+std::declval<X>())>::value>::type * =
                nullptr>
        static int Test(int);

        // Otherwise, this overload is used.
        template<typename>
        static char Test(...);

    public:
        static constexpr bool value =
                std::is_same<decltype(Test<typename std::remove_reference<T>::type>(0)),
                        int>::value;
    };

}  // namespace type_traits_impl

// Determines if the given type is integral, or an enum that
// converts implicitly to an integral type.
template<typename T>
struct IsIntlike {
private:
    using X = typename std::remove_reference<T>::type;

public:
    static constexpr bool value =
            std::is_integral<X>::value || type_traits_impl::IsIntEnum<X>::value;
};

namespace test_enum_intlike {

    enum E1 {
        e1
    };
    enum {
        e2
    };
    enum class E3 {
        e3
    };
    struct S {
    };

    static_assert(type_traits_impl::IsIntEnum<E1>::value, "");
    static_assert(type_traits_impl::IsIntEnum<decltype(e2)>::value, "");
    static_assert(!type_traits_impl::IsIntEnum<E3>::value, "");
    static_assert(!type_traits_impl::IsIntEnum<int>::value, "");
    static_assert(!type_traits_impl::IsIntEnum<float>::value, "");
    static_assert(!type_traits_impl::IsIntEnum<S>::value, "");

    static_assert(IsIntlike<E1>::value, "");
    static_assert(IsIntlike<decltype(e2)>::value, "");
    static_assert(!IsIntlike<E3>::value, "");
    static_assert(IsIntlike<int>::value, "");
    static_assert(!IsIntlike<float>::value, "");
    static_assert(!IsIntlike<S>::value, "");

}  // test_enum_intlike


#endif  // RTC_BASE_TYPE_TRAITS_H_


// tl;dr: ArrayView is the same thing as gsl::span from the Guideline
//        Support Library.
//
// Many functions read from or write to arrays. The obvious way to do this is
// to use two arguments, a pointer to the first element and an element count:
//
//   bool Contains17(const int* arr, size_t size) {
//     for (size_t i = 0; i < size; ++i) {
//       if (arr[i] == 17)
//         return true;
//     }
//     return false;
//   }
//
// This is flexible, since it doesn't matter how the array is stored (C array,
// std::vector, Buffer, ...), but it's error-prone because the caller has
// to correctly specify the array length:
//
//   Contains17(arr, arraysize(arr));     // C array
//   Contains17(arr.data(), arr.size());  // std::vector
//   Contains17(arr, size);               // pointer + size
//   ...
//
// It's also kind of messy to have two separate arguments for what is
// conceptually a single thing.
//
// Enter ArrayView<T>. It contains a T pointer (to an array it doesn't
// own) and a count, and supports the basic things you'd expect, such as
// indexing and iteration. It allows us to write our function like this:
//
//   bool Contains17(ArrayView<const int> arr) {
//     for (auto e : arr) {
//       if (e == 17)
//         return true;
//     }
//     return false;
//   }
//
// And even better, because a bunch of things will implicitly convert to
// ArrayView, we can call it like this:
//
//   Contains17(arr);                             // C array
//   Contains17(arr);                             // std::vector
//   Contains17(ArrayView<int>(arr, size));  // pointer + size
//   Contains17(nullptr);                         // nullptr -> empty ArrayView
//   ...
//
// ArrayView<T> stores both a pointer and a size, but you may also use
// ArrayView<T, N>, which has a size that's fixed at compile time (which means
// it only has to store the pointer).
//
// One important point is that ArrayView<T> and ArrayView<const T> are
// different types, which allow and don't allow mutation of the array elements,
// respectively. The implicit conversions work just like you'd hope, so that
// e.g. vector<int> will convert to either ArrayView<int> or ArrayView<const
// int>, but const vector<int> will convert only to ArrayView<const int>.
// (ArrayView itself can be the source type in such conversions, so
// ArrayView<int> will convert to ArrayView<const int>.)
//
// Note: ArrayView is tiny (just a pointer and a count if variable-sized, just
// a pointer if fix-sized) and trivially copyable, so it's probably cheaper to
// pass it by value than by const reference.

namespace impl {

// Magic constant for indicating that the size of an ArrayView is variable
// instead of fixed.
    enum : std::ptrdiff_t {
        kArrayViewVarSize = -4711
    };

// Base class for ArrayViews of fixed nonzero size.
    template<typename T, std::ptrdiff_t Size>
    class ArrayViewBase {
        static_assert(Size > 0, "ArrayView size must be variable or non-negative");

    public:
        ArrayViewBase(T *data, size_t size) : data_(data) {}

        static constexpr size_t size() { return Size; }

        static constexpr bool empty() { return false; }

        T *data() const { return data_; }

    protected:
        static constexpr bool fixed_size() { return true; }

    private:
        T *data_;
    };

// Specialized base class for ArrayViews of fixed zero size.
    template<typename T>
    class ArrayViewBase<T, 0> {
    public:
        explicit ArrayViewBase(T *data, size_t size) {}

        static constexpr size_t size() { return 0; }

        static constexpr bool empty() { return true; }

        T *data() const { return nullptr; }

    protected:
        static constexpr bool fixed_size() { return true; }
    };

// Specialized base class for ArrayViews of variable size.
    template<typename T>
    class ArrayViewBase<T, impl::kArrayViewVarSize> {
    public:
        ArrayViewBase(T *data, size_t size)
                : data_(size == 0 ? nullptr : data), size_(size) {}

        size_t size() const { return size_; }

        bool empty() const { return size_ == 0; }

        T *data() const { return data_; }

    protected:
        static constexpr bool fixed_size() { return false; }

    private:
        T *data_;
        size_t size_;
    };

}  // namespace impl

template<typename T, std::ptrdiff_t Size = impl::kArrayViewVarSize>
class ArrayView final : public impl::ArrayViewBase<T, Size> {
public:
    using value_type = T;
    using const_iterator = const T *;

    // Construct an ArrayView from a pointer and a length.
    template<typename U>
    ArrayView(U *data, size_t size)
            : impl::ArrayViewBase<T, Size>::ArrayViewBase(data, size) {
        RTC_DCHECK_EQ(size == 0 ? nullptr : data, this->data());
        RTC_DCHECK_EQ(size, this->size());
        RTC_DCHECK_EQ(!this->data(),
                      this->size() == 0);  // data is null iff size == 0.
    }

    // Construct an empty ArrayView. Note that fixed-size ArrayViews of size > 0
    // cannot be empty.
    ArrayView() : ArrayView(nullptr, 0) {}

    ArrayView(std::nullptr_t)  // NOLINT
            : ArrayView() {}

    ArrayView(std::nullptr_t, size_t size)
            : ArrayView(static_cast<T *>(nullptr), size) {
        static_assert(Size == 0 || Size == impl::kArrayViewVarSize, "");
        RTC_DCHECK_EQ(0, size);
    }

    // Construct an ArrayView from an array.
    template<typename U, size_t N>
    ArrayView(U (&array)[N])  // NOLINT
            : ArrayView(array, N) {
        static_assert(Size == N || Size == impl::kArrayViewVarSize,
                      "Array size must match ArrayView size");
    }

    // (Only if size is fixed.) Construct an ArrayView from any type U that has a
    // static constexpr size() method whose return value is equal to Size, and a
    // data() method whose return value converts implicitly to T*. In particular,
    // this means we allow conversion from ArrayView<T, N> to ArrayView<const T,
    // N>, but not the other way around. We also don't allow conversion from
    // ArrayView<T> to ArrayView<T, N>, or from ArrayView<T, M> to ArrayView<T,
    // N> when M != N.
    template<
            typename U,
            typename std::enable_if<Size != impl::kArrayViewVarSize &&
                                    HasDataAndSize<U, T>::value>::type * = nullptr>
    ArrayView(U &u)  // NOLINT
            : ArrayView(u.data(), u.size()) {
        static_assert(U::size() == Size, "Sizes must match exactly");
    }

    // (Only if size is variable.) Construct an ArrayView from any type U that
    // has a size() method whose return value converts implicitly to size_t, and
    // a data() method whose return value converts implicitly to T*. In
    // particular, this means we allow conversion from ArrayView<T> to
    // ArrayView<const T>, but not the other way around. Other allowed
    // conversions include
    // ArrayView<T, N> to ArrayView<T> or ArrayView<const T>,
    // std::vector<T> to ArrayView<T> or ArrayView<const T>,
    // const std::vector<T> to ArrayView<const T>,
    // Buffer to ArrayView<uint8_t> or ArrayView<const uint8_t>, and
    // const Buffer to ArrayView<const uint8_t>.
    template<
            typename U,
            typename std::enable_if<Size == impl::kArrayViewVarSize &&
                                    HasDataAndSize<U, T>::value>::type * = nullptr>
    ArrayView(U &u)  // NOLINT
            : ArrayView(u.data(), u.size()) {}

    // Indexing and iteration. These allow mutation even if the ArrayView is
    // const, because the ArrayView doesn't own the array. (To prevent mutation,
    // use a const element type.)
    T &operator[](size_t idx) const {
        RTC_DCHECK_LT(idx, this->size());
        RTC_DCHECK(this->data());
        return this->data()[idx];
    }

    T *begin() const { return this->data(); }

    T *end() const { return this->data() + this->size(); }

    const T *cbegin() const { return this->data(); }

    const T *cend() const { return this->data() + this->size(); }

    ArrayView<T> subview(size_t offset, size_t size) const {
        return offset < this->size()
               ? ArrayView<T>(this->data() + offset,
                              std::min(size, this->size() - offset))
               : ArrayView<T>();
    }

    ArrayView<T> subview(size_t offset) const {
        return subview(offset, this->size());
    }
};

// Comparing two ArrayViews compares their (pointer,size) pairs; it does *not*
// dereference the pointers.
template<typename T, std::ptrdiff_t Size1, std::ptrdiff_t Size2>
bool operator==(const ArrayView<T, Size1> &a, const ArrayView<T, Size2> &b) {
    return a.data() == b.data() && a.size() == b.size();
}

template<typename T, std::ptrdiff_t Size1, std::ptrdiff_t Size2>
bool operator!=(const ArrayView<T, Size1> &a, const ArrayView<T, Size2> &b) {
    return !(a == b);
}

// Variable-size ArrayViews are the size of two pointers; fixed-size ArrayViews
// are the size of one pointer. (And as a special case, fixed-size ArrayViews
// of size 0 require no storage.)
static_assert(sizeof(ArrayView<int>) == 2 * sizeof(int *), "");
static_assert(sizeof(ArrayView<int, 17>) == sizeof(int *), "");
static_assert(std::is_empty<ArrayView<int, 0>>::value, "");

template<typename T>
inline ArrayView<T> MakeArrayView(T *data, size_t size) {
    return ArrayView<T>(data, size);
}


#endif  // API_ARRAY_VIEW_H_

#ifndef RTC_BASE_ZERO_MEMORY_H_
#define RTC_BASE_ZERO_MEMORY_H_


// Fill memory with zeros in a way that the compiler doesn't optimize it away
// even if the pointer is not used afterwards.
void ExplicitZeroMemory(void *ptr, size_t len);

template<typename T,
        typename std::enable_if<!std::is_const<T>::value &&
                                std::is_trivial<T>::value>::type * = nullptr>
void ExplicitZeroMemory(ArrayView<T> a) {
    ExplicitZeroMemory(a.data(), a.size());
}

#endif  // RTC_BASE_ZERO_MEMORY_H_

/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_BUFFER_H_
#define RTC_BASE_BUFFER_H_

#include <algorithm>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>


// (Internal; please don't use outside this file.) Determines if elements of
// type U are compatible with a BufferT<T>. For most types, we just ignore
// top-level const and forbid top-level volatile and require T and U to be
// otherwise equal, but all byte-sized integers (notably char, int8_t, and
// uint8_t) are compatible with each other. (Note: We aim to get rid of this
// behavior, and treat all types the same.)
template<typename T, typename U>
struct BufferCompat {
    static constexpr bool value =
            !std::is_volatile<U>::value &&
            ((std::is_integral<T>::value && sizeof(T) == 1)
             ? (std::is_integral<U>::value && sizeof(U) == 1)
             : (std::is_same<T, typename std::remove_const<U>::type>::value));
};


// Basic buffer class, can be grown and shrunk dynamically.
// Unlike std::string/vector, does not initialize data when increasing size.
// If "ZeroOnFree" is true, any memory is explicitly cleared before releasing.
// The type alias "ZeroOnFreeBuffer" below should be used instead of setting
// "ZeroOnFree" in the template manually to "true".
template<typename T, bool ZeroOnFree = false>
class BufferT {
    // We want T's destructor and default constructor to be trivial, i.e. perform
    // no action, so that we don't have to touch the memory we allocate and
    // deallocate. And we want T to be trivially copyable, so that we can copy T
    // instances with std::memcpy. This is precisely the definition of a trivial
    // type.
    static_assert(std::is_trivial<T>::value, "T must be a trivial type.");

    // This class relies heavily on being able to mutate its data.
    static_assert(!std::is_const<T>::value, "T may not be const");

public:
    using value_type = T;

    // An empty BufferT.
    BufferT() : size_(0), capacity_(0), data_(nullptr) {
        RTC_DCHECK(IsConsistent());
    }

    // Disable copy construction and copy assignment, since copying a buffer is
    // expensive enough that we want to force the user to be explicit about it.
    BufferT(const BufferT &) = delete;

    BufferT &operator=(const BufferT &) = delete;

    BufferT(BufferT &&buf)
            : size_(buf.size()),
              capacity_(buf.capacity()),
              data_(std::move(buf.data_)) {
        RTC_DCHECK(IsConsistent());
        buf.OnMovedFrom();
    }

    // Construct a buffer with the specified number of uninitialized elements.
    explicit BufferT(size_t size) : BufferT(size, size) {}

    BufferT(size_t size, size_t capacity)
            : size_(size),
              capacity_(std::max(size, capacity)),
              data_(new T[capacity_]) {
        RTC_DCHECK(IsConsistent());
    }

    // Construct a buffer and copy the specified number of elements into it.
    template<typename U,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    BufferT(const U *data, size_t size) : BufferT(data, size, size) {}

    template<typename U,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    BufferT(U *data, size_t size, size_t capacity) : BufferT(size, capacity) {
        static_assert(sizeof(T) == sizeof(U), "");
        std::memcpy(data_.get(), data, size * sizeof(U));
    }

    // Construct a buffer from the contents of an array.
    template<typename U,
            size_t N,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    BufferT(U (&array)[N]) : BufferT(array, N) {}

    ~BufferT() { MaybeZeroCompleteBuffer(); }

    // Get a pointer to the data. Just .data() will give you a (const) T*, but if
    // T is a byte-sized integer, you may also use .data<U>() for any other
    // byte-sized integer U.
    template<typename U = T,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    const U *data() const {
        RTC_DCHECK(IsConsistent());
        return reinterpret_cast<U *>(data_.get());
    }

    template<typename U = T,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    U *data() {
        RTC_DCHECK(IsConsistent());
        return reinterpret_cast<U *>(data_.get());
    }

    bool empty() const {
        RTC_DCHECK(IsConsistent());
        return size_ == 0;
    }

    size_t size() const {
        RTC_DCHECK(IsConsistent());
        return size_;
    }

    size_t capacity() const {
        RTC_DCHECK(IsConsistent());
        return capacity_;
    }

    BufferT &operator=(BufferT &&buf) {
        RTC_DCHECK(IsConsistent());
        RTC_DCHECK(buf.IsConsistent());
        size_ = buf.size_;
        capacity_ = buf.capacity_;
        data_ = std::move(buf.data_);
        buf.OnMovedFrom();
        return *this;
    }

    bool operator==(const BufferT &buf) const {
        RTC_DCHECK(IsConsistent());
        if (size_ != buf.size_) {
            return false;
        }
        if (std::is_integral<T>::value) {
            // Optimization.
            return std::memcmp(data_.get(), buf.data_.get(), size_ * sizeof(T)) == 0;
        }
        for (size_t i = 0; i < size_; ++i) {
            if (data_[i] != buf.data_[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const BufferT &buf) const { return !(*this == buf); }

    T &operator[](size_t index) {
        RTC_DCHECK_LT(index, size_);
        return data()[index];
    }

    T operator[](size_t index) const {
        RTC_DCHECK_LT(index, size_);
        return data()[index];
    }

    T *begin() { return data(); }

    T *end() { return data() + size(); }

    const T *begin() const { return data(); }

    const T *end() const { return data() + size(); }

    const T *cbegin() const { return data(); }

    const T *cend() const { return data() + size(); }

    // The SetData functions replace the contents of the buffer. They accept the
    // same input types as the constructors.
    template<typename U,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    void SetData(const U *data, size_t size) {
        RTC_DCHECK(IsConsistent());
        const size_t old_size = size_;
        size_ = 0;
        AppendData(data, size);
        if (ZeroOnFree && size_ < old_size) {
            ZeroTrailingData(old_size - size_);
        }
    }

    template<typename U,
            size_t N,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    void SetData(const U (&array)[N]) {
        SetData(array, N);
    }

    template<typename W,
            typename std::enable_if<
                    HasDataAndSize<const W, const T>::value>::type * = nullptr>
    void SetData(const W &w) {
        SetData(w.data(), w.size());
    }

    // Replaces the data in the buffer with at most |max_elements| of data, using
    // the function |setter|, which should have the following signature:
    //
    //   size_t setter(ArrayView<U> view)
    //
    // |setter| is given an appropriately typed ArrayView of length exactly
    // |max_elements| that describes the area where it should write the data; it
    // should return the number of elements actually written. (If it doesn't fill
    // the whole ArrayView, it should leave the unused space at the end.)
    template<typename U = T,
            typename F,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    size_t SetData(size_t max_elements, F &&setter) {
        RTC_DCHECK(IsConsistent());
        const size_t old_size = size_;
        size_ = 0;
        const size_t written = AppendData<U>(max_elements, std::forward<F>(setter));
        if (ZeroOnFree && size_ < old_size) {
            ZeroTrailingData(old_size - size_);
        }
        return written;
    }

    // The AppendData functions add data to the end of the buffer. They accept
    // the same input types as the constructors.
    template<typename U,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    void AppendData(const U *data, size_t size) {
        RTC_DCHECK(IsConsistent());
        const size_t new_size = size_ + size;
        EnsureCapacityWithHeadroom(new_size, true);
        static_assert(sizeof(T) == sizeof(U), "");
        std::memcpy(data_.get() + size_, data, size * sizeof(U));
        size_ = new_size;
        RTC_DCHECK(IsConsistent());
    }

    template<typename U,
            size_t N,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    void AppendData(const U (&array)[N]) {
        AppendData(array, N);
    }

    template<typename W,
            typename std::enable_if<
                    HasDataAndSize<const W, const T>::value>::type * = nullptr>
    void AppendData(const W &w) {
        AppendData(w.data(), w.size());
    }

    template<typename U,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    void AppendData(const U &item) {
        AppendData(&item, 1);
    }

    // Appends at most |max_elements| to the end of the buffer, using the function
    // |setter|, which should have the following signature:
    //
    //   size_t setter(ArrayView<U> view)
    //
    // |setter| is given an appropriately typed ArrayView of length exactly
    // |max_elements| that describes the area where it should write the data; it
    // should return the number of elements actually written. (If it doesn't fill
    // the whole ArrayView, it should leave the unused space at the end.)
    template<typename U = T,
            typename F,
            typename std::enable_if<
                    BufferCompat<T, U>::value>::type * = nullptr>
    size_t AppendData(size_t max_elements, F &&setter) {
        RTC_DCHECK(IsConsistent());
        const size_t old_size = size_;
        SetSize(old_size + max_elements);
        U *base_ptr = data<U>() + old_size;
        size_t written_elements = setter(ArrayView<U>(base_ptr, max_elements));

        RTC_CHECK_LE(written_elements, max_elements);
        size_ = old_size + written_elements;
        RTC_DCHECK(IsConsistent());
        return written_elements;
    }

    // Sets the size of the buffer. If the new size is smaller than the old, the
    // buffer contents will be kept but truncated; if the new size is greater,
    // the existing contents will be kept and the new space will be
    // uninitialized.
    void SetSize(size_t size) {
        const size_t old_size = size_;
        EnsureCapacityWithHeadroom(size, true);
        size_ = size;
        if (ZeroOnFree && size_ < old_size) {
            ZeroTrailingData(old_size - size_);
        }
    }

    // Ensure that the buffer size can be increased to at least capacity without
    // further reallocation. (Of course, this operation might need to reallocate
    // the buffer.)
    void EnsureCapacity(size_t capacity) {
        // Don't allocate extra headroom, since the user is asking for a specific
        // capacity.
        EnsureCapacityWithHeadroom(capacity, false);
    }

    // Resets the buffer to zero size without altering capacity. Works even if the
    // buffer has been moved from.
    void Clear() {
        MaybeZeroCompleteBuffer();
        size_ = 0;
        RTC_DCHECK(IsConsistent());
    }

    // Swaps two buffers. Also works for buffers that have been moved from.
    friend void swap(BufferT &a, BufferT &b) {
        using std::swap;
        swap(a.size_, b.size_);
        swap(a.capacity_, b.capacity_);
        swap(a.data_, b.data_);
    }

private:
    void EnsureCapacityWithHeadroom(size_t capacity, bool extra_headroom) {
        RTC_DCHECK(IsConsistent());
        if (capacity <= capacity_)
            return;

        // If the caller asks for extra headroom, ensure that the new capacity is
        // >= 1.5 times the old capacity. Any constant > 1 is sufficient to prevent
        // quadratic behavior; as to why we pick 1.5 in particular, see
        // https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md and
        // http://www.gahcep.com/cpp-internals-stl-vector-part-1/.
        const size_t new_capacity =
                extra_headroom ? std::max(capacity, capacity_ + capacity_ / 2)
                               : capacity;

        std::unique_ptr<T[]> new_data(new T[new_capacity]);
        std::memcpy(new_data.get(), data_.get(), size_ * sizeof(T));
        MaybeZeroCompleteBuffer();
        data_ = std::move(new_data);
        capacity_ = new_capacity;
        RTC_DCHECK(IsConsistent());
    }

    // Zero the complete buffer if template argument "ZeroOnFree" is true.
    void MaybeZeroCompleteBuffer() {
        if (ZeroOnFree && capacity_) {
            // It would be sufficient to only zero "size_" elements, as all other
            // methods already ensure that the unused capacity contains no sensitive
            // data - but better safe than sorry.
            ExplicitZeroMemory(data_.get(), capacity_ * sizeof(T));
        }
    }

    // Zero the first "count" elements of unused capacity.
    void ZeroTrailingData(size_t count) {
        RTC_DCHECK(IsConsistent());
        RTC_DCHECK_LE(count, capacity_ - size_);
        ExplicitZeroMemory(data_.get() + size_, count * sizeof(T));
    }

    // Precondition for all methods except Clear and the destructor.
    // Postcondition for all methods except move construction and move
    // assignment, which leave the moved-from object in a possibly inconsistent
    // state.
    bool IsConsistent() const {
        return (data_ || capacity_ == 0) && capacity_ >= size_;
    }

    // Called when *this has been moved from. Conceptually it's a no-op, but we
    // can mutate the state slightly to help subsequent sanity checks catch bugs.
    void OnMovedFrom() {
#if RTC_DCHECK_IS_ON
        // Make *this consistent and empty. Shouldn't be necessary, but better safe
        // than sorry.
        size_ = 0;
        capacity_ = 0;
#else
        // Ensure that *this is always inconsistent, to provoke bugs.
        size_ = 1;
        capacity_ = 0;
#endif
    }

    size_t size_;
    size_t capacity_;
    std::unique_ptr<T[]> data_;
};

// By far the most common sort of buffer.
using Buffer = BufferT<uint8_t>;

// A buffer that zeros memory before releasing it.
template<typename T>
using ZeroOnFreeBuffer = BufferT<T, true>;


#endif  // RTC_BASE_BUFFER_H_


class ComfortNoiseDecoder {
public:
    ComfortNoiseDecoder();

    ~ComfortNoiseDecoder() = default;

    ComfortNoiseDecoder(const ComfortNoiseDecoder &) = delete;

    ComfortNoiseDecoder &operator=(const ComfortNoiseDecoder &) = delete;

    void Reset();

    // Updates the CN state when a new SID packet arrives.
    // |sid| is a view of the SID packet without the headers.
    void UpdateSid(ArrayView<const uint8_t> sid);

    // Generates comfort noise.
    // |out_data| will be filled with samples - its size determines the number of
    // samples generated. When |new_period| is true, CNG history will be reset
    // before any audio is generated.  Returns |false| if outData is too large -
    // currently 640 bytes (equalling 10ms at 64kHz).
    // TODO(ossu): Specify better limits for the size of out_data. Either let it
    //             be unbounded or limit to 10ms in the current sample rate.
    bool Generate(ArrayView<int16_t> out_data, bool new_period);

private:
    uint32_t dec_seed_;
    int32_t dec_target_energy_;
    int32_t dec_used_energy_;
    int16_t dec_target_reflCoefs_[WEBRTC_CNG_MAX_LPC_ORDER + 1];
    int16_t dec_used_reflCoefs_[WEBRTC_CNG_MAX_LPC_ORDER + 1];
    int16_t dec_filtstate_[WEBRTC_CNG_MAX_LPC_ORDER + 1];
    int16_t dec_filtstateLow_[WEBRTC_CNG_MAX_LPC_ORDER + 1];
    uint16_t dec_order_;
    int16_t dec_target_scale_factor_;  /* Q29 */
    int16_t dec_used_scale_factor_;  /* Q29 */
};

class ComfortNoiseEncoder {
public:
    // Creates a comfort noise encoder.
    // |fs| selects sample rate: 8000 for narrowband or 16000 for wideband.
    // |interval| sets the interval at which to generate SID data (in ms).
    // |quality| selects the number of refl. coeffs. Maximum allowed is 12.
    ComfortNoiseEncoder(int fs, int interval, int quality);

    ~ComfortNoiseEncoder() = default;

    ComfortNoiseEncoder(const ComfortNoiseEncoder &) = delete;

    ComfortNoiseEncoder &operator=(const ComfortNoiseEncoder &) = delete;

    // Resets the comfort noise encoder to its initial state.
    // Parameters are set as during construction.
    void Reset(int fs, int interval, int quality);

    // Analyzes background noise from |speech| and appends coefficients to
    // |output|.  Returns the number of coefficients generated.  If |force_sid| is
    // true, a SID frame is forced and the internal sid interval counter is reset.
    // Will fail if the input size is too large (> 640 samples, see
    // ComfortNoiseDecoder::Generate).
    size_t Encode(ArrayView<const int16_t> speech,
                  bool force_sid,
                  Buffer *output);

private:
    size_t enc_nrOfCoefs_;
    int enc_sampfreq_;
    int16_t enc_interval_;
    int16_t enc_msSinceSid_;
    int32_t enc_Energy_;
    int16_t enc_reflCoefs_[WEBRTC_CNG_MAX_LPC_ORDER + 1];
    int32_t enc_corrVector_[WEBRTC_CNG_MAX_LPC_ORDER + 1];
    uint32_t enc_seed_;
};


#endif  // MODULES_AUDIO_CODING_CODECS_CNG_WEBRTC_CNG_H_
