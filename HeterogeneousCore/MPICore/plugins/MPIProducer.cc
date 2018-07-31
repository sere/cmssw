#include <unistd.h>
#include <mpi.h>

#include "FWCore/Framework/interface/stream/EDProducer.h"

// Configuration
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"


class MPIProducer: public edm::stream::EDProducer<> {
public:
	explicit MPIProducer(const edm::ParameterSet& config);
	~MPIProducer() override = default;
	static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
private:
	void produce(edm::Event& event, edm::EventSetup const& setup) override;

	std::string message_;
};

MPIProducer::MPIProducer(const edm::ParameterSet& config) :
	message_(config.getParameter<std::string>("mpi_message")) {

	produces<std::string>();
}

void MPIProducer::produce(edm::Event& event, edm::EventSetup const& setup) {
	MPI_Message message;
	MPI_Status status;	
	int count;

	MPI_Mprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &message,
                       &status);
	std::cout << "Probed tag " << status.MPI_TAG
		<< " from node " << status.MPI_SOURCE << std::endl;

	MPI_Get_count(&status, MPI_CHAR, &count);

	char *rec_buf = new char[count];
	MPI_Mrecv(static_cast<void *>(rec_buf), count, MPI_CHAR, &message,
			&status);
	std::cout << "Buffer received, value " << rec_buf << std::endl;
	message_ = rec_buf;
	auto msg = std::make_unique<std::string> (message_);
	event.put(std::move(msg));
}

void
MPIProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
	edm::ParameterSetDescription desc;
	desc.add<std::string>("mpi_message", "");
	descriptions.add("mpiProducer", desc);
}

DEFINE_FWK_MODULE(MPIProducer);
