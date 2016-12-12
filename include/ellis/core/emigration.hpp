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
#include <ellis/core/stream_encoder.hpp>
#include <ellis/core/sync_output_stream.hpp>
#include <memory>


namespace ellis {


/** Synchronous (blocking) dump of an ellis node to the given output stream,
 * using the given encoder.
 *
 * On success, true.
 *
 * On failure, returns false, and sets err_ret.
 */
bool dump(
    const node *nod,
    sync_output_stream *out,
    stream_encoder *enco,
    std::unique_ptr<err> *err_ret);


/** Alternate versions that take references or rvalue references.
 *
 * This allows for the convenience of on-the-fly instantiation of particular
 * streams and codecs within the function call.
 *
 * TODO: reference search term regarding universal reference stuff.
 */
template<typename TSTREAM, typename TENCODER>
void dump(const node *nod, TSTREAM &&out, TENCODER &&enco)
{
  std::unique_ptr<err> e;
  dump(nod, &out, &enco, &e);
  if (e) {
    throw *e;
  }
}


}  /* namespace ellis */

#endif  /* ELLIS_CORE_EMIGRATION_HPP_ */
