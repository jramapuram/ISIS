#include "speech.h"

using namespace isis;

speech::speech()
    :Application("speech")
{

} // Constructor

speech::~speech() {
} // Destructor

void speech::Run() {
    while (Isis::isRunning) {
		try {
			boost::posix_time::seconds workTime(5);
			boost::this_thread::sleep(workTime);
		} catch (std::exception& e) {
            log()->fatal("Exception: %s", e.what());
        }
	}
}
