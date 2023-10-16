#include <algorithm>
#include <unordered_set>

#include "QM.h"

// used for find PI
static std::vector<std::unordered_set<std::string>> PITable;
static std::vector<std::unordered_set<std::string>> tempPITable;
static std::vector<std::unordered_set<std::string>> removePITable;

// used for find EPI
static std::unordered_set<std::string> PISet;
static std::unordered_set<std::string> EPISet;
static std::unordered_set<std::string> essentialTerms;

static std::vector<std::string> termTable;
static std::vector<std::string> resultTerms;
static unsigned int literalCount = 0;

static unsigned int countLiterals(const std::string &termPresent);
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

static unsigned int countLiterals(const std::string &termPresent) {
    unsigned int result = 0;
    for (char c : termPresent)
    {
        if (c != '-')
            result++;
    }
    return result;
}

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
    termTable.reserve(PISet.size());
    for (const auto &pi : PISet)
        termTable.push_back(pi);

    std::vector<std::vector<unsigned int>> polyTerms;
    polyTerms.resize(essentialTerms.size());

    unsigned int index = 0;
    for (const auto &minterm : essentialTerms) {
        for (int i = 0; i < termTable.size(); i++) {
            if (isTermIncluded(termTable[i], minterm))
                polyTerms[index].push_back(1 << i);
        }
        index++;
    }

    std::vector<uint64_t> result;
    result.push_back(0);
    for (const auto &polyTerm : polyTerms) {
        std::vector<uint64_t> temp;
        for (const auto &oldLiteral : result) {
            for (const auto &newLiteral : polyTerm)
                temp.push_back(newLiteral | oldLiteral);
        }
        result = std::move(temp);
    }
    unsigned int minOneCount = UINT32_MAX;
    std::vector<std::pair<unsigned int, uint64_t>> countOneRecords;
    countOneRecords.reserve(result.size());
    for (auto term : result) {
        const auto& oneRecord = std::make_pair(countSetBits(term), term);
        countOneRecords.push_back(oneRecord);
        if (oneRecord.first < minOneCount)
            minOneCount = oneRecord.first;
    }

    unsigned int minLiteralCount = UINT32_MAX;
    std::vector<std::pair<unsigned int, uint64_t>> countLiteralRecords;
    for (int i = 0; i < result.size(); i++)
    {
        if (countOneRecords[i].first == minOneCount) {
            unsigned int literals = 0;

            uint64_t record = countOneRecords[i].second;
            unsigned int index = 0;
            while (record) {
                if (record & 1)
                    literals += countLiterals(termTable[index]);
                record >>= 1;
                index++;
            }

            if (literals < minLiteralCount) {
                minLiteralCount = literals;
                countLiteralRecords.push_back(std::make_pair(literals, countOneRecords[i].second));
            }
        }
    }
    for (auto &record : countLiteralRecords)
    {
        if (record.first == minLiteralCount) {
            uint64_t termPresentRecord = record.second;
            unsigned int indexCounter = 0;
            while (termPresentRecord) {
                if (termPresentRecord & 1)
                    resultTerms.push_back(termTable[indexCounter]);
                termPresentRecord >>= 1;
                indexCounter++;
            }
            break;
        }
    }
}

void updateTermsInfo(TermsInfo &termsInfo) {
    unsigned int counter = 0;
    termsInfo.terms.clear();
    for (auto e : EPISet) {
        counter++;
        literalCount += countLiterals(e);
        termsInfo.terms.push_back(e);
    }
    for (auto e : resultTerms) {
        counter++;
        literalCount += countLiterals(e);
        termsInfo.terms.push_back(e);
    }
    termsInfo.termsCount = counter;
}

void initQM(const TermsInfo &termsInfo) {
    PITable.clear();
    tempPITable.clear();
    removePITable.clear();
    essentialTerms.clear();
    PISet.clear();
    EPISet.clear();
    resultTerms.clear();
    termTable.clear();
    literalCount = 0;

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

unsigned int QM(TermsInfo &termsInfo) {
    initQM(termsInfo);
    findPI();
    // convert data structure
    for (auto &table: PITable) {
        for (const auto &term : table)
            PISet.insert(term);
    }
    findEPI();

    std::unordered_set<std::string> removeMinterms;
    for (const auto &epi : EPISet) {
        PISet.erase(epi);
        for (const auto &essentialTerm : essentialTerms) {
            if (isTermIncluded(epi, essentialTerm))
                removeMinterms.insert(essentialTerm);
        }
    }
    for (const auto &term : removeMinterms)
        essentialTerms.erase(term);

    Polynomial();
    updateTermsInfo(termsInfo);
    return literalCount;
}