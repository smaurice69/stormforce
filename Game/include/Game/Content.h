#pragma once

#include "GameData.h"

#include "Engine/DataStore.h"

#include <memory>

namespace Game
{
    class Content
    {
    public:
        explicit Content(std::wstring dataRoot);

        bool Initialize();
        void Update();

        const CampaignData& GetCampaign() const { return m_campaign; }

    private:
        std::unique_ptr<Engine::DataStore> m_store;
        CampaignData m_campaign;

        void LoadTowers(const Engine::DataStore::Json& json);
        void LoadEnemies(const Engine::DataStore::Json& json);
        void LoadWaves(const Engine::DataStore::Json& json);
        void LoadWorld(const Engine::DataStore::Json& json);
    };
}
