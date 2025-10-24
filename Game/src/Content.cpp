#include "Game/Content.h"

#include "Engine/StringUtil.h"

#include <stdexcept>
#include <utility>

namespace Game
{
    namespace
    {
        std::wstring AsWide(const nlohmann::json& value)
        {
            return Engine::ToWide(value.get<std::string>());
        }
    }

    Content::Content(std::wstring dataRoot)
        : m_store(std::make_unique<Engine::DataStore>(std::move(dataRoot)))
    {
    }

    bool Content::Initialize()
    {
        try
        {
            m_store->Register(L"towers.json", [this](const std::wstring&, const Engine::DataStore::Json& json) {
                LoadTowers(json);
            });
            m_store->Register(L"enemies.json", [this](const std::wstring&, const Engine::DataStore::Json& json) {
                LoadEnemies(json);
            });
            m_store->Register(L"waves.json", [this](const std::wstring&, const Engine::DataStore::Json& json) {
                LoadWaves(json);
            });
            m_store->Register(L"world.json", [this](const std::wstring&, const Engine::DataStore::Json& json) {
                LoadWorld(json);
            });
        }
        catch (const std::exception&)
        {
            return false;
        }
        return true;
    }

    void Content::Update()
    {
        m_store->PollChanges();
    }

    void Content::LoadTowers(const Engine::DataStore::Json& json)
    {
        m_campaign.towers.clear();
        for (const auto& entry : json)
        {
            TowerDefinition def;
            def.id = AsWide(entry.at("id"));
            def.name = AsWide(entry.at("name"));
            def.cost = entry.value("cost", 10);
            def.range = entry.value("range", 120.0f);
            def.damage = entry.value("damage", 10.0f);
            def.fireRate = entry.value("fireRate", 1.0f);
            def.projectileSpeed = entry.value("projectileSpeed", 200.0f);
            def.projectileColor = AsWide(entry.value("projectileColor", std::string("#FFFFFF")));
            m_campaign.towers[def.id] = std::move(def);
        }
    }

    void Content::LoadEnemies(const Engine::DataStore::Json& json)
    {
        m_campaign.enemies.clear();
        for (const auto& entry : json)
        {
            EnemyDefinition def;
            def.id = AsWide(entry.at("id"));
            def.name = AsWide(entry.at("name"));
            def.health = entry.value("health", 10.0f);
            def.speed = entry.value("speed", 40.0f);
            def.bounty = entry.value("bounty", 1);
            def.tint = AsWide(entry.value("tint", std::string("#FFFFFF")));
            m_campaign.enemies[def.id] = std::move(def);
        }
    }

    void Content::LoadWaves(const Engine::DataStore::Json& json)
    {
        m_campaign.wavesets.clear();
        for (const auto& entry : json)
        {
            WaveSet waveSet;
            waveSet.id = AsWide(entry.at("id"));
            for (const auto& waveEntry : entry.at("entries"))
            {
                WaveEntry w;
                w.enemyId = AsWide(waveEntry.at("enemy"));
                w.count = waveEntry.value("count", 1);
                w.interval = waveEntry.value("interval", 1.0);
                w.startDelay = waveEntry.value("start", 0.0);
                waveSet.entries.push_back(w);
            }
            m_campaign.wavesets[waveSet.id] = std::move(waveSet);
        }
    }

    void Content::LoadWorld(const Engine::DataStore::Json& json)
    {
        m_campaign.sectors.clear();
        for (const auto& entry : json)
        {
            SectorDescriptor sector;
            sector.id = AsWide(entry.at("id"));
            sector.name = AsWide(entry.at("name"));
            sector.gridX = entry.value("x", 0);
            sector.gridY = entry.value("y", 0);
            sector.difficulty = entry.value("difficulty", 1);
            sector.waveSetId = AsWide(entry.at("waves"));
            if (entry.contains("neighbors"))
            {
                for (const auto& n : entry.at("neighbors"))
                {
                    sector.neighbors.push_back(AsWide(n));
                }
            }
            m_campaign.sectors[sector.id] = std::move(sector);
        }
    }
}
