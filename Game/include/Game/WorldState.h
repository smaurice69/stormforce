#pragma once

#include "GameData.h"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace Game
{
    enum class SectorOwnership
    {
        Player,
        Enemy,
        Contested
    };

    struct SectorState
    {
        SectorDescriptor descriptor;
        SectorOwnership ownership{SectorOwnership::Enemy};
        bool completed{false};
        int resistance{0};
    };

    struct BattleSnapshot
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

    class WorldState
    {
    public:
        void Initialize(const CampaignData& campaign);

        SectorState* GetSector(const std::wstring& id);
        const SectorState* GetSector(const std::wstring& id) const;
        std::vector<SectorState*> GetFrontlineSectors();

        void ResolveBattle(const std::wstring& sectorId, bool victory);
        std::wstring GetSuggestedSector() const;

        void SetResources(int value) { m_resources = value; }
        int GetResources() const { return m_resources; }
        void AddResources(int delta) { m_resources += delta; }

        void IncrementDay();
        int GetDay() const { return m_day; }

        void SetActiveBattle(std::optional<BattleSnapshot> snapshot);
        const std::optional<BattleSnapshot>& GetActiveBattle() const { return m_activeBattle; }

        void Save(const std::filesystem::path& path) const;
        bool Load(const std::filesystem::path& path, const CampaignData& campaign);

    private:
        std::unordered_map<std::wstring, SectorState> m_sectors;
        int m_resources{50};
        int m_day{1};
        std::optional<BattleSnapshot> m_activeBattle;
    };
}
