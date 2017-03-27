/*
 * Copyright (c) 2016 Surround.IO Corporation. All Rights Reserved.
 * Copyright (c) 2017 Xevo Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


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
