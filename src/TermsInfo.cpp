
#include <sstream>
#include <cassert>
#include <stdexcept>
#include "TermsInfo.h"

static void writeResult(std::vector<std::string> &results, std::vector<std::string> &dontCares, const std::string &term, const std::string &value, unsigned int termIndex);

std::istream& operator>>(std::istream& in, TermsInfo& termsInfo) {
    while(true){
        std::string token;
        in >> token;
        if (token == ".i"){ // input count
            std::string value;
            in >> value;
            termsInfo.inputCount = std::stoi(value);
        } else if (token == ".o") { // output count
            std::string value;
            in >> value;
            termsInfo.outputCount = std::stoi(value);
        } else if (token == ".ilb") { // input variables name
            for (size_t i = 0; i < termsInfo.inputCount; i++) {
                std::string value;
                in >> value;
                termsInfo.inputNames.push_back(value);
            }
        } else if (token == ".ob") { // output variables name
            for (size_t i = 0; i < termsInfo.outputCount; i++) {
                std::string value;
                in >> value;
                termsInfo.outputNames.push_back(value);
            }
        } else if (token == ".p") { // read terms count and get terms
            std::string value;
            in >> value;
            termsInfo.termsCount = std::stoi(value);
            for (size_t i = 0; i < termsInfo.termsCount; i++) {
                std::string value[2];
                in >> value[0] >> value[1];
                writeResult(termsInfo.terms, termsInfo.dontCareTerms, value[0], value[1], 0);
            }
        } else if (token == ".e"){ // exit
            return in;
        } else {
            if (!in)
                break;
            unsigned int counter = 0;
            std::string value[2];
            value[0] = std::move(token);
            while (value[0] != ".e") {
                counter++;
                in >> value[1];
                writeResult(termsInfo.terms, termsInfo.dontCareTerms, value[0], value[1], 0);
                if (!(in >> value[0]))
                    break;
            }
            termsInfo.termsCount = counter;
            return in;
        }
    }
    return in;
}

std::ostream& operator<<(std::ostream& out, TermsInfo& termsInfo) {
    out << ".i " << termsInfo.inputCount << '\n';
    out << ".o " << termsInfo.outputCount << '\n';
    out << ".ilb ";
    for (size_t i = 0; i < termsInfo.inputCount; i++)
    {
        out << termsInfo.inputNames[i];
        if (i != termsInfo.inputCount - 1) {
            out << ' ';
        }
    }
    out << '\n';
    out << ".ob ";
    for (size_t i = 0; i < termsInfo.outputCount; i++)
    {
        out << termsInfo.outputNames[i];
        if (i != termsInfo.outputCount - 1) {
            out << ' ';
        }
    }
    out << '\n';
    out << ".p " << termsInfo.termsCount << '\n';
    for (auto e: termsInfo.terms) {
        out << e << " 1" << '\n';
    }
    out << ".e";
    return out;
}

static void writeResult(std::vector<std::string> &results, std::vector<std::string> &dontCares, const std::string &term, const std::string &value, unsigned int termIndex) {
    if (termIndex < term.length()){
        char token = term[termIndex];
        if (token == '1') {
            writeResult(results, dontCares, term, value,  termIndex + 1);
        } else if (token == '0') {
            writeResult(results, dontCares, term, value,  termIndex + 1);
        } else if (token == '-') {
            std::string newTerm = term;
            newTerm[termIndex] = '0';
            writeResult(results, dontCares, newTerm, value,  termIndex + 1);
            newTerm[termIndex] = '1';
            writeResult(results, dontCares, newTerm, value,  termIndex + 1);
        } else {
            throw std::runtime_error("[Runtime Error]Unknown Term\'s Token:" + token);
        }
    } else {
        if (value == "1") {
        results.push_back(term);
        } else if (value == "-") {
            dontCares.push_back(term);
        }
    }
}
