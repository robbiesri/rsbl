// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

namespace rsbl
{
namespace Internal
{
    template <class T>
    struct RemoveReference
    {
        using type = T;
    };
    template <class T>
    struct RemoveReference<T&>
    {
        using type = T;
    };
    template <class T>
    struct RemoveReference<T&&>
    {
        using type = T;
    };

    template <class T>
    struct RemoveConst
    {
        using type = T;
    };
    template <class T>
    struct RemoveConst<const T>
    {
        using type = T;
    };

    template <class T>
    struct RemoveVolatile
    {
        using type = T;
    };
    template <class T>
    struct RemoveVolatile<volatile T>
    {
        using type = T;
    };

    template <class T>
    struct RemoveCV
    {
        using type = typename RemoveVolatile<typename RemoveConst<T>::type>::type;
    };

    template <class T>
    struct Decay
    {
        using type = typename RemoveCV<typename RemoveReference<T>::type>::type;
    };
} // namespace Internal

// Basically the same as std::remove_reference except we use the Internal namespace to demarcate
// usage of RemoveReference. There's no reason to use the ::type manually.
template <class T>
using RemoveReference = typename Internal::RemoveReference<T>::type;

template <class T>
using RemoveConst = typename Internal::RemoveConst<T>::type;

template <class T>
using RemoveVolatile = typename Internal::RemoveVolatile<T>::type;

template <class T>
using RemoveCV = typename Internal::RemoveCV<T>::type;

template <class T>
using Decay = typename Internal::Decay<T>::type;

} // namespace rsbl

// Borrowed from and justified by  https://www.foonathan.net/2020/09/move-forward/
// tl;dr - functions bad
#define rsblMove(...) static_cast<rsbl::RemoveReference<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define rsblForward(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace rsbl
{
// ReferenceWrapper - similar to std::reference_wrapper
// Wraps a reference in a copyable, assignable object
template <typename T>
class ReferenceWrapper
{
  public:
    using type = T;

    // Constructor from lvalue reference
    ReferenceWrapper(T& ref) noexcept
        : m_ptr(&ref)
    {
    }

    // Copy constructor
    ReferenceWrapper(const ReferenceWrapper&) noexcept = default;

    // Copy assignment
    ReferenceWrapper& operator=(const ReferenceWrapper&) noexcept = default;

    // Implicit conversion to T&
    operator T&() const noexcept
    {
        return *m_ptr;
    }

    // Get the underlying reference
    T& Get() const noexcept
    {
        return *m_ptr;
    }

    // Call operator forwarding (for callable objects)
    // I actually don't know how much I care about this, but I let Claude implement it :p
    // I'm not crazy about the trailing return mish-mash needed to get decltype to infer the return
    // type. Interestingly, MSVC did not like using m_ptr in there, so we cast nullptr wtf
    template <typename... Args>
    auto operator()(Args&&... args) const
        -> decltype((*(static_cast<T*>(nullptr)))(rsblForward(args)...))
    {
        return (*m_ptr)(rsblForward(args)...);
    }

  private:
    T* m_ptr;
};

// Helper function to create ReferenceWrapper (like std::ref)
template <typename T>
ReferenceWrapper<T> Ref(T& ref) noexcept
{
    return ReferenceWrapper<T>(ref);
}

// Helper function to create const ReferenceWrapper (like std::cref)
template <typename T>
ReferenceWrapper<const T> CRef(const T& ref) noexcept
{
    return ReferenceWrapper<const T>(ref);
}

// Unwrap reference wrappers - returns the underlying type
template <typename T>
struct UnwrapReference
{
    using type = T;
};

template <typename T>
struct UnwrapReference<ReferenceWrapper<T>>
{
    using type = T&;
};

template <typename T>
using UnwrapReferenceType = typename UnwrapReference<T>::type;

} // namespace rsbl
