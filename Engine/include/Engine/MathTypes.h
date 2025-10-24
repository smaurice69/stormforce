#pragma once

#include <cstdint>

namespace Engine
{
    struct Vector2f
    {
        float x{0.0f};
        float y{0.0f};

        Vector2f() = default;
        Vector2f(float inX, float inY) : x(inX), y(inY) {}

        Vector2f operator+(const Vector2f& rhs) const { return {x + rhs.x, y + rhs.y}; }
        Vector2f operator-(const Vector2f& rhs) const { return {x - rhs.x, y - rhs.y}; }
        Vector2f operator*(float scalar) const { return {x * scalar, y * scalar}; }
        Vector2f& operator+=(const Vector2f& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }
        Vector2f& operator-=(const Vector2f& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }
    };

    struct Rectf
    {
        float left{0.0f};
        float top{0.0f};
        float right{0.0f};
        float bottom{0.0f};

        Rectf() = default;
        Rectf(float inLeft, float inTop, float inRight, float inBottom)
            : left(inLeft), top(inTop), right(inRight), bottom(inBottom)
        {
        }

        float Width() const { return right - left; }
        float Height() const { return bottom - top; }
        bool Contains(const Vector2f& p) const { return p.x >= left && p.x <= right && p.y >= top && p.y <= bottom; }
    };
}
