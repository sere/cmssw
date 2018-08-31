#include "constants.h"
#include <iostream>
#include <random>
#include <unistd.h>
#include <vector>

#include "FWCore/Framework/interface/stream/EDProducer.h"

// Configuration
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

class ArraysProducer : public edm::stream::EDProducer<> {
public:
    explicit ArraysProducer(const edm::ParameterSet &config);
    ~ArraysProducer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    int vectorLength_;
    std::vector<double> arrays_;
    void produce(edm::Event &event, edm::EventSetup const &setup) override;
};

ArraysProducer::ArraysProducer(const edm::ParameterSet &config)
    : vectorLength_(config.getParameter<int>("vectorLength")),
      arrays_(vectorLength_ * 2) {

    std::uniform_real_distribution<double> distribution(-1000.f, 1000.f);
    std::random_device dev;
    std::default_random_engine generator{dev()};
    auto rand = std::bind(distribution, std::ref(generator));
    std::generate(arrays_.begin(), arrays_.end(), rand);

#if DEBUG
    edm::LogPrint("ArraysProducer") << "First array:";
    for (int i = 0; i < std::min(10, vectorLength_); i++)
        edm::LogPrint("ArraysProducer") << arrays_[i];
    edm::LogPrint("ArrayProducer");
    edm::LogPrint("ArraysProducer") << "Second array:";
    for (int i = 0; i < std::min(10, vectorLength_); i++)
        edm::LogPrint("ArraysProducer") << arrays_[i + vectorLength_];
#endif
    produces<std::vector<double>>();
}

void ArraysProducer::produce(edm::Event &event, edm::EventSetup const &setup) {

    auto msg = std::make_unique<std::vector<double>>(arrays_);
    event.put(std::move(msg));
}

void ArraysProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<int>("vectorLength", 1);
    descriptions.add("arraysProducer", desc);
}

DEFINE_FWK_MODULE(ArraysProducer);
