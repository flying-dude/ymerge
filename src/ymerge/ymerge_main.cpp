#include <exception>
#include <string>

#include "log.hpp"
#include "ymerge.hpp"

int main(int argc, const char **argv) {
  try {
    ymerge::main_throws(argc, argv);
  } catch (const std::exception &err) {
    std::string msg = err.what();
    if (msg.length() > 0) ymerge::error("{}", err.what());
    return 1;
  }
}
