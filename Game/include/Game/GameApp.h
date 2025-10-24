#pragma once

#include "BattleState.h"
#include "Content.h"
#include "WorldState.h"

#include "Engine/Application.h"

#include <filesystem>

namespace Game
{
    enum class Screen
    {
        WorldMap,
        Battle,
        Debrief
    };

    class GameApp : public Engine::Application
    {
    public:
        GameApp();
        ~GameApp() override;

    protected:
        bool OnInitialize() override;
        void OnUpdate(double deltaSeconds) override;
        void OnRender(Engine::Renderer2D& renderer) override;
        void OnShutdown() override;
        void OnKeyEvent(uint8_t key, bool pressed) override;
        void OnMouseButton(int button, bool pressed) override;

    private:
        Content m_content;
        WorldState m_world;
        BattleState m_battle;
        Screen m_screen{Screen::WorldMap};
        std::wstring m_selectedSector;
        bool m_leftMouseDown{false};
        bool m_rightMouseDown{false};

        std::filesystem::path m_savePath;

        void RenderWorld(Engine::Renderer2D& renderer);
        void RenderDebrief(Engine::Renderer2D& renderer);
        void StartBattle(const std::wstring& sectorId);
        void UpdateWorldScreen(double dt);
        void UpdateBattle(double dt);
        void HandleSave();
        void HandleLoad();
    };
}
