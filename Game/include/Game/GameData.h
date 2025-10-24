#pragma once

#include "Engine/MathTypes.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace Game
{
    struct TowerDefinition
    {
        std::wstring id;
        std::wstring name;
        int cost{0};
        float range{120.0f};
        float damage{10.0f};
        float fireRate{1.0f};
        float projectileSpeed{180.0f};
        std::wstring projectileColor;
    };

    struct EnemyDefinition
    {
        std::wstring id;
        std::wstring name;
        float health{10.0f};
        float speed{40.0f};
        int bounty{1};
        std::wstring tint;
    };

    struct WaveEntry
    {
        std::wstring enemyId;
        int count{1};
        double interval{1.0};
        double startDelay{0.0};
    };

    struct SectorDescriptor
    {
        std::wstring id;
        std::wstring name;
        int gridX{0};
        int gridY{0};
        int difficulty{1};
        std::wstring waveSetId;
        std::vector<std::wstring> neighbors;
    };

    struct WaveSet
    {
        std::wstring id;
        std::vector<WaveEntry> entries;
    };

    struct TowerSaveData
    {
        std::wstring towerId;
        int gridX{0};
        int gridY{0};
    };

    struct EnemySaveData
    {
        std::wstring enemyId;
        Engine::Vector2f position;
        float health{0.0f};
        size_t nextNode{0};
    };

    struct CampaignData
    {
        std::map<std::wstring, TowerDefinition> towers;
        std::map<std::wstring, EnemyDefinition> enemies;
        std::map<std::wstring, SectorDescriptor> sectors;
        std::map<std::wstring, WaveSet> wavesets;
    };
}
