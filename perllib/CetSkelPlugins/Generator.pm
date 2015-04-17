use strict;

package CetSkelPlugins::Generator;

use vars qw(@ISA);

eval "use CetSkel::artdaq::PluginVersionInfo";
unless ($@) {
  push @ISA, "CetSkel::artdaq::PluginVersionInfo";
}

sub new {
  my $class = shift;
  my $self = { };
  return bless \$self, $class;
}

sub type { return "generator"; }

sub source { return __FILE__; }

sub baseClasses {
  return
    [
     { header => "artdaq/DAQdata/FragmentGenerator.h",
       class => "artdaq::FragmentGenerator",
       protection => "public" # Default.
     }
    ];
}

sub constructors {
  return [ {
            explicit => 1,
            args => [ "fhicl::ParameterSet const & p" ],
           } ];
}

sub defineMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DEFINE_ARTDAQ_GENERATOR(${qual_name})
EOF
}

sub implHeaders {
  return [ '"fhiclcpp/ParameterSet.h"' ];
}

sub macrosInclude {
  return "artdaq/DAQdata/GeneratorMacros.hh";
}

# No sub optionalEntries necessary.

sub pluginSuffix {
  return "_generator";
}

sub requiredEntries {
  return
    {
     getNext_ => "bool getNext_(FragmentPtrs & output) override"
    };
}

1;
