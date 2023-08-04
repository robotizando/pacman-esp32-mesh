
//#define CHARACTER_PACMAN
//#define CHARACTER_GHOST

#ifdef CHARACTER_PACMAN
  AsyncWebServer server(80);
#endif

const uint32_t pacman = 1699379629;
const uint32_t ghosts[4] = {1699262633,3672423717,2808773001,3209670157};

Scheduler     userScheduler; // to control your personal task

#define AUDIO_START_PIN 15