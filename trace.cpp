#include "trace.h"
#include "Isis.h"
#include "Config.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <cstdarg>
#include <sstream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/utility/empty_deleter.hpp>
#include <boost/filesystem.hpp>
//------------------------------------------------------------------------------

using namespace isis;
namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_file_backend > file_sink;
typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > ostream_sink;
typedef boost::log::trivial::severity_level priority;
typedef boost::log::sources::severity_channel_logger_mt<boost::log::trivial::severity_level, std::string> Logger;

trace::trace() {
  addStdSink();
  addFileSink();
  logging::add_common_attributes();
}

trace::~trace() {
  logging::core::get()->remove_all_sinks();
}


/**
 * Helper to format all streams appropriately
 */
template <class T>
void trace::setSinkFormat(T* SinkToFormat){
    (*SinkToFormat)->set_formatter(expr::stream
            <<  expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
            << " [" << expr::attr< std::string >("Channel")
            << "] " << logging::trivial::severity
            << ": " << expr::smessage
        );
}

/**
 * Helper to add an std stream based backend sink
 */
void trace::addStdSink()
{
    boost::shared_ptr< sinks::text_ostream_backend > StdBackend =
        boost::make_shared< sinks::text_ostream_backend >();
    boost::shared_ptr< std::ostream > StdStream(&std::clog, boost::empty_deleter()); //create cout sink
    boost::shared_ptr< ostream_sink > StdSink (new ostream_sink(StdBackend));
    StdSink->locked_backend()->add_stream(StdStream);
    StdBackend->auto_flush(true);
    setSinkFormat(&StdSink);
    StdSink->set_filter(boost::log::trivial::severity >= Isis::config->verbose);
    logging::core::get()->add_sink(StdSink);
}

/**
 * Helper to add a file based backend sink
 */
void trace::addFileSink()
{
    boost::shared_ptr<sinks::text_file_backend> file_backend =
        boost::make_shared<sinks::text_file_backend>(
            keywords::file_name = Isis::config->logPath + "/" + LOG_PREFIX + "-%Y-%m-%d_%H-%M-%S.%N.log",
            keywords::rotation_size = 5 * 1024 * 1024
        );

    boost::shared_ptr<file_sink> FileSink(new file_sink(file_backend));
    FileSink->locked_backend()->set_file_collector(sinks::file::make_collector(
        keywords::target = Isis::config->logPath,   /*< the target directory >*/
        keywords::max_size = 20 * 1024 * 1024,          /*< maximum total size of the stored files, in bytes >*/
        keywords::min_free_space = 100 * 1024 * 1024    /*< minimum free space on the drive, in bytes >*/
    ));

    FileSink->locked_backend()->scan_for_files();// sinks::file::scan_all for all in folder
    setSinkFormat(&FileSink);
    FileSink->set_filter(boost::log::trivial::severity >= Isis::config->verbose);
    file_backend->auto_flush(true);
    logging::core::get()->add_sink(FileSink);
}

//------------------------------------------------------------------------------
