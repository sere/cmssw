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

    edm::EDGetTokenT<std::vector<double>> token_;
};

OffloadProducer::OffloadProducer(const edm::ParameterSet &config)
    : token_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("arrays"))) {

    produces<std::vector<double>>();
}

void OffloadProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> handle;
    event.getByToken(token_, handle);

    int gpu_pe = 1;
    double starttime, endtime;
    starttime = MPI_Wtime();
    MPI_Send(static_cast<void const *>(handle.product()->data()),
             handle.product()->size(), MPI_DOUBLE, gpu_pe, WORKTAG,
             MPI_COMM_WORLD);
    endtime = MPI_Wtime();
    edm::LogInfo("OffloadProducer")
            << "Buffer sent to node " << gpu_pe << " in " << endtime - starttime
            << " seconds.";

    MPI_Message message;
    MPI_Status status;
    MPI_Mprobe(gpu_pe, WORKTAG, MPI_COMM_WORLD, &message, &status);

    int count;
    MPI_Get_count(&status, MPI_DOUBLE, &count);

    std::vector<double> recv(count);
    assert(count == ARRAY_SIZE);
    MPI_Mrecv(static_cast<void *>(recv.data()), count, MPI_DOUBLE, &message,
              &status);

    auto msg = std::make_unique<std::vector<double>>(recv);
    event.put(std::move(msg));
}

void OffloadProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("arrays", edm::InputTag());
    descriptions.add("offloadProducer", desc);
}

DEFINE_FWK_MODULE(OffloadProducer);
