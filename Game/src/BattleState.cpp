#include "Game/BattleState.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <cwchar>

namespace Game
{
    BattleState::BattleState(const CampaignData& campaign, int gridWidth, int gridHeight, float cellSize)
        : m_campaign(campaign)
        , m_pathfinder(gridWidth, gridHeight)
        , m_gridWidth(gridWidth)
        , m_gridHeight(gridHeight)
        , m_cellSize(cellSize)
    {
        m_spawnCell = {0, gridHeight / 2};
        m_goalCell = {gridWidth - 1, gridHeight / 2};
        m_camera.SetViewport(gridWidth * cellSize, gridHeight * cellSize);
        m_camera.SetTarget({gridWidth * cellSize * 0.5f, gridHeight * cellSize * 0.5f});
        m_camera.SetZoom(1.0f);
        if (!campaign.towers.empty())
        {
            m_selectedTower = &campaign.towers.begin()->second;
            m_selectedTowerId = m_selectedTower->id;
        }
    }

    void BattleState::Start(const BattleConfig& config)
    {
        m_active = true;
        m_result = BattleResult::None;
        m_towers.clear();
        m_enemies.clear();
        m_projectiles.clear();
        m_waveSchedule.clear();
        m_basePath.clear();
        m_hoverCell.reset();
        m_lastLeftDown = false;
        m_lastRightDown = false;
        m_entryTimer = 0.0;
        m_spawnedInWave = 0;
        m_waveIndex = 0;
        m_time = 0.0;
        m_fastForward = false;
        m_credits = config.startingCredits;
        m_lives = config.startingLives;

        if (config.sector)
        {
            m_currentSectorId = config.sector->id;
            auto iter = m_campaign.wavesets.find(config.sector->waveSetId);
            if (iter != m_campaign.wavesets.end())
            {
                m_waveSchedule = iter->second.entries;
            }
        }
        else
        {
            m_currentSectorId.clear();
        }

        if (!m_selectedTower && !m_campaign.towers.empty())
        {
            m_selectedTower = &m_campaign.towers.begin()->second;
            m_selectedTowerId = m_selectedTower->id;
        }

        for (int y = 0; y < m_gridHeight; ++y)
        {
            for (int x = 0; x < m_gridWidth; ++x)
            {
                m_pathfinder.SetBlocked(x, y, false);
            }
        }

        auto path = m_pathfinder.FindPath(m_spawnCell, m_goalCell);
        if (path)
        {
            m_basePath = *path;
        }
        else
        {
            m_active = false;
            m_result = BattleResult::Defeat;
        }
    }

    void BattleState::Load(const BattleConfig& config, const BattleSave& save)
    {
        Start(config);
        m_currentSectorId = save.sectorId;
        if (!m_selectedTower && !m_campaign.towers.empty())
        {
            m_selectedTower = &m_campaign.towers.begin()->second;
            m_selectedTowerId = m_selectedTower->id;
        }
        m_credits = save.credits;
        m_lives = save.lives;
        m_time = save.time;
        m_waveIndex = std::min(save.waveIndex, m_waveSchedule.size());
        m_spawnedInWave = save.spawnedInWave;
        if (m_waveIndex < m_waveSchedule.size())
        {
            m_spawnedInWave = std::min(m_spawnedInWave, m_waveSchedule[m_waveIndex].count);
        }
        m_entryTimer = save.entryTimer;
        m_towers.clear();
        for (const auto& saved : save.towers)
        {
            auto defIter = m_campaign.towers.find(saved.towerId);
            if (defIter != m_campaign.towers.end())
            {
                GridCoord cell{saved.gridX, saved.gridY};
                m_pathfinder.SetBlocked(cell.x, cell.y, true);
                TowerInstance tower;
                tower.definition = &defIter->second;
                tower.gridX = cell.x;
                tower.gridY = cell.y;
                tower.worldPosition = CellToWorld(cell.x, cell.y);
                m_towers.push_back(tower);
            }
        }
        m_enemies.clear();
        for (const auto& savedEnemy : save.enemies)
        {
            auto enemyIter = m_campaign.enemies.find(savedEnemy.enemyId);
            if (enemyIter != m_campaign.enemies.end())
            {
                EnemyInstance enemy;
                enemy.definition = &enemyIter->second;
                enemy.position = savedEnemy.position;
                enemy.health = savedEnemy.health;
                enemy.alive = savedEnemy.health > 0.0f;
                enemy.nextNode = savedEnemy.nextNode;
                enemy.path.clear();
                for (const auto& cell : m_basePath)
                {
                    enemy.path.push_back(CellToWorld(cell.x, cell.y));
                }
                m_enemies.push_back(enemy);
            }
        }
        RecomputePaths();
    }

    void BattleState::Update(double dt)
    {
        if (!m_active)
        {
            return;
        }

        double speedMultiplier = m_fastForward ? 3.0 : 1.0;
        double scaledDt = dt * speedMultiplier;
        m_time += scaledDt;

        if (m_waveIndex < m_waveSchedule.size())
        {
            auto& entry = m_waveSchedule[m_waveIndex];
            if (m_time >= entry.startDelay)
            {
                m_entryTimer += scaledDt;
                if (m_spawnedInWave < entry.count && m_entryTimer >= entry.interval)
                {
                    SpawnEnemy(entry);
                    m_entryTimer = 0.0;
                    ++m_spawnedInWave;
                }
                if (m_spawnedInWave >= entry.count)
                {
                    ++m_waveIndex;
                    m_spawnedInWave = 0;
                    m_entryTimer = 0.0;
                }
            }
        }

        UpdateEnemies(scaledDt);
        UpdateTowers(scaledDt);
        UpdateProjectiles(scaledDt);
        CheckVictory();
    }

    void BattleState::Render(Engine::Renderer2D& renderer)
    {
        renderer.BeginFrame({18, 20, 28});

        for (int y = 0; y < m_gridHeight; ++y)
        {
            for (int x = 0; x < m_gridWidth; ++x)
            {
                Engine::Rectf rect{CellToWorld(x, y).x - m_cellSize * 0.5f, CellToWorld(x, y).y - m_cellSize * 0.5f,
                                   CellToWorld(x, y).x + m_cellSize * 0.5f, CellToWorld(x, y).y + m_cellSize * 0.5f};
                renderer.DrawRectOutline(rect, {40, 45, 58});
            }
        }

        for (const auto& tower : m_towers)
        {
            auto center = CellToWorld(tower.gridX, tower.gridY);
            Engine::Rectf rect{center.x - m_cellSize * 0.4f, center.y - m_cellSize * 0.4f, center.x + m_cellSize * 0.4f,
                               center.y + m_cellSize * 0.4f};
            renderer.DrawFilledRect(rect, {110, 200, 230});
        }

        for (const auto& enemy : m_enemies)
        {
            if (!enemy.alive)
            {
                continue;
            }
            auto color = enemy.definition ? ParseColor(enemy.definition->tint) : Engine::Color{220, 90, 90};
            renderer.DrawCircle(enemy.position, m_cellSize * 0.3f, color, true);
        }

        for (const auto& projectile : m_projectiles)
        {
            if (!projectile.active)
            {
                continue;
            }
            auto color = ParseColor(projectile.color);
            renderer.DrawCircle(projectile.position, m_cellSize * 0.15f, color, true);
        }

        if (m_hoverCell)
        {
            auto center = CellToWorld(m_hoverCell->x, m_hoverCell->y);
            Engine::Rectf rect{center.x - m_cellSize * 0.5f, center.y - m_cellSize * 0.5f, center.x + m_cellSize * 0.5f,
                               center.y + m_cellSize * 0.5f};
            renderer.DrawRectOutline(rect, {180, 220, 255}, 2);
        }

        renderer.DrawText(L"Credits: " + std::to_wstring(m_credits), {10.0f, 10.0f}, {220, 220, 220});
        renderer.DrawText(L"Lives: " + std::to_wstring(m_lives), {10.0f, 30.0f}, {220, 220, 220});
        renderer.DrawText(L"Fast Forward: " + std::wstring(m_fastForward ? L"ON" : L"OFF"), {10.0f, 50.0f},
                          {220, 220, 220});
        if (m_selectedTower)
        {
            renderer.DrawText(L"Tower: " + m_selectedTower->name + L" (" + std::to_wstring(m_selectedTower->cost) + L")",
                              {10.0f, 70.0f}, {200, 220, 240});
        }

        if (m_result == BattleResult::Victory)
        {
            renderer.DrawText(L"Victory! Press Enter", {m_cellSize * 3.0f, m_cellSize}, {240, 240, 255}, 24);
        }
        else if (m_result == BattleResult::Defeat)
        {
            renderer.DrawText(L"Defeat... Press Enter", {m_cellSize * 3.0f, m_cellSize}, {240, 200, 200}, 24);
        }

        renderer.EndFrame();
    }

    void BattleState::HandleMouse(const Engine::Vector2f& worldPos, bool leftDown, bool rightDown)
    {
        m_hoverCell = WorldToCell(worldPos);
        if (m_hoverCell && m_selectedTower)
        {
            auto cell = *m_hoverCell;
            if (leftDown && !m_lastLeftDown)
            {
                if (!m_pathfinder.IsBlocked(cell.x, cell.y) && cell != m_spawnCell && cell != m_goalCell &&
                    m_credits >= m_selectedTower->cost)
                {
                    m_pathfinder.SetBlocked(cell.x, cell.y, true);
                    auto newPath = m_pathfinder.FindPath(m_spawnCell, m_goalCell);
                    if (newPath)
                    {
                        TowerInstance tower;
                        tower.definition = m_selectedTower;
                        tower.gridX = cell.x;
                        tower.gridY = cell.y;
                        tower.worldPosition = CellToWorld(cell.x, cell.y);
                        m_towers.push_back(tower);
                        m_credits -= m_selectedTower->cost;
                        m_basePath = *newPath;
                        RecomputePaths();
                    }
                    else
                    {
                        m_pathfinder.SetBlocked(cell.x, cell.y, false);
                    }
                }
            }
            if (rightDown && !m_lastRightDown)
            {
                auto iter = std::find_if(m_towers.begin(), m_towers.end(), [cell](const TowerInstance& tower) {
                    return tower.gridX == cell.x && tower.gridY == cell.y;
                });
                if (iter != m_towers.end())
                {
                    m_pathfinder.SetBlocked(iter->gridX, iter->gridY, false);
                    m_credits += iter->definition->cost / 2;
                    m_towers.erase(iter);
                    RecomputePaths();
                }
            }
        }
        m_lastLeftDown = leftDown;
        m_lastRightDown = rightDown;
    }

    void BattleState::HandleKey(uint8_t key, bool pressed)
    {
        if (!pressed)
        {
            return;
        }
        if (key == VK_SPACE)
        {
            m_fastForward = !m_fastForward;
        }
        else if (key >= '1' && key <= '9')
        {
            int index = key - '1';
            if (index < static_cast<int>(m_campaign.towers.size()))
            {
                auto iter = m_campaign.towers.begin();
                std::advance(iter, index);
                m_selectedTower = &iter->second;
                m_selectedTowerId = iter->first;
            }
        }
    }

    const BattleSave BattleState::CreateSnapshot() const
    {
        BattleSave save;
        save.sectorId = m_currentSectorId;
        save.credits = m_credits;
        save.lives = m_lives;
        save.time = m_time;
        save.waveIndex = m_waveIndex;
        save.spawnedInWave = m_spawnedInWave;
        save.entryTimer = m_entryTimer;
        for (const auto& tower : m_towers)
        {
            TowerSaveData saved;
            saved.towerId = tower.definition ? tower.definition->id : L"";
            saved.gridX = tower.gridX;
            saved.gridY = tower.gridY;
            save.towers.push_back(saved);
        }
        for (const auto& enemy : m_enemies)
        {
            if (!enemy.alive || !enemy.definition)
            {
                continue;
            }
            EnemySaveData data;
            data.enemyId = enemy.definition->id;
            data.position = enemy.position;
            data.health = enemy.health;
            data.nextNode = enemy.nextNode;
            save.enemies.push_back(data);
        }
        return save;
    }

    Engine::Color BattleState::ParseColor(const std::wstring& hex) const
    {
        if (hex.size() < 7)
        {
            return {255, 255, 255};
        }
        unsigned int value = 0;
        std::wstring trimmed = hex;
        if (!trimmed.empty() && trimmed[0] == L'#')
        {
            trimmed = trimmed.substr(1);
        }
        swscanf(trimmed.c_str(), L"%x", &value);
        return {static_cast<uint8_t>((value >> 16) & 0xFF), static_cast<uint8_t>((value >> 8) & 0xFF),
                static_cast<uint8_t>(value & 0xFF)};
    }

    Engine::Vector2f BattleState::CellToWorld(int x, int y) const
    {
        return {(x + 0.5f) * m_cellSize, (y + 0.5f) * m_cellSize};
    }

    std::optional<GridCoord> BattleState::WorldToCell(const Engine::Vector2f& world) const
    {
        int x = static_cast<int>(world.x / m_cellSize);
        int y = static_cast<int>(world.y / m_cellSize);
        if (x < 0 || y < 0 || x >= m_gridWidth || y >= m_gridHeight)
        {
            return std::nullopt;
        }
        return GridCoord{x, y};
    }

    void BattleState::SpawnEnemy(const WaveEntry& entry)
    {
        auto enemyIter = m_campaign.enemies.find(entry.enemyId);
        if (enemyIter == m_campaign.enemies.end() || m_basePath.empty())
        {
            return;
        }
        EnemyInstance enemy;
        enemy.definition = &enemyIter->second;
        enemy.position = CellToWorld(m_spawnCell.x, m_spawnCell.y);
        enemy.health = enemy.definition->health;
        enemy.path.clear();
        for (const auto& cell : m_basePath)
        {
            enemy.path.push_back(CellToWorld(cell.x, cell.y));
        }
        enemy.nextNode = enemy.path.size() > 1 ? 1 : 0;
        m_enemies.push_back(enemy);
    }

    void BattleState::UpdateEnemies(double dt)
    {
        for (auto& enemy : m_enemies)
        {
            if (!enemy.alive)
            {
                continue;
            }
            if (enemy.nextNode >= enemy.path.size())
            {
                enemy.alive = false;
                if (m_lives > 0)
                {
                    --m_lives;
                }
                continue;
            }
            auto target = enemy.path[enemy.nextNode];
            Engine::Vector2f dir{target.x - enemy.position.x, target.y - enemy.position.y};
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 1e-3f)
            {
                dir.x /= len;
                dir.y /= len;
            }
            float speed = enemy.definition->speed;
            enemy.position.x += dir.x * speed * static_cast<float>(dt);
            enemy.position.y += dir.y * speed * static_cast<float>(dt);
            if (len <= static_cast<float>(speed * dt))
            {
                ++enemy.nextNode;
            }
        }
    }

    void BattleState::UpdateTowers(double dt)
    {
        for (auto& tower : m_towers)
        {
            tower.cooldown -= dt;
            if (tower.cooldown > 0.0)
            {
                continue;
            }
            if (!tower.definition)
            {
                continue;
            }

            EnemyInstance* targetEnemy = nullptr;
            float bestDist = std::numeric_limits<float>::max();
            for (auto& enemy : m_enemies)
            {
                if (!enemy.alive || !enemy.definition)
                {
                    continue;
                }
                float dx = enemy.position.x - tower.worldPosition.x;
                float dy = enemy.position.y - tower.worldPosition.y;
                float distSq = dx * dx + dy * dy;
                float range = tower.definition->range;
                if (distSq <= range * range && distSq < bestDist)
                {
                    bestDist = distSq;
                    targetEnemy = &enemy;
                }
            }

            if (targetEnemy)
            {
                Projectile projectile;
                projectile.position = tower.worldPosition;
                Engine::Vector2f dir{targetEnemy->position.x - tower.worldPosition.x,
                                     targetEnemy->position.y - tower.worldPosition.y};
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len > 1e-3f)
                {
                    dir.x /= len;
                    dir.y /= len;
                }
                projectile.velocity = dir * tower.definition->projectileSpeed;
                projectile.damage = tower.definition->damage;
                projectile.targetIndex = static_cast<int>(targetEnemy - m_enemies.data());
                projectile.color = tower.definition->projectileColor;
                projectile.maxLifetime = 2.0f;
                m_projectiles.push_back(projectile);
                tower.cooldown = 1.0 / tower.definition->fireRate;
            }
        }
    }

    void BattleState::UpdateProjectiles(double dt)
    {
        for (auto& projectile : m_projectiles)
        {
            if (!projectile.active)
            {
                continue;
            }
            projectile.lifetime += static_cast<float>(dt);
            if (projectile.lifetime >= projectile.maxLifetime)
            {
                projectile.active = false;
                continue;
            }
            projectile.position += projectile.velocity * static_cast<float>(dt);
            if (projectile.targetIndex >= 0 && projectile.targetIndex < static_cast<int>(m_enemies.size()))
            {
                auto& enemy = m_enemies[projectile.targetIndex];
                if (!enemy.alive || !enemy.definition)
                {
                    continue;
                }
                float dx = enemy.position.x - projectile.position.x;
                float dy = enemy.position.y - projectile.position.y;
                float distSq = dx * dx + dy * dy;
                if (distSq <= (m_cellSize * 0.2f) * (m_cellSize * 0.2f))
                {
                    enemy.health -= projectile.damage;
                    projectile.active = false;
                    if (enemy.health <= 0.0f)
                    {
                        enemy.alive = false;
                        m_credits += enemy.definition->bounty;
                    }
                }
            }
        }
    }

    void BattleState::CheckVictory()
    {
        if (m_lives <= 0)
        {
            m_result = BattleResult::Defeat;
            m_active = false;
            return;
        }
        bool enemiesRemaining = false;
        for (const auto& enemy : m_enemies)
        {
            if (enemy.alive)
            {
                enemiesRemaining = true;
                break;
            }
        }
        if (!enemiesRemaining && m_waveIndex >= m_waveSchedule.size())
        {
            m_result = BattleResult::Victory;
            m_active = false;
        }
    }

    void BattleState::RecomputePaths()
    {
        auto newPath = m_pathfinder.FindPath(m_spawnCell, m_goalCell);
        if (newPath)
        {
            m_basePath = *newPath;
            for (auto& enemy : m_enemies)
            {
                if (!enemy.alive)
                {
                    continue;
                }
                enemy.path.clear();
                for (const auto& cell : m_basePath)
                {
                    enemy.path.push_back(CellToWorld(cell.x, cell.y));
                }
                enemy.nextNode = std::min(enemy.nextNode, enemy.path.size() > 1 ? enemy.path.size() - 1 : size_t{0});
            }
        }
    }
}
