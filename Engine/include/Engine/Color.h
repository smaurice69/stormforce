#pragma once

#include <cstdint>

namespace Engine
{
    struct Color
    {
        uint8_t r{0};
        uint8_t g{0};
        uint8_t b{0};
        uint8_t a{255};

        constexpr Color() = default;
        constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
            : r(red), g(green), b(blue), a(alpha)
        {
        }

        static constexpr Color FromFloats(float red, float green, float blue, float alpha = 1.0f)
        {
            return Color(static_cast<uint8_t>(red * 255.0f), static_cast<uint8_t>(green * 255.0f),
                         static_cast<uint8_t>(blue * 255.0f), static_cast<uint8_t>(alpha * 255.0f));
        }
    };
}
