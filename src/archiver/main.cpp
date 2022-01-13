#include <iostream>
#include "archiver.hpp"

int main () {
    archiver::ArchiverCPU arch;

    std::vector <int> data = {
        1,2,3,2,1,1,1,1,2,2,2,2,3,3,4, 6, 4, 90,56, 5, 6,7,
    };

    arch.archive (data);
}