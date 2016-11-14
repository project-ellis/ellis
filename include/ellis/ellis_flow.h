/**
 * @file ellis_flow.h
 *
 * @brief Ellis Flow public C header.
 *
 * This is the C header for Ellis Flow, which implements Ellis encoders and
 * decoders.
 */

#pragma once
#ifndef ELLIS_FLOW_H_
#define ELLIS_FLOW_H_

#include <ellis/ellis.h>

/*
 *  ____                     _
 * |  _ \  ___  ___ ___   __| | ___ _ __ ___
 * | | | |/ _ \/ __/ _ \ / _` |/ _ \ '__/ __|
 * | |_| |  __/ (_| (_) | (_| |  __/ |  \__ \
 * |____/ \___|\___\___/ \__,_|\___|_|  |___/
 *
*/

/**
 * The callback table for an ellis_decoder.
 */
typedef struct ellis_decoder {
  /**
   * Hook to initialize an ellis_decoder.
   *
   * @param data Private, source-specific data for the ellis_decoder.
   */

  void (*init)(void *data);
  /**
   * Hook to deinitialize an ellis_decoder.
   *
   * @param data Private, source-specific data for the ellis_decoder.
   */
  void (*deinit)(void *data);

  /**
   * Hook to construct an Ellis node from raw data. The caller is responsible
   * for freeing the yielded ellis_node.
   *
   * TODO: zero-copy interface.
   */
  ellis_node *(*decode_mem)(const uint8_t *data, size_t length);
} ellis_decoder;

/**
 * Register an ellis_decoder, associating it with the given name.
 * @param[in] name The name with which to associate the ellis_decoder.
 * @param[in] src The ellis_decoder callback table.
 *
 * @return 0 if successful, -1 if not.
 */
int ellis_register_source(const char *name, const ellis_decoder *src);

/**
 * Lookup an ellis_decoder.
 *
 * @param[in] name The source name to lookup.
 *
 * @return A pointer to the ellis_decoder with which this name is associated.
 */
const ellis_decoder *ellis_get_source(const char *name);

/**
 * Construct an ellis_node from a source using the provided data.
 * TODO: zero-copy
 */
ellis_node *ellis_src_decode(
  const ellis_decoder *src,
  const uint8_t *data,
  size_t length);


/**
 *  _____                     _
 * | ____|_ __   ___ ___   __| | ___ _ __ ___
 * |  _| | '_ \ / __/ _ \ / _` |/ _ \ '__/ __|
 * | |___| | | | (_| (_) | (_| |  __/ |  \__ \
 * |_____|_| |_|\___\___/ \__,_|\___|_|  |___/
 *
 */

/**
 * The callback table for an ellis_encoder.
 */
typedef struct ellis_encoder {

  /**
   * Hook to initialize an ellis_encoder.
   *
   * @param data Private, sink-specific data for the ellis_encoder.
   */

  void (*init)(void *data);
  /**
   * Hook to deinitialize an ellis_encoder.
   *
   * @param data Private, sink-specific data for the ellis_encoder.
   */
  void (*deinit)(void *data);

  /**
   * Hook to construct an Ellis node from raw data. The caller is responsible
   * for freeing the yielded ellis_node.
   *
   * TODO: zero-copy interface.
   */
  uint8_t *(*encode_mem)(const struct ellis_node *node, size_t *length);
} ellis_encoder;

/**
 * Register an ellis_encoder, associating it with the given name.
 * @param[in] name The name with which to associate the ellis_encoder.
 * @param[in] src The ellis_encoder callback table.
 *
 * @return 0 if successful, -1 if not.
 */
int ellis_register_sink(const char *name, const ellis_encoder *sink);

/**
 * Lookup an ellis_encoder.
 *
 * @param[in] name The sink name to lookup.
 *
 * @return A pointer to the ellis_encoder with which this name is associated.
 */

const ellis_encoder *ellis_get_sink(const char *name)

/**
 * Encode an ellis_node into bytes using the given sink.
 *
 * @param[in] sink The sink to use for encoding.
 * TODO: zero-copy
 */
uint8_t *ellis_encoder_encode(
    const ellis_encoder *sink,
    ellis_node const *node,
    size_t *length);

#endif /* ELLIS_FLOW_H_ */
