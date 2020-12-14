#include "artdaq-core/Utilities/configureMessageFacility.hh"
#include "artdaq-core/Utilities/ExceptionHandler.hh"

#define BOOST_TEST_MODULE GenFile_t
#include "cetlib/quiet_unit_test.hpp"
#include "cetlib_except/exception.h"
#include <boost/filesystem.hpp>
#include <fhiclcpp/make_ParameterSet.h>

#define TRACE_NAME "GenFile_t"
#include "tracemf.h"

BOOST_AUTO_TEST_SUITE(GenFile_test)

BOOST_AUTO_TEST_CASE(genFileFileNameFlags)
{
	setenv("ARTDAQ_LOG_ROOT", "/tmp", 1);
	auto pstr = artdaq::generateMessageFacilityConfiguration("configureMessageFacility_t", true, true, "-%N-%H-%T-%U-%%-%?N-%?L-");

	fhicl::ParameterSet pset;
	try
	{
		fhicl::make_ParameterSet(pstr, pset);
	}
	catch (cet::exception const&)
	{
		artdaq::ExceptionHandler(artdaq::ExceptionHandlerRethrow::yes, std::string("Exception occurred while processing fhicl ParameterSet string ") + pstr + ":");
	}
#if CANVAS_HEX_VERSION >= 0x30300  // art v2_11_00
	mf::StartMessageFacility(pset, "configureMessageFacility_t");

#else
	mf::StartMessageFacility(pset);

	mf::SetApplicationName("configureMessageFacility_t");

	mf::setEnabledState("");
#endif

	mf::LogInfo("Test") << "Test Message";
	TLOG(TLVL_INFO) << "Test TRACE";

	boost::filesystem::remove_all("/tmp/configureMessageFacility_t");

	
}

BOOST_AUTO_TEST_SUITE_END()