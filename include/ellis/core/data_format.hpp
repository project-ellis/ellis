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
 */


/*
 * @file ellis/core/data_format.hpp
 *
 * @brief Ellis data_format C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_DATA_FORMAT_HPP_
#define ELLIS_CORE_DATA_FORMAT_HPP_

#include <functional>
#include <memory>
#include <string>
//#include <ellis/core/defs.hpp>


namespace ellis {


// forward declaration
class decoder;
class encoder;
class node;
// TODO(jmc) class schema;


/**
 * A data format represents a particular configuration of codecs and schemas,
 * such as JSON codecs used used in the context of stock market data.
 *
 * Each data format is registered against a unique name, but can also be
 * associated with a filename extension that may be ambiguous.
 *
 * There are functions to return encoder and decoder objects, so that these
 * can be reused multiple times, possibly with different schemas.
 */
struct data_format {
    std::string m_unique_name;
    std::string m_extension;
    std::string m_description;
    std::function<std::unique_ptr<decoder>
      (void)> m_make_decoder;
    std::function<std::unique_ptr<encoder>
      (void)> m_make_encoder;
    // TODO(jmc) std::unique_ptr<schema> m_schema;
    //std::function<std::unique_ptr<node>
    //  (/* const schema *, */
    //  uint64_t amount,
    //  uint64_t seed)> m_randgen;

    /* Simple constructor, no copying allowed. */
    data_format(
        std::string unique_name,
        std::string extension,
        std::string description,
        std::function<std::unique_ptr<decoder>
          (void)> make_decoder,
        std::function<std::unique_ptr<encoder>
          (void)> make_encoder) :
      m_unique_name(unique_name),
      m_extension(extension),
      m_description(description),
      m_make_decoder(make_decoder),
      m_make_encoder(make_encoder)
    {
    }
    data_format(const data_format &) = delete;
    data_format & operator=(const data_format &) = delete;
};


}  /* namespace ellis */

#endif  /* ELLIS_CORE_DATA_FORMAT_HPP_ */
