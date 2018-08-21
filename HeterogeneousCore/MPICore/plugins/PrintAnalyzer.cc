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

class PrintAnalyzer : public edm::stream::EDAnalyzer<edm::GlobalCache<void>> {
public:
    explicit PrintAnalyzer(const edm::ParameterSet &config);
    static std::shared_ptr<void>
    initializeGlobalCache(edm::ParameterSet const &) {
        return std::shared_ptr<void>();
    }
    ~PrintAnalyzer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
    static void globalEndJob(void const *);

private:
    void analyze(edm::Event const &event,
                 edm::EventSetup const &setup) override;

    edm::EDGetTokenT<std::vector<double>> token_;
    edm::EDGetTokenT<std::map<std::string, double>> timesToken_;
};

PrintAnalyzer::PrintAnalyzer(const edm::ParameterSet &config)
    : token_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("result"))),
      timesToken_(consumes<std::map<std::string, double>>(
              config.getParameter<edm::InputTag>("times"))) {}

void PrintAnalyzer::analyze(edm::Event const &event,
                            edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> handle;
    edm::Handle<std::map<std::string, double>> timesHandle;
    event.getByToken(token_, handle);
    event.getByToken(timesToken_, timesHandle);
    auto times = *timesHandle;

    // for (auto time : (*timesHandle)) {
    //     edm::LogPrint("PrintAnalyzer") << time.first << " " << time.second;
    // }
    // std::cout << "algo, job, offload, send job, send answer, resAlloc";

    std::cout << times["0"] - times["1"] << ", " << times["2"] - times["3"]
              << ", " << times["offloadEnd"] - times["offloadStart"] << ", "
              << times["sendJobEnd"] - times["offloadStart"] << ", "
              << times["5"] - times["2"] << ", " << times["1"] - times["4"]
              << std::endl;
#if DEBUG
    edm::LogPrint("PrintAnalyzer")
            << "Result array; dimension " << (*handle).size();
    for (int i = 0; i < std::min(10, (int)(*handle).size()); i++) {
        edm::LogPrint("PrintAnalyzer") << (*handle)[i];
    }
#endif
}

void PrintAnalyzer::globalEndJob(void const* Void) {
    const int gpu_pe = 1;
    MPI_Ssend(0, 0, MPI_CHAR, gpu_pe, DIETAG, MPI_COMM_WORLD);
}

void PrintAnalyzer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("result", edm::InputTag());
    desc.add<edm::InputTag>("times", edm::InputTag());
    descriptions.add("printAnalyzer", desc);
}

DEFINE_FWK_MODULE(PrintAnalyzer);
