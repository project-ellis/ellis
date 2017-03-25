/*
 * @file ellis/core/emigration.hpp
 *
 * @brief Ellis emigration methodology.
 */

#pragma once
#ifndef ELLIS_CORE_EMIGRATION_HPP_
#define ELLIS_CORE_EMIGRATION_HPP_

#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/encoder.hpp>
#include <ellis/core/sync_output_stream.hpp>
#include <memory>


namespace ellis {


/**
 * Synchronous (blocking) dump of an ellis node to the given output stream,
 * using the given encoder.
 *
 * On failure, throws an ellis::err.
 */
void dump(
    const node *nod,
    sync_output_stream *out,
    encoder *enco);


/**
 * Alternate versions that take references or rvalue references.
 *
 * Similar semantics and exceptions as dump() above.
 *
 * See the comments in immigration.hpp regarding universal references.
 */
template<typename TSTREAM, typename TENCODER>
void dump(const node *nod, TSTREAM &&out, TENCODER &&enco)
{
  dump(nod, (sync_output_stream*)&out, (encoder*)&enco);
}


/**
 * Synchronous (blocking) dump to an open socket/fd.
 *
 * Similar semantics and exceptions as dump() above.
 */
void dump_fd(
    const node *nod,
    int fd,
    encoder *enco);

/* See universal references above. */
template<typename TENCODER>
void dump_fd(const node *nod, int fd, TENCODER &&enco)
{
  dump_fd(nod, fd, (encoder*)&enco);
}


/**
 * Synchronous (blocking) dump to a file.
 *
 * Similar semantics and exceptions as dump() above.
 */
void dump_file(
    const node *nod,
    const char *filename,
    encoder *enco);

/* See universal references above. */
template<typename TENCODER>
void dump_file(const node *nod, const char *filename, TENCODER &&enco)
{
  dump_file(nod, filename, (encoder*)&enco);
}


/**
 * Dump to a memory buffer.
 *
 * Similar semantics and exceptions as dump() above.
 */
void dump_mem(
    const node *nod,
    void *buf,
    size_t len,
    encoder *enco);

/* See universal references above. */
template<typename TENCODER>
void dump_mem(const node *nod, void *buf, size_t len, TENCODER &&enco)
{
  dump_mem(nod, buf, len, (encoder*)&enco);
}


/**
 * Synchronous (blocking) dump to a c++ stream.
 *
 * Similar semantics and exceptions as dump() above.
 */
void dump_stream(
    const node *nod,
    std::ostream &os,
    encoder *enco);

/* See universal references above. */
template<typename TENCODER>
void dump_stream(const node *nod, std::ostream &os, TENCODER &&enco)
{
  dump_stream(nod, os, (encoder*)&enco);
}


/**
 * Synchronous dump to a file, selecting encoder based on extension.
 *
 * Throws exceptions from dump as well as filename extension lookup.
 */
void dump_file_autoencode(
    const node *nod,
    const char *filename);


}  /* namespace ellis */

#endif  /* ELLIS_CORE_EMIGRATION_HPP_ */
