#pragma once

#include "Camera.h"
#include "Enemy.h"
#include "GameData.h"
#include "Pathfinder.h"
#include "Projectile.h"
#include "Tower.h"

#include "Engine/Color.h"
#include "Engine/Renderer2D.h"

#include <optional>

namespace Game
{
    enum class BattleResult
    {
        None,
        Victory,
        Defeat
    };

    struct BattleConfig
    {
        const SectorDescriptor* sector{nullptr};
        int startingCredits{40};
        int startingLives{20};
    };

    struct BattleSave
    {
        std::wstring sectorId;
        int credits{0};
        int lives{0};
        double time{0.0};
        std::vector<TowerSaveData> towers;
        std::vector<EnemySaveData> enemies;
        size_t waveIndex{0};
        int spawnedInWave{0};
        double entryTimer{0.0};
    };

    class BattleState
    {
    public:
        BattleState(const CampaignData& campaign, int gridWidth, int gridHeight, float cellSize);

        void Start(const BattleConfig& config);
        void Load(const BattleConfig& config, const BattleSave& save);
        void Update(double dt);
        void Render(Engine::Renderer2D& renderer);

        void HandleMouse(const Engine::Vector2f& worldPos, bool leftDown, bool rightDown);
        void HandleKey(uint8_t key, bool pressed);

        BattleResult GetResult() const { return m_result; }
        bool IsActive() const { return m_active; }

        const BattleSave CreateSnapshot() const;
        int GetCredits() const { return m_credits; }
        int GetLives() const { return m_lives; }
        double GetElapsed() const { return m_time; }

    private:
        const CampaignData& m_campaign;
        Pathfinder m_pathfinder;
        Camera m_camera;
        int m_gridWidth;
        int m_gridHeight;
        float m_cellSize;

        BattleResult m_result{BattleResult::None};
        bool m_active{false};

        std::vector<TowerInstance> m_towers;
        std::vector<EnemyInstance> m_enemies;
        std::vector<Projectile> m_projectiles;

        std::vector<WaveEntry> m_waveSchedule;
        size_t m_waveIndex{0};
        double m_time{0.0};
        int m_credits{0};
        int m_lives{0};
        bool m_fastForward{false};

        std::optional<GridCoord> m_hoverCell;
        const TowerDefinition* m_selectedTower{nullptr};
        std::wstring m_selectedTowerId;
        GridCoord m_spawnCell{0, 0};
        GridCoord m_goalCell{0, 0};
        std::wstring m_currentSectorId;
        bool m_lastLeftDown{false};
        bool m_lastRightDown{false};
        std::vector<GridCoord> m_basePath;
        int m_spawnedInWave{0};
        double m_entryTimer{0.0};

        Engine::Color ParseColor(const std::wstring& hex) const;
        Engine::Vector2f CellToWorld(int x, int y) const;
        std::optional<GridCoord> WorldToCell(const Engine::Vector2f& world) const;

        void SpawnEnemy(const WaveEntry& entry);
        void UpdateEnemies(double dt);
        void UpdateTowers(double dt);
        void UpdateProjectiles(double dt);
        void CheckVictory();
        void RecomputePaths();
    };
}
