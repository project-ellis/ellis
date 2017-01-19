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
