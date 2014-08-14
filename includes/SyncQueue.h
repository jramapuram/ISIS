/*
 * SyncQueue.h
 *
 *  Created on: Jul 8, 2013
 *      Author: jason
 */

#ifndef SYNCQUEUE_H_
#define SYNCQUEUE_H_

#include <boost/circular_buffer.hpp>
#include <boost/thread.hpp>

namespace isis{
template<typename T>
class SyncQueue {
public:
    SyncQueue(size_t sz)
        : m_queue(sz)
    {
		RequestToEnd = false;
		EnqueueData = true;
	}

	void Enqueue(const T& data) {
		boost::unique_lock<boost::mutex> lock(m_mutex);

		if (EnqueueData) {
            m_queue.push_front(data);
			m_cond.notify_one();
		}
	}

	bool TryDequeue(T& result) {
		boost::unique_lock<boost::mutex> lock(m_mutex);

		while (m_queue.empty() && (!RequestToEnd)) {
			m_cond.wait(lock);
		}

		if (RequestToEnd) {
			DoEndActions();
			return false;
		}

		result = m_queue.front();

		return true;
	}

	void StopQueue() {
		boost::unique_lock<boost::mutex> lock(m_mutex);
		RequestToEnd = true;
		m_cond.notify_one();
	}

	int Size() {
		boost::unique_lock<boost::mutex> lock(m_mutex);
		return m_queue.size();
	}

    void ClearQueue(){
        m_queue.clear();
    }

private:
	void DoEndActions() {
		EnqueueData = false;
        ClearQueue(); //new way of swap
	}

    boost::circular_buffer<T> m_queue;              // Use boost circ queue to store data
	boost::mutex m_mutex;               // The mutex to synchronise on
	boost::condition_variable m_cond;            // The condition to wait for

	bool RequestToEnd;
	bool EnqueueData;
};

}//namespace
#endif /* SYNCQUEUE_H_ */
