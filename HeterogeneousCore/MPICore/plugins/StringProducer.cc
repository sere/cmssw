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

class StringProducer : public edm::stream::EDProducer<> {
public:
    explicit StringProducer(const edm::ParameterSet &config);
    ~StringProducer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void beginStream(edm::StreamID sid);
    void produce(edm::Event &event, edm::EventSetup const &setup) override;

    std::string message_;
    unsigned int sid_;
};

StringProducer::StringProducer(const edm::ParameterSet &config)
    : message_(config.getParameter<std::string>("message")) {

    produces<std::string>();
}

void StringProducer::beginStream(edm::StreamID sid) { sid_ = sid; }

void StringProducer::produce(edm::Event &event, edm::EventSetup const &setup) {
    auto msg = std::make_unique<std::string>(message_);
    *msg += " from sid ";
    *msg += std::to_string(sid_);
    usleep(1000000);
    event.put(std::move(msg));
}

void StringProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<std::string>("message", "");
    descriptions.add("stringProducer", desc);
}

DEFINE_FWK_MODULE(StringProducer);
