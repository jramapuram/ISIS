#ifndef ISIS_H
#define ISIS_H

#include "trace.h"
#include "Config.h"

#include <boost/system/error_code.hpp>
#include <boost/thread.hpp>

//------------------------------------------------------------------------------

namespace isis {


//------------------------------------------------------------------------------

class Isis {
public:
    Isis(int argc, char **argv);
    ~Isis();

    void initialize();

    static bool                 isRunning;
    static trace*               logging;
    static Config*              config;
    static LogModule*           main;
    static boost::thread_group  IsisThreads;

private:
    int    CreateThreads();

    std::unique_ptr<Config>    pConfig_;
    std::unique_ptr<trace>     pLogging_;
    std::unique_ptr<LogModule> pMain_;


    int    argc_;
    char** argv_;

};

//------------------------------------------------------------------------------

} // namespace isis

//------------------------------------------------------------------------------

#endif
