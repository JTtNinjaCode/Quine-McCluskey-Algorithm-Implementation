#include <fstream>
#include "TermsInfo.h"
#include "QM.h"

int main(int argc, char **argv){
    TermsInfo termsInfo;
    std::ifstream ifs(argv[1]);
    ifs >> termsInfo;

    QM(termsInfo);

    std::ofstream ofs("out.txt");
    ofs << termsInfo;
}