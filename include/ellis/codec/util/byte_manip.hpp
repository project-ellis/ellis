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


/* TODO: doc */
template <typename T>
static inline T from_be(const byte *buf);

template <>
int8_t from_be(const byte *buf)
{
  return *buf;
}

template <>
int16_t from_be(const byte *buf)
{
  return be16toh(*((uint16_t *)buf));
}

template <>
int32_t from_be(const byte *buf)
{
  return be32toh(*((uint32_t *)buf));
}

template <>
int64_t from_be(const byte *buf)
{
  return be64toh(*((uint64_t *)buf));
}

template <>
uint8_t from_be(const byte *buf)
{
  return *buf;
}

template <>
uint16_t from_be(const byte *buf)
{
  return be16toh(*((uint16_t *)buf));
}

template <>
uint32_t from_be(const byte *buf)
{
  return be32toh(*((uint32_t *)buf));
}

template <>
float from_be(const byte *buf)
{
  return union_cast<uint32_t, float>(be32toh(*((uint32_t *)buf)));
}

template <>
double from_be(const byte *buf)
{
  return union_cast<uint64_t, double>(be64toh(*((uint64_t *)buf)));
}
