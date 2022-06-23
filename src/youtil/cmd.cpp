#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include <cmd.hpp>

using namespace std;

namespace fly {

// https://stackoverflow.com/questions/5237482/how-do-i-execute-an-external-program-within-c-code-in-linux-with-arguments
int exec_prog(vector<string> argv_, cmd_options opt) {
	pid_t my_pid;
	if (0 == (my_pid = fork())) {
		// if requested, redirect stdout and stderr
		// https://stackoverflow.com/questions/29154056/redirect-stdout-to-a-file
		if (auto stdout_file = opt.stdout_file) freopen(stdout_file->c_str(), "a+", stdout);
		if (auto stderr_file = opt.stderr_file) freopen(stderr_file->c_str(), "a+", stderr);

		// if requested, change working directory
		// https://stackoverflow.com/questions/9547528/setting-a-custom-working-directory-for-a-process-started-with-exec
		if (auto working_dir = opt.working_dir) {
			int err = chdir(working_dir->c_str());
			if (err) {
				perror("error");
				cerr << "cannot change working into directory: \"" << working_dir->c_str() << "\""
				     << endl;
				exit(1);
			}
		}

		/* a trailing nullptr signals the end of argv to execvpe. the received list of args
		 * is not required to provide this. so we add it here before invoking execvpe. */
		unique_ptr<const char *[]> argv = make_unique<const char *[]>(argv_.size() + 1);
		for (int i = 0; i < argv_.size(); i++) argv[i] = argv_[i].c_str();
		argv[argv_.size()] = nullptr;

		/* note that unless exevpe fails, this program is also terminates when calling execvpe
		 * (that is why we need to fork before starting the external program). */
		execvpe(argv[0], (char **const)&argv[0], envp);
		perror("execvpe failed");
		exit(1);
	}

	int status;
	waitpid(my_pid, &status, 0);
	if (WIFEXITED(status) != 1 || WEXITSTATUS(status) != 0) return 1;

	return 0;
}

} // namespace fly
