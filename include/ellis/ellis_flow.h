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
 * The callback table for an EllisDecoder.
 */
typedef struct EllisDecoder {
  /**
   * Hook to initialize an EllisDecoder.
   *
   * @param data Private, source-specific data for the EllisDecoder.
   */

  void (*init)(void *data);
  /**
   * Hook to deinitialize an EllisDecoder.
   *
   * @param data Private, source-specific data for the EllisDecoder.
   */
  void (*deinit)(void *data);

  /**
   * Hook to construct an Ellis node from raw data. The caller is responsible
   * for freeing the yielded EllisNode.
   *
   * TODO: zero-copy interface.
   */
  EllisNode *(*decodeMem)(const uint8_t *data, size_t length);
} EllisDecoder;

/**
 * Register an EllisDecoder, associating it with the given name.
 * @param[in] name The name with which to associate the EllisDecoder.
 * @param[in] src The EllisDecoder callback table.
 *
 * @return 0 if successful, -1 if not.
 */
int EllisRegisterSource(const char *name, const EllisDecoder *src);

/**
 * Lookup an EllisDecoder.
 *
 * @param[in] name The source name to lookup.
 *
 * @return A pointer to the EllisDecoder with which this name is associated.
 */
EllisDecoder const *EllisGetSource(const char *name);

/**
 * Construct an EllisNode from a source using the provided data.
 * TODO: zero-copy
 */
EllisNode *EllisSrcDecode(
  EllisDecoder const *src,
  uint8_t const *data,
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
 * The callback table for an EllisEncoder.
 */
typedef struct EllisEncoder {

  /**
   * Hook to initialize an EllisEncoder.
   *
   * @param data Private, sink-specific data for the EllisEncoder.
   */

  void (*init)(void *data);
  /**
   * Hook to deinitialize an EllisEncoder.
   *
   * @param data Private, sink-specific data for the EllisEncoder.
   */
  void (*deinit)(void *data);

  /**
   * Hook to construct an Ellis node from raw data. The caller is responsible
   * for freeing the yielded EllisNode.
   *
   * TODO: zero-copy interface.
   */
  uint8_t *(*encodeMem)(struct EllisNode const *node, size_t *length);
} EllisEncoder;

/**
 * Register an EllisEncoder, associating it with the given name.
 * @param[in] name The name with which to associate the EllisEncoder.
 * @param[in] src The EllisEncoder callback table.
 *
 * @return 0 if successful, -1 if not.
 */
int EllisRegisterSink(const char *name, const EllisEncoder *sink);

/**
 * Lookup an EllisEncoder.
 *
 * @param[in] name The sink name to lookup.
 *
 * @return A pointer to the EllisEncoder with which this name is associated.
 */

EllisEncoder const *EllisGetSink(char const *name)

/**
 * Encode an EllisNode into bytes using the given sink.
 *
 * @param[in] sink The sink to use for encoding.
 * TODO: zero-copy
 */
uint8_t *EllisEncoderEncode(EllisEncoder const *sink, EllisNode const *node, size_t
  *length);

#endif /* ELLIS_FLOW_H_ */
