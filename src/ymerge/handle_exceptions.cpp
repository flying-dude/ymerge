#include "handle_exceptions.hpp"

#include <exception>
#include <string>

#include "cmd.hpp"
#include "log.hpp"
#include "ymerge.hpp"

int handle_exceptions(int argc, const char **argv) {
  try {
    ymerge::main_throws(argc, argv);
  } catch (const ymerge::not_root_exception &err) { return err.result; } catch (const std::exception &err) {
    std::string msg = err.what();
    if (msg.length() > 0) ymerge::error("{}", err.what());
    return 1;
  }
}
