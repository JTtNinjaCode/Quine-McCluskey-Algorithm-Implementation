#ifndef TERMSINFO_H
#define TERMSINFO_H
#include <iostream>
#include <string>
#include <vector>

struct TermsInfo {
    unsigned int inputCount = 0;
    unsigned int outputCount = 0;
    unsigned int termsCount = 0;
    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;
    std::vector<std::string> terms;
    std::vector<std::string> dontCares;
};

std::istream& operator>>(std::istream& in, TermsInfo& termsInfo);
std::ostream& operator<<(std::ostream& out, TermsInfo& termsInfo);
#endif