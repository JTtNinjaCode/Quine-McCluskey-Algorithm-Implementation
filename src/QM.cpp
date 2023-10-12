#include <algorithm>
#include <unordered_set>

#include "QM.h"

static std::vector<std::unordered_set<std::string>> PITable;
static std::vector<std::unordered_set<std::string>> tempPITable;
static std::vector<std::unordered_set<std::string>> removePITable;
static std::unordered_set<std::string> essentialTerms;
static std::unordered_set<std::string> PISet;
static std::unordered_set<std::string> EPISet;
static std::vector<std::string> resultTerms;

static unsigned int countSetBits(unsigned int n);
static unsigned int countOne(const std::string &term);
static std::pair<bool, std::string> merge(const std::string &str1, const std::string &str2);
static bool isTermIncluded(const std::string &term1, const std::string &term2);
static void findPI();
static void findEPI();
static void Polynomial();
static void updateTermsInfo(TermsInfo &termsInfo);
static void initQM(const TermsInfo &termsInfo);
static void clearQM();

unsigned int countSetBits(unsigned int n) {
    unsigned int bitCount = 0;
    while (n) {
        if (n & 1)
            bitCount++;
        n >>= 1;
    }
    return bitCount;
}

unsigned int countOne(const std::string &term) {
    unsigned int counter = 0;
    for (auto c : term)
        if (c == '1')
            counter++;
    return counter;
}

std::pair<bool, std::string> merge(const std::string &str1, const std::string &str2) {
    unsigned int counter = 0;
    std::string resultString = str1;
    for (int i = 0; i < str1.size(); i++) {
        char c1 = str1[i], c2 = str2[i];
        if (c1 == c2) {
            continue;
        } else if (c1 == '-' && c2 != '-' || c1 != '-' && c2 == '-') {
            return std::make_pair(false, "");
        } else if (c1 == '1' && c2 == '0' || c1 == '0' && c2 == '1') {
            resultString[i] = '-';
            counter++;
        }
    }
    if (counter != 1)
        return std::make_pair(false, "");
    return std::make_pair(true, resultString);
}

bool isTermIncluded(const std::string &term1, const std::string &term2) {
    for (int i = 0; i < term1.size(); i++) {
        if (term1[i] == '1' && term2[i] == '0' || term1[i] == '0' && term2[i] == '1')
            return false;
    }
    return true;
}

void findPI() {
    bool stopFlag = false;
    while (stopFlag == false) {
        stopFlag = true;
        for (int i = 0; i < PITable.size() - 1; i++) {
            const auto &littleSet = PITable[i];
            const auto &bigSet = PITable[i + 1];
            for (const auto &term1 : littleSet) {
                for (const auto &term2 : bigSet) {
                    std::pair<bool, std::string> result = merge(term1, term2);
                    if (result.first) {  // if merge success
                        removePITable[i].insert(term1);
                        removePITable[i + 1].insert(term2);
                        tempPITable[i].insert(result.second);
                        stopFlag = false;
                    }
                }
            }
        }

        // remove old term
        for (int i = 0; i < removePITable.size(); i++) {
            for (auto e : removePITable[i])
                PITable[i].erase(e);
            removePITable[i].clear();
        }

        // add new term
        for (int i = 0; i < tempPITable.size(); i++) {
            for (auto e : tempPITable[i])
                PITable[i].insert(e);
            tempPITable[i].clear();
        }
    }
}

void findEPI() {
    for (const auto &minterm : essentialTerms) {
        std::string candidateEPI;
        unsigned int counter = 0;
        for (const auto &term : PISet) {
            if (isTermIncluded(term, minterm)) {
                counter++;
                candidateEPI = term;
                if (counter == 2)
                    break;
            }
        }
        if (counter == 1) EPISet.insert(candidateEPI);
    }
}

void Polynomial() {
    std::vector<std::string> table;
    table.reserve(PISet.size());
    for (const auto &pi : PISet)
        table.push_back(pi);

    std::vector<std::vector<unsigned int>> polyTerms;
    polyTerms.resize(essentialTerms.size());

    unsigned int index = 0;
    for (const auto &minterm : essentialTerms) {
        for (int i = 0; i < table.size(); i++) {
            if (isTermIncluded(table[i], minterm))
                polyTerms[index].push_back(1 << i);
        }
        index++;
    }

    std::unordered_set<uint64_t> result;
    result.insert(0);
    for (const auto &polyTerm : polyTerms) {
        std::unordered_set<uint64_t> temp;
        for (const auto &oldLiteral : result) {
            for (const auto &newLiteral : polyTerm)
                temp.insert(newLiteral | oldLiteral);
        }
        result = std::move(temp);
    }

    unsigned int minOneCount = UINT32_MAX;
    std::vector<std::pair<unsigned int, uint64_t>> countRecords;
    countRecords.reserve(result.size());

    for (auto term : result) {
        const auto& record = std::make_pair(countSetBits(term), term);
        countRecords.push_back(record);
        if (record.first < minOneCount)
            minOneCount = record.first;
    }

    index = 0;
    for (auto term : result) {
        if (countRecords[index].first == minOneCount) {
            uint64_t termPresentRecord = countRecords[index].second;
            unsigned int indexCounter = table.size() - 1;
            while (termPresentRecord) {
                if (termPresentRecord & 1)
                    resultTerms.push_back(table[indexCounter]);
                termPresentRecord >>= 1;
                indexCounter--;
            }
            break;
        }
        index++;
    }
}

void updateTermsInfo(TermsInfo &termsInfo) {
    unsigned int counter = 0;
    termsInfo.terms.clear();
    for (auto e : EPISet) {
        counter++;
        termsInfo.terms.push_back(e);
    }
    for (auto e : resultTerms) {
        counter++;
        termsInfo.terms.push_back(e);
    }
    termsInfo.termsCount = counter;
}

void initQM(const TermsInfo &termsInfo) {
    PITable.resize(termsInfo.inputCount + 1);
    tempPITable.resize(termsInfo.inputCount + 1);
    removePITable.resize(termsInfo.inputCount + 1);

    for (const auto &e : termsInfo.terms)
        essentialTerms.insert(e);
    for (const auto &e : termsInfo.terms) {
        unsigned int index = countOne(e);
        PITable[index].insert(e);
    }
    for (const auto &e : termsInfo.dontCareTerms) {
        unsigned int index = countOne(e);
        PITable[index].insert(e);
    }
}

void clearQM() {
    PITable.clear();
    tempPITable.clear();
    removePITable.clear();
    essentialTerms.clear();
    PISet.clear();
    EPISet.clear();
    resultTerms.clear();
}

void QM(TermsInfo &termsInfo) {
    initQM(termsInfo);
    findPI();

    // convert data structure
    for (int i = 0; i < PITable.size(); i++) {
        for (const auto &term : PITable[i]) {
            PISet.insert(term);
        }
    }

    findEPI();

    std::unordered_set<std::string> removeMinterms;
    for (const auto &i : EPISet) {
        PISet.erase(i);
        for (const auto &j : essentialTerms) {
            if (isTermIncluded(i, j)) removeMinterms.insert(j);
        }
    }
    for (const auto &e : removeMinterms)
        essentialTerms.erase(e);

    Polynomial();
    updateTermsInfo(termsInfo);
    clearQM();
}