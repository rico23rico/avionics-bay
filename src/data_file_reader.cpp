#include "data_file_reader.hpp"
#include "plugin.hpp"

#include <filesystem>

#define LOG *this->logger

namespace xpfiles {

DataFileReader::DataFileReader(const std::string &xplane_directory) : xplane_directory(xplane_directory) {
    this->logger = get_logger();

    LOG << logger_level_t::DEBUG << "Initializing DataFileReader..." << ENDL;

    std::filesystem::path dir_path(xplane_directory);

    if (!std::filesystem::is_directory(xplane_directory)) {
        throw std::runtime_error("Directory " + xplane_directory + "is not a directory");
    }
    
    this->my_thread = std::thread(&DataFileReader::worker, this);
    this->my_thread.detach();
    
    LOG << logger_level_t::INFO << "DataFileReader thread started." << ENDL;

}

void DataFileReader::worker() noexcept {
    for (volatile int i=0; i<100000; i++) {
    
    }
}

} // namespace xpfiles
