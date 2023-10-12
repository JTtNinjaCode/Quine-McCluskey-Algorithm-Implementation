#include "QM.h"
#include <algorithm>
#include <unordered_set>

static std::vector<std::unordered_set<std::string>> PITable;
static std::vector<std::unordered_set<std::string>> tempPITable;
static std::vector<std::unordered_set<std::string>> removePITable;
static std::unordered_set<std::string> essentialTerm;
static std::unordered_set<std::string> PISet;
static std::unordered_set<std::string> EPISet;
static std::vector<std::string> resultTerms;

unsigned int countOne(const std::string &term);
std::pair<bool, std::string> merge(const std::string &str1, const std::string &str2);
bool isIncludedInTerm (const std::string &term, const std::string &minterm);
void findPI();
void findEPI();
void Polynomial();

unsigned int countSetBits(unsigned int n)
{
    unsigned int bitCount = 0;
    while(n) {
        if (n & 1)
            bitCount++;
        n >>= 1;
    }
    return bitCount;
}

unsigned int countOne(const std::string & term) {
    unsigned int count = 0;
    for (auto e: term)
        if (e == '1') count++;
    return count;
}

std::pair<bool, std::string> merge(const std::string &str1, const std::string &str2) {
    unsigned int exitCount = 0;
    std::string resultString = str1;
    for (size_t i = 0; i < str1.size(); i++)
    {
        if (str1[i] != str2[i]) {
            exitCount++;
            if (exitCount == 2) {
                return std::pair<bool, std::string>(false, "");
            }
        }
        if (str1[i] == '1' && str2[i] == '0' || str1[i] == '0' && str2[i] == '1') {
            resultString[i] = '-';
        }
    }
    if (exitCount == 0) return std::pair<bool, std::string>(false, "");
    return std::pair<bool, std::string>(true, resultString);
}

bool isIncludedInTerm (const std::string &term, const std::string &minterm) {
    for (size_t i = 0; i < term.size(); i++){
        if(minterm[i] == '1' && term[i] == '0' || minterm[i] == '0' && term[i] == '1')
            return false;
    }
    return true;
}

void findPI() {
    bool stopFlag = false;
    while (stopFlag == false) {
        stopFlag = true;
        for (size_t i = 0; i < PITable.size() - 1; i++)
        {
            auto littleSet = PITable[i];
            auto bigSet = PITable[i+1];
            for (auto j: littleSet)
            {
                for (auto k: bigSet) {
                    std::pair<bool, std::string> result = merge(j, k);
                    if (result.first) { // if merge success!
                        removePITable[i].insert(j); // remove old term
                        removePITable[i + 1].insert(k); // remove old term
                        tempPITable[i].insert(result.second); // add new term to temp
                        stopFlag = false;
                    }
                }
            }
        }
        // remove old term
        for (size_t i = 0; i < removePITable.size(); i++) {
            for(auto e: removePITable[i]) {
                PITable[i].erase(e);
            }
            removePITable[i].clear();
        }
        // add new term
        for (size_t i = 0; i < tempPITable.size(); i++) {
            for(auto e: tempPITable[i]) {
                PITable[i].insert(e);
            }
            tempPITable[i].clear();
        }
    }
}

void findEPI() {
    for (size_t i = 0; i < PITable.size(); i++) {
        for(auto e: PITable[i]) {
            PISet.insert(e);
        }
    }
    for (auto i: essentialTerm) {
        std::string candidateEPI;
        unsigned int count = 0;
        for (auto j: PISet) {
            if(isIncludedInTerm(j, i)) {
                count++;
                if (count == 2) {
                    break;
                }
                candidateEPI = j;
            }
        }
        if (count == 1) {
            EPISet.insert(candidateEPI);
        }
    }
}

void Polynomial() {
    std::vector<std::string> table;
    table.reserve(PISet.size());
    for (const auto& pi : PISet)
        table.push_back(pi);
    std::vector<std::vector<unsigned int>> polyTerms;
    polyTerms.resize(essentialTerm.size());
    unsigned int index = 0;
    for (const auto& minterm: essentialTerm)
    {
        for (int i = 0; i < table.size(); i++)
        {
            if (isIncludedInTerm(table[i], minterm))
                polyTerms[index].push_back(1 << i);
        }
        index++;
    }
    std::unordered_set<uint64_t> result;
    result.insert(0);
    for (const auto& polyTerm: polyTerms)
    {
        std::unordered_set<uint64_t> temp;
        for (const auto& oldLiteral: result)
        {
            for (const auto& newLiteral: polyTerm)
                temp.insert(newLiteral | oldLiteral);
        }
        result = std::move(temp);
    }
    unsigned int minOneCount = UINT32_MAX;
    std::vector<std::pair<unsigned int, unsigned int>> counts;
    counts.resize(result.size());
    index = 0;
    for (auto term: result)
    {
        counts[index] = std::make_pair(countSetBits(term), term);
        if(counts[index].first < minOneCount)
            minOneCount = counts[index].first;
        index++;
    }
    for (auto &&i : counts)
    {
        std::cout << i.first << ' ' << i.second << std::endl;
    }

    index = 0;
    for (auto term: result)
    {
        if(counts[index].first == minOneCount) {
            unsigned int i= counts[index].second;
            unsigned int indexCounter = table.size() - 1;
            while(i) {
                if (i & 1)
                    resultTerms.push_back(table[indexCounter]);
                i >>= 1;
                indexCounter--;
            }
            break;
        }
        index++;
    }
}

void QM(TermsInfo &termsInfo) {
    PITable.resize(termsInfo.inputCount + 1);
    tempPITable.resize(termsInfo.inputCount + 1);
    removePITable.resize(termsInfo.inputCount + 1);
    for (auto e: termsInfo.terms) {
        unsigned int index = countOne(e);
        PITable[index].insert(e);
    }
    for (auto e: termsInfo.dontCares) {
        unsigned int index = countOne(e);
        PITable[index].insert(e);
    }
    findPI();
    for (auto e: termsInfo.terms) {
        essentialTerm.insert(e);
    }
    findEPI();
    std::unordered_set<std::string> temp;
    for (auto i: EPISet) {
        PISet.erase(i);
        for (auto j: essentialTerm) {
            if (isIncludedInTerm(i, j)) {
                temp.insert(j);
            }
        }
    }
    for (auto e: temp) {
        essentialTerm.erase(e);
    }
    Polynomial();
    unsigned int count = 0;
    termsInfo.terms.clear();
    for (auto e: EPISet) {
        count++;
        termsInfo.terms.push_back(e);
    }
    for (auto e: resultTerms) {
        count++;
        termsInfo.terms.push_back(e);
    }
    termsInfo.termsCount = count;
}