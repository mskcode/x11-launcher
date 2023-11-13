#include <X11/Xlib.h>
#include <cstdio>
#include <cstdlib>
#include <string>

void die(const std::string &message) {
  std::perror(message.c_str());
  std::exit(1);
}

int main(int argc, char **argv) {
  Display *d = XOpenDisplay(NULL);
  if (d == nullptr) {
    die("Cannot open display");
  }

  int s = DefaultScreen(d);
  Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 100, 100, 1,
                                 BlackPixel(d, s), WhitePixel(d, s));

  XSelectInput(d, w, ExposureMask | KeyPressMask);
  XMapWindow(d, w);

  XEvent e;
  bool closeRequested = false;
  while (true) {
    XNextEvent(d, &e);
    switch (e.type) {
    case Expose:
      break;
    case KeyPress:
      closeRequested = true;
      break;
    default:
      break;
    }

    if (closeRequested) {
      break;
    }
  }

  XCloseDisplay(d);

  return 0;
}
