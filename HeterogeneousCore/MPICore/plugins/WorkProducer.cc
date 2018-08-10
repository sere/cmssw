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
    edm::EDGetTokenT<std::map<std::string, double>> timesToken_;
};

WorkProducer::WorkProducer(const edm::ParameterSet &config)
    : token_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("job"))),
      timesToken_(consumes<std::map<std::string, double>>(
              config.getParameter<edm::InputTag>("times"))) {

    callCudaFree();

    produces<std::vector<double>>();
    produces<std::map<std::string, double>>();
}

void WorkProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> handle;
    edm::Handle<std::map<std::string, double>> timesHandle;
    event.getByToken(token_, handle);
    event.getByToken(timesToken_, timesHandle);
    auto times = *timesHandle;

    std::vector<double> result(ARRAY_SIZE);
    times["algoStart"] = MPI_Wtime();
    // call_cuda_kernel(*handle, std::ref(result));
    addVector(*handle, std::ref(result));
    times["algoEnd"] = MPI_Wtime();

    auto msg = std::make_unique<std::vector<double>>(result);
    auto timesUniquePtr =
            std::make_unique<std::map<std::string, double>>(times);
    event.put(std::move(msg));
    event.put(std::move(timesUniquePtr));
}

void WorkProducer::addVector(std::vector<double> const &arrays,
                             std::vector<double> &result) {
    auto mid = arrays.begin() + arrays.size() / 2;
    std::transform(arrays.begin(), mid - 1, mid, result.begin(),
                   std::plus<double>());
}

void WorkProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("job", edm::InputTag());
    desc.add<edm::InputTag>("times", edm::InputTag());
    descriptions.add("workProducer", desc);
}

DEFINE_FWK_MODULE(WorkProducer);
