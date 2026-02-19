#ifndef ENUM_ARRAY_H
#define ENUM_ARRAY_H

#include <array>

// enum values must be contiguous, start at 0, and < S
template<typename T, typename E, size_t S>
class EnumArray {
public:

    size_t size() const { return array.size(); }

    T& operator[](size_t size) { return array[size]; }
    T& operator[](E enum_) { return array[static_cast<size_t>(enum_)]; }

    auto begin() { return array.begin(); }
    auto end() { return array.end(); }
    auto rbegin() { return array.rbegin(); }
    auto rend() { return array.rend(); }

    T& front() { return array.front(); }
    T& back() { return array.back(); }

    // const EnumArray support
    const T& operator[](size_t size) const { return array[size]; }
    const T& operator[](E enum_) const { return array[static_cast<size_t>(enum_)]; }

    auto begin() const { return array.begin(); }
    auto end() const { return array.end(); }
    auto rbegin() const { return array.rbegin(); }
    auto rend() const { return array.rend(); }

    const T& front() const { return array.front(); }
    const T& back() const { return array.back(); }


private:
	std::array<T, S> array;
};

#endif
