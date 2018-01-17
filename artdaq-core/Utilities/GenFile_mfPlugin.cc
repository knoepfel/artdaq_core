#include "cetlib/PluginTypeDeducer.h"
#include "cetlib/ostream_handle.h"
#include "fhiclcpp/ParameterSet.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#include "messagefacility/MessageService/ELdestination.h"
# include "messagefacility/Utilities/ELseverityLevel.h"
#if MESSAGEFACILITY_HEX_VERSION < 0x20002 // v2_00_02 is s50, pre v2_00_02 is s48
# include "messagefacility/MessageService/ELcontextSupplier.h"
# include "messagefacility/MessageLogger/MessageDrop.h"
#endif
#include "messagefacility/Utilities/exception.h"

#include <fstream>

namespace mfplugins
{
	using mf::service::ELdestination;
	using mf::ELseverityLevel;
	using mf::ErrorObj;
#if MESSAGEFACILITY_HEX_VERSION < 0x20002 // v2_00_02 is s50, pre v2_00_02 is s48
	using mf::service::ELcontextSupplier;
#endif

	/// <summary>
	/// Message Facility destination which generates the output file name based on some combination of 
	/// PID, hostname, application name, and/or timestamp
	/// </summary>
	class ELGenFileOutput : public ELdestination
	{
	public:

		ELGenFileOutput(const fhicl::ParameterSet& pset);

		virtual ~ELGenFileOutput() {}

		virtual void routePayload(const std::ostringstream&, const ErrorObj&
# if MESSAGEFACILITY_HEX_VERSION < 0x20002 // v2_00_02 is s50, pre v2_00_02 is s48
								  , const ELcontextSupplier&
#endif
		) override;

		virtual void flush(
# if MESSAGEFACILITY_HEX_VERSION < 0x20002 // v2_00_02 is s50, pre v2_00_02 is s48
			const ELcontextSupplier&
#endif
		) override;

	private:
		bool append_;
		std::unique_ptr<cet::ostream_handle> output_;
	};

	// END DECLARATION
	//======================================================================
	// BEGIN IMPLEMENTATION


	//======================================================================
	// ELGenFileOutput c'tor
	//======================================================================

	ELGenFileOutput::ELGenFileOutput(const fhicl::ParameterSet& pset)
		: ELdestination(pset)
		, append_(pset.get<bool>("append", true))
	{
		std::string baseDir = pset.get<std::string>("directory", "/tmp");
		std::string sep = pset.get<std::string>("separator", "-");
		std::string timePattern = pset.get<std::string>("timestamp_pattern", "%Y%m%d%H%M%S"); // For strftime
		std::string filePattern = pset.get<std::string>("pattern", "%N-%?H%t-%p.log");

		auto pid = getpid();
		std::string exeString;
		std::string hostString;
		std::string timeBuffISO; // Using boost::posix_time::to_iso_string (%T)
		std::string timeBuff; // Using timestamp_pattern (%t)


		// Determine image name
		if (filePattern.find("%N") != std::string::npos || filePattern.find("%?N") != std::string::npos)
		{
			// get process name from '/proc/pid/exe'
			std::string exe;
			std::ostringstream pid_ostr;
			pid_ostr << "/proc/" << pid << "/exe";
			exe = std::string(realpath(pid_ostr.str().c_str(), NULL));

			size_t end = exe.find('\0');
			size_t start = exe.find_last_of('/', end);
			exeString = exe.substr(start + 1, end - start - 1);
		}

		// Get Host name
		if (filePattern.find("%H") != std::string::npos || filePattern.find("%?H") != std::string::npos)
		{
			char hostname[256];
			std::string hostString = "";
			if (gethostname(&hostname[0], 256) == 0)
			{
				std::string tmpString(hostname);
				hostString = tmpString;
				size_t pos = hostString.find(".");
				if (pos != std::string::npos && pos > 2)
				{
					hostString = hostString.substr(0, pos);
				}
			}
		}
		if (filePattern.find("%T") != std::string::npos)
		{
			timeBuffISO = boost::posix_time::to_iso_string(boost::posix_time::second_clock::universal_time());
		}
		if (filePattern.find("%t") != std::string::npos)
		{

			time_t rawtime;
			struct tm* timeinfo;
			char timeBuffC[256];
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(timeBuffC, 256, timePattern.c_str(), timeinfo);
			timeBuff = std::string(timeBuffC);
		}

		size_t pos = 0;
		while (filePattern.find("%", pos) != std::string::npos)
		{
			pos = filePattern.find("%", pos) + 1;
			switch (filePattern[pos])
			{
			case '%': // "%%"
				pos++;
				break;
			case '?':
			{
				char next = filePattern[pos + 1];
				switch (next) {
				case 'N':
					if (filePattern.find(exeString) != std::string::npos) {
						filePattern.erase(pos - 1, 3);
					}
					else {
						std::string repString = exeString;
						// Only append separator if we're not at the end of the pattern
						if (!(pos + 1 == filePattern.size() - 1 || pos + 2 == filePattern.find_last_of('.'))) {
							repString += sep;
						}
						filePattern.replace(pos - 1, 3, repString);
					}
					break;
				case 'H':
					if (filePattern.find(hostString) != std::string::npos) {
						filePattern.erase(pos - 1, 3);
					}
					else {
						std::string repString = hostString;
						// Only append separator if we're not at the end of the pattern
						if (!(pos + 1 == filePattern.size() - 1 || pos + 2 == filePattern.find_last_of('.'))) {
							repString += sep;
							filePattern.replace(pos - 1, 3, repString);
						}
						break;
					}
				}
			}
			break;
			case 'N':
				filePattern.replace(pos - 1, 2, exeString);
				break;
			case 'H':
				filePattern.replace(pos - 1, 2, hostString);
				break;
			case 'p':
				filePattern.replace(pos - 1, 2, std::to_string(pid));
				break;
			case 't':
				filePattern.replace(pos - 1, 2, timeBuff);
				break;
			case 'T':
				filePattern.replace(pos - 1, 2, timeBuffISO);
				break;
			}
		}
		std::string fileName = baseDir + "/" + filePattern;

		output_ = std::make_unique<cet::ostream_handle>(fileName.c_str(), append_ ? std::ios::app : std::ios::trunc);
	}

	//======================================================================
	// Message router ( overriddes ELdestination::routePayload )
	//======================================================================
	void ELGenFileOutput::routePayload(const std::ostringstream& oss, const ErrorObj&
# if MESSAGEFACILITY_HEX_VERSION < 0x20002 // v2_00_02 is s50, pre v2_00_02 is s48
									   , ELcontextSupplier const& sup
#endif
	)
	{
		*output_ << oss.str();
		flush(
# if MESSAGEFACILITY_HEX_VERSION < 0x20002 // v2_00_02 is s50, pre v2_00_02 is s48
			sup
#endif
		);
	}

	void ELGenFileOutput::flush(
# if MESSAGEFACILITY_HEX_VERSION < 0x20002 // v2_00_02 is s50, pre v2_00_02 is s48
		ELcontextSupplier const&
#endif
	)
	{
		output_->flush();
	}
} // end namespace mfplugins

  //======================================================================
  //
  // makePlugin function
  //
  //======================================================================

extern "C"
{
	auto makePlugin(const std::string&,
					const fhicl::ParameterSet& pset)
	{
		return std::make_unique<mfplugins::ELGenFileOutput>(pset);
	}
}

DEFINE_BASIC_PLUGINTYPE_FUNC(mf::service::ELdestination)
