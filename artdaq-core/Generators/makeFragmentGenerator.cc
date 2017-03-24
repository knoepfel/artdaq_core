#include "artdaq-core/Generators/makeFragmentGenerator.hh"

#include "artdaq-core/Generators/GeneratorMacros.hh"
#include "fhiclcpp/ParameterSet.h"
#include "cetlib/BasicPluginFactory.h"

std::unique_ptr<artdaq::FragmentGenerator>
artdaq::makeFragmentGenerator(std::string const& generator_plugin_spec,
							  fhicl::ParameterSet const& ps)
{
	static cet::BasicPluginFactory bpf("generator", "make");

	return bpf.makePlugin<std::unique_ptr<artdaq::FragmentGenerator>,
						  fhicl::ParameterSet const &>(generator_plugin_spec, ps);
}
