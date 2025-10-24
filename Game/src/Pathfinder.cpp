#include "Game/Pathfinder.h"

#include <queue>
#include <algorithm>

namespace Game
{
    Pathfinder::Pathfinder(int width, int height)
        : m_width(width)
        , m_height(height)
        , m_blocked(width * height, false)
    {
    }

    void Pathfinder::SetBlocked(int x, int y, bool blocked)
    {
        if (x < 0 || y < 0 || x >= m_width || y >= m_height)
        {
            return;
        }
        m_blocked[y * m_width + x] = blocked;
    }

    bool Pathfinder::IsBlocked(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= m_width || y >= m_height)
        {
            return true;
        }
        return m_blocked[y * m_width + x];
    }

    std::optional<std::vector<GridCoord>> Pathfinder::FindPath(GridCoord start, GridCoord goal) const
    {
        if (IsBlocked(goal.x, goal.y))
        {
            return std::nullopt;
        }

        std::vector<int> cameFrom(m_width * m_height, -1);
        std::queue<GridCoord> open;
        open.push(start);
        auto index = [this](int x, int y) { return y * m_width + x; };
        cameFrom[index(start.x, start.y)] = index(start.x, start.y);

        const GridCoord dirs[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

        while (!open.empty())
        {
            GridCoord current = open.front();
            open.pop();

            if (current == goal)
            {
                break;
            }

            for (const auto& dir : dirs)
            {
                GridCoord next{current.x + dir.x, current.y + dir.y};
                if (next.x < 0 || next.y < 0 || next.x >= m_width || next.y >= m_height)
                {
                    continue;
                }
                if (IsBlocked(next.x, next.y))
                {
                    continue;
                }
                int idx = index(next.x, next.y);
                if (cameFrom[idx] == -1)
                {
                    cameFrom[idx] = index(current.x, current.y);
                    open.push(next);
                }
            }
        }

        if (cameFrom[index(goal.x, goal.y)] == -1)
        {
            return std::nullopt;
        }

        std::vector<GridCoord> path;
        GridCoord current = goal;
        while (!(current == start))
        {
            path.push_back(current);
            int prev = cameFrom[index(current.x, current.y)];
            current = {prev % m_width, prev / m_width};
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        return path;
    }
}
