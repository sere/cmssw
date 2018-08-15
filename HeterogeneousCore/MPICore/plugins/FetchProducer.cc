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

class FetchProducer : public edm::stream::EDProducer<> {
public:
    explicit FetchProducer(const edm::ParameterSet &config);
    ~FetchProducer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void produce(edm::Event &event, edm::EventSetup const &setup) override;
};

FetchProducer::FetchProducer(const edm::ParameterSet &config) {
    produces<std::vector<double>>();
    produces<std::map<std::string, double>>();
}

void FetchProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    MPI_Message message;
    MPI_Status status;
    int count;

    MPI_Mprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &message, &status);

    MPI_Get_count(&status, MPI_DOUBLE, &count);

    std::vector<double> rec_buf(count);
    std::map<std::string, double> times;
    MPI_Mrecv(static_cast<void *>(rec_buf.data()), count, MPI_DOUBLE, &message,
              &status);
    times["jobStart"] = MPI_Wtime();

    auto msg = std::make_unique<std::vector<double>>(rec_buf);
    auto timesUniquePtr =
            std::make_unique<std::map<std::string, double>>(times);
    event.put(std::move(msg));
    event.put(std::move(timesUniquePtr));
}

void FetchProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    descriptions.add("fetchProducer", desc);
}

DEFINE_FWK_MODULE(FetchProducer);
