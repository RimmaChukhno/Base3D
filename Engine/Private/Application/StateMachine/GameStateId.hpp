#pragma once

#include <cstdint>

enum class GameStateId : int32_t
{
  Splash = 0,
  MainMenu = 1,
  Loading = 2,
  Gameplay = 3,
  Pause = 4,
  GameOver = 5,
  Victory = 6,
};

