#include <create_temporary_file.hpp>
#include <fstream>

using namespace std;
using namespace std::filesystem;

namespace fly {

// https://stackoverflow.com/questions/3379956/how-to-create-a-temporary-directory-in-c
// https://github.com/cirosantilli/linux-kernel-module-cheat/blob/master/userland/cpp/temporary_directory.cpp
path create_temporary_file(string prefix, unsigned long long max_tries) {
  auto tmp_dir = temp_directory_path();
  random_device dev;
  mt19937 prng(dev());
  uniform_int_distribution<uint64_t> rand(0);
  filesystem::path file_path;

  for (unsigned long long i = 0; i < max_tries; i++) {
    stringstream ss;
    ss << std::hex << rand(prng);
    file_path = tmp_dir / (prefix + ss.str());
    if (exists (file_path))
      continue;

    // create empty file: https://stackoverflow.com/questions/45922907/how-to-create-an-empty-file-in-linux-using-c
    std::ofstream out(file_path);
    return file_path;
  }

  throw runtime_error("reached max tries when trying to create temporary file.");
}

shared_ptr<temporary_file> temporary_file::New(string prefix) {
  std::filesystem::path dir = create_temporary_file(prefix);
  return make_shared<temporary_file>(dir);
}

void temporary_file::Delete() {
  if (deleted) return;
  std::remove(path.c_str());
  deleted = true;
}

}  // namespace fly
