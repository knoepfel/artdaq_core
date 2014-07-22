#ifndef artdaq_core_Core_GlobalQueue_hh
#define artdaq_core_Core_GlobalQueue_hh

#include "artdaq-core/Core/ConcurrentQueue.hh"
#include "artdaq-core/Data/RawEvent.hh"
#include <memory>

namespace artdaq {
  typedef std::shared_ptr<RawEvent> RawEvent_ptr;
  typedef daqrate::ConcurrentQueue<RawEvent_ptr> RawEventQueue;
  typedef daqrate::ConcurrentQueue<RawEvent_ptr>::SizeType SizeType;

  // The first thread to call getGlobalQueue() causes the creation of
  // the queue. The queue will be destroyed at static destruction
  // time.
  RawEventQueue & getGlobalQueue(SizeType maxSize=std::numeric_limits<SizeType>::max());
}

#endif /* artdaq_core_Core_GlobalQueue_hh */
