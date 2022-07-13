#include <create_temporary_directory.hpp>

using namespace std;
using namespace std::filesystem;

namespace fly {

// https://stackoverflow.com/questions/3379956/how-to-create-a-temporary-directory-in-c
// https://github.com/cirosantilli/linux-kernel-module-cheat/blob/master/userland/cpp/temporary_directory.cpp
path create_temporary_directory(string prefix, unsigned long long max_tries) {
  auto tmp_dir = temp_directory_path();
  random_device dev;
  mt19937 prng(dev());
  uniform_int_distribution<uint64_t> rand(0);
  filesystem::path path;

  for (unsigned long long i = 0; i < max_tries; i++) {
    stringstream ss;
    ss << std::hex << rand(prng);
    path = tmp_dir / (prefix + ss.str());
    // true if the directory was created.
    if (create_directory(path)) { return path; }
  }

  throw runtime_error("reached max tries when trying to create temporary directory.");
}

shared_ptr<temporary_directory> temporary_directory::New(string prefix) {
  std::filesystem::path dir = create_temporary_directory(prefix);
  return make_shared<temporary_directory>(dir);
}

void temporary_directory::Delete() {
  if (deleted) return;

  remove_all(this->path);
  deleted = true;
}

}  // namespace fly
