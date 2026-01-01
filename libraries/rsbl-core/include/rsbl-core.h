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
