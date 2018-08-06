#include <mpi.h>
#include <unistd.h>

#include "FWCore/Framework/interface/stream/EDProducer.h"

// Configuration
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

class FetchProducer : public edm::stream::EDProducer<> {
public:
    explicit FetchProducer(const edm::ParameterSet &config);
    ~FetchProducer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void produce(edm::Event &event, edm::EventSetup const &setup) override;
};

FetchProducer::FetchProducer(const edm::ParameterSet &config) {
    produces<std::string>();
}

void FetchProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    MPI_Message message;
    MPI_Status status;
    int count;

    MPI_Mprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &message, &status);
    edm::LogInfo("FetchProducer")
            << status.MPI_TAG << " from node " << status.MPI_SOURCE;

    MPI_Get_count(&status, MPI_CHAR, &count);

    char *rec_buf = new char[count];
    MPI_Mrecv(static_cast<void *>(rec_buf), count, MPI_CHAR, &message, &status);
    edm::LogInfo("FetchProducer") << "Buffer received, value " << rec_buf;

    auto msg = std::make_unique<std::string>(rec_buf);
    event.put(std::move(msg));
}

void FetchProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<std::string>("mpi_message", "");
    descriptions.add("fetchProducer", desc);
}

DEFINE_FWK_MODULE(FetchProducer);
