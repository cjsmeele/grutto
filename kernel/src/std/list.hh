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

template<typename T>
class BiList {

public:
    struct Item {
        Item *prev;
        Item *next;
        T val;

              T &operator* ()       { return  val; }
        const T &operator* () const { return  val; }
              T *operator->()       { return &val; }
        const T *operator->() const { return &val; }

        Item &append(const T &v) {
            Item *it = new Item{this, nullptr, v};
            if (next)
                it->next = next;
            next = it;
            return *it;
        }
        Item &prepend(const T &v) {
            Item *it = new Item{nullptr, this, v};
            if (prev)
                it->prev = prev;
            prev = it;
            return *it;
        }

        Item() = delete;
    };

    struct Iter {
        Item *item;

        Iter &operator++() { item = item->next; return *this; }
        bool  operator==(const Iter &o) const { return item == o.item; }
        bool  operator!=(const Iter &o) const { return item != o.item; }

              Item &operator*()       { return *item; }
        const Item &operator*() const { return *item; }

        Iter(Item *item) : item(item) { }
    };

private:
    Item *first_ = nullptr;
    Item *last_  = nullptr;

    Item *first() { if (first_) while (first_->prev) first_ = first_->prev; return first_; }
    Item *last()  { if (last_)  while (last_->next)  last_  =  last_->next; return last_;  }
    const Item *first() const { const auto *i = first_; if (i) while (i->prev) i = i->prev; return i; }
    const Item *last()  const { const auto *i = last_;  if (i) while (i->next) i = i->next; return i; }

public:
          Iter begin()       { return Iter{first()}; }
    const Iter begin() const { return Iter{first()}; }
          Iter end  ()       { return Iter{nullptr}; }
    const Iter end  () const { return Iter{nullptr}; }

    void append(const T &v) {
        if (last())
             last_ = &last()->append(v);
        else last_ = first_ = new Item{nullptr, nullptr, v};
    }
    void prepend(const T &v) {
        if (first())
             first_ = &first()->prepend(v);
        else first_ = last_ = new Item{nullptr, nullptr, v};
    }

    ~BiList() {
        auto *it = first();
        while (it) {
            auto *next = it->next;
            delete it;
            it = next;
        }
    }
};

template<typename T> using List = BiList<T>;

// struct X {
//     static int z;
//     int x;
//     X &operator=(const X &o) { x = ++z; koi.fmt("x {} alloced @{} (assigned from x {})\n", x, this, o.x); return *this; }
//     X(X&&o) { x = ++z; koi.fmt("x {} alloced @{} (moved from x {})\n", x, this, o.x); }
//     X(const X&o) { x = ++z; koi.fmt("x {} alloced @{} (copied from x {})\n", x, this, o.x); }
//      X() { x = ++z; koi.fmt("x {} alloced @{}\n", x, this); }
//     ~X() { koi.fmt("x {} deleted @{}\n", x, this); }
// };

// int X::z = 0;
