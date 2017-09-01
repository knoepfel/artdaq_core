#ifndef ARTDAQ_CORE_UTILITIES_TRACELOCK
#define ARTDAQ_CORE_UTILITIES_TRACELOCK_HH 1

#include "tracemf.h"
#include <mutex>

class TraceLock {
    public:
        TraceLock(std::mutex& mutex,int level, std::string description);
        virtual ~TraceLock();

    private:
        std::unique_lock<std::mutex> lock_;
		std::string description_;
		int level_;
};

inline TraceLock::TraceLock(std::mutex& mutex,int level, std::string description)
    : lock_(mutex)
	, description_(description)
	, level_(level)
{
	TLOG_ARB(level_, "TraceLock") << "Acquired Lock " << description_ << ", mutex=" << (void*)&mutex << ", lock=" << (void*)&lock_ << TLOG_ENDL;
}

inline TraceLock::~TraceLock() {
	TLOG_ARB(level_, "TraceLock") << "Releasing lock " << description_ << ", lock=" << (void*)&lock_ << TLOG_ENDL;
}

#endif

