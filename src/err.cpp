#include <ellis/err.hpp>

#include <ellis/private/using.hpp>


namespace ellis {


err::err(
    int code,
    const std::string &file,
    int line,
    const std::string &msg) :
  runtime_error(msg),
  m_code(code),
  m_file(file),
  m_line(line)
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
