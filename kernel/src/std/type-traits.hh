/* Copyright (c) 2018, Chris Smeele
 *
 * This file is part of Grutto.
 *
 * Grutto is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Grutto is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Grutto.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "types.hh"

template<typename T> struct is_void       { static constexpr bool value = false; };
template<>           struct is_void<void> { static constexpr bool value = true;  };

template<typename T> struct is_integral       { static constexpr bool value = false; };
template<>           struct is_integral<char> { static constexpr bool value = true;  };
template<>           struct is_integral< u8>  { static constexpr bool value = true;  };
template<>           struct is_integral< s8>  { static constexpr bool value = true;  };
template<>           struct is_integral<u16>  { static constexpr bool value = true;  };
template<>           struct is_integral<s16>  { static constexpr bool value = true;  };
template<>           struct is_integral<u32>  { static constexpr bool value = true;  };
template<>           struct is_integral<s32>  { static constexpr bool value = true;  };
template<>           struct is_integral<u64>  { static constexpr bool value = true;  };
template<>           struct is_integral<s64>  { static constexpr bool value = true;  };

template<typename T> struct is_float              { static constexpr bool value = false; };
template<>           struct is_float<float>       { static constexpr bool value = true;  };
template<>           struct is_float<double>      { static constexpr bool value = true;  };
template<>           struct is_float<long double> { static constexpr bool value = true;  };

template<typename T> struct is_signed   { static constexpr bool value = static_cast<T>(-1) < static_cast<T>(0); };
template<typename T> struct is_unsigned { static constexpr bool value = !is_signed<T>::value; };

template<typename T1, typename T2> struct is_same         { static constexpr bool value = false; };
template<typename T1>              struct is_same<T1, T1> { static constexpr bool value = true;  };

template<typename T> struct add_signed               { using type = T;   };
template<>           struct add_signed<char>         { using type =  s8; };
template<>           struct add_signed< u8>          { using type =  s8; };
template<>           struct add_signed<u16>          { using type = s16; };
template<>           struct add_signed<u32>          { using type = s32; };
template<>           struct add_signed<u64>          { using type = s64; };
template<typename T> struct add_unsigned             { using type = T;   };
template<>           struct add_unsigned<char>       { using type =  u8; };
template<>           struct add_unsigned< s8>        { using type =  u8; };
template<>           struct add_unsigned<s16>        { using type = u16; };
template<>           struct add_unsigned<s32>        { using type = u32; };
template<>           struct add_unsigned<s64>        { using type = u64; };

template<typename T> struct remove_const                { using type = T; };
template<typename T> struct remove_const<const T>       { using type = T; };
template<typename T> struct remove_volatile             { using type = T; };
template<typename T> struct remove_volatile<volatile T> { using type = T; };
template<typename T> struct remove_cv {
    using type = typename remove_volatile<typename remove_const<T>::type>::type;
};

template<bool, typename T> struct enable_if          { };
template<typename T>       struct enable_if<true, T> { using type = T; };


template<typename T>
struct intmax {
    constexpr static size_t value = is_signed<T>::value
        ? ~T(1 << (sizeof(T)*8-1)) : static_cast<T>(-1);
};
