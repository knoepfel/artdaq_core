#include "artdaq-core/Core/SharedMemoryFragmentManager.hh"

#define BOOST_TEST_MODULE(SharedMemoryFragmentManager_t)
#include "cetlib/quiet_unit_test.hpp"
#include "cetlib_except/exception.h"


BOOST_AUTO_TEST_SUITE(SharedMemoryFragmentManager_test)

BOOST_AUTO_TEST_CASE(Construct)
{
	artdaq::SharedMemoryFragmentManager man(0x7357 + rand() % 0x10000000, 10, 0x1000);
	BOOST_REQUIRE_EQUAL(man.IsValid(), true);
	BOOST_REQUIRE_EQUAL(man.GetMyId(), 0);
	BOOST_REQUIRE_EQUAL(man.size(), 10);
	BOOST_REQUIRE_EQUAL(man.GetMaxId(), 0);
}

BOOST_AUTO_TEST_CASE(Attach)
{
	int key = 0x7357 + rand() % 0x10000000;
	artdaq::SharedMemoryFragmentManager man(key, 10, 0x1000);
	artdaq::SharedMemoryFragmentManager man2(key, 10, 0x1000);

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
	std::cout << "Initializing SharedMemoryFragmentManagers for DataFlow test" << std::endl;
	int key = 0x7357 + rand() % 0x10000000;
	artdaq::SharedMemoryFragmentManager man(key, 10, 0x1000);
	artdaq::SharedMemoryFragmentManager man2(key, 10, 0x1000);

	auto fragSizeWords = 0x1000 / sizeof(artdaq::RawDataType) - artdaq::detail::RawFragmentHeader::num_words() - 1;

	std::cout << "Creating test Fragment" << std::endl;
	artdaq::Fragment frag(fragSizeWords);
	frag.setSequenceID(0x10);
	frag.setFragmentID(0x20);
	auto type = artdaq::Fragment::DataFragmentType;
	frag.setSystemType(type);
	frag.setTimestamp(0x30);
	for (size_t ii = 0; ii < fragSizeWords; ++ii)
	{
		*(frag.dataBegin() + ii) = ii;
	}

	std::cout << "Writing Test Fragment to Shared Memory" << std::endl;
	man.WriteFragment(std::move(frag), false);

	std::cout << "Reading Test Fragment Header" << std::endl;
	artdaq::detail::RawFragmentHeader header;
	auto sts = man2.ReadFragmentHeader(header);
	
	std::cout << "Checking Test Fragment Header Contents" << std::endl;
	BOOST_REQUIRE_EQUAL(sts, 0);
	BOOST_REQUIRE_EQUAL(header.word_count, frag.size());
	BOOST_REQUIRE_EQUAL(header.sequence_id, 0x10);
	BOOST_REQUIRE_EQUAL(header.fragment_id, 0x20);
	BOOST_REQUIRE_EQUAL(header.type, type);
	BOOST_REQUIRE_EQUAL(header.timestamp, 0x30);

	std::cout << "Reading Test Fragment data" << std::endl;
	artdaq::Fragment frag2(header.word_count);
	sts = man2.ReadFragmentData(frag2.dataBegin(), header.word_count - header.num_words());

	std::cout << "Checking Test Fragment contents" << std::endl;
	BOOST_REQUIRE_EQUAL(sts, 0);
	for(size_t ii = 0; ii < fragSizeWords; ++ii)
	{
		BOOST_REQUIRE_EQUAL(*(frag.dataBegin() + ii), *(frag2.dataBegin() + ii));
	}
	std::cout << "SharedMemoryFragmentManager test complete" << std::endl;
}

BOOST_AUTO_TEST_CASE(WholeFragment)
{
	std::cout << "Initializing SharedMemoryFragmentManagers for WholeFragment Test" << std::endl;
	int key = 0x7357 + rand() % 0x10000000;
	artdaq::SharedMemoryFragmentManager man(key, 10, 0x1000);
	artdaq::SharedMemoryFragmentManager man2(key, 10, 0x1000);

	auto fragSizeWords = 0x1000 / sizeof(artdaq::RawDataType) - artdaq::detail::RawFragmentHeader::num_words() - 1;

	std::cout << "Creating test Fragment" << std::endl;
	artdaq::Fragment frag(fragSizeWords);
	frag.setSequenceID(0x10);
	frag.setFragmentID(0x20);
	auto type = artdaq::Fragment::DataFragmentType;
	frag.setSystemType(type);
	frag.setTimestamp(0x30);
	for (size_t ii = 0; ii < fragSizeWords; ++ii)
	{
		*(frag.dataBegin() + ii) = ii;
	}

	std::cout << "Writing Test Fragment to Shared Memory" << std::endl;
	man.WriteFragment(std::move(frag), false);

	std::cout << "Reading Test Fragment Header" << std::endl;
	artdaq::Fragment recvdFrag;
	auto sts = man2.ReadFragment(recvdFrag);

	std::cout << "Checking Test Fragment Header Contents" << std::endl;
	BOOST_REQUIRE_EQUAL(sts, 0);
	BOOST_REQUIRE_EQUAL(recvdFrag.size(), frag.size());
	BOOST_REQUIRE_EQUAL(recvdFrag.sequenceID(), 0x10);
	BOOST_REQUIRE_EQUAL(recvdFrag.fragmentID(), 0x20);
	BOOST_REQUIRE_EQUAL(recvdFrag.type(), type);
	BOOST_REQUIRE_EQUAL(recvdFrag.timestamp(), 0x30);

	std::cout << "Checking Test Fragment Data Contents" << std::endl;
	for (size_t ii = 0; ii < fragSizeWords; ++ii)
	{
		//std::cout << std::to_string(*(frag.dataBegin() + ii)) << " =?= " << *(recvdFrag.dataBegin() + ii) << std::endl;
		BOOST_REQUIRE_EQUAL(*(frag.dataBegin() + ii), *(recvdFrag.dataBegin() + ii));
	}
	std::cout << "SharedMemoryFragmentManager test complete" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
