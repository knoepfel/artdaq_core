#ifndef _artdaq_core_Plugins_ArtdaqFragmentNameHelper_hh_
#define _artdaq_core_Plugins_ArtdaqFragmentNameHelper_hh_

#include <set>
#include <string>
#include <vector>

#include "artdaq-core/Plugins/FragmentNameHelper.hh"
#include "artdaq-core/Data/Fragment.hh"

namespace artdaq {
	/**
	 * @brief Default implementation of FragmentNameHelper
	*/
	class ArtdaqFragmentNameHelper : public FragmentNameHelper
	{
	public:
		/**
		 * \brief DefaultArtdaqFragmentNameHelper Destructor
		 */
		virtual ~ArtdaqFragmentNameHelper();

		/**
		 * @brief ArtdaqFragmentNameHelper Constructor
		 * @param unidentified_instance_name Name to use for unidentified Fragments
		 * @param extraTypes Additional types to register
		*/
		ArtdaqFragmentNameHelper(std::string unidentified_instance_name, std::vector<std::pair<artdaq::Fragment::type_t, std::string>> extraTypes);

		/**
				 * \brief Returns the basic translation for the specified type.  Defaults to the specified
				 *        unidentified_instance_name if no translation can be found.
				 */
		virtual std::string GetInstanceNameForType(artdaq::Fragment::type_t type_id) const;

		/**
				 * \brief Returns the full set of product instance names which may be present in the data, based on
				 *        the types that have been specified in the SetBasicTypes() and AddExtraType() methods.  This
				 *        *does* include "container" types, if the container type mapping is part of the basic types.
				 */
		virtual std::set<std::string> GetAllProductInstanceNames() const;

		/**
				 * \brief Returns the product instance name for the specified fragment, based on the types that have
				 *        been specified in the SetBasicTypes() and AddExtraType() methods.  This *does* include the
				 *        use of "container" types, if the container type mapping is part of the basic types.  If no
				 *        mapping is found, the specified unidentified_instance_name is returned.
				 */
		virtual std::pair<bool, std::string>
			GetInstanceNameForFragment(artdaq::Fragment const& fragment) const;

	private:
		ArtdaqFragmentNameHelper(ArtdaqFragmentNameHelper const&) = delete;
		ArtdaqFragmentNameHelper(ArtdaqFragmentNameHelper&&) = delete;
		ArtdaqFragmentNameHelper& operator=(ArtdaqFragmentNameHelper const&) = delete;
		ArtdaqFragmentNameHelper& operator=(ArtdaqFragmentNameHelper&&) = delete;
	};
}  // namespace artdaq

#endif  //_artdaq_core_Plugins_ArtdaqFragmentNameHelper_hh_