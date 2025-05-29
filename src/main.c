#include "game.h"

int main()
{
    InitGame();

    while (IsGameRunning()) {
        UpdateGame();
        DrawGame();
    }

    CloseGame();
    return 0;
}
