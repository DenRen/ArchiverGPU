#include <iostream>
#include "archiver.hpp"

int main () {
    archiver::ArchiverCPU arch;

    std::vector <int> data = {
        1,2,3,2, 1,1,1,1 ,2,2,2,2, 3,3,4,6, 4,9,6,5, 6,7,9,8
    };

    // arch.archive (data);

    try {
        cppl::DeviceProvider deviceProvider;
        cl::Device device = deviceProvider.getDefaultDevice ();
        archiver::AchiverGPU archGpu {device};

        archGpu.calc_freq_table (data, 1, 15);
    } catch (cl::Error& exc) {
        cppl::printError (exc);
    } catch (std::exception& exc) {
        std::cerr << exc.what () << std::endl;
    }
}