#ifndef CONFIG_H
#define CONFIG_H

#ifndef INCLUDED_BOOST_PROGRAM_OPTIONS_HPP
#include <boost/program_options.hpp>
#define INCLUDED_BOOST_PROGRAM_OPTIONS_HPP
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif


namespace po = boost::program_options;
//------------------------------------------------------------------------------

namespace isis {

//------------------------------------------------------------------------------

class Config {
public:
  Config(int argc,char**argv);
 ~Config();

  void specifyOptions();

  void validateParams(const po::variables_map&,
                      const po::options_description&);

  // Our parameters
  const size_t      DEFAULT_SAVE_INTERVAL       = 100; //xx [TODO: figure a good val for this]
  const size_t      DEFAULT_NUM_CAMS            = 1;
  const int         DEFAULT_SINGLE_CAM          = -1;
  const size_t      DEFAULT_HEIGHT              = 480;
  const size_t      DEFAULT_WIDTH               = 640;

  const std::string DEFAULT_WEIGHT_PATH         = "weights_%s.dat";
  const std::string DEFAULT_LOG_DIR             = "logs";


  std::string              logPath
                          , weightPath;

  size_t                   saveInterval
                          ,verbose
                          ,numCams
                          ,height
                          ,width;

  int                     singleCam;


private:
    po::options_description    desc_;
    po::variables_map          vm_;
};

//------------------------------------------------------------------------------

} // namespace sherlock

//------------------------------------------------------------------------------

#endif // CONFIG_H
