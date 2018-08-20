#include <mpi.h>
#include <unistd.h>

#include "constants.h"

#include "FWCore/Framework/interface/stream/EDProducer.h"

// Configuration
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

void call_cuda_kernel(std::vector<double> const &arrays,
                      std::vector<double> &result);
void callCudaFree();

class WorkProducer : public edm::stream::EDProducer<> {
public:
    explicit WorkProducer(const edm::ParameterSet &config);
    ~WorkProducer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void produce(edm::Event &event, edm::EventSetup const &setup) override;
    void addVector(std::vector<double> const &arrays,
                   std::vector<double> &result);
    edm::EDGetTokenT<std::vector<double>> token_;
    edm::EDGetTokenT<int> cpuIDToken_;
    edm::EDGetTokenT<std::map<std::string, double>> timesToken_;
    bool runOnGPU_;
};

WorkProducer::WorkProducer(const edm::ParameterSet &config)
    : token_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("job"))),
      cpuIDToken_(consumes<int>(config.getParameter<edm::InputTag>("cpuID"))),
      timesToken_(consumes<std::map<std::string, double>>(
              config.getParameter<edm::InputTag>("times"))),
      runOnGPU_(config.getParameter<bool>("runOnGPU")) {

    callCudaFree();

    produces<std::vector<double>>();
    produces<int>();
    produces<std::map<std::string, double>>();
}

void WorkProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> handle;
    edm::Handle<int> cpuIDHandle;
    edm::Handle<std::map<std::string, double>> timesHandle;
    event.getByToken(token_, handle);
    event.getByToken(cpuIDToken_, cpuIDHandle);
    event.getByToken(timesToken_, timesHandle);
    auto times = *timesHandle;

    times["preAllocRes"] = MPI_Wtime();
    auto result = std::make_unique<std::vector<double>>((*handle).size() / 2);
    times["algoStart"] = MPI_Wtime();
    if (runOnGPU_)
        call_cuda_kernel(*handle, *result);
    else
        addVector(*handle, *result);
    times["algoEnd"] = MPI_Wtime();

    auto timesUniquePtr =
            std::make_unique<std::map<std::string, double>>(times);
    event.put(std::move(result));
    std::unique_ptr<int> cpuID(new int);
    *cpuID = *cpuIDHandle;
    event.put(std::move(cpuID));
    event.put(std::move(timesUniquePtr));
}

void WorkProducer::addVector(std::vector<double> const &arrays,
                             std::vector<double> &result) {
    auto mid = arrays.begin() + arrays.size() / 2;
    std::transform(arrays.begin(), mid, mid, result.begin(),
                   std::plus<double>());
}

void WorkProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<bool>("runOnGPU", false);
    desc.add<edm::InputTag>("job", edm::InputTag());
    desc.add<edm::InputTag>("cpuID", edm::InputTag());
    desc.add<edm::InputTag>("times", edm::InputTag());
    descriptions.add("workProducer", desc);
}

DEFINE_FWK_MODULE(WorkProducer);
