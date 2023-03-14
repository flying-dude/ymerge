#include "nspawn.hpp"

using namespace std;

namespace ymerge {

extern void init_nspawn() {
  // TODO check if systemd is actually running, before invoking systemd-nspawn
  // we might be running inside a container or chroot, where systemd isn't started


  todo("implement init_nspawn()");
}

}
