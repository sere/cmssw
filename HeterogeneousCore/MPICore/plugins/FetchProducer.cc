#include <mpi.h>
#include <unistd.h>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "HeterogeneousCore/MPICore/interface/WrapperHandle.h"
#include "constants.h"
#include "HeterogeneousCore/MPICore/interface/serialization.h"

class FetchProducer : public edm::stream::EDProducer<> {
public:
    explicit FetchProducer(const edm::ParameterSet &config);
    ~FetchProducer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void beginStream(edm::StreamID sid);
    void produce(edm::Event &event, edm::EventSetup const &setup) override;
    edm::EDPutToken vectorToken_;
    unsigned int sid_;
};

FetchProducer::FetchProducer(const edm::ParameterSet &config) {
    vectorToken_ = produces<std::vector<double>>();
    produces<int>();
    produces<int>("mpiTag");
    produces<std::map<std::string, double>>();
}

void FetchProducer::beginStream(edm::StreamID sid) { sid_ = sid; }

void FetchProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    MPI_Message message;
    MPI_Status status;
    int size;

#if DEBUG
    edm::LogPrint("FetchProducer")
            << "stream " << sid_ << " probing on ANY_TAG ";
#endif
    //------------fetch0------------
    MPI_Mprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &message, &status);
    //------------fetch1------------
#if DEBUG
    edm::LogPrint("FetchProducer")
            << "stream " << sid_ << " probed tag " << status.MPI_TAG;
#endif

    //    if (status.MPI_TAG == DIETAG) {
    //        MPI_Mrecv(0, 0, MPI_CHAR, &message, &status);
    //        MPI_Finalize();
    //        exit(0);
    //    }

    //------------fetch2------------
    MPI_Get_count(&status, MPI_CHAR, &size);

    std::map<std::string, double> times;
    io::unique_buffer write_buffer(size);
    MPI_Mrecv(write_buffer.data(), size, MPI_CHAR, &message, &status);
    //------------fetch3------------
    times["jobStart"] = MPI_Wtime();
#if DEBUG
    edm::LogPrint("FetchProducer")
            << "stream " << sid_ << " received with tag " << status.MPI_TAG;
#endif
    auto product = io::deserialize(write_buffer);

    auto timesUniquePtr =
            std::make_unique<std::map<std::string, double>>(times);
    event.put(vectorToken_, std::move(product));
    std::unique_ptr<int> offloaderID(new int);
    *offloaderID = status.MPI_SOURCE;
    event.put(std::move(offloaderID));
    std::unique_ptr<int> mpiTag(new int);
    *mpiTag = status.MPI_TAG;
    event.put(std::move(mpiTag), "mpiTag");
    event.put(std::move(timesUniquePtr));
}

void FetchProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    descriptions.add("fetchProducer", desc);
}

DEFINE_FWK_MODULE(FetchProducer);
