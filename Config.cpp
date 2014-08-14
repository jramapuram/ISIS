#include "Config.h"
#include "Isis.h"

#include <fstream> //for settings write & read
#include <boost/exception/all.hpp>

//------------------------------------------------------------------------------

using namespace isis;

//------------------------------------------------------------------------------

Config::Config(int argc, char**argv)
{
  specifyOptions();

  // Load from file first, then override w/ command-line

  po::store(po::parse_command_line(argc, argv, desc_), vm_);
  po::notify(vm_);

  validateParams(vm_, desc_);
}

Config::~Config() {
}

void Config::validateParams(const po::variables_map& vm,
                       const po::options_description& desc) {

  // throw exceptions for invalid config

  if(vm.count("help")) {
    std::cerr << std::endl << desc_ << std::endl;
    exit(0); // HACK
  }
}

void Config::specifyOptions(){
  desc_.add_options()
    ("help,h","Produce this help message")
    ("verbose,v",po::value<size_t>(&verbose)->default_value(0),
     "Verbosity level for logging")
    ("log-path,L",po::value<std::string>(&logPath)->default_value(DEFAULT_LOG_DIR),
     "Path to log file")
    ("num-cams,N",po::value<size_t>(&numCams)->default_value(DEFAULT_NUM_CAMS),
      "Number of cameras to look for")
    ("single-cam,C",po::value<int>(&singleCam)->default_value(DEFAULT_SINGLE_CAM),
      "Number of cameras to look for")
    ("height",po::value<size_t>(&height)->default_value(DEFAULT_HEIGHT),
        "Try to set camera image height")
    ("width",po::value<size_t>(&width)->default_value(DEFAULT_WIDTH),
      "Try to set camera image width")
    ("save-interval,s",po::value<size_t>(&saveInterval)->default_value(DEFAULT_SAVE_INTERVAL),
      "Interval to save weight matrix")
    ("load-weights,w",po::value<std::string>(&weightPath)->default_value(DEFAULT_WEIGHT_PATH),
      "Path to our serialized weight & bias matrices. Use boost format filename structure.")
  ;
}

//------------------------------------------------------------------------------
