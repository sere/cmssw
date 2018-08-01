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

class CoutAnalyzer : public edm::stream::EDAnalyzer<> {
public:
    explicit CoutAnalyzer(const edm::ParameterSet &config);
    ~CoutAnalyzer() override = default;
    static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
    void analyze(edm::Event const &event,
                 edm::EventSetup const &setup) override;

    edm::EDGetTokenT<std::string> token_;
};

CoutAnalyzer::CoutAnalyzer(const edm::ParameterSet &config)
    : token_(consumes<std::string>(
              config.getParameter<edm::InputTag>("source"))) {}

void CoutAnalyzer::analyze(edm::Event const &event,
                           edm::EventSetup const &setup) {
    edm::Handle<std::string> handle;
    event.getByToken(token_, handle);
    edm::LogPrint("CoutAnalyzer") << *handle << std::endl;
    std::cout << "In CoutAnalyzer::analyze" << std::endl;
}

void CoutAnalyzer::fillDescriptions(
        edm::ConfigurationDescriptions &descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("source", edm::InputTag());
    descriptions.add("coutAnalyzer", desc);
}

DEFINE_FWK_MODULE(CoutAnalyzer);
