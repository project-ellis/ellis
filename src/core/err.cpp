#include <ellis/core/err.hpp>

#include <ellis_private/using.hpp>


namespace ellis {


err::err(
    err_code c,
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


err_code err::code() const
{
  return m_code;
}


std::string err::file() const
{
  return m_file;
}


int err::line() const
{
  return m_line;
}


std::string err::msg() const
{
  return what();
}


std::string err::summary() const
{
  return string(what()) + " at " + m_file + ":" + std::to_string(m_line);
}


}  /* namespace ellis */
