#pragma once

#include <Arduino.h>
#include <cmath>
#include <type_traits>

template <typename T>
class Vector2
{
    T _x;
    T _y;

public:
    constexpr Vector2(T x, T y) noexcept : _x{x}, _y{y} { }
    constexpr Vector2() noexcept : Vector2<T>{0, 0} { }

    [[nodiscard]]
    constexpr bool operator==(const Vector2<T> &o) const noexcept {
        return _x == o._x && _y == o._y;
    }
    [[nodiscard]]
    constexpr bool operator!=(const Vector2<T> &o) const noexcept {
        return _x != o._x || _y != o._y;
    }

    template <typename U>
    [[nodiscard]]
    constexpr explicit operator Vector2<U>() const noexcept {
        if constexpr (std::is_floating_point<T>::value && ! std::is_floating_point<U>::value)
            return {static_cast<U>(std::round(_x)), static_cast<U>(std::round(_y))};
        else
            return {static_cast<U>(_x), static_cast<U>(_y)};
    }

    [[nodiscard]]
    constexpr T x() const noexcept { return _x; }
    [[nodiscard]]
    constexpr T y() const noexcept { return _y; }

    constexpr Vector2<T> &setX(T x) noexcept { _x = x; return *this; }
    constexpr Vector2<T> &setY(T y) noexcept { _y = y; return *this; }
    constexpr Vector2<T> &set(T x, T y) noexcept { _x = x; _y = y; return *this; }

    [[nodiscard]]
    constexpr Vector2<T> operator+(const Vector2<T> &o) const noexcept {
        return Vector2<T>{_x + o._x, _y + o._y};
    }

    [[nodiscard]]
    constexpr Vector2<T> operator-(const Vector2<T> &o) const noexcept {
        return Vector2<T>{_x - o._x, _y - o._y};
    }

    [[nodiscard]]
    constexpr Vector2<T> operator*(T s) const noexcept {
        return Vector2<T>{_x * s, _y * s};
    }

    [[nodiscard]]
    constexpr T operator*(const Vector2<T> &o) const noexcept {
        return _x * o._x + _y * o._y;
    }

    [[nodiscard]]
    constexpr Vector2<T> operator/(T s) const {
        return Vector2<T>{_x / s, _y / s};
    }

    Vector2<T> operator+=(const Vector2<T> &o) noexcept {
        _x = _x + o._x;
        _y = _y + o._y;
        return *this;
    }

    Vector2<T> operator-=(const Vector2<T> &o) noexcept {
        _x = _x - o._x;
        _y = _y - o._y;
        return *this;
    }

    Vector2<T> operator*=(T s) noexcept {
        _x = _x * s;
        _y = _y * s;
        return *this;
    }

    Vector2<T> operator/=(T s) {
        _x = _x / s;
        _y = _y / s;
        return *this;
    }

    [[nodiscard]]
    constexpr Vector2<T> operator-() const noexcept {
        return Vector2<T>{-_x, -_y};
    }

    [[nodiscard]]
    constexpr Vector2<T> operator~() const noexcept {
        return Vector2<T>{_y, _x};
    }

    [[nodiscard]]
    constexpr T length() const noexcept {
        return std::sqrt(_x * _x + _y * _y);
    }

    [[nodiscard]]
    constexpr Vector2<T> normalized() const {
        return *this / length();
    }

    Vector2<T> normalize() {
        return *this /= length();
    }

    [[nodiscard]]
    constexpr double direction() const {
        return std::atan2(_y, _x);
    }

    [[nodiscard]]
    constexpr Vector2<T> rotated(double angle) const noexcept {
        const auto cos = std::cos(angle);
        const auto sin = std::sin(angle);
        return static_cast<Vector2<T>>( Vector2<double>{_x * cos - _y * sin, _x * sin + _y * cos});
    }

    Vector2<T> rotate(double angle) noexcept {
        return *this = this->rotated(angle);
    }

    operator String() const {
        String s;
        s.reserve(10);
        s += '(';
        s += _x;
        s += 'x';
        s += _y;
        s += ')';
        return s;
    }

    static constexpr Vector2<T> fromDirLen(double direction, double length) {
        return (Vector2<T>::X() * length).rotated(direction);
    }

    static constexpr Vector2<T> X() { return Vector2(1, 0); }
    static constexpr Vector2<T> Y() { return Vector2{0, 1}; }
    static constexpr Vector2<T> ZERO() { return Vector2{0, 0}; }
};
