#ifndef SPEECH_H_
#define SPEECH_H_

#include <iostream>
#include <boost/date_time.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
//#include "trace.h"
#include "Application.h"
#include "SingletonApplication.h"

namespace isis{

class speech:public SingletonApplication<speech>,public Application {
private:

public:
    speech(); // Constructor
    ~speech(); // Destructor
	void Run();
};

}

#endif
