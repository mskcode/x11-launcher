#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using u8 = unsigned char;
using s8 = char;
using u32 = unsigned int;
using s32 = int;

void die(const std::string &message) {
  std::fprintf(stderr, "DIE: %s\n", message.c_str());
  std::exit(1);
}

namespace xwin {

int xlib_error_handler(Display *display, XErrorEvent *error_event) {
  std::fprintf(stderr,
               "XLIB ERROR: type=%d, serial=%ld, error_code=%d, "
               "request_code=%d, minor_code=%d\n",
               error_event->type, error_event->serial, error_event->error_code,
               error_event->request_code, error_event->minor_code);

  // according to documentation the return value is ignored
  return 0;
}

int xlib_io_error_handler(Display *display) {
  die("Fatal Xlib I/O error");
  // we're not expected to return from this function, if we do, the client
  // process exists
  return 0;
}

class KeyCode {
public:
  enum class Code : u32 {
    ESC = 0x09,
  };

  static auto is(u32 value, Code keycode) -> bool {
    return value == static_cast<u32>(keycode);
  }
};

enum class EventHandlerResult {
  OK,
  QUIT_APPLICATION,
};

using EventType = int;
using EventListener = std::function<EventHandlerResult(const XEvent &)>;
using EventListeners = std::vector<EventListener>;
using EventListenerMap = std::unordered_map<EventType, EventListeners>;

struct XWindowConfig {
  u32 width = 800;
  u32 height = 600;
  u32 pos_x = 0; // leave to zero so that WM honors the window splash type...
  u32 pos_y = 0; // ...and centers the window
  u32 border_width = 1;
};

class XWindow {
public:
  XWindow(const std::string &title, const XWindowConfig &config = {}) {
    display_ = ::XOpenDisplay(NULL);
    assert(display_);

    screen_ = DefaultScreen(display_);

    window_ = XCreateSimpleWindow(
        display_, RootWindow(display_, screen_), config.pos_x, config.pos_y,
        config.width, config.height, config.border_width,
        BlackPixel(display_, screen_), WhitePixel(display_, screen_));

    XStoreName(display_, window_, title.c_str());

    Atom windowTypeSplash =
        XInternAtom(display_, "_NET_WM_WINDOW_TYPE_SPLASH", false);

    Atom windowTypeAtom = XInternAtom(display_, "_NET_WM_WINDOW_TYPE", false);
    XChangeProperty(display_, window_, windowTypeAtom, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&windowTypeSplash, 1);

    XSelectInput(display_, window_,
                 ExposureMask | KeyPressMask | ButtonPressMask);

    XMapWindow(display_, window_);
  }

  ~XWindow() {
    if (display_ != nullptr) {
      XCloseDisplay(display_);
      display_ = nullptr;
    }
  }

  [[nodiscard]] auto handle() const -> Display * { return display_; }
  [[nodiscard]] auto window() const -> Window { return window_; }

  void register_event_listener(EventType type, EventListener listener) {
    event_listeners_[type].push_back(listener);
  }

  void start_event_loop() {
    XEvent event;
    bool closeRequested = false;
    while (true) {
      // the following call blocks until an event is emitted
      XNextEvent(display_, &event);

      if (!event_listeners_.contains(event.type)) {
        // no listeners for this event
        continue;
      }

      // go through event listeners for the specific event type
      for (auto &listener : event_listeners_[event.type]) {
        auto result = listener(event);
        switch (result) {
        case EventHandlerResult::QUIT_APPLICATION:
          closeRequested = true;
          break;
        default:
          break;
        }
      }

      if (closeRequested) {
        break;
      }
    }
  }

private:
  Display *display_{nullptr};
  int screen_{0};
  Window window_{0};

  EventListenerMap event_listeners_;
};

} // namespace xwin

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  XSetErrorHandler(xwin::xlib_error_handler);
  XSetIOErrorHandler(xwin::xlib_io_error_handler);

  xwin::XWindow window{"X11 Launcher"};

  window.register_event_listener(KeyPress, [](auto &event) {
    return xwin::KeyCode::is(event.xkey.keycode, xwin::KeyCode::Code::ESC)
               ? xwin::EventHandlerResult::QUIT_APPLICATION
               : xwin::EventHandlerResult::OK;
  });

  window.start_event_loop();
  return 0;
}
