#include <mpi.h>
#include <unistd.h>

#include <chrono>
#include <thread>

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

    edm::EDGetTokenT<std::string> token_;
};

WorkProducer::WorkProducer(const edm::ParameterSet &config)
    : token_(consumes<std::string>(config.getParameter<edm::InputTag>("job"))) {

    produces<std::string>();
}

void WorkProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    edm::Handle<std::string> handle;
    event.getByToken(token_, handle);

    int mpi_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_id);
    std::string result = "mpi_id ";
    result += std::to_string(mpi_id);

    std::this_thread::sleep_for(
            std::chrono::milliseconds((std::rand() % 100 + 1) * 10));

    auto msg = std::make_unique<std::string>(result);
    event.put(std::move(msg));
}

void WorkProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<std::string>("job", "");
    descriptions.add("workProducer", desc);
}

DEFINE_FWK_MODULE(WorkProducer);
