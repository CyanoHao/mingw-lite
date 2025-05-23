#include <filesystem>
#include <iostream>
#include <set>
#include <string_view>

namespace fs = std::filesystem;

using std::cerr;
using std::cout;
using std::error_code;
using std::string_view;

const fs::path dir = fs::u8path("test--filesystem--‘dir’");

string_view content[2] = {"test\n", "测试\n"};

bool test_create_directory() {
  error_code ec;
  fs::create_directory(dir, ec);
  if (ec) {
    cerr << "[test_create_directory] ";
    cerr << "fs::create_directory failed: " << ec.message() << '\n';
    return false;
  }
  return true;
}

bool do_write() {
  FILE *fp = fopen("test/test.txt", "wb");
  if (!fp) {
    cerr << "[do_write] ";
    cerr << "fopen failed" << '\n';
    return false;
  }
  for (const auto &line : content)
    fwrite(line.data(), 1, line.size(), fp);
  fclose(fp);
  return true;
}

bool test_copy_file() {
  error_code ec;
  fs::copy_file("test/test.txt", L"test/测试.txt",
                fs::copy_options::overwrite_existing, ec);
  if (ec) {
    cerr << "[test_copy_file] ";
    cerr << "fs::copy_file failed: " << ec.message() << '\n';
    return false;
  }
  return true;
}

bool test_resize_file() {
  error_code ec;
  fs::resize_file("test/test.txt", content[0].size(), ec);
  if (ec) {
    cerr << "[test_copy_file] ";
    cerr << "fs::resize_file failed: " << ec.message() << '\n';
    return false;
  }
  FILE *fp = fopen("test/test.txt", "rb");
  if (!fp) {
    cerr << "[test_copy_file] ";
    cerr << "fopen failed" << '\n';
    return false;
  }
  fseek(fp, 0, SEEK_END);
  size_t len = ftell(fp);
  if (len != content[0].size()) {
    cerr << "[test_copy_file] ";
    cerr << "length mismatch: " << len << " != " << content[0].size() << '\n';
    return false;
  }
  fseek(fp, 0, SEEK_SET);
  char buf[1024];
  fread(buf, 1, len, fp);
  fclose(fp);
  if (content[0] != string_view(buf, len)) {
    cerr << "[test_copy_file] ";
    cerr << "read content mismatch" << '\n';
    return false;
  }
  return true;
}

bool test_copy() {
  error_code ec;
  fs::copy("test", L"测试",
           fs::copy_options::overwrite_existing | fs::copy_options::recursive,
           ec);
  if (ec) {
    cerr << "[test_copy] ";
    cerr << "fs::copy failed: " << ec.message() << '\n';
    return false;
  }
  return true;
}

bool test_directory_iterator() {
  error_code ec;
  auto cwd = fs::current_path(ec);
  if (ec) {
    cerr << "[test_directory_iterator] ";
    cerr << "fs::current_path failed: " << ec.message() << '\n';
    return false;
  }

  auto dir = fs::recursive_directory_iterator(".", ec);
  if (ec) {
    cerr << "[test_directory_iterator] ";
    cerr << "fs::recursive_directory_iterator failed: " << ec.message() << '\n';
    return false;
  }

  std::set<fs::path> files;
  for (const auto &entry : dir) {
    auto rel = fs::relative(entry.path(), cwd, ec);
    if (ec) {
      cerr << "[test_directory_iterator] ";
      cerr << "fs::relative failed: " << ec.message() << '\n';
      return false;
    }
    files.insert(rel);
  }
  for (const auto &file : files)
    cout << file << '\n';
  return true;
}

bool test_equivalent() {
  error_code ec;
  bool eq;

  eq = fs::equivalent("test/test.txt", "test\\test.txt", ec);
  if (ec) {
    cerr << "[test_equivalent] ";
    cerr << "fs::equivalent failed: " << ec.message() << '\n';
    return false;
  }
  if (!eq) {
    cerr << "[test_equivalent] ";
    cerr << "fs::equivalent returned false" << '\n';
    return false;
  }

  eq = fs::equivalent("test/test.txt", "test/测试.txt", ec);
  if (ec) {
    cerr << "[test_equivalent] ";
    cerr << "fs::equivalent failed: " << ec.message() << '\n';
    return false;
  }
  if (eq) {
    cerr << "[test_equivalent] ";
    cerr << "fs::equivalent returned true" << '\n';
    return false;
  }
  return true;
}

bool test_rename() {
  error_code ec;
  fs::rename("test/test.txt", "test/test2.txt", ec);
  if (ec) {
    cerr << "[test_rename] ";
    cerr << "fs::rename failed: " << ec.message() << '\n';
    return false;
  }
  bool exist = fs::exists("test/test.txt", ec);
  if (ec) {
    cerr << "[test_rename] ";
    cerr << "fs::exists failed: " << ec.message() << '\n';
    return false;
  }
  if (exist) {
    cerr << "[test_rename] ";
    cerr << "rename failed" << '\n';
    return false;
  }

  // check size
  size_t size = fs::file_size("test/test2.txt", ec);
  if (ec) {
    cerr << "[test_rename] ";
    cerr << "fs::file_size failed: " << ec.message() << '\n';
    return false;
  }
  if (size == size_t(-1)) {
    cerr << "[test_rename] ";
    cerr << "fs::file_size returned -1" << '\n';
    return false;
  }
  if (size != content[0].size()) {
    cerr << "[test_rename] ";
    cerr << "size mismatch: " << size << " != " << content[0].size() << '\n';
  }

  // check content
  FILE *fp = fopen("test/test2.txt", "rb");
  if (!fp) {
    cerr << "[test_rename] ";
    cerr << "fopen failed" << '\n';
    return false;
  }
  char buf[1024];
  size_t len = fread(buf, 1, size, fp);
  if (len != size) {
    cerr << "[test_rename] ";
    cerr << "fread failed" << '\n';
    return false;
  }
  fclose(fp);
  if (content[0] != string_view(buf, len)) {
    cerr << "[test_rename] ";
    cerr << "read content mismatch" << '\n';
    return false;
  }
  return true;
}

bool test_remove() {
  error_code ec;
  fs::remove(L"test/测试.txt", ec);
  if (ec) {
    cerr << "[test_remove] ";
    cerr << "fs::remove failed: " << ec.message() << '\n';
    return false;
  }
  bool exist = fs::exists(L"test/测试.txt", ec);
  if (ec) {
    cerr << "[test_remove] ";
    cerr << "fs::exists failed: " << ec.message() << '\n';
    return false;
  }
  if (exist) {
    cerr << "[test_remove] ";
    cerr << "failed" << '\n';
    return false;
  }

  fs::remove_all("test", ec);
  if (ec) {
    cerr << "[test_remove] ";
    cerr << "fs::remove_all failed: " << ec.message() << '\n';
    return false;
  }
  exist = fs::exists("test", ec);
  if (ec) {
    cerr << "[test_remove] ";
    cerr << "fs::exists failed: " << ec.message() << '\n';
    return false;
  }

  return true;
}

int main() {
  if (!test_create_directory())
    return 1;
  if (!do_write())
    return 1;
  if (!test_copy_file())
    return 1;
  if (!test_resize_file())
    return 1;
  if (!test_copy())
    return 1;
  if (!test_directory_iterator())
    return 1;
  if (!test_equivalent())
    return 1;
  if (!test_rename())
    return 1;
  if (!test_remove())
    return 1;
  return 0;
}
