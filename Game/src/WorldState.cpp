#include "Game/WorldState.h"

#include "Engine/StringUtil.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

namespace Game
{
    void WorldState::Initialize(const CampaignData& campaign)
    {
        m_sectors.clear();
        for (const auto& [id, descriptor] : campaign.sectors)
        {
            SectorState state;
            state.descriptor = descriptor;
            state.ownership = descriptor.gridX == 0 ? SectorOwnership::Player : SectorOwnership::Enemy;
            state.completed = descriptor.gridX == 0;
            state.resistance = descriptor.difficulty;
            m_sectors[id] = std::move(state);
        }
        m_resources = 60;
        m_day = 1;
        m_activeBattle.reset();
    }

    SectorState* WorldState::GetSector(const std::wstring& id)
    {
        auto iter = m_sectors.find(id);
        if (iter == m_sectors.end())
        {
            return nullptr;
        }
        return &iter->second;
    }

    const SectorState* WorldState::GetSector(const std::wstring& id) const
    {
        auto iter = m_sectors.find(id);
        if (iter == m_sectors.end())
        {
            return nullptr;
        }
        return &iter->second;
    }

    std::vector<SectorState*> WorldState::GetFrontlineSectors()
    {
        std::vector<SectorState*> frontline;
        for (auto& [id, sector] : m_sectors)
        {
            if (sector.ownership == SectorOwnership::Enemy)
            {
                for (const auto& neighborId : sector.descriptor.neighbors)
                {
                    auto neighborIter = m_sectors.find(neighborId);
                    if (neighborIter != m_sectors.end() && neighborIter->second.ownership == SectorOwnership::Player)
                    {
                        frontline.push_back(&sector);
                        break;
                    }
                }
            }
        }
        return frontline;
    }

    void WorldState::ResolveBattle(const std::wstring& sectorId, bool victory)
    {
        auto* sector = GetSector(sectorId);
        if (!sector)
        {
            return;
        }

        if (victory)
        {
            sector->ownership = SectorOwnership::Player;
            sector->completed = true;
            m_resources += 10 + sector->descriptor.difficulty * 5;
        }
        else
        {
            sector->ownership = SectorOwnership::Enemy;
            if (sector->resistance < sector->descriptor.difficulty + 2)
            {
                ++sector->resistance;
            }
        }

        m_activeBattle.reset();
    }

    std::wstring WorldState::GetSuggestedSector() const
    {
        for (const auto& [id, sector] : m_sectors)
        {
            if (sector.ownership == SectorOwnership::Enemy)
            {
                for (const auto& neighborId : sector.descriptor.neighbors)
                {
                    auto neighborIter = m_sectors.find(neighborId);
                    if (neighborIter != m_sectors.end() && neighborIter->second.ownership == SectorOwnership::Player)
                    {
                        return id;
                    }
                }
            }
        }
        return m_sectors.empty() ? L"" : m_sectors.begin()->first;
    }

    void WorldState::IncrementDay()
    {
        ++m_day;
        for (auto& [id, sector] : m_sectors)
        {
            if (sector.ownership == SectorOwnership::Enemy && sector.resistance > 0)
            {
                ++sector.resistance;
            }
        }
    }

    void WorldState::SetActiveBattle(std::optional<BattleSnapshot> snapshot)
    {
        m_activeBattle = std::move(snapshot);
    }

    void WorldState::Save(const std::filesystem::path& path) const
    {
        nlohmann::json json;
        json["resources"] = m_resources;
        json["day"] = m_day;
        nlohmann::json sectorArray = nlohmann::json::array();
        for (const auto& [id, sector] : m_sectors)
        {
            nlohmann::json obj;
            obj["id"] = Engine::ToNarrow(id);
            obj["ownership"] = static_cast<int>(sector.ownership);
            obj["completed"] = sector.completed;
            obj["resistance"] = sector.resistance;
            sectorArray.push_back(obj);
        }
        json["sectors"] = sectorArray;
        if (m_activeBattle)
        {
            nlohmann::json battle;
            battle["sector"] = Engine::ToNarrow(m_activeBattle->sectorId);
            battle["credits"] = m_activeBattle->credits;
            battle["lives"] = m_activeBattle->lives;
            battle["time"] = m_activeBattle->time;
            battle["waveIndex"] = m_activeBattle->waveIndex;
            battle["spawnedInWave"] = m_activeBattle->spawnedInWave;
            battle["entryTimer"] = m_activeBattle->entryTimer;
            nlohmann::json towers = nlohmann::json::array();
            for (const auto& tower : m_activeBattle->towers)
            {
                nlohmann::json t;
                t["id"] = Engine::ToNarrow(tower.towerId);
                t["x"] = tower.gridX;
                t["y"] = tower.gridY;
                towers.push_back(t);
            }
            battle["towers"] = towers;
            nlohmann::json enemies = nlohmann::json::array();
            for (const auto& enemy : m_activeBattle->enemies)
            {
                nlohmann::json e;
                e["id"] = Engine::ToNarrow(enemy.enemyId);
                e["x"] = enemy.position.x;
                e["y"] = enemy.position.y;
                e["health"] = enemy.health;
                e["node"] = enemy.nextNode;
                enemies.push_back(e);
            }
            battle["enemies"] = enemies;
            json["battle"] = battle;
        }
        std::ofstream file(path);
        file << json.dump(1);
    }

    bool WorldState::Load(const std::filesystem::path& path, const CampaignData& campaign)
    {
        if (!std::filesystem::exists(path))
        {
            return false;
        }
        std::ifstream file(path);
        if (!file)
        {
            return false;
        }
        nlohmann::json json;
        file >> json;
        Initialize(campaign);
        m_resources = json.value("resources", m_resources);
        m_day = json.value("day", m_day);
        if (json.contains("sectors"))
        {
            for (const auto& obj : json["sectors"])
            {
                auto id = Engine::ToWide(obj.value("id", std::string("")));
                auto iter = m_sectors.find(id);
                if (iter != m_sectors.end())
                {
                    iter->second.ownership = static_cast<SectorOwnership>(obj.value("ownership", 1));
                    iter->second.completed = obj.value("completed", false);
                    iter->second.resistance = obj.value("resistance", iter->second.resistance);
                }
            }
        }
        if (json.contains("battle"))
        {
            BattleSnapshot snapshot;
            snapshot.sectorId = Engine::ToWide(json["battle"].value("sector", std::string("")));
            snapshot.credits = json["battle"].value("credits", 0);
            snapshot.lives = json["battle"].value("lives", 0);
            snapshot.time = json["battle"].value("time", 0.0);
            snapshot.waveIndex = json["battle"].value("waveIndex", 0);
            snapshot.spawnedInWave = json["battle"].value("spawnedInWave", 0);
            snapshot.entryTimer = json["battle"].value("entryTimer", 0.0);
            if (json["battle"].contains("towers"))
            {
                for (const auto& tower : json["battle"]["towers"])
                {
                    TowerSaveData data;
                    data.towerId = Engine::ToWide(tower.value("id", std::string("")));
                    data.gridX = tower.value("x", 0);
                    data.gridY = tower.value("y", 0);
                    snapshot.towers.push_back(data);
                }
            }
            if (json["battle"].contains("enemies"))
            {
                for (const auto& enemy : json["battle"]["enemies"])
                {
                    EnemySaveData e;
                    e.enemyId = Engine::ToWide(enemy.value("id", std::string("")));
                    e.position = {enemy.value("x", 0.0f), enemy.value("y", 0.0f)};
                    e.health = enemy.value("health", 0.0f);
                    e.nextNode = enemy.value("node", 0);
                    snapshot.enemies.push_back(e);
                }
            }
            m_activeBattle = snapshot;
        }
        else
        {
            m_activeBattle.reset();
        }
        return true;
    }
}
