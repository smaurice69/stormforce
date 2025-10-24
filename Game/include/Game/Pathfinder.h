#pragma once

#include "Engine/MathTypes.h"

#include <optional>
#include <vector>

namespace Game
{
    struct GridCoord
    {
        int x{0};
        int y{0};
        bool operator==(const GridCoord& other) const { return x == other.x && y == other.y; }
    };

    class Pathfinder
    {
    public:
        Pathfinder(int width, int height);

        void SetBlocked(int x, int y, bool blocked);
        bool IsBlocked(int x, int y) const;

        std::optional<std::vector<GridCoord>> FindPath(GridCoord start, GridCoord goal) const;

    private:
        int m_width{0};
        int m_height{0};
        std::vector<bool> m_blocked;
    };
}
