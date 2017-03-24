#ifndef artdaq_core_Data_ContainerFragment_hh
#define artdaq_core_Data_ContainerFragment_hh

#include "artdaq-core/Data/Fragment.hh"
#include "cetlib/exception.h"

//#include <ostream>
//#include <vector>

// Implementation of "ContainerFragment", an artdaq::Fragment overlay class

#ifndef CONTAINER_FRAGMENT_CAPACITY
/// The maximum capacity of the ContainerFragment (in fragments)
#define CONTAINER_FRAGMENT_CAPACITY 100
#endif

namespace artdaq
{
	class ContainerFragment;

	/**
	 * \brief The maximum capacity of the ContainerFragment (in fragments)
	 */
	static const int CONTAINER_FRAGMENT_COUNT_MAX = CONTAINER_FRAGMENT_CAPACITY;
}

/**
 * \brief The artdaq::ContainerFragment class represents a Fragment which contains other Fragments
 */
class artdaq::ContainerFragment
{
public:

	/**
	 * \brief Contains the information necessary for retrieving Fragment objects from the ContainerFragment
	 */
	struct Metadata
	{
		typedef uint8_t data_t; ///< Basic unit of data-retrieval
		typedef uint64_t count_t; ///< Size of block_count variables

		count_t block_count : 55; ///< The number of Fragment objects stored in the ContainerFragment
		count_t fragment_type : 8; ///< The Fragment::type_t of stored Fragment objects
		count_t missing_data : 1; ///< Flag if the ContainerFragment knows that it is missing data

		/// Offset of each Fragment within the ContainerFragment
		size_t index[CONTAINER_FRAGMENT_COUNT_MAX];

		/// Size of the Metadata object
		static size_t const size_words = 8ul + CONTAINER_FRAGMENT_COUNT_MAX * sizeof(size_t) / sizeof(data_t); // Units of Header::data_t
	};

	static_assert (sizeof(Metadata) == Metadata::size_words * sizeof(Metadata::data_t), "ContainerFragment::Metadata size changed");

	/**
	 * \param f The Fragment object to use for data storage
	 * 
	 * The constructor simply sets its const private member "artdaq_Fragment_"
	 * to refer to the artdaq::Fragment object
	*/
	explicit ContainerFragment(Fragment const& f) : artdaq_Fragment_(f) { }

	/**
	 * \brief const getter function for the Metadata
	 * \return const pointer to the Metadata
	 */
	Metadata const* metadata() const { return artdaq_Fragment_.metadata<Metadata>(); }
	
	/**
	 * \brief Gets the number of fragments stored in the ContainerFragment
	 * \return The number of Fragment objects stored in the ContainerFragment
	 */
	Metadata::count_t block_count() const { return metadata()->block_count; }
	/**
	 * \brief Get the Fragment::type_t of stored Fragment objects
	 * \return The Fragment::type_t of stored Fragment objects
	 */
	Fragment::type_t fragment_type() const { return static_cast<Fragment::type_t>(metadata()->fragment_type); }
	/**
	 * \brief Gets the flag if the ContainerFragment knows that it is missing data
	 * \return The flag if the ContainerFragment knows that it is missing data
	 */
	bool missing_data() const { return static_cast<bool>(metadata()->missing_data); }

	/**
	 * \brief Gets the start of the data
	 * \return Pointer to the first Fragment in the ContainerFragment
	 */
	Fragment const* dataBegin() const
	{
		return reinterpret_cast<Fragment const *>(&*artdaq_Fragment_.dataBegin());
	}

	/**
	 * \brief Gets the last Fragment in the ContainerFragment
	 * \return Pointer to the last Fragment in the ContainerFragment
	 */
	Fragment const* dataEnd() const
	{
		return reinterpret_cast<Fragment const *>(reinterpret_cast<uint8_t const *>(dataBegin()) + lastFragmentIndex());
	}

	/**
	 * \brief Gets a specific Fragment from the ContainerFragment
	 * \param index The Fragment index to return
	 * \return Pointer to the specified Fragment in the ContainerFragment
	 * \exception cet::exception if the index is out-of-range
	 */
	Fragment const* at(size_t index) const
	{
		if (index > block_count()) throw cet::exception("Buffer overrun detected! ContainerFragment::at was asked for a non-existant Fragment!");
		return reinterpret_cast<Fragment const *>(reinterpret_cast<uint8_t const *>(dataBegin()) + fragmentIndex(index));
	}

	/**
	 * \brief Gets the size of the Fragment at the specified location in the ContainerFragment
	 * \param index The Fragment index
	 * \return The size of the Fragment at the specified location in the ContainerFragment
	 * \exception cet::exception if the index is out-of-range
	 */
	size_t fragSize(size_t index) const
	{
		if (index >= block_count()) throw cet::exception("Buffer overrun detected! ContainerFragment::at was asked for a non-existant Fragment!");
		auto end = metadata()->index[index];
		if (index == 0) return end;
		return end - metadata()->index[index - 1];
	}

	/**
	 * \brief Alias to ContainerFragment::at()
	 * \param index The Fragment index to return
	 * \return Pointer to the specified Fragment in the ContainerFragment
	 * \exception cet::exception if the index is out-of-range
	 */
	Fragment const* operator[](size_t index) const
	{
		return this->at(index);
	}

	/**
	 * \brief Get the offset of a Fragment within the ContainerFragment
	 * \param index The Fragment index
	 * \return The offset of the requested Fragment within the ContainerFragment
	 * \exception cet::exception if the index is out-of-range
	 */
	size_t fragmentIndex(size_t index) const
	{
		if (index > block_count()) throw cet::exception("Buffer overrun detected! ContainerFragment::at was asked for a non-existant Fragment!");
		if (index == 0) { return 0; }
		return metadata()->index[index - 1];
	}

	/**
	 * \brief Returns the offset of the last Fragment in the ContainerFragment
	 * \return The offset of the last Fragment in the ContainerFragment
	 */
	size_t lastFragmentIndex() const
	{
		return fragmentIndex(block_count());
	}

protected:

	/**
	 * \brief Gets the ratio between the fundamental data storage type and the representation within the Fragment
	 * \return The ratio between the fundamental data storage type and the representation within the Fragment
	 */
	static constexpr size_t words_per_frag_word_()
	{
		return sizeof(Fragment::value_type) / sizeof(Metadata::data_t);
	}

private:

	Fragment const& artdaq_Fragment_;
};

#endif /* artdaq_core_Data_ContainerFragment_hh */
