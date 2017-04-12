#include "ExceptionHandler.hh"

#include "canvas/Utilities/Exception.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <boost/exception/all.hpp>

namespace artdaq
{
	void ExceptionHandler(ExceptionHandlerRethrow decision, std::string optional_message)
	{
		if (optional_message != "")
		{
			mf::LogError("ExceptionHandler") << optional_message;
		}

		try
		{
			throw;
		}
		catch (const art::Exception& e)
		{
			mf::LogError("ExceptionHandler") << "art::Exception object caught:" <<
				" returnCode = " << std::to_string(e.returnCode()) <<
				", categoryCode = " << e.categoryCode() <<
				", category = " << e.category();
			mf::LogError("ExceptionHandler") << "art::Exception object stream:" << e;

			if (decision == ExceptionHandlerRethrow::yes) { throw; }
		}
		catch (const cet::exception& e)
		{
			mf::LogError("ExceptionHandler") << "cet::exception object caught:" <<
				e.explain_self();

			if (decision == ExceptionHandlerRethrow::yes) { throw; }
		}
		catch (const boost::exception& e)
		{
			mf::LogError("ExceptionHandler") << "boost::exception object caught: " <<
				boost::diagnostic_information(e);

			if (decision == ExceptionHandlerRethrow::yes) { throw; }
		}
		catch (const std::exception& e)
		{
			mf::LogError("ExceptionHandler") << "std::exception caught: " << e.what();

			if (decision == ExceptionHandlerRethrow::yes) { throw; }
		}
		catch (...)
		{
			mf::LogError("ExceptionHandler") << "Exception of type unknown to artdaq::ExceptionHandler caught";

			if (decision == ExceptionHandlerRethrow::yes) { throw; }
		}
	}
}
