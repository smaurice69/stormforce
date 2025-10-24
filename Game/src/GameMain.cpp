#include "Game/GameApp.h"

#include <windows.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    Game::GameApp app;
    if (!app.Initialize(L"Fieldrunners: Frontiers", 1280, 720))
    {
        return -1;
    }
    return app.Run();
}
