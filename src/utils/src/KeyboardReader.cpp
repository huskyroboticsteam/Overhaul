#include "../include/utils/KeyboardReader.h"

namespace utils {

#ifndef _WIN32
KeyboardReader::KeyboardReader() : kfd(0) {
  // get the console in raw mode
  tcgetattr(kfd, &cooked);
  struct termios raw;
  memcpy(&raw, &cooked, sizeof(struct termios));
  raw.c_lflag &=~ (ICANON | ECHO);
  // Setting a new line, then end of file
  raw.c_cc[VEOL] = 1;
  raw.c_cc[VEOF] = 2;
  tcsetattr(kfd, TCSANOW, &raw);
}
#else
KeyboardReader::KeyboardReader() {}
#endif

void KeyboardReader::readOne(char *c) {
#ifndef _WIN32
    int rc = read(kfd, c, 1);
    if (rc < 0) {
      throw std::runtime_error("read failed");
    }
#else
    for(;;) {
      HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
      INPUT_RECORD buffer;
      DWORD events;
      PeekConsoleInput(handle, &buffer, 1, &events);
      if(events > 0) {
        ReadConsoleInput(handle, &buffer, 1, &events);
        if (buffer.Event.KeyEvent.wVirtualKeyCode == VK_LEFT) {
          *c = KEYCODE_LEFT;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == VK_UP) {
          *c = KEYCODE_UP;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == VK_RIGHT) {
          *c = KEYCODE_RIGHT;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == VK_DOWN) {
          *c = KEYCODE_DOWN;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x42) {
          *c = KEYCODE_B;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x43) {
          *c = KEYCODE_C;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x44) {
          *c = KEYCODE_D;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x45) {
          *c = KEYCODE_E;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x46) {
          *c = KEYCODE_F;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x47) {
          *c = KEYCODE_G;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x51) {
          *c = KEYCODE_Q;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x52) {
          *c = KEYCODE_R;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x54) {
          *c = KEYCODE_T;
        } else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x56) {
          *c = KEYCODE_V;
        }
      }
    }
#endif
}

#ifndef _WIN32
void KeyboardReader::shutdown() {
  tcsetattr(kfd, TCSANOW, &cooked);
}
#endif

} // namespace utils