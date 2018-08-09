#include <iostream>

#include "FWCore/Framework/interface/stream/EDAnalyzer.h"

// Configuration
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class PrintAnalyzer : public edm::stream::EDAnalyzer<> {
public:
    explicit PrintAnalyzer(const edm::ParameterSet &config);
    ~PrintAnalyzer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void analyze(edm::Event const &event,
                 edm::EventSetup const &setup) override;

    edm::EDGetTokenT<std::vector<double>> token_;
};

PrintAnalyzer::PrintAnalyzer(const edm::ParameterSet &config)
    : token_(consumes<std::vector<double>>(
              config.getParameter<edm::InputTag>("result"))) {}

void PrintAnalyzer::analyze(edm::Event const &event,
                           edm::EventSetup const &setup) {
    edm::Handle<std::vector<double>> handle;
    event.getByToken(token_, handle);
    edm::LogPrint("PrintAnalyzer") << "received results" << std::endl;
}

void PrintAnalyzer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("result", edm::InputTag());
    descriptions.add("printAnalyzer", desc);
}

DEFINE_FWK_MODULE(PrintAnalyzer);
