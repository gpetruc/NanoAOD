#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"
#include "PhysicsTools/NanoAOD/interface/MergableCounterTable.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h"

#include <vector>
#include <iostream>
#include <regex>

namespace {
    struct Counter {
        /// Class to accumulate sums of weights, for systematics 
        MergableCounterTable::float_accumulator sum0;
        std::vector<MergableCounterTable::float_accumulator> sumPDF, sumScale, sumNamed;
        Counter() : sum0(0), sumPDF(), sumScale(), sumNamed() {}
        void clear() { 
            sum0 = 0; 
            sumPDF.clear();
            sumScale.clear();
            sumNamed.clear();
        }
        void inc(const Counter & other) { 
            sum0 += other.sum0;
            for (unsigned int i = 0, n = sumScale.size(); i < n; ++i) sumScale[i] += other.sumScale[i];
            for (unsigned int i = 0, n = sumPDF.size(); i < n; ++i) sumPDF[i] += other.sumPDF[i];
            for (unsigned int i = 0, n = sumNamed.size(); i < n; ++i) sumNamed[i] += other.sumNamed[i];
        }
        void inc(double w0, const std::vector<double> & wScale, const std::vector<double> & wPDF, const std::vector<double> & wNamed) {
            sum0 += w0;
            if (!wScale.empty()) {
                if (sumScale.empty()) sumScale.resize(wScale.size(), 0);
                for (unsigned int i = 0, n = wScale.size(); i < n; ++i) sumScale[i] += (w0 * wScale[i]);
            }
            if (!wPDF.empty()) {
                if (sumPDF.empty()) sumPDF.resize(wPDF.size(), 0);
                for (unsigned int i = 0, n = wPDF.size(); i < n; ++i) sumPDF[i] += (w0 * wPDF[i]);
            }
            if (!wNamed.empty()) {
                if (sumNamed.empty()) sumNamed.resize(wNamed.size(), 0);
                for (unsigned int i = 0, n = wNamed.size(); i < n; ++i) sumNamed[i] += (w0 * wNamed[i]);
            }
        }
    };
    struct DynamicWeightChoice {
        // choice of LHE weights
        // ---- scale ----
        std::vector<std::string> scaleWeightIDs; 
        std::string scaleWeightsDoc;
        // ---- pdf ----
        std::vector<std::string> pdfWeightIDs; 
        std::string pdfWeightsDoc;
    };
    struct ScaleVarWeight {
        std::string wid, label;
        std::pair<float,float> scales;
        ScaleVarWeight(const std::string & id, const std::string & text, const std::string & muR, const std::string & muF) :
            wid(id), label(text), scales(std::stof(muR), std::stof(muF)) {}
        bool operator<(const ScaleVarWeight & other) { return (scales == other.scales ? wid < other.wid : scales < other.scales); }
    };
    struct PDFSetWeights {
        std::vector<std::string> wids;
        std::pair<unsigned int,unsigned int> lhaIDs;
        PDFSetWeights(const std::string & wid, unsigned int lhaID) : wids(1,wid), lhaIDs(lhaID,lhaID) {}
        bool operator<(const PDFSetWeights & other) const { return lhaIDs < other.lhaIDs; }
        bool maybe_add(const std::string & wid, unsigned int lhaID) {
            if (lhaID == lhaIDs.second+1) {
                lhaIDs.second++;
                wids.push_back(wid);
                return true;
            } else {
                return false;
            }
        }
    };
}
class LHETablesProducer : public edm::global::EDProducer<edm::StreamCache<Counter>, edm::RunCache<DynamicWeightChoice>, edm::RunSummaryCache<Counter>, edm::EndRunProducer> {
    public:

        LHETablesProducer( edm::ParameterSet const & params ) :
            lheLabel_(params.getParameter<edm::InputTag>("lheInfo")),
            lheTag_(consumes<LHEEventProduct>(lheLabel_)),
            lheRunTag_(consumes<LHERunInfoProduct, edm::InRun>(lheLabel_)),
            preferredPDFLHAIDs_(params.getParameter<std::vector<uint32_t>>("preferredPDFs")),
            namedWeightIDs_(params.getParameter<std::vector<std::string>>("namedWeightIDs")),
            namedWeightLabels_(params.getParameter<std::vector<std::string>>("namedWeightLabels")),
            hasIssuedWarning_(false)
        {
            produces<FlatTable>("Objects");
            produces<FlatTable>("Weights");
            produces<FlatTable>("Scale");
            produces<FlatTable>("Pdf");
            produces<MergableCounterTable,edm::InRun>();
            if (namedWeightIDs_.size() != namedWeightLabels_.size()) {
                throw cms::Exception("Configuration", "Size mismatch between namedWeightIDs & namedWeightLabels");
            }
        }

        virtual ~LHETablesProducer() {}

        void produce(edm::StreamID id, edm::Event& iEvent, const edm::EventSetup& iSetup) const override {
            auto lheTab  = std::make_unique<FlatTable>(1, "LHE", true);
            auto lheWTab = std::make_unique<FlatTable>(1, "LHEWeights", true);
            std::unique_ptr<FlatTable> lheSTab, lhePTab; // these have a variable number of entries, so they must be created later

            edm::Handle<LHEEventProduct> lheInfo;
            if (iEvent.getByToken(lheTag_, lheInfo)) {
                fillLHEObjectTable(*lheInfo, *lheTab);
                const DynamicWeightChoice & weightChoice = *runCache(iEvent.getRun().index());
                fillLHEWeightTables(id, *lheInfo, weightChoice, *lheWTab, lheSTab, lhePTab); // need the id to increment the running sums
            } else {
                // make dummy values 
                lheSTab.reset(new FlatTable(1, "LHEScaleWeights", true));
                lhePTab.reset(new FlatTable(1, "LHEPdfWeights", true));
                if (!hasIssuedWarning_) {
                    edm::LogWarning("LHETablesProducer") << "No LHEEventProduct, so there will be no LHE Tables\n";
                    hasIssuedWarning_ = true;
                }
            }

            iEvent.put(std::move(lheTab), "Objects");
            iEvent.put(std::move(lheWTab), "Weights");
            iEvent.put(std::move(lheSTab), "Scale");
            iEvent.put(std::move(lhePTab), "Pdf");
        }

        void fillLHEObjectTable(const LHEEventProduct & lheProd, FlatTable & out) const {
            double lheHT = 0, lheHTIncoming = 0;
            unsigned int lheNj = 0, lheNb = 0, lheNc = 0, lheNuds = 0, lheNglu = 0;
            double lheVpt = 0;

            const auto & hepeup = lheProd.hepeup();
            const auto & pup = hepeup.PUP;
            int lep = -1, lepBar = -1, nu = -1, nuBar = -1;
            for (unsigned int i = 0, n = pup.size(); i  < n; ++i) {
                int status = hepeup.ISTUP[i];
                int idabs = std::abs(hepeup.IDUP[i]);
                if ( (status == 1) && ( ( idabs == 21 ) || (idabs > 0 && idabs < 7) ) ) { //# gluons and quarks
                    // object counters
                    lheNj++;
                    if (idabs==5) lheNb++;
                    else if (idabs==4) lheNc++;
                    else if (idabs <= 3 ) lheNuds++;
                    else if (idabs == 21) lheNglu++;
                    // HT
                    double pt = std::hypot( pup[i][0], pup[i][1] ); // first entry is px, second py
                    lheHT += pt;
                    int mothIdx = std::max(hepeup.MOTHUP[i].first-1,0); //first and last mother as pair; first entry has index 1 in LHE; incoming particles return motherindex 0
                    int mothIdxTwo = std::max(hepeup.MOTHUP[i].second-1,0);
                    int mothStatus  = hepeup.ISTUP[mothIdx];
                    int mothStatusTwo  = hepeup.ISTUP[mothIdxTwo];
                    bool hasIncomingAsMother = mothStatus<0 || mothStatusTwo<0;
                    if (hasIncomingAsMother) lheHTIncoming += pt;
                } else if (idabs == 12 || idabs == 14 || idabs == 16) {
                    (hepeup.IDUP[i] > 0 ? nu : nuBar) = i;
                } else if (idabs == 11 || idabs == 13 || idabs == 15) {
                    (hepeup.IDUP[i] > 0 ? lep : lepBar) = i;
                }
            }
            std::pair<int,int> v(0,0);
            if      (lep != -1 && lepBar != -1) v = std::make_pair(lep,lepBar);
            else if (lep != -1 &&  nuBar != -1) v = std::make_pair(lep, nuBar);
            else if (nu  != -1 && lepBar != -1) v = std::make_pair(nu ,lepBar);
            else if (nu  != -1 &&  nuBar != -1) v = std::make_pair(nu , nuBar);
            if (v.first != -1 && v.second != -1) {
                lheVpt = std::hypot( pup[v.first][0] + pup[v.second][0], pup[v.first][1] + pup[v.second][1] ); 
            }

            out.addColumnValue<uint8_t>("Njets", lheNj, "Number of jets (partons) at LHE step", FlatTable::UInt8Column);
            out.addColumnValue<uint8_t>("Nb",    lheNb, "Number of b partons at LHE step", FlatTable::UInt8Column);
            out.addColumnValue<uint8_t>("Nc",    lheNc, "Number of c partons at LHE step", FlatTable::UInt8Column);
            out.addColumnValue<uint8_t>("Nuds", lheNuds, "Number of u,d,s partons at LHE step", FlatTable::UInt8Column);
            out.addColumnValue<uint8_t>("Nglu", lheNglu, "Number of gluon partons at LHE step", FlatTable::UInt8Column);
            out.addColumnValue<float>("HT", lheHT, "HT, scalar sum of parton pTs at LHE step", FlatTable::FloatColumn);
            out.addColumnValue<float>("HTIncoming", lheHTIncoming, "HT, scalar sum of parton pTs at LHE step, restricted to partons", FlatTable::FloatColumn);
            out.addColumnValue<float>("Vpt", lheVpt, "pT of the W or Z boson at LHE step", FlatTable::FloatColumn);
        }

        void fillLHEWeightTables(edm::StreamID id, const LHEEventProduct & lheProd, const DynamicWeightChoice & weightChoice, FlatTable & out, std::unique_ptr<FlatTable> & outScale, std::unique_ptr<FlatTable> & outPdf ) const {
            const std::vector<std::string> & scaleWeightIDs = weightChoice.scaleWeightIDs;
            const std::vector<std::string> & pdfWeightIDs   = weightChoice.pdfWeightIDs;

            double w0 = lheProd.originalXWGTUP();
            out.addColumnValue<float>("originalXWGTUP", lheProd.originalXWGTUP(), "Nominal event weight in the LHE file", FlatTable::FloatColumn);

            std::vector<double> wScale(scaleWeightIDs.size(), 1), wPDF(pdfWeightIDs.size(), 1), wNamed(namedWeightIDs_.size(), 1);
            for (auto & weight : lheProd.weights()) {
                if (!hasIssuedWarning_) printf("Weight  %+9.5f   rel %+9.5f   for id %s\n", weight.wgt, weight.wgt/w0,  weight.id.c_str());
                // now we do it slowly, can be optimized
                auto mScale = std::find(scaleWeightIDs.begin(), scaleWeightIDs.end(), weight.id);
                if (mScale != scaleWeightIDs.end()) wScale[mScale-scaleWeightIDs.begin()] = weight.wgt/w0;

                auto mPDF = std::find(pdfWeightIDs.begin(), pdfWeightIDs.end(), weight.id);
                if (mPDF != pdfWeightIDs.end()) wPDF[mPDF-pdfWeightIDs.begin()] = weight.wgt/w0;

                auto mNamed = std::find(namedWeightIDs_.begin(), namedWeightIDs_.end(), weight.id);
                if (mNamed != namedWeightIDs_.end()) wNamed[mNamed-namedWeightIDs_.begin()] = weight.wgt/w0;
            } 

            // dumb addition now
            // named weights
            for (unsigned int i = 0, n = wNamed.size(); i < n; ++i) {
                out.addColumnValue<float>(namedWeightLabels_[i], wNamed[i], "LHE weight for id "+namedWeightIDs_[i]+", relative to originalXWGTUP", FlatTable::FloatColumn);
            }
            // vector of scale weights
            
            outScale.reset(new FlatTable(wScale.size(), "LHEScaleWeights", false));
            outScale->addColumn<float>("", wScale, weightChoice.scaleWeightsDoc, FlatTable::FloatColumn); 

            outPdf.reset(new FlatTable(wPDF.size(), "LHEPdfWeights", false));
            outPdf->addColumn<float>("", wPDF, weightChoice.pdfWeightsDoc, FlatTable::FloatColumn); 

            streamCache(id)->inc(w0, wScale, wPDF, wNamed);

            hasIssuedWarning_ = true;
        }

        // create an empty counter
        std::unique_ptr<Counter> beginStream(edm::StreamID) const override { 
            return std::make_unique<Counter>(); 
        }
        // inizialize to zero at begin run
        void streamBeginRun(edm::StreamID id, edm::Run const&, edm::EventSetup const&) const override { 
            streamCache(id)->clear(); 
        }
        // create an empty counter
        std::shared_ptr<DynamicWeightChoice> globalBeginRun(edm::Run const& iRun, edm::EventSetup const&) const override {
            edm::Handle<LHERunInfoProduct> lheInfo;

            auto weightChoice = std::make_shared<DynamicWeightChoice>();

            // getByToken throws since we're not in the endRun (see https://github.com/cms-sw/cmssw/pull/18499)
            //if (iRun.getByToken(lheRunTag_, lheInfo)) {
            if (iRun.getByLabel(lheLabel_, lheInfo)) { 
                std::vector<ScaleVarWeight> scaleVariationIDs;
                std::vector<PDFSetWeights>  pdfSetWeightIDs;
                
                std::regex weightgroup("<weightgroup\\s+combine=\"(.*)\"\\s+name=\"(.*)\"\\s*>");
                std::regex endweightgroup("</weightgroup>");
                std::regex scalew("<weight\\s+id=\"(\\d+)\">\\s*(muR=(\\S+)\\s+muF=(\\S+)(\\s+.*)?)</weight>");
                std::regex pdfw("<weight\\s+id=\"(\\d+)\">\\s*PDF set\\s*=\\s*(\\d+)\\s*</weight>");
                std::smatch groups;
                for (auto iter=lheInfo->headers_begin(), end = lheInfo->headers_end(); iter != end; ++iter) {
                    if (iter->tag() != "initrwgt") continue;
                    std::cout << "============= BEGIN [ " << iter->tag() << " ] ============ " << std::endl;
                    const std::vector<std::string> & lines = iter->lines();
                    for (unsigned int iLine = 0, nLines = lines.size(); iLine < nLines; ++iLine) {
                        std::cout << lines[iLine];
                        if (std::regex_search(lines[iLine], groups, weightgroup)) {
                            std::cout << ">>> Looks like the beginning of a weight group for " << groups.str(2) << std::endl;
                            if (groups.str(2) == "scale_variation") {
                                for ( ++iLine; iLine < nLines; ++iLine) {
                                    //std::cout << "    " << lines[iLine];
                                    if (std::regex_search(lines[iLine], groups, scalew)) {
                                        std::cout << "    >>> Scale weight " << groups[1].str() << " for " << groups[3].str() << " , " << groups[4].str() << " , " << groups[5].str() << std::endl;
                                        scaleVariationIDs.emplace_back(groups.str(1), groups.str(2), groups.str(3), groups.str(4));
                                    } else if (std::regex_search(lines[iLine], endweightgroup)) {
                                        //std::cout << ">>> Looks like the end of a weight group" << std::endl;
                                        break;
                                    } else if (std::regex_search(lines[iLine], weightgroup)) {
                                        //std::cout << ">>> Looks like the beginning of a new weight group, I will assume I missed the end of the group." << std::endl;
                                        --iLine; // rewind by one, and go back to the outer loop
                                        break;
                                    } else {
                                        std::cout << "    I do not know how to parse: " << lines[iLine];
                                    }
                                }
                            } else if (groups.str(2) == "PDF_variation") {
                                for ( ++iLine; iLine < nLines; ++iLine) {
                                    //std::cout << "    " << lines[iLine];
                                    if (std::regex_search(lines[iLine], groups, pdfw)) {
                                        unsigned int lhaID = std::stoi(groups.str(2));
                                        std::cout << "    >>> PDF weight " << groups.str(1) << " for " << groups.str(2) << " = " << lhaID << std::endl;
                                        if (pdfSetWeightIDs.empty() || ! pdfSetWeightIDs.back().maybe_add(groups.str(1),lhaID)) {
                                            pdfSetWeightIDs.emplace_back(groups.str(1),lhaID);
                                        }
                                    } else if (std::regex_search(lines[iLine], endweightgroup)) {
                                        //std::cout << ">>> Looks like the end of a weight group" << std::endl;
                                        break;
                                    } else if (std::regex_search(lines[iLine], weightgroup)) {
                                        //std::cout << ">>> Looks like the beginning of a new weight group, I will assume I missed the end of the group." << std::endl;
                                        --iLine; // rewind by one, and go back to the outer loop
                                        break;
                                    } else {
                                        std::cout << "    I do not know how to parse: " << lines[iLine];
                                    }
                                }
                            } else {
                                for ( ++iLine; iLine < nLines; ++iLine) {
                                    //std::cout << "    " << lines[iLine];
                                    if (std::regex_search(lines[iLine], groups, endweightgroup)) {
                                        //std::cout << ">>> Looks like the end of a weight group" << std::endl;
                                        break;
                                    } else if (std::regex_search(lines[iLine], weightgroup)) {
                                        //std::cout << ">>> Looks like the beginning of a new weight group, I will assume I missed the end of the group." << std::endl;
                                        --iLine; // rewind by one, and go back to the outer loop
                                        break;
                                    } else {
                                        std::cout << "    I do not know how to parse: " << lines[iLine];
                                    }
                                }
                            }
                        }
                    }
                    //std::cout << "============= END [ " << iter->tag() << " ] ============ \n\n" << std::endl;

                    // ----- SCALE VARIATIONS -----
                    std::sort(scaleVariationIDs.begin(), scaleVariationIDs.end());
                    //std::cout << "Found " << scaleVariationIDs.size() << " scale variations: " << std::endl;
                    std::stringstream scaleDoc("LHE scale variation weights (w_var / w_nominal); ");
                    for (unsigned int isw = 0, nsw = scaleVariationIDs.size(); isw <= nsw; ++isw) {
                        const auto & sw = scaleVariationIDs[isw];
                        if (isw) scaleDoc << "; ";
                        scaleDoc << "[" << isw << "] is " << sw.label;
                        weightChoice->scaleWeightIDs.push_back(sw.wid);
                        //printf("    id %s: scales ren = % .2f  fact = % .2f  text = %s\n", sw.wid.c_str(), sw.scales.first, sw.scales.second, sw.label.c_str());
                    }
                    if (!scaleVariationIDs.empty()) weightChoice->scaleWeightsDoc = scaleDoc.str();

                    // ------ PDF VARIATIONS (take the preferred one) -----
                    //std::sort(pdfSetWeightIDs.begin(), pdfSetWeightIDs.end());
                    //std::cout << "Found " << pdfSetWeightIDs.size() << " PDF set errors: " << std::endl;
                    //for (const auto & pw : pdfSetWeightIDs) printf("lhaIDs %6d - %6d (%3lu weights: %s, ... )\n", pw.lhaIDs.first, pw.lhaIDs.second, pw.wids.size(), pw.wids.front().c_str());
                    
                    std::stringstream pdfDoc("LHE pdf variation weights (w_var / w_nominal) for LHA IDs ");
                    bool found = false;
                    for (uint32_t lhaid : preferredPDFLHAIDs_) {
                        for (const auto & pw : pdfSetWeightIDs) {
                            if (pw.lhaIDs.first != lhaid) continue;
                            pdfDoc << pw.lhaIDs.first << " - " << pw.lhaIDs.second << std::endl;
                            weightChoice->pdfWeightIDs = pw.wids;
                            weightChoice->pdfWeightsDoc = pdfDoc.str(); 
                            found = true; break;
                        }
                        if (found) break;
                    }
                }
            }
            return weightChoice; 
        }
        // create an empty counter
        std::shared_ptr<Counter> globalBeginRunSummary(edm::Run const& iRun, edm::EventSetup const&) const override { 
            return std::make_shared<Counter>();
        }
        // add this stream to the summary
        void streamEndRunSummary(edm::StreamID id, edm::Run const&, edm::EventSetup const&, Counter* runCounter) const override { 
            runCounter->inc(*streamCache(id)); 
        }
        // nothing to do per se
        void globalEndRunSummary(edm::Run const&, edm::EventSetup const&, Counter* runCounter) const override { 
        }
        // write the total to the run 
        void globalEndRunProduce(edm::Run& iRun, edm::EventSetup const&, Counter const* runCounter) const override {
            auto out = std::make_unique<MergableCounterTable>();
            out->addFloat("LHEWeights_originalXWGTUP", "Sum of LHE originalXWGTUP", runCounter->sum0);
            //out->addInt(name_+"Count", "event count for "+doc_, runCounter->num);
            //out->addFloat(name_+"Sumw", "sum of weights for "+doc_, runCounter->sumw);
            //out->addFloat(name_+"Sumw2", "sum of (weight^2) for "+doc_, runCounter->sumw2);
            iRun.put(std::move(out));
        }
        // nothing to do here
        void globalEndRun(edm::Run const&, edm::EventSetup const&) const override { }
    protected:
        const edm::InputTag lheLabel_;
        const edm::EDGetTokenT<LHEEventProduct> lheTag_;
        const edm::EDGetTokenT<LHERunInfoProduct> lheRunTag_;

        std::vector<uint32_t> preferredPDFLHAIDs_;
        std::vector<std::string> namedWeightIDs_;
        std::vector<std::string> namedWeightLabels_;

        mutable bool hasIssuedWarning_;
};

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(LHETablesProducer);

