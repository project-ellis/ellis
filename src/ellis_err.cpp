#include <ellis/ellis_err.hpp>

#include <ellis/private/using.hpp>


namespace ellis {


ellis_err::ellis_err(
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


ellis_err::~ellis_err()
{
}


int ellis_err::code()
{
  return m_code;
}


std::string ellis_err::file()
{
  return m_file;
}


int ellis_err::line()
{
  return m_line;
}


std::string ellis_err::msg()
{
  return what();
}


std::string ellis_err::summary()
{
  return string(what()) + " at " + m_file + ":" + std::to_string(m_line);
}


}  /* namespace ellis */
