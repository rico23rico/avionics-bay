#include "data_file_reader.hpp"
#include "plugin.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>

#define LOG *this->logger

#define NAV_FILE_PATH  "Resources/default data/earth_nav.dat"
#define FIX_FILE_PATH  "Resources/default data/earth_fix.dat"
#define ARPT_FILE_PATH "Resources/default scenery/default apt dat/Earth nav data/apt.dat"


namespace xpfiles {

static bool is_a_directory(const std::string& name) noexcept {
    struct stat info;
    if( stat( name.c_str(), &info ) != 0 ) {
        return false;
    } else if( info.st_mode & S_IFDIR ) { 
        return true;
    } else {
        return false;
    }
}

static bool is_a_file (const std::string& name) noexcept {
    struct stat info;
    if( stat( name.c_str(), &info ) != 0 ) {
        return false;
    } else if( info.st_mode & S_IFREG ) { 
        return true;
    } else {
        return false;
    }
}

void DataFileReader::perform_init_checks() {
    if (!is_a_directory(xplane_directory)) {
        throw std::runtime_error("Directory " + xplane_directory + " is not a directory or is not accessible.");
    }
    
    std::string filename = xplane_directory + "/" + NAV_FILE_PATH;
    if (!is_a_file(filename)) {
        throw std::runtime_error("File " + filename + " is not accessible.");
    }

    filename = xplane_directory + "/" + FIX_FILE_PATH;
    if (!is_a_file(filename)) {
        throw std::runtime_error("File " + filename + " is not accessible.");
    }

    filename = xplane_directory + "/" + ARPT_FILE_PATH;
    if (!is_a_file(filename)) {
        throw std::runtime_error("File " + filename + " is not accessible.");
    }

}

DataFileReader::DataFileReader(const std::string &xplane_directory) : xplane_directory(xplane_directory) {
    this->logger = get_logger();

    LOG << logger_level_t::DEBUG << "Initializing DataFileReader..." << ENDL;

    perform_init_checks();
    
    this->my_thread = std::thread(&DataFileReader::worker, this);
    this->my_thread.detach();
    
    LOG << logger_level_t::INFO << "DataFileReader thread started." << ENDL;

}

void DataFileReader::worker() noexcept {
    for (volatile int i=0; i<100000; i++) {
    
    }
}

} // namespace xpfiles
