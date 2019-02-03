/* Copyright (c) 2019, Chris Smeele
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

template<typename T>
class own_ptr_base {
protected:
    T *ptr;

public:
    constexpr T *operator->() { return ptr; }
    constexpr T *operator*()  { return ptr; }

    constexpr explicit operator addr_t() const { return (addr_t)ptr; }

    constexpr explicit operator       T*()       { return ptr; }
    constexpr explicit operator const T*() const { return ptr; }

    constexpr operator bool() const { return ptr != nullptr; }

    [[nodiscard]]
    constexpr T *release() {
        auto x = ptr;
        ptr = nullptr;
        return x;
    }

protected:
    constexpr own_ptr_base() = default;
    ~own_ptr_base() = default;
};

template<typename T>
class own_ptr : public own_ptr_base<T> {
public:
    constexpr own_ptr &operator=(own_ptr &&ptr) { this->ptr = ptr.release(); return *this; }

    constexpr own_ptr(own_ptr &&ptr)    { this->ptr = ptr.release(); }
    constexpr own_ptr(T *ptr = nullptr) { this->ptr = ptr; }

    ~own_ptr() {
        if (this->ptr)
            delete this->ptr;
    }

    own_ptr(const own_ptr&) = delete;
};

template<typename T>
class own_array : public own_ptr_base<T> {
public:
    constexpr const T &operator[](size_t i) const { return this->ptr[i]; }
    constexpr       T &operator[](size_t i)       { return this->ptr[i]; }

    constexpr own_array(own_array &&ptr)  { this->ptr = ptr.release(); }
    constexpr own_array(T *ptr = nullptr) { this->ptr = ptr; }

    ~own_array() {
        if (this->ptr)
            delete[] this->ptr;
    }
};
