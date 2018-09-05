#include <iostream>
#include <mpi.h>

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "HeterogeneousCore/MPICore/interface/WrapperHandle.h"
#include "constants.h"
#include "HeterogeneousCore/MPICore/interface/serialization.h"

class SendAnalyzer : public edm::stream::EDAnalyzer<> {
public:
    explicit SendAnalyzer(const edm::ParameterSet &config);
    ~SendAnalyzer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void beginStream(edm::StreamID sid);
    void analyze(edm::Event const &event,
                 edm::EventSetup const &setup) override;

    unsigned int sid_;
    edm::EDGetTokenT<std::vector<double>> vectorToken_;
    edm::EDGetTokenT<int> offloaderIDToken_;
    edm::EDGetTokenT<int> mpiTagToken_;
    edm::EDGetTokenT<std::map<std::string, double>> timesToken_;
};

SendAnalyzer::SendAnalyzer(const edm::ParameterSet &config)
    : vectorToken_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("result"))),
      offloaderIDToken_(consumes<int>(config.getParameter<edm::InputTag>("offloaderID"))),
      mpiTagToken_(consumes<int>(config.getParameter<edm::InputTag>("mpiTag"))),
      timesToken_(consumes<std::map<std::string, double>>(
              config.getParameter<edm::InputTag>("times"))) {}

void SendAnalyzer::beginStream(edm::StreamID sid) { sid_ = sid; }

void SendAnalyzer::analyze(edm::Event const &event,
                           edm::EventSetup const &setup) {
    edm::Handle<edm::WrapperBase> vectorHandle("std::vector<double>");
    edm::Handle<int> offloaderIDHandle, mpiTagHandle;
    edm::Handle<std::map<std::string, double>> timesHandle;
    event.getByToken(vectorToken_, vectorHandle);
    auto buffer = io::serialize(*vectorHandle);
    event.getByToken(offloaderIDToken_, offloaderIDHandle);
    event.getByToken(mpiTagToken_, mpiTagHandle);
    event.getByToken(timesToken_, timesHandle);
    auto times = *timesHandle;

#if DEBUG
    edm::LogPrint("SendAnalyzer")
            << "send sends tag " << *mpiTagHandle << ", stream " << sid_
            << " and offloaderID " << *offloaderIDHandle;
#endif
    times["jobEnd"] = MPI_Wtime();
    MPI_Ssend(buffer.data(), buffer.size(),
              MPI_CHAR, *offloaderIDHandle, *mpiTagHandle,
              MPI_COMM_WORLD);
    times["sendResEnd"] = MPI_Wtime();
#if DEBUG
    edm::LogPrint("SendAnalyzer")
            << "stream " << sid_ << " finished sending data";
#endif

    std::vector<std::string> labels;
    std::vector<double> values;
    for (auto time : times) {
        labels.push_back(time.first);
        values.push_back(time.second);
    }
#if DEBUG
    for (unsigned int i = 0; i < labels.size(); i++) {
        edm::LogPrint("SendAnalyzer") << i << ": " << labels[i];
    }
#endif
    // MPI_Ssend(static_cast<void *>(labels.data()), sizeof(labels.data()),
    // MPI_CHAR,
    //           *offloaderIDHandle, 0, MPI_COMM_WORLD);
#if DEBUG
    edm::LogPrint("SendAnalyzer")
            << "send sends times " << *mpiTagHandle << ", stream " << sid_
            << " and offloaderID " << *offloaderIDHandle;
#endif
    MPI_Ssend(static_cast<void const *>(values.data()), values.size(),
              MPI_DOUBLE, *offloaderIDHandle, *offloaderIDHandle, MPI_COMM_WORLD);
#if DEBUG
    edm::LogPrint("SendAnalyzer")
            << "stream " << sid_ << " finished sending times";
#endif
}

void SendAnalyzer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("result", edm::InputTag());
    desc.add<edm::InputTag>("offloaderID", edm::InputTag());
    desc.add<edm::InputTag>("mpiTag", edm::InputTag());
    desc.add<edm::InputTag>("times", edm::InputTag());
    descriptions.add("sendAnalyzer", desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(SendAnalyzer);
