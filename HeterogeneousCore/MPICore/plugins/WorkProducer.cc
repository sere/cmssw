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
};

WorkProducer::WorkProducer(const edm::ParameterSet &config)
    : token_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("job"))) {

    produces<std::vector<double>>();
}

void WorkProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> handle;
    event.getByToken(token_, handle);

    std::vector<double> result(ARRAY_SIZE);
    std::vector<double> resultc(ARRAY_SIZE);
    call_cuda_kernel(*handle, std::ref(resultc));
    addVector(*handle, std::ref(result));

    auto msg = std::make_unique<std::vector<double>>(result);
    usleep(1000000);
    event.put(std::move(msg));
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
    descriptions.add("workProducer", desc);
}

DEFINE_FWK_MODULE(WorkProducer);
