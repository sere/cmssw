#include "constants.h"
#include <random>
#include <unistd.h>
#include <iostream>
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
    std::vector<double> arrays_;
    void produce(edm::Event &event, edm::EventSetup const &setup) override;
};

ArraysProducer::ArraysProducer(const edm::ParameterSet &config)
    : arrays_(MATR_SIZE) {
    std::uniform_real_distribution<double> distribution(-1000.f, 1000.f);
    std::default_random_engine generator;
    auto rand = std::bind(distribution, std::ref(generator));
    std::generate(arrays_.begin(), arrays_.end(), rand);

    produces<std::vector<double>>();
}

void ArraysProducer::produce(edm::Event &event, edm::EventSetup const &setup) {

    auto msg = std::make_unique<std::vector<double>>(arrays_);
    event.put(std::move(msg));
}

void ArraysProducer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    descriptions.add("arraysProducer", desc);
}

DEFINE_FWK_MODULE(ArraysProducer);
