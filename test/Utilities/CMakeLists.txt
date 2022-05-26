cet_test(TimeUtils_t USE_BOOST_UNIT
	LIBRARIES artdaq-core_Utilities
)

cet_test(ExceptionHandler_t USE_BOOST_UNIT
	LIBRARIES artdaq-core_Utilities
	cetlib_except::cetlib_except
	canvas::canvas
)

cet_test(SimpleLookupPolicy_t USE_BOOST_UNIT
	DATAFILES fcl/LookupTarget.fcl
	LIBRARIES artdaq-core_Utilities
  messagefacility::MF_MessageLogger
)

cet_test(ExceptionStackTrace_t USE_BOOST_UNIT
	LIBRARIES artdaq-core_Utilities_ExceptionStackTrace
  messagefacility::MF_MessageLogger
)

cet_test(configureMessageFacility_t USE_BOOST_UNIT
	LIBRARIES artdaq-core_Utilities
  messagefacility::MF_MessageLogger
)

cet_test(GenFile_t USE_BOOST_UNIT
	LIBRARIES artdaq-core_Utilities
  messagefacility::MF_MessageLogger
)