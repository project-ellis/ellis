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
 * See the comments in immigration.hpp regarding universal references.
 */
template<typename TSTREAM, typename TENCODER>
void dump(const node *nod, TSTREAM &&out, TENCODER &&enco)
{
  dump(nod, (sync_output_stream*)&out, (encoder*)&enco);
}


}  /* namespace ellis */

#endif  /* ELLIS_CORE_EMIGRATION_HPP_ */
