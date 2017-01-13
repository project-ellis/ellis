#undef NDEBUG
#include <ellis/codec/delimited_text.hpp>
#include <ellis/codec/json.hpp>
#include <ellis/codec/msgpack.hpp>
#include <ellis/core/emigration.hpp>
#include <ellis/core/immigration.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/using.hpp>
#include <fstream>
#include <iostream>
#include <string.h>


// TODO: general function for utility command failure

string get_format(const char *filename) {
  const char *ext = strrchr(filename, '.');
  if (ext == nullptr || *(ext+1) == '\0') {
    fprintf(stderr, "Unable to determine format of file %s\n", filename);
    exit(1);
  }
  ext++;
  return ext;
}


unique_ptr<ellis::decoder> get_decoder(const string &format)
{
  unique_ptr<ellis::decoder> rv;
  if (format == "txt") {
    rv.reset(new ellis::delimited_text_decoder());
  }
  else if (format == "json") {
    rv.reset(new ellis::json_decoder());
  }
  else if (format == "msgpack") {
    rv.reset(new ellis::msgpack_decoder());
  }
  else {
    ELLIS_LOG(NOTI, "Unable to find decoder (%s)", format.c_str());
    /* But allow to return with undefined decoder. */
  }
  
  return rv;
}


unique_ptr<ellis::encoder> get_encoder(const string &format)
{
  unique_ptr<ellis::encoder> rv;
  if (format == "txt") {
    rv.reset(new ellis::delimited_text_encoder());
  }
  else if (format == "json") {
    rv.reset(new ellis::json_encoder());
  }
  else if (format == "msgpack") {
    rv.reset(new ellis::msgpack_encoder());
  }
  else {
    ELLIS_LOG(NOTI, "Unable to find encoder (%s)", format.c_str());
    /* But allow to return with empty result. */
  }
  return rv;
}


int main(int argc, char *argv[]) {
  using namespace ellis;

  if (argc != 3) {
    fprintf(stderr, "Syntax: %s in_filename out_filename\n", argv[0]);
    exit(1);
  }

  const char *in_filename = argv[1];
  const char *out_filename = argv[2];

  string in_fmt = get_format(in_filename);
  string out_fmt = get_format(out_filename);

  auto dec = get_decoder(in_fmt);
  if (!dec) {
    fprintf(stderr, "Failed looking up decoder (%s)\n", in_fmt.c_str());
    exit(1);
  }
  auto enc = get_encoder(out_fmt);
  if (!enc) {
    fprintf(stderr, "Failed looking up encoder (%s)\n", out_fmt.c_str());
    exit(1);
  }

  std::ifstream in_stream;
  std::ofstream out_stream;
  in_stream.open(in_filename);
  out_stream.open(out_filename);
  auto n = ellis::load_stream(in_stream, dec.get());
  ellis::dump_stream(n.get(), out_stream, enc.get());
  in_stream.close();
  out_stream.close();
  return 0;
}
