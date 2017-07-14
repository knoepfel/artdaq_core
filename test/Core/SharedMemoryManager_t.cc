#include "artdaq-core/Core/SharedMemoryManager.hh"

#define BOOST_TEST_MODULE(SharedMemoryManager_t)
#include "cetlib/quiet_unit_test.hpp"
#include "cetlib_except/exception.h"

BOOST_AUTO_TEST_SUITE(SharedMemoryManager_test)

BOOST_AUTO_TEST_CASE(Construct)
{
	artdaq::SharedMemoryManager man(0x7357 + rand() % 0x10000000, 10, 0x1000,0x10000);
	BOOST_REQUIRE_EQUAL(man.IsValid(), true);
	BOOST_REQUIRE_EQUAL(man.GetMyId(), 0);
	BOOST_REQUIRE_EQUAL(man.size(), 10);
	BOOST_REQUIRE_EQUAL(man.GetMaxId(), 0);
}

BOOST_AUTO_TEST_CASE(Attach)
{
	uint32_t key = 0x7357 + rand() % 0x10000000;
	artdaq::SharedMemoryManager man(key, 10, 0x1000, 0x10000);
	artdaq::SharedMemoryManager man2(key, 10, 0x1000, 0x10000);

	BOOST_REQUIRE_EQUAL(man.IsValid(), true);
	BOOST_REQUIRE_EQUAL(man.GetMyId(), 0);
	BOOST_REQUIRE_EQUAL(man.size(), 10);
	BOOST_REQUIRE_EQUAL(man.GetMaxId(), 1);

	BOOST_REQUIRE_EQUAL(man2.IsValid(), true);
	BOOST_REQUIRE_EQUAL(man2.GetMyId(), 1);
	BOOST_REQUIRE_EQUAL(man2.size(), 10);
	BOOST_REQUIRE_EQUAL(man2.GetMaxId(), 1);

}

BOOST_AUTO_TEST_CASE(DataFlow)
{
	uint32_t key = 0x7357 + rand() % 0x10000000;
	artdaq::SharedMemoryManager man(key, 10, 0x1000, 0x10000);
	artdaq::SharedMemoryManager man2(key, 10, 0x1000, 0x10000);

	BOOST_REQUIRE_EQUAL(man.ReadyForWrite(false), true);
	BOOST_REQUIRE_EQUAL(man.WriteReadyCount(false), 10);
	BOOST_REQUIRE_EQUAL(man.ReadyForRead(), false);
	BOOST_REQUIRE_EQUAL(man.ReadReadyCount(), 0);

	int buf = man.GetBufferForWriting(false);
	BOOST_REQUIRE_EQUAL(man.CheckBuffer(buf, artdaq::SharedMemoryManager::BufferSemaphoreFlags::Writing), true);
	BOOST_REQUIRE_EQUAL(man.GetBuffersOwnedByManager().size(), 1);
	BOOST_REQUIRE_EQUAL(man.GetBuffersOwnedByManager()[0], buf);
	BOOST_REQUIRE_EQUAL(man2.GetBuffersOwnedByManager().size(),0);
	BOOST_REQUIRE_EQUAL(man.BufferDataSize(buf), 0);

	uint8_t n = 0;
	uint8_t data[0x1000];
	std::generate_n(data, 0x1000, [&]() {return ++n; });
	man.Write(buf, data, 0x1000);
	BOOST_REQUIRE_EQUAL(man.BufferDataSize(buf), 0x1000);
	BOOST_REQUIRE_EQUAL(man2.BufferDataSize(buf), 0x1000);
	man.MarkBufferFull(buf, 1);
	BOOST_REQUIRE_EQUAL(man2.CheckBuffer(buf, artdaq::SharedMemoryManager::BufferSemaphoreFlags::Full), true);
	BOOST_REQUIRE_EQUAL(man.GetBuffersOwnedByManager().size(), 0);
	BOOST_REQUIRE_EQUAL(man2.GetBuffersOwnedByManager().size(), 1);

	BOOST_REQUIRE_EQUAL(man.ReadyForRead(), false);
	BOOST_REQUIRE_EQUAL(man2.ReadyForRead(), true);
	BOOST_REQUIRE_EQUAL(man2.ReadReadyCount(), 1);

	auto readbuf = man2.GetBufferForReading();
	BOOST_REQUIRE_EQUAL(man2.CheckBuffer(buf, artdaq::SharedMemoryManager::BufferSemaphoreFlags::Reading), true);
	BOOST_REQUIRE_EQUAL(man2.MoreDataInBuffer(readbuf), true);
	uint8_t byte;
	auto sts = man2.Read(readbuf, &byte, 1);
	BOOST_REQUIRE_EQUAL(sts, true);
	BOOST_REQUIRE_EQUAL(byte, 1); // ++n means that the first entry will be 1
	BOOST_REQUIRE_EQUAL(man2.MoreDataInBuffer(readbuf), true);
	sts = man2.Read(readbuf, &byte, 1);
	BOOST_REQUIRE_EQUAL(sts, true);
	BOOST_REQUIRE_EQUAL(byte, 2); // Second entry is 2
	man2.IncrementReadPos(readbuf, 0x10);
	BOOST_REQUIRE_EQUAL(man2.MoreDataInBuffer(readbuf), true);
	sts = man2.Read(readbuf, &byte, 1);
	BOOST_REQUIRE_EQUAL(sts, true);
	BOOST_REQUIRE_EQUAL(byte, 0x13); // Read increments, so it would have read 3, but we added 0x10, so we expect 0x13.
	man2.ResetReadPos(readbuf);
	BOOST_REQUIRE_EQUAL(man2.MoreDataInBuffer(readbuf), true);
	sts = man2.Read(readbuf, &byte, 1);
	BOOST_REQUIRE_EQUAL(sts, true);
	BOOST_REQUIRE_EQUAL(byte, 1);
	man2.IncrementReadPos(readbuf, 0xFFF);
	BOOST_REQUIRE_EQUAL(man2.MoreDataInBuffer(readbuf), false);
	man2.MarkBufferEmpty(readbuf);

	BOOST_REQUIRE_EQUAL(man2.GetBuffersOwnedByManager().size(), 0);
	BOOST_REQUIRE_EQUAL(man.GetBuffersOwnedByManager().size(), 0);
	BOOST_REQUIRE_EQUAL(man.WriteReadyCount(false), 10);
}

BOOST_AUTO_TEST_CASE(Exceptions)
{
	uint32_t key = 0x7357 + rand() % 0x10000000;
	artdaq::SharedMemoryManager man(key, 10, 0x1000, 0x10000);
	artdaq::SharedMemoryManager man2(key, 10, 0x1000, 0x10000);
	BOOST_REQUIRE_EQUAL(man.ReadyForWrite(false), true);
	BOOST_REQUIRE_EQUAL(man.WriteReadyCount(false), 10);
	BOOST_REQUIRE_EQUAL(man.ReadyForRead(), false);
	BOOST_REQUIRE_EQUAL(man.ReadReadyCount(), 0);

	// Trying to get an invalid buffer is an exception
	BOOST_REQUIRE_EXCEPTION(man.ResetReadPos(11), cet::exception, [&](cet::exception e) { return e.category() == "ArgumentOutOfRange"; });

	// Trying to access a buffer that is in the wrong state is an exception
	BOOST_REQUIRE_EXCEPTION(man.MarkBufferFull(0),cet::exception, [&](cet::exception e) { return e.category() == "StateAccessViolation"; });

	// Writing too much data is an exception
	int buf = man.GetBufferForWriting(false);
	BOOST_REQUIRE_EQUAL(man.CheckBuffer(buf, artdaq::SharedMemoryManager::BufferSemaphoreFlags::Writing), true);
	BOOST_REQUIRE_EQUAL(man.GetBuffersOwnedByManager().size(), 1);
	BOOST_REQUIRE_EQUAL(man.GetBuffersOwnedByManager()[0], buf);
	BOOST_REQUIRE_EQUAL(man.BufferDataSize(buf), 0);

	uint8_t n = 0;
	uint8_t data[0x2000];
	std::generate_n(data, 0x2000, [&]() {return ++n; });
	BOOST_REQUIRE_EXCEPTION(man.Write(buf, data, 0x2000), cet::exception, [&](cet::exception e) { return e.category() == "SharedMemoryWrite"; });
	man.Write(buf, data, 0x1000);
	man.MarkBufferFull(buf);

	// Reading too much data is an exception
	int readbuf = man.GetBufferForReading();
	BOOST_REQUIRE_EXCEPTION(man.Read(readbuf, data, 0x1001), cet::exception, [&](cet::exception e) { return e.category() == "SharedMemoryRead"; });

	// Accessing a buffer that is not owned by the manager is an exception
	BOOST_REQUIRE_EXCEPTION(man2.MarkBufferEmpty(readbuf), cet::exception, [&](cet::exception e) { return e.category() == "OwnerAccessViolation"; });

}

BOOST_AUTO_TEST_SUITE_END()
