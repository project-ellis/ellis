/**
 * @file flow.h
 *
 * @brief Ellis Flow public C header.
 *
 * This is the C header for Ellis Flow, which implements Ellis encoders and
 * decoders.
 */

#pragma once
#ifndef ELLIS_FLOW_H_
#define ELLIS_FLOW_H_

#include <ellis/core/ellis.h>

#ifndef __cplusplus
extern "C" {
#endif


/*
 *  ____                     _
 * |  _ \  ___  ___ ___   __| | ___ _ __ ___
 * | | | |/ _ \/ __/ _ \ / _` |/ _ \ '__/ __|
 * | |_| |  __/ (_| (_) | (_| |  __/ |  \__ \
 * |____/ \___|\___\___/ \__,_|\___|_|  |___/
 *
*/

/**
 * The callback table for an decoder.
 */
typedef struct decoder {
  /**
   * Hook to initialize an decoder.
   *
   * @param data Private, source-specific data for the decoder.
   */

  void (*init)(void *data);
  /**
   * Hook to deinitialize an decoder.
   *
   * @param data Private, source-specific data for the decoder.
   */
  void (*deinit)(void *data);

  /**
   * Hook to construct an Ellis node from raw data. The caller is responsible
   * for freeing the yielded node.
   *
   * TODO: zero-copy interface.
   */
  node *(*decode_mem)(const uint8_t *data, size_t length);
} decoder;

/**
 * Register an decoder, associating it with the given name.
 * @param[in] name The name with which to associate the decoder.
 * @param[in] src The decoder callback table.
 *
 * @return 0 if successful, -1 if not.
 */
int register_source(const char *name, const decoder *src);

/**
 * Lookup an decoder.
 *
 * @param[in] name The source name to lookup.
 *
 * @return A pointer to the decoder with which this name is associated.
 */
const decoder *get_source(const char *name);

/**
 * Construct an node from a source using the provided data.
 * TODO: zero-copy
 */
node *src_decode(
  const decoder *src,
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
 * The callback table for an encoder.
 */
typedef struct encoder {

  /**
   * Hook to initialize an encoder.
   *
   * @param data Private, sink-specific data for the encoder.
   */

  void (*init)(void *data);
  /**
   * Hook to deinitialize an encoder.
   *
   * @param data Private, sink-specific data for the encoder.
   */
  void (*deinit)(void *data);

  /**
   * Hook to construct an Ellis node from raw data. The caller is responsible
   * for freeing the yielded node.
   *
   * TODO: zero-copy interface.
   */
  uint8_t *(*encode_mem)(const struct node *node, size_t *length);
} encoder;

/**
 * Register an encoder, associating it with the given name.
 * @param[in] name The name with which to associate the encoder.
 * @param[in] src The encoder callback table.
 *
 * @return 0 if successful, -1 if not.
 */
int register_sink(const char *name, const encoder *sink);

/**
 * Lookup an encoder.
 *
 * @param[in] name The sink name to lookup.
 *
 * @return A pointer to the encoder with which this name is associated.
 */

const encoder *get_sink(const char *name)

/**
 * Encode an node into bytes using the given sink.
 *
 * @param[in] sink The sink to use for encoding.
 * TODO: zero-copy
 */
uint8_t *encoder_encode(
    const encoder *sink,
    node const *node,
    size_t *length);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* ELLIS_FLOW_H_ */
