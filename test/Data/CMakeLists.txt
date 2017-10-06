
cet_test(Fragment_t USE_BOOST_UNIT
  LIBRARIES artdaq-core_Data
  )

cet_test(ContainerFragment_t USE_BOOST_UNIT
  LIBRARIES artdaq-core_Data
  ${MF_MESSAGEFACILITY}
  ${MF_MESSAGELOGGER}
  )

cet_test(RawEvent_t USE_BOOST_UNIT
  LIBRARIES artdaq-core_Data
  )
