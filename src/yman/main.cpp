#include "ymerge.hpp"
#include "handle_exceptions.hpp"
#include "log.hpp"

extern "C" int pacman_main(int argc, char *argv[]);

int main(int argc, const char *argv[]) {
  try {
    ymerge::argc = argc;
    ymerge::argv = argv;
    ymerge::as_sudo();
    return pacman_main(argc, (char **)argv);
  } catch (const ymerge::not_root_exception &err) { return err.result; } catch (const std::exception &err) {
    std::string msg = err.what();
    if (msg.length() > 0) ymerge::error("{}", err.what());
    return 1;
  }
}
