#ifndef artdaq_core_Data_ContainerFragment_hh
#define artdaq_core_Data_ContainerFragment_hh

#include "artdaq-core/Data/Fragment.hh"
#include "cetlib/exception.h"

#include <ostream>
#include <vector>

// Implementation of "ContainerFragment", an artdaq::Fragment overlay class

// The maximum capacity of the ContainerFragment (in fragments)
#ifndef CONTAINER_FRAGMENT_CAPACITY
#define CONTAINER_FRAGMENT_CAPACITY 100
#endif

namespace artdaq {
	class ContainerFragment;

	static const int FRAGMENT_COUNT_MAX = CONTAINER_FRAGMENT_CAPACITY;
}

class artdaq::ContainerFragment {
public:

	struct Metadata {
		typedef uint8_t data_t;
		typedef uint64_t count_t;

		count_t     block_count : 56;
		count_t     fragment_type : 8;

		size_t index[FRAGMENT_COUNT_MAX];

		static size_t const size_words = 8ul + FRAGMENT_COUNT_MAX * sizeof(size_t) / sizeof(data_t); // Units of Header::data_t
	};

	static_assert (sizeof(Metadata) == Metadata::size_words * sizeof(Metadata::data_t), "ContainerFragment::Metadata size changed");

	// The constructor simply sets its const private member "artdaq_Fragment_"
	// to refer to the artdaq::Fragment object

	ContainerFragment(Fragment const & f) : artdaq_Fragment_(f) { }

	// const getter functions for the metadata
	Metadata const * metadata() const { return artdaq_Fragment_.metadata<Metadata>(); }
	Metadata::count_t block_count()   const { return metadata()->block_count; }
	Fragment::type_t  fragment_type() const { return static_cast<Fragment::type_t>(metadata()->fragment_type); }

	// Start of the Fragments
	Fragment const * dataBegin() const {
		return reinterpret_cast<Fragment const *>(&*artdaq_Fragment_.dataBegin());
	}

	Fragment const * dataEnd() const {
		return reinterpret_cast<Fragment const *>(reinterpret_cast<uint8_t const *>(dataBegin()) + lastFragmentIndex());
	}

	Fragment const * operator[](size_t index) const {
		if (index > block_count()) throw cet::exception("Buffer overrun detected! ContainerFragment::operator[] was asked for a non-existant Fragment!");
		return reinterpret_cast<Fragment const *>(reinterpret_cast<uint8_t const *>(dataBegin()) + fragmentIndex(index));
	}

	size_t fragmentIndex(size_t index) const {
		assert(index <= block_count());
		if (index == 0) { return 0; }
		return metadata()->index[index - 1];
	}

	size_t  lastFragmentIndex() const {
		return fragmentIndex(block_count());
	}

protected:

	static constexpr size_t words_per_frag_word_() {
		return sizeof(Fragment::value_type) / sizeof(Metadata::data_t);
	}

private:

	Fragment const & artdaq_Fragment_;
};

#endif /* artdaq_core_Data_ContainerFragment_hh */
