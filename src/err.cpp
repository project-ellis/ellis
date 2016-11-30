#include <ellis/err.hpp>

#include <ellis/private/using.hpp>


namespace ellis {


err::err(
    int c,
    const std::string &f,
    int l,
    const std::string &m) :
  runtime_error(m),
  m_code(c),
  m_file(f),
  m_line(l)
{
}


err::~err()
{
}


int err::code()
{
  return m_code;
}


std::string err::file()
{
  return m_file;
}


int err::line()
{
  return m_line;
}


std::string err::msg()
{
  return what();
}


std::string err::summary()
{
  return string(what()) + " at " + m_file + ":" + std::to_string(m_line);
}


}  /* namespace ellis */
