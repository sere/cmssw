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
    edm::EDGetTokenT<std::map<std::string, double>> timesToken_;
};

SendAnalyzer::SendAnalyzer(const edm::ParameterSet &config)
    : token_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("result"))),
      timesToken_(consumes<std::map<std::string, double>>(
              config.getParameter<edm::InputTag>("times"))) {}

void SendAnalyzer::analyze(edm::Event const &event,
                           edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> handle;
    edm::Handle<std::map<std::string, double>> timesHandle;
    event.getByToken(token_, handle);
    event.getByToken(timesToken_, timesHandle);
    auto times = *timesHandle;

    int cpu_pe = 0;

    times["jobEnd"] = MPI_Wtime();
    MPI_Ssend(static_cast<void const *>((*handle).data()), (*handle).size(),
              MPI_DOUBLE, cpu_pe, WORKTAG, MPI_COMM_WORLD);
    times["sendResEnd"] = MPI_Wtime();

    std::vector<std::string> labels;
    std::vector<double> values;
    for (auto time : times) {
        labels.push_back(time.first);
        values.push_back(time.second);
    }
#if DEBUG
    for (unsigned int i = 0; i < labels.size(); i++) {
        std::cout << i << ": " << labels[i] << std::endl;
    }
#endif
    // MPI_Ssend(static_cast<void *>(labels.data()), sizeof(labels.data()),
    // MPI_CHAR,
    //           cpu_pe, 0, MPI_COMM_WORLD);

    MPI_Ssend(static_cast<void *>(values.data()), values.size(), MPI_DOUBLE,
              cpu_pe, 0, MPI_COMM_WORLD);
}

void SendAnalyzer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("result", edm::InputTag());
    desc.add<edm::InputTag>("times", edm::InputTag());
    descriptions.add("sendAnalyzer", desc);
}

DEFINE_FWK_MODULE(SendAnalyzer);
