/*
 * @file ellis/core/immigration.hpp
 *
 * @brief Ellis immigration methodology.
 */

#pragma once
#ifndef ELLIS_CORE_IMMIGRATION_HPP_
#define ELLIS_CORE_IMMIGRATION_HPP_

#include <ellis/core/err.hpp>
#include <ellis/core/decoder.hpp>
#include <ellis/core/sync_input_stream.hpp>
#include <memory>


namespace ellis {


/**
 * Synchronous (blocking) load of an ellis node from the given input stream,
 * using the given decoder.
 *
 * On success, returns the newly constructed node.
 *
 * On failure, throws an ellis::err.
 */
std::unique_ptr<node> load(
    sync_input_stream *in,
    decoder *deco);


/**
 * Alternate templated version(s) of load (see above) that take regular
 * references or rvalue references.
 *
 * Using rvalue references allows for construction of the codec and/or
 * stream during function argument evaluation, for on-the-fly instantiation of
 * a temporary stream and/or decoder, as a matter of convenience.
 *
 * Using regular references allows for enduring codec or stream objects whose
 * state is tracked across multiple calls.
 *
 * This flexibility is achieved via universal references (&&in and &&deco):
 * https://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers
 */
template<typename TSTREAM, typename TDECODER>
std::unique_ptr<node> load(TSTREAM &&in, TDECODER &&deco)
{
  return load((sync_input_stream*)&in, (decoder*)&deco);
}


/**
 * Synchronous (blocking) load from an open socket/fd.
 *
 * Similar behavior to load() above.
 */
std::unique_ptr<node> load_fd(
    int fd,
    decoder *deco);

/* See universal references above. */
template<typename TDECODER>
std::unique_ptr<node> load_fd(int fd, TDECODER &&deco)
{
  return load_fd(fd, (decoder*)&deco);
}


/**
 * Synchronous (blocking) load from a file.
 *
 * Similar behavior to load() above.
 */
std::unique_ptr<node> load_file(
    const char *filename,
    decoder *deco);

/* See universal references above. */
template<typename TDECODER>
std::unique_ptr<node> load_file(const char *filename, TDECODER &&deco)
{
  return load_file(filename, (decoder*)&deco);
}


/**
 * Synchronous (blocking) load from a memory buffer.
 *
 * Similar behavior to load() above.
 */
std::unique_ptr<node> load_mem(
    const void *buf,
    size_t len,
    decoder *deco);

/* See universal references above. */
template<typename TDECODER>
std::unique_ptr<node> load_mem(const void *buf, size_t len, TDECODER &&deco)
{
  return load_mem(buf, len, (decoder*)&deco);
}


/**
 * Synchronous (blocking) load from a stream.
 *
 * Similar behavior to load() above.
 */
std::unique_ptr<node> load_stream(
    std::istream &is,
    decoder *deco);

/* See universal references above. */
template<typename TDECODER>
std::unique_ptr<node> load_stream(std::istream &is, TDECODER &&deco)
{
  return load_stream(is, (decoder*)&deco);
}


/**
 * Synchronous load from a file, selecting decoder based on extension.
 *
 * Throws exceptions from load as well as filename extension lookup.
 */
std::unique_ptr<node> load_file_autodecode(
        const char *filename);


}  /* namespace ellis */

#endif  /* ELLIS_CORE_IMMIGRATION_HPP_ */
