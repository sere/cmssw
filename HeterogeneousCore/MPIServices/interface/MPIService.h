// -*- C++ -*-

#include <cassert>

#include <mpi.h>

#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/ActivityRegistry.h"

class MPIService {
public:
  MPIService(edm::ParameterSet const& config, edm::ActivityRegistry & registry);
  ~MPIService();

  static void fillDescriptions(edm::ConfigurationDescriptions & descriptions);
};
