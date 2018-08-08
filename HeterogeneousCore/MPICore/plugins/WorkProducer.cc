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

class WorkProducer : public edm::stream::EDProducer<> {
public:
    explicit WorkProducer(const edm::ParameterSet &config);
    ~WorkProducer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void produce(edm::Event &event, edm::EventSetup const &setup) override;
    edm::EDGetTokenT<std::vector<double>> token_;
    std::vector<double> result_;
};

WorkProducer::WorkProducer(const edm::ParameterSet &config)
    : token_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("job"))),
      result_(ARRAY_SIZE) {

    produces<std::vector<double>>();
}

void WorkProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> handle;
    event.getByToken(token_, handle);

    auto mid = (*handle).begin() + (*handle).size() / 2;
    std::transform((*handle).begin(), mid - 1, mid, result_.begin(),
                   std::plus<double>());

    auto msg = std::make_unique<std::vector<double>>(result_);
    usleep(1000000);
    event.put(std::move(msg));
}

void WorkProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("job", edm::InputTag());
    descriptions.add("workProducer", desc);
}

DEFINE_FWK_MODULE(WorkProducer);
