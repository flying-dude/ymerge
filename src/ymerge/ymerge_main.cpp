#include <exception>
#include <string>

#include "ymerge.hpp"
#include "log.hpp"

using namespace std;
using namespace ymerge;

int main(int argc, const char **argv) {
    try {
        main_throws(argc, argv);
    } catch (const exception &err) {
        string msg = err.what();
        if (msg.length() > 0) error("{}", err.what());
        return 1;
    }
}
