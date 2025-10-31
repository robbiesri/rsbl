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
    } // namespace Internal

    // Basically the same as std::remove_reference except we use the Internal namespace to demarcate
    // usage of RemoveReference. There's no reason to use the ::type manually.
    template <class T>
    using RemoveReference = typename Internal::RemoveReference<T>::type;

} // namespace rsbl

// Borrowed from and justified by  https://www.foonathan.net/2020/09/move-forward/
// tl;dr - functions bad
#define rsblMove(...) static_cast<rsbl::RemoveReference<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define rsblForward(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
