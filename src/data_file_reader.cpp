#include "data_file_reader.hpp"

#include "constants.hpp"
#include "data_types.hpp"
#include "plugin.hpp"

#include <cassert>
#include <fstream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

#define LOG *this->logger << STARTL

#define NAV_FILE_PATH  "Resources/default data/earth_nav.dat"
#define FIX_FILE_PATH  "Resources/default data/earth_fix.dat"
#define ARPT_FILE_PATH "Resources/default scenery/default apt dat/Earth nav data/apt.dat"


namespace xpfiles {

std::list<std::string> all_string_container;

//**************************************************************************************************
// String support
//**************************************************************************************************

std::vector<std::string> str_explode(std::string const & s, char delim)
{
    // From: https://stackoverflow.com/questions/12966957/is-there-an-equivalent-in-c-of-phps-explode-function
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, delim); )
    {
        if (token.size() > 0) {
            result.push_back(std::move(token));
        }
    }

    return result;
}

template <typename I>
std::string str_implode(I begin, I end, std::string const& separator)
{
    // From: https://stackoverflow.com/questions/6097927/is-there-a-way-to-implement-analog-of-pythons-separator-join-in-c
    std::ostringstream result;
    if (begin != end) {
        result << *begin++;
    }
    while (begin != end) {
        result << separator << *begin++;
    }
    return result.str();
}


//**************************************************************************************************
// INITIALIZATION
//**************************************************************************************************

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
    this->xpdata = get_xpdata();
    
    assert(this->logger && this->xpdata);
    
    LOG << logger_level_t::DEBUG << "Initializing DataFileReader..." << ENDL;


    perform_init_checks();

    
    this->my_thread = std::thread(&DataFileReader::worker, this);
    this->my_thread.detach();

    
    LOG << logger_level_t::INFO << "DataFileReader thread started." << ENDL;

}

//**************************************************************************************************
// WORKER
//**************************************************************************************************


void DataFileReader::worker() noexcept {

    try {
        parse_navaids_file();
        get_xpdata()->index_navaids_by_name();
        get_xpdata()->index_navaids_by_freq();
        get_xpdata()->index_navaids_by_coords();
    } 
    catch(const std::ifstream::failure &e) {
        LOG << logger_level_t::ERROR << "[DataFileReader] File exception: " << e.what() << ENDL;
        return;
    }
    catch(...) {
        LOG << logger_level_t::CRIT << "[DataFileReader] Unexpected exception." << ENDL;
        return;
    }
    get_xpdata()->set_is_ready(true);
}

//**************************************************************************************************
// WORKER functions - NAVAIDS
//**************************************************************************************************
void DataFileReader::parse_navaids_file() {
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::badbit);
    
    std::string filename = xplane_directory + NAV_FILE_PATH;
    LOG << logger_level_t::DEBUG << "[DataFileReader] Trying to open " << filename << "..." << ENDL;
    ifs.open(filename, std::ifstream::in);
    
    std::string line;
    int line_no = 0;
    while (!ifs.eof() && std::getline(ifs, line)) {
        if (line.size() > 0 and line[0] != 'I' and (line[0] != '9' or line[1] != '9')) {
            parse_navaids_file_line(line_no, line);
        }
        line_no++;
    }

    ifs.close();
    LOG << logger_level_t::DEBUG << "[DataFileReader] Total lines read from " << filename << ": " << line_no << ENDL;
}

void DataFileReader::parse_navaids_file_line(int line_no, const std::string &line) {
    auto splitted = str_explode(line, ' ');
    
    if (splitted.size() < 5) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_nav.dat:" << line_no << ": invalid nr. parameters." << ENDL;
        return;     // Something invalid here
    }
    
    try {

        // Read the first field: line id
        auto type = std::stoi(splitted[0]);
        if ((type < NAV_ID_NDB || type > NAV_ID_IM) && (type < NAV_ID_DME or type > NAV_ID_DME_ALONE)) {
            return; // Not interesting point (actually no line should match this condition)
        }

        // Concatenate the navaid full name
        all_string_container.emplace_back(str_implode(splitted.begin()+10, splitted.end(), " "));
        const char* full_name = all_string_container.back().c_str();
        int full_name_len = all_string_container.back().size();
        
        all_string_container.push_back(splitted[7]);
        const char* icao_name = all_string_container.back().c_str();
        int icao_name_len = all_string_container.back().size();
        
        xpdata_navaid_t navaid = {
            .id       = icao_name,
            .id_len   = icao_name_len,
            .full_name= full_name,
            .full_name_len = full_name_len,
            .type     = type,
            .coords   = {
                .lat = std::stod(splitted[1]),
                .lon = std::stod(splitted[2])
            },
            .altitude = std::stoi(splitted[3]),
            .frequency = static_cast<unsigned>(std::stoi(splitted[4]))
        };
    
        xpdata->push_navaid(std::move(navaid));
    } catch(const std::invalid_argument &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_nav.dat:" << line_no << ": invalid parameter (failed str->int conversion)." << ENDL;
        return;
    } catch(const std::out_of_range &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_nav.dat:" << line_no << ": invalid parameter (out-of-range str->int conversion)." << ENDL;
        return;
    }
}

} // namespace xpfiles
