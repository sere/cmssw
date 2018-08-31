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

    edm::EDGetTokenT<std::vector<double>> vectorToken_;
    edm::EDGetTokenT<std::map<std::string, double>> timesToken_;
};

PrintAnalyzer::PrintAnalyzer(const edm::ParameterSet &config)
    : vectorToken_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("result"))),
      timesToken_(consumes<std::map<std::string, double>>(
              config.getParameter<edm::InputTag>("times"))) {}

void PrintAnalyzer::analyze(edm::Event const &event,
                            edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> vectorHandle;
    edm::Handle<std::map<std::string, double>> timesHandle;
    event.getByToken(vectorToken_, vectorHandle);
    event.getByToken(timesToken_, timesHandle);
    auto times = *timesHandle;

    // for (auto time : (*timesHandle)) {
    //     edm::LogPrint("PrintAnalyzer") << time.first << " " << time.second;
    // }
    // std::cout << "algo, job, offload, send job, send answer, resAlloc";

    std::cout << times["eventNr"] << ", " << times["0"] - times["1"] << ", "
              << times["2"] - times["3"] << ", "
              << times["offloadEnd"] - times["offloadStart"] << ", "
              << times["sendJobEnd"] - times["offloadStart"] << ", "
              << times["5"] - times["2"] << ", " << times["1"] - times["4"]
              << std::endl;
#if DEBUG
    edm::LogPrint("PrintAnalyzer")
            << "Result array; dimension " << (*vectorHandle).size();
    for (int i = 0; i < std::min(10, (int)(*vectorHandle).size()); i++) {
        edm::LogPrint("PrintAnalyzer") << (*vectorHandle)[i];
    }
#endif
}

void PrintAnalyzer::globalEndJob(void const *Void) {
    //    int mpiRank, mpiID;
    //    MPI_Comm_size(MPI_COMM_WORLD, &mpiRank);
    //    MPI_Comm_rank(MPI_COMM_WORLD, &mpiID);
    //
    //    MPI_Group world_group, offloaderGroup;
    //    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    //    const int ranks[] = {0};
    //    MPI_Group_excl(world_group, 1, ranks, &offloaderGroup);
    //    MPI_Comm offloaderComm;
    //    MPI_Comm_create_group(MPI_COMM_WORLD, offloaderGroup, 0, &offloaderComm);
    //
    //    MPI_Barrier(offloaderComm);
    //
    //    if (mpiID == mpiRank - 1) {
    //        MPI_Ssend(0, 0, MPI_CHAR, workerPE, DIETAG, MPI_COMM_WORLD);
    //    }
}

void PrintAnalyzer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("result", edm::InputTag());
    desc.add<edm::InputTag>("times", edm::InputTag());
    descriptions.add("printAnalyzer", desc);
}

DEFINE_FWK_MODULE(PrintAnalyzer);
