#include <iostream>
#include <mpi.h>

#include "constants.h"

#include "FWCore/Framework/interface/stream/EDProducer.h"

// Configuration
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class OffloadProducer : public edm::stream::EDProducer<> {
public:
    explicit OffloadProducer(const edm::ParameterSet &config);
    ~OffloadProducer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void produce(edm::Event &event, edm::EventSetup const &setup) override;

    edm::EDGetTokenT<std::string> token_;
};

OffloadProducer::OffloadProducer(const edm::ParameterSet &config)
    : token_(consumes<std::string>(
              config.getParameter<edm::InputTag>("msg_source"))) {

    produces<std::string>();
}

void OffloadProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    edm::Handle<std::string> handle;
    event.getByToken(token_, handle);

    int gpu_pe = 1;
    MPI_Send(static_cast<void const *>(handle.product()->data()),
             handle.product()->size(), MPI_CHAR, gpu_pe, WORKTAG,
             MPI_COMM_WORLD);
    edm::LogInfo("OffloadProducer") << "Buffer sent to node " << gpu_pe;

    MPI_Message message;
    MPI_Status status;
    MPI_Mprobe(gpu_pe, WORKTAG, MPI_COMM_WORLD, &message, &status);

    int count;
    MPI_Get_count(&status, MPI_CHAR, &count);
    std::cout << count << std::endl;

    char *rec_buf = new char[count];
    MPI_Mrecv(static_cast<void *>(rec_buf), count, MPI_CHAR, &message, &status);
    std::string recv = rec_buf;

    auto msg = std::make_unique<std::string>(*handle.product());
    *msg += " return value: ";
    *msg += recv;
    usleep(1000000);
    event.put(std::move(msg));
}

void OffloadProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("msg_source", edm::InputTag());
    descriptions.add("offloadProducer", desc);
}

DEFINE_FWK_MODULE(OffloadProducer);
