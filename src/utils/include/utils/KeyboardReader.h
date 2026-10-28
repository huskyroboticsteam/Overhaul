#pragma once

#include <signal.h>
#include <stdio.h>
#ifndef _WIN32
# include <cstring>
# include <stdexcept>
# include <termios.h>
# include <unistd.h>
#else
# include <windows.h>
#endif

#define KEYCODE_RIGHT 0x43
#define KEYCODE_LEFT 0x44
#define KEYCODE_UP 0x41
#define KEYCODE_DOWN 0x42
#define KEYCODE_B 0x62
#define KEYCODE_C 0x63
#define KEYCODE_D 0x64
#define KEYCODE_E 0x65
#define KEYCODE_F 0x66
#define KEYCODE_G 0x67
#define KEYCODE_Q 0x71
#define KEYCODE_R 0x72
#define KEYCODE_T 0x74
#define KEYCODE_V 0x76

namespace utils {

class KeyboardReader {
  public:
    KeyboardReader();

    void readOne(char *c);

#ifndef _WIN32
    void shutdown();
#endif

  private:
#ifndef _WIN32
    int kfd;
    struct termios cooked;
#endif
};

} // namespace utils