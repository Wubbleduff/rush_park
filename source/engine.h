
#pragma once

enum EngineMode
{
  MAIN_MENU,

  LOCAL_GAME,
  CLIENT_GAME,
  SERVER_GAME,
};


void start_engine();

void change_engine_mode(EngineMode mode);

void stop_engine();
