#include <iostream>
#include <mpi.h>

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "WrapperHandle.h"
#include "constants.h"
#include "serialization.h"

class OffloadProducer : public edm::stream::EDProducer<> {
public:
    explicit OffloadProducer(const edm::ParameterSet &config);
    ~OffloadProducer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void produce(edm::Event &event, edm::EventSetup const &setup) override;

    unsigned int eventNr_ = 0;
    edm::EDGetTokenT<std::vector<double>> token_;
    edm::EDPutTokenT<std::vector<double>> product_;
};

OffloadProducer::OffloadProducer(const edm::ParameterSet &config)
    : token_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("arrays"))) {

    product_ = produces<std::vector<double>>();
    produces<std::map<std::string, double>>();
}

void OffloadProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    edm::Handle<edm::WrapperBase> handle("std::vector<double>");
    event.getByToken(token_, handle);

    int mpiRank, mpiID;
    MPI_Comm_size(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiID);

    MPI_Group world_group, offloaderGroup;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    const int ranks[] = {0};
    MPI_Group_excl(world_group, 1, ranks, &offloaderGroup);
    MPI_Comm offloaderComm;
    MPI_Comm_create_group(MPI_COMM_WORLD, offloaderGroup, 0, &offloaderComm);

    std::map<std::string, double> times;

#if DEBUG
    edm::LogPrint("OffloaderProducer")
            << "id: " << mpiID << " sending work with tag " << WORKTAG + mpiID;
#endif
    times["eventNr"] = ++eventNr_;
    times["offloadStart"] = MPI_Wtime();
    auto buffer = serialize(*handle);
    MPI_Ssend(buffer.first.get(), buffer.second, 
              MPI_CHAR, workerPE, WORKTAG + mpiID,
              MPI_COMM_WORLD);
    times["sendJobEnd"] = MPI_Wtime();
#if DEBUG
    edm::LogPrint("OffloaderProducer")
            << "id: " << mpiID << " sent work with tag " << WORKTAG + mpiID;
#endif

    MPI_Message message;
    MPI_Status status;
#if DEBUG
    edm::LogPrint("OffloaderProducer")
            << "id: " << mpiID << " probing with tag " << WORKTAG + mpiID;
#endif
    MPI_Mprobe(workerPE, WORKTAG + mpiID, MPI_COMM_WORLD, &message, &status);

    int size;
    MPI_Get_count(&status, MPI_CHAR, &size);
    auto recv = std::make_unique<char[]>(size);
    MPI_Mrecv(recv.get(), size, MPI_CHAR, &message, &status);
    auto product = deserialize(recv.get(), size);
    times["offloadEnd"] = MPI_Wtime();
#if DEBUG
    edm::LogPrint("OffloaderProducer")
            << "id: " << mpiID << " received with tag " << WORKTAG + mpiID;
#endif

    int count;
    MPI_Mprobe(workerPE, mpiID, MPI_COMM_WORLD, &message, &status);
    MPI_Get_count(&status, MPI_DOUBLE, &count);
    std::vector<double> values(count);
    MPI_Mrecv(static_cast<void *>(values.data()), count, MPI_DOUBLE, &message,
              &status);

    for (unsigned int i = 0; i < values.size(); i++) {
        times[std::to_string(i)] = values[i];
    }
    auto timesUniquePtr = std::make_unique<std::map<std::string, double>>(times);
    event.put(product_, std::move(product));
    event.put(std::move(timesUniquePtr));
}

void OffloadProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("arrays", edm::InputTag());
    descriptions.add("offloadProducer", desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(OffloadProducer);
