#ifndef artdaq_core_Utilities_ExceptionHandler_hh
#define artdaq_core_Utilities_ExceptionHandler_hh

#include <string>

namespace artdaq {

  enum ExceptionHandlerRethrow {yes, no};

  void ExceptionHandler(ExceptionHandlerRethrow decision, std::string optional_message = "");

}

#endif
