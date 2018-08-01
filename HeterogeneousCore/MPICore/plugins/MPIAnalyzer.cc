#include <iostream>
#include <mpi.h>

#include "FWCore/Framework/interface/stream/EDAnalyzer.h"

// Configuration
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class MPIAnalyzer : public edm::stream::EDAnalyzer<> {
public:
    explicit MPIAnalyzer(const edm::ParameterSet &config);
    ~MPIAnalyzer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void analyze(edm::Event const &event,
                 edm::EventSetup const &setup) override;

    edm::EDGetTokenT<std::string> token_;
};

MPIAnalyzer::MPIAnalyzer(const edm::ParameterSet &config)
    : token_(consumes<std::string>(
              config.getParameter<edm::InputTag>("msg_source"))) {}

void MPIAnalyzer::analyze(edm::Event const &event,
                          edm::EventSetup const &setup) {
    edm::Handle<std::string> handle;
    event.getByToken(token_, handle);
    MPI_Send(static_cast<void const *>(handle.product()->data()),
             handle.product()->size(), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
    std::cout << "Buffer sent" << std::endl;
}

void MPIAnalyzer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("msg_source", edm::InputTag());
    descriptions.add("mpiAnalyzer", desc);
}

DEFINE_FWK_MODULE(MPIAnalyzer);
