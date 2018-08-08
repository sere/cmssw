#include "constants.h"
#include <iostream>
#include <mpi.h>

#include "FWCore/Framework/interface/stream/EDAnalyzer.h"

// Configuration
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class SendAnalyzer : public edm::stream::EDAnalyzer<> {
public:
    explicit SendAnalyzer(const edm::ParameterSet &config);
    ~SendAnalyzer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void analyze(edm::Event const &event,
                 edm::EventSetup const &setup) override;

    edm::EDGetTokenT<std::vector<double>> token_;
};

SendAnalyzer::SendAnalyzer(const edm::ParameterSet &config)
    : token_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("result"))) {}

void SendAnalyzer::analyze(edm::Event const &event,
                           edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> handle;
    event.getByToken(token_, handle);

    int cpu_pe = 0;

    MPI_Send(static_cast<void const *>((*handle).data()),
             (*handle).size(), MPI_DOUBLE, cpu_pe, WORKTAG,
             MPI_COMM_WORLD);
}

void SendAnalyzer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("result", edm::InputTag());
    descriptions.add("sendAnalyzer", desc);
}

DEFINE_FWK_MODULE(SendAnalyzer);
