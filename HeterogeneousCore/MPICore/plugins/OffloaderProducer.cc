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
    produces<std::map<std::string, double>>();
}

void OffloadProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> handle;
    event.getByToken(token_, handle);

    int gpu_pe = 1;
    std::map<std::string, double> times;
    times["offloadStart"] = MPI_Wtime();
    MPI_Ssend(static_cast<void const *>(handle.product()->data()),
              handle.product()->size(), MPI_DOUBLE, gpu_pe, WORKTAG,
              MPI_COMM_WORLD);
    times["sendJobEnd"] = MPI_Wtime();

    MPI_Message message;
    MPI_Status status;
    MPI_Mprobe(gpu_pe, WORKTAG, MPI_COMM_WORLD, &message, &status);

    int count;
    MPI_Get_count(&status, MPI_DOUBLE, &count);

    std::vector<double> recv(count);
    assert(count == ARRAY_SIZE);
    MPI_Mrecv(static_cast<void *>(recv.data()), count, MPI_DOUBLE, &message,
              &status);
    times["offloadEnd"] = MPI_Wtime();

    // MPI_Mprobe(gpu_pe, 0, MPI_COMM_WORLD, &message, &status);
    // MPI_Get_count(&status, MPI_CHAR, &count);
    // std::vector<std::string> labels(count);
    // MPI_Mrecv(static_cast<void *>(labels.data()), count, MPI_CHAR, &message,
    //           &status);

    MPI_Mprobe(gpu_pe, 0, MPI_COMM_WORLD, &message, &status);
    MPI_Get_count(&status, MPI_DOUBLE, &count);
    std::vector<double> values(count);
    MPI_Mrecv(static_cast<void *>(values.data()), count, MPI_DOUBLE, &message,
              &status);

    for (unsigned int i = 0; i < values.size(); i++) {
        times[std::to_string(i)] = values[i];
    }
    auto msg = std::make_unique<std::vector<double>>(recv);
    auto timesUniquePtr =
            std::make_unique<std::map<std::string, double>>(times);
    event.put(std::move(msg));
    event.put(std::move(timesUniquePtr));
}

void OffloadProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("arrays", edm::InputTag());
    descriptions.add("offloadProducer", desc);
}

DEFINE_FWK_MODULE(OffloadProducer);
