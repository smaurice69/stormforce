#include "Game/GameApp.h"

#include "Engine/StringUtil.h"

#include <algorithm>
#include <filesystem>

namespace Game
{
    GameApp::GameApp()
        : m_content((std::filesystem::current_path() / L"Data").wstring())
        , m_battle(m_content.GetCampaign(), 24, 14, 48.0f)
    {
        m_savePath = std::filesystem::current_path() / L"saves" / L"save.json";
    }

    GameApp::~GameApp() = default;

    bool GameApp::OnInitialize()
    {
        std::filesystem::create_directories(m_savePath.parent_path());
        if (!m_content.Initialize())
        {
            return false;
        }

        m_world.Initialize(m_content.GetCampaign());
        m_selectedSector = m_world.GetSuggestedSector();
        HandleLoad();
        return true;
    }

    void GameApp::OnUpdate(double deltaSeconds)
    {
        m_content.Update();
        switch (m_screen)
        {
        case Screen::WorldMap:
            UpdateWorldScreen(deltaSeconds);
            break;
        case Screen::Battle:
            UpdateBattle(deltaSeconds);
            break;
        case Screen::Debrief:
        default:
            break;
        }
    }

    void GameApp::OnRender(Engine::Renderer2D& renderer)
    {
        switch (m_screen)
        {
        case Screen::WorldMap:
            RenderWorld(renderer);
            break;
        case Screen::Battle:
            m_battle.Render(renderer);
            break;
        case Screen::Debrief:
            RenderDebrief(renderer);
            break;
        }
    }

    void GameApp::OnShutdown()
    {
        HandleSave();
    }

    void GameApp::OnKeyEvent(uint8_t key, bool pressed)
    {
        if (!pressed)
        {
            return;
        }

        if (key == VK_F5)
        {
            HandleSave();
            return;
        }
        if (key == VK_F9)
        {
            HandleLoad();
            return;
        }

        if (m_screen == Screen::WorldMap)
        {
            if (key == VK_RETURN)
            {
                StartBattle(m_selectedSector);
            }
            else if (key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN)
            {
                auto frontline = m_world.GetFrontlineSectors();
                if (frontline.empty())
                {
                    return;
                }
                auto iter = std::find_if(frontline.begin(), frontline.end(), [this](const SectorState* state) {
                    return state->descriptor.id == m_selectedSector;
                });
                if (iter == frontline.end())
                {
                    m_selectedSector = frontline.front()->descriptor.id;
                    return;
                }
                size_t index = std::distance(frontline.begin(), iter);
                if (key == VK_LEFT || key == VK_UP)
                {
                    index = (index + frontline.size() - 1) % frontline.size();
                }
                else
                {
                    index = (index + 1) % frontline.size();
                }
                m_selectedSector = frontline[index]->descriptor.id;
            }
        }
        else if (m_screen == Screen::Battle)
        {
            m_battle.HandleKey(key, pressed);
            if (key == VK_RETURN && m_battle.GetResult() != BattleResult::None)
            {
                m_screen = Screen::Debrief;
            }
        }
        else if (m_screen == Screen::Debrief)
        {
            if (key == VK_RETURN)
            {
                m_screen = Screen::WorldMap;
                m_selectedSector = m_world.GetSuggestedSector();
            }
        }
    }

    void GameApp::OnMouseButton(int button, bool pressed)
    {
        if (button == 0)
        {
            m_leftMouseDown = pressed;
        }
        else if (button == 1)
        {
            m_rightMouseDown = pressed;
        }
    }

    void GameApp::RenderWorld(Engine::Renderer2D& renderer)
    {
        renderer.BeginFrame({20, 24, 32});
        const float cellSize = 84.0f;
        const float margin = 60.0f;

        renderer.DrawText(L"Day " + std::to_wstring(m_world.GetDay()) + L"  Resources: " +
                              std::to_wstring(m_world.GetResources()),
                          {20.0f, 20.0f}, {220, 220, 220});
        renderer.DrawText(L"Arrow keys select. Enter to deploy. F5 save, F9 load.", {20.0f, 44.0f}, {180, 200, 220});

        for (const auto& [id, sector] : m_content.GetCampaign().sectors)
        {
            float x = margin + sector.gridX * (cellSize + 20.0f);
            float y = margin + sector.gridY * (cellSize + 20.0f);
            Engine::Rectf rect{x, y, x + cellSize, y + cellSize};
            Engine::Color color;
            auto* state = m_world.GetSector(id);
            if (!state)
            {
                continue;
            }
            switch (state->ownership)
            {
            case SectorOwnership::Player:
                color = {70, 160, 110};
                break;
            case SectorOwnership::Enemy:
                color = {170, 70, 70};
                break;
            case SectorOwnership::Contested:
            default:
                color = {200, 170, 60};
                break;
            }
            renderer.DrawFilledRect(rect, color);
            if (id == m_selectedSector)
            {
                renderer.DrawRectOutline(rect, {240, 240, 255}, 3);
            }
            renderer.DrawText(sector.name, {x + 6.0f, y + 6.0f}, {20, 20, 20});
            renderer.DrawText(L"Diff " + std::to_wstring(sector.difficulty), {x + 6.0f, y + cellSize - 22.0f},
                              {30, 30, 30});
        }

        renderer.EndFrame();
    }

    void GameApp::RenderDebrief(Engine::Renderer2D& renderer)
    {
        renderer.BeginFrame({12, 12, 20});
        renderer.DrawText(L"Campaign Update", {40.0f, 40.0f}, {230, 230, 240}, 28);
        renderer.DrawText(L"Resources: " + std::to_wstring(m_world.GetResources()), {40.0f, 80.0f}, {220, 220, 220});
        renderer.DrawText(L"Press Enter to continue", {40.0f, 120.0f}, {200, 200, 220});
        renderer.EndFrame();
    }

    void GameApp::StartBattle(const std::wstring& sectorId)
    {
        if (sectorId.empty())
        {
            return;
        }
        auto sectorIter = m_content.GetCampaign().sectors.find(sectorId);
        if (sectorIter == m_content.GetCampaign().sectors.end())
        {
            return;
        }
        BattleConfig config;
        config.sector = &sectorIter->second;
        config.startingCredits = std::max(40, m_world.GetResources());
        config.startingLives = 20;
        m_battle.Start(config);

        BattleSnapshot snapshot;
        snapshot.sectorId = sectorId;
        snapshot.credits = config.startingCredits;
        snapshot.lives = config.startingLives;
        snapshot.time = 0.0;
        snapshot.waveIndex = 0;
        snapshot.spawnedInWave = 0;
        snapshot.entryTimer = 0.0;
        m_world.SetActiveBattle(snapshot);

        m_screen = Screen::Battle;
    }

    void GameApp::UpdateWorldScreen(double)
    {
        if (m_selectedSector.empty())
        {
            m_selectedSector = m_world.GetSuggestedSector();
        }
    }

    void GameApp::UpdateBattle(double dt)
    {
        Engine::Vector2f mousePos{static_cast<float>(GetInputState().GetMouseX()),
                                  static_cast<float>(GetInputState().GetMouseY())};
        m_battle.HandleMouse(mousePos, m_leftMouseDown, m_rightMouseDown);
        m_battle.Update(dt);

        if (m_battle.IsActive())
        {
            BattleSnapshot snapshot;
            auto save = m_battle.CreateSnapshot();
            snapshot.sectorId = save.sectorId;
            snapshot.credits = save.credits;
            snapshot.lives = save.lives;
            snapshot.time = save.time;
            snapshot.towers = save.towers;
            snapshot.enemies = save.enemies;
            snapshot.waveIndex = save.waveIndex;
            snapshot.spawnedInWave = save.spawnedInWave;
            snapshot.entryTimer = save.entryTimer;
            m_world.SetActiveBattle(snapshot);
        }
        else if (m_battle.GetResult() != BattleResult::None)
        {
            bool victory = m_battle.GetResult() == BattleResult::Victory;
            m_world.ResolveBattle(m_selectedSector, victory);
            m_world.SetActiveBattle(std::nullopt);
            if (victory)
            {
                m_world.AddResources(20);
            }
            m_world.IncrementDay();
            m_screen = Screen::Debrief;
        }
    }

    void GameApp::HandleSave()
    {
        if (m_screen != Screen::Battle)
        {
            m_world.SetActiveBattle(std::nullopt);
        }
        std::filesystem::create_directories(m_savePath.parent_path());
        m_world.Save(m_savePath);
    }

    void GameApp::HandleLoad()
    {
        if (m_world.Load(m_savePath, m_content.GetCampaign()))
        {
            if (auto snapshot = m_world.GetActiveBattle())
            {
                auto sectorIter = m_content.GetCampaign().sectors.find(snapshot->sectorId);
                if (sectorIter != m_content.GetCampaign().sectors.end())
                {
                    BattleConfig config;
                    config.sector = &sectorIter->second;
                    config.startingCredits = snapshot->credits;
                    config.startingLives = snapshot->lives;
                    BattleSave save;
                    save.sectorId = snapshot->sectorId;
                    save.credits = snapshot->credits;
                    save.lives = snapshot->lives;
                    save.time = snapshot->time;
                    save.towers = snapshot->towers;
                    save.enemies = snapshot->enemies;
                    save.waveIndex = snapshot->waveIndex;
                    save.spawnedInWave = snapshot->spawnedInWave;
                    save.entryTimer = snapshot->entryTimer;
                    m_battle.Load(config, save);
                    m_screen = Screen::Battle;
                    m_selectedSector = snapshot->sectorId;
                    return;
                }
            }
        }
        m_screen = Screen::WorldMap;
        m_selectedSector = m_world.GetSuggestedSector();
    }
}
