#include "COM.h"
#include <boost/thread.hpp>

using namespace isis;

COM::COM()
    :Application("COM")
{

}

COM::~COM() {
}

void COM::Run() {
    while (Isis::isRunning) {
		try {
			boost::posix_time::seconds workTime(5);
			boost::this_thread::sleep(workTime);
		} catch (std::exception& e) {
            log()->fatal("Exception: %s" ,e.what());
		}
	}
}
