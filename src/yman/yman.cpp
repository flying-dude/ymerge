#include "handle_exceptions.hpp"
#include "log.hpp"
#include "ymerge.hpp"

extern "C" int pacman_main(int argc, char *argv[]);

const char *argv_[5] = {nullptr, "--sync", "--refresh", "--sysupgrade", nullptr};

int main(int argc, const char *argv[]) {
    try {
        ymerge::argc = argc;
        ymerge::argv = argv;
        ymerge::as_sudo();

        if (argc > 1) {
            return pacman_main(argc, (char **)argv);
        } else {
            // update system if no args are given
            argv_[0] = argv[0];
            pacman_main(4, (char **)argv_);
        }
    } catch (const ymerge::not_root_exception &err) { return err.result; } catch (const std::exception &err) {
        std::string msg = err.what();
        if (msg.length() > 0) ymerge::error("{}", err.what());
        return 1;
    }
}
