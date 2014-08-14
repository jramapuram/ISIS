#ifndef INCLUDED_TRACE_H
#define INCLUDED_TRACE_H

#ifndef INCLUDED_STREAM
#include <sstream>
#define INCLUDED_STREAM
#endif

#ifndef INCLUDED_BOOST_FORMAT_HPP
#include <boost/format.hpp>
#define INCLUDED_BOOST_FORMAT_HPP
#endif

#ifndef INCLUDED_BOOST_LOG_SOURCES_SEVERITY_CHANNEL_LOGGER_HPP
#include <boost/log/sources/severity_channel_logger.hpp>
#define INCLUDED_BOOST_LOG_SOURCES_SEVERITY_CHANNEL_LOGGER_HPP
#endif

#ifndef INCLUDED_BOOST_LOG_TRIVIAL_HPP
#include <boost/log/trivial.hpp>
#define INCLUDED_BOOST_LOG_TRIVIAL_HPP
#endif

#ifndef INCLUDED_BOOST_CIRCULAR_BUFFER_HPP
#include <boost/circular_buffer.hpp>
#define INCLUDED_BOOST_CIRCULAR_BUFFER_HPP
#endif

//------------------------------------------------------------------------------

namespace isis {

//------------------------------------------------------------------------------

struct LogModule {
  typedef boost::log::trivial::severity_level priority;
  typedef boost::log::sources::severity_channel_logger_mt<
    boost::log::trivial::severity_level, std::string
  > Logger;
  template<typename...Ts>
  void fatal(std::string fmt, Ts... args) {
    std::stringstream ss;
    formatMessage(ss, fmt, args...);
    BOOST_LOG_SEV(logger_, priority::fatal) << ss.str();
  }

  template<typename...Ts>
  void error(std::string fmt, Ts... args) {
    std::stringstream ss;
    formatMessage(ss, fmt, args...);
    BOOST_LOG_SEV(logger_, priority::error) << ss.str();
  }

  template<typename...Ts>
  void warn(std::string fmt, Ts... args) {
    std::stringstream ss;
    formatMessage(ss, fmt, args...);
    BOOST_LOG_SEV(logger_, priority::warning) << ss.str();
  }

  template<typename...Ts>
  void info(std::string fmt, Ts... args) {
    std::stringstream ss;
    formatMessage(ss, fmt, args...);
    BOOST_LOG_SEV(logger_, priority::info) << ss.str();
  }

  template<typename...Ts>
  void debug(std::string fmt, Ts... args) {
    std::stringstream ss;
    formatMessage(ss, fmt, args...);
    BOOST_LOG_SEV(logger_, priority::debug) << ss.str();
  }

  template<typename...Ts>
  void trace(std::string fmt, Ts... args) {
    std::stringstream ss;
    formatMessage(ss, fmt, args...);
    BOOST_LOG_SEV(logger_, priority::trace) << ss.str();
  }

  template<typename ...Ts>
  void array(Ts... args){
    std::stringstream ss;
    for(auto t : {args...}){
      for(auto i : t){
        ss<<i<<" ";
      }
    }
    BOOST_LOG_SEV(logger_,priority::trace)<<ss.str();
  }

//  template<typename T>
//  void matrix(T arg){

//  }

  template<typename ...Ts>
  void matrix(Ts... args){
      BOOST_LOG_SEV(logger_, priority::trace)<<matrix(args...);
  }

private:
  void formatMessage(std::stringstream& strm, boost::format& message) {
    strm << message.str();
  }

  template<typename T, typename...Ts>
  void formatMessage(std::stringstream& strm,
                     boost::format& message,
                     T arg, Ts... args) {
    message % arg;
    formatMessage(strm, message, args...);
  }

  template<typename...Ts>
  void formatMessage(std::stringstream& strm,
                     std::string fmt,
                     Ts... args) {
    boost::format message(fmt);
    formatMessage(strm, message, args...);
  }

  Logger logger_;
  friend struct trace;
};

struct trace {
  trace();
 ~trace();

  LogModule* getModule(const std::string& name) {
    auto module = new LogModule();
    module->logger_.channel(name);
    return module;
  }

private:
  const std::string LOG_PREFIX = "isis";

  template <class T> void setSinkFormat(T* SinkToFormat);
  void addStdSink();
  void addFileSink();
};

//------------------------------------------------------------------------------

} // namespace isis

//------------------------------------------------------------------------------

#endif
