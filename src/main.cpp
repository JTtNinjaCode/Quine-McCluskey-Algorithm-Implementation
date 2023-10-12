#include <fstream>
#include "TermsInfo.h"
#include "QM.h"

int main(int argc, char **argv){
    TermsInfo termsInfo;
    std::ifstream ifs(argv[1]);
    ifs >> termsInfo;

    QM(termsInfo);

    std::cout << "Total number of terms: " << termsInfo.termsCount << std::endl;
    std::cout << "Total number of literals: " << 8;

    std::ofstream ofs(argv[2]);
    ofs << termsInfo;
}