#include <iostream>
#include "archiver.hpp"
#include "print_lib.hpp"

int main () {
    archiver::ArchiverCPU arch;

    std::vector <int> data (122*5*200 + 170, 2);
    for (int i = 0; i < 170; ++i) {
        data.push_back (14);

        data.push_back (12);
        data.push_back (12);
        data.push_back (12);

        data.push_back (1);
        data.push_back (1);
    }

    try {
        cppl::DeviceProvider deviceProvider;
        cl::Device device = deviceProvider.getDefaultDevice ();
        archiver::AchiverGPU archGpu {device};

        arch.archive (data, 1, 15);
    } catch (cl::Error& exc) {
        cppl::printError (exc);
    } catch (std::exception& exc) {
        std::cerr << exc.what () << std::endl;
    }
}