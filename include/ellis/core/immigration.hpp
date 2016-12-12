/*
 * @file ellis/core/immigration.hpp
 *
 * @brief Ellis immigration methodology.
 */

#pragma once
#ifndef ELLIS_CORE_IMMIGRATION_HPP_
#define ELLIS_CORE_IMMIGRATION_HPP_

#include <ellis/core/err.hpp>
#include <ellis/core/stream_decoder.hpp>
#include <ellis/core/sync_input_stream.hpp>
#include <memory>


namespace ellis {


/** Synchronous (blocking) load of an ellis node from the given input stream,
 * using the given decoder.
 *
 * On success, returns the newly constructed node.
 *
 * On failure, returns nullptr, and sets err_ret.
 *
 * @pre err_ret is not NULL.
 */
std::unique_ptr<node> load(
    sync_input_stream *in,
    stream_decoder *deco,
    std::unique_ptr<err> *err_ret);


/** Alternate versions that take references or rvalue references.
 *
 * This allows for the convenience of on-the-fly instantiation of particular
 * streams and codecs within the function call.
 *
 * TODO: reference search term regarding universal reference stuff.
 */
template<typename TSTREAM, typename TDECODER>
std::unique_ptr<node> load(TSTREAM &&in, TDECODER &&deco)
{
  std::unique_ptr<err> e;
  std::unique_ptr<node> rv = load(&in, &deco, &e);
  if (e) {
    throw *e;
  }
  return rv;
}


/** Synchronous (blocking) load from an open socket/fd. */
std::unique_ptr<node> load_fd(
    int fd,
    stream_decoder *deco,
    std::unique_ptr<err> *err_ret);


/** Synchronous (blocking) load from a file. */
std::unique_ptr<node> load_file(
    const char *filename,
    stream_decoder *deco,
    std::unique_ptr<err> *err_ret);


/** Synchronous (blocking) load from a memory buffer. */
std::unique_ptr<node> load_mem(
    byte *buf,
    size_t len,
    stream_decoder *deco,
    std::unique_ptr<err> *err_ret);


/** Synchronous (blocking) load from a stream. */
std::unique_ptr<node> load_stream(
    std::istream &is,
    stream_decoder *deco,
    std::unique_ptr<err> *err_ret);


}  /* namespace ellis */

#endif  /* ELLIS_CORE_IMMIGRATION_HPP_ */
