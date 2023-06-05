// https://cplusplus.com/forum/beginner/212605/

/**
 * Need to provide a SIGINT handler, so that ymerge cleanup code
 * will run after abort with CTRL+C on the command line.
 */

#include <unistd.h>

#include <csignal>
#include <iostream>

volatile sig_atomic_t stopFlag = 0;

/* an empty handler already does the trick.
 * if we simply handle SIGINT, then ymerge cleanup code
 * will run after CTRL+C is pressed by the user. */
static void handler(int signum) {
  // std::cout << "SIGINT :: getpid() " << getpid() << " :: signum " << signum << std::endl;
  stopFlag = signum;
}

extern int handle_exceptions(int argc, const char **argv);
int main(int argc, const char **argv) {
  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;  // Restart functions if interrupted by handler.

  sigaction(SIGINT, &sa, NULL);

  handle_exceptions(argc, argv);

  // std::cout << "exiting :: getpid() " << getpid() << std::endl;

  // reset handler to default and exit by SIGINT
  // are the following lines correct?
  if (stopFlag) {
    sa.sa_handler = SIG_DFL;
    sigaction(SIGINT, &sa, NULL);
    raise(SIGINT);
  }
}
