#include "AI.h"
#include "Utility.h"

using namespace Utility;
using namespace isis;

AI::AI(unsigned int index, SyncQueue<cv::Mat>& Q)
    : Application("AI_"+std::to_string(index))
    , index_(index)
    , imgQueue (Q)
{
    SetEvents();
}

AI::~AI() {

}

void AI::PullImgQueue() {
    imgQueue.TryDequeue(latestImg_);
}

void AI::SetEvents(){
    addSignal("SVMTrainingComplete");
    addSignal("StackedDAETrainingComplete");
}

void AI::Run() {
    while (Isis::isRunning) {
		try {
			PullImgQueue();
		} catch (std::exception& e) {
            log()->error("Exception in Run: %s",e.what());
		}
	}
}
