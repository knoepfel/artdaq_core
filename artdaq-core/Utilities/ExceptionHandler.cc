
// JCF, 5/28/15

// The ExceptionHandler() function is designed to be called within a catch-all block:

// try {
//
//   ...Code that might throw an exception...
//
// } catch (...) {
//
//  ExceptionHandler(artdaq::ExceptionHandlerRethrow::yes,
//		 "Optional string providing additional info");
//
// }

// Where above, you could switch out
// "artdaq::ExceptionHandlerRethrow::yes" with
// "artdaq::ExceptionHandlerRethrow::no", depending on what you wish
// to do

// The details of ExceptionHandler() are as follows:

// -If an optional string is passed to it, use messagefacility to write the string with mf::LogError()

// -Apply a set of different catch-blocks to the original exception,
// printing out as much information as possible contained within the
// different exception types (art::Exception, cet::exception,
// boost::exception and std::exception), again using mf::LogError()

// -If artdaq::ExceptionHandlerRethrow::yes was passed to
//  ExceptionHandler(), re-throw the exception rather than swallow it

#include "ExceptionHandler.hh"

#include "art/Utilities/Exception.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <boost/exception/all.hpp>

#include <stdexcept>

namespace artdaq {

  void ExceptionHandler(ExceptionHandlerRethrow decision, std::string optional_message) {

    if (optional_message != "") {
      mf::LogError("ExceptionHandler") << optional_message;
    }

    try {
      throw;
    } catch (const art::Exception& e) {

      mf::LogError("ExceptionHandler") << "art::Exception object caught:" <<
	" returnCode = " << e.returnCode() <<
	", categoryCode = " << e.categoryCode() << 
	", category = " << e.category();
      mf::LogError("ExceptionHandler") << "art::Exception object stream:" << e;

      if (decision == ExceptionHandlerRethrow::yes) { throw; }

    } catch (const cet::exception &e) {

      mf::LogError("ExceptionHandler") << "cet::exception object caught:" <<
	e.explain_self();

      if (decision == ExceptionHandlerRethrow::yes) { throw; }

    } catch (const boost::exception& e) {
      
      mf::LogError("ExceptionHandler") << "boost::exception object caught: " <<
	boost::diagnostic_information(e);

      if (decision == ExceptionHandlerRethrow::yes) { throw; }

    } catch (const std::exception& e  ) {

      mf::LogError ("ExceptionHandler") << "std::exception caught: " << e.what();

      if (decision == ExceptionHandlerRethrow::yes) { throw; }

    } catch (...) {

      mf::LogError ("ExceptionHandler") << "Exception of type unknown to artdaq::ExceptionHandler caught";

      if (decision == ExceptionHandlerRethrow::yes) { throw; }

    }
  }

}
