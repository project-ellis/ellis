#include <ellis_private/convenience/file.hpp>

#include <ellis/core/err.hpp>
#include <string.h>


std::string get_extension(const char *filename) {
  if (filename == nullptr) {
    THROW_ELLIS_ERR(INVALID_ARGS, "Can not get extension--null pointer");
  }
  const char *ext = strrchr(filename, '.');
  if (ext == nullptr || *(ext+1) == '\0') {
    THROW_ELLIS_ERR(INVALID_ARGS, "Can not get extension of file " << filename);
  }
  ext++;
  return ext;
}
