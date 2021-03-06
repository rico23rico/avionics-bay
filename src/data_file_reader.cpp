#include "data_file_reader.hpp"

#include "constants.hpp"
#include "data_types.hpp"
#include "plugin.hpp"

#include <cassert>
#include <chrono>
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
#define APT_FILE_PATH "Resources/default scenery/default apt dat/Earth nav data/apt.dat"

#define NEAREST_APT_UPDATE_SEC 2

namespace avionicsbay {

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

    filename = xplane_directory + "/" + APT_FILE_PATH;
    if (!is_a_file(filename)) {
        throw std::runtime_error("File " + filename + " is not accessible.");
    }

}

DataFileReader::DataFileReader(const std::string &xplane_directory) : stop(false), xplane_directory(xplane_directory) {
    this->logger = get_logger();
    this->xpdata = get_xpdata();
    
    assert(this->logger && this->xpdata);
    
    LOG << logger_level_t::DEBUG << "Initializing DataFileReader..." << ENDL;


    perform_init_checks();

    
    this->my_thread = std::thread(&DataFileReader::worker, this);
    
    LOG << logger_level_t::INFO << "DataFileReader thread started." << ENDL;

}

//**************************************************************************************************
// WORKER
//**************************************************************************************************


void DataFileReader::worker() noexcept {

    this->running = true;

#if defined(__linux__)
    pthread_setname_np(pthread_self(), "avionicsbay_DataFileReader");   // For debugging purposes
#endif

    try {
        parse_navaids_file();
        get_xpdata()->index_navaids_by_name();
        get_xpdata()->index_navaids_by_freq();
        get_xpdata()->index_navaids_by_coords();
    } 
    catch(const std::ifstream::failure &e) {
        LOG << logger_level_t::ERROR << "[DataFileReader] NAVAIDS I/O exception: " << e.what() << ENDL;
        return;
    }
    catch(...) {
        LOG << logger_level_t::CRIT << "[DataFileReader] NAVAIDS Unexpected exception." << ENDL;
        return;
    }

    try {
        parse_fixes_file();
        get_xpdata()->index_fixes_by_name();
        get_xpdata()->index_fixes_by_coords();
    } 
    catch(const std::ifstream::failure &e) {
        LOG << logger_level_t::ERROR << "[DataFileReader] FIX I/O exception: " << e.what() << ENDL;
        return;
    }
    catch(...) {
        LOG << logger_level_t::CRIT << "[DataFileReader] FIX Unexpected exception." << ENDL;
        return;
    }

    try {
        parse_apts_file();
        get_xpdata()->index_apts_by_name();
        get_xpdata()->index_apts_by_coords();
    } 
    catch(const std::ifstream::failure &e) {
        LOG << logger_level_t::ERROR << "[DataFileReader] APT I/O exception: " << e.what() << ENDL;
        return;
    }
    catch(...) {
        LOG << logger_level_t::CRIT << "[DataFileReader] APT Unexpected exception." << ENDL;
        return;
    }

    get_xpdata()->set_is_ready(true);

    LOG << logger_level_t::INFO << "[DataFileReader] Data Ready." << ENDL;

    while(!this->stop) {
        get_xpdata()->update_nearest_airport(); // No need synchronization for this

        std::unique_lock<std::mutex> lk(mx_apt_details);
        cv_apt_details.wait_for(lk, std::chrono::seconds(NEAREST_APT_UPDATE_SEC));
        
        // TODO: Check if we need to update the OANS
    }
    
    LOG << logger_level_t::INFO << "[DataFileReader] Thread shutting down..." << ENDL;

    this->running = false;

}

//**************************************************************************************************
// WORKER functions - NAVAIDS
//**************************************************************************************************
void DataFileReader::parse_navaids_file() {
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::badbit);
    
    std::string filename = xplane_directory + NAV_FILE_PATH;
    LOG << logger_level_t::INFO << "[DataFileReader] Trying to open " << filename << "..." << ENDL;
    ifs.open(filename, std::ifstream::in);
    
    std::string line;
    int line_no = 0;
    while (!ifs.eof() && std::getline(ifs, line) && !this->stop) {
        if (line.size() > 0 and line[0] != 'I' and (line[0] != '9' or line[1] != '9') and (line_no > 1)) {
            parse_navaids_file_line(line_no, line);
        }
        line_no++;
    }

    ifs.close();
    LOG << logger_level_t::INFO << "[DataFileReader] Total lines read from " << filename << ": " << line_no << ENDL;
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
            .frequency = static_cast<unsigned>(std::stoi(splitted[4])),
            .is_coupled_dme = false
        };
        
        if (type == NAV_ID_DME) {
            // In this case we set the flag on the prevous loaded VOR that the DME is coupled
            xpdata->flag_navaid_coupled();
        }
        
        xpdata->push_navaid(std::move(navaid));
    } catch(const std::invalid_argument &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_nav.dat:" << line_no << ": invalid parameter (failed str->int conversion)." << ENDL;
        return;
    } catch(const std::out_of_range &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_nav.dat:" << line_no << ": invalid parameter (out-of-range str->int conversion)." << ENDL;
        return;
    }
}


//**************************************************************************************************
// WORKER functions - FIXES
//**************************************************************************************************
void DataFileReader::parse_fixes_file() {
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::badbit);
    
    std::string filename = xplane_directory + FIX_FILE_PATH;
    LOG << logger_level_t::INFO << "[DataFileReader] Trying to open " << filename << "..." << ENDL;
    ifs.open(filename, std::ifstream::in);
    
    std::string line;
    int line_no = 0;
    while (!ifs.eof() && std::getline(ifs, line) && !this->stop) {
        if (line.size() > 0 and line[0] != 'I' and (line[0] != '9' or line[1] != '9')) {
            parse_fixes_file_line(line_no, line);
        }
        line_no++;
    }

    ifs.close();
    LOG << logger_level_t::INFO << "[DataFileReader] Total lines read from " << filename << ": " << line_no << ENDL;
}


void DataFileReader::parse_fixes_file_line(int line_no, const std::string &line) {
    auto splitted = str_explode(line, ' ');
    
    if (splitted.size() != 6) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_fix.dat:" << line_no << ": invalid nr. parameters." << ENDL;
        return;     // Something invalid here
    }
    
    try {
        
        all_string_container.push_back(splitted[2]);
        const char* fix_name = all_string_container.back().c_str();
        int fix_name_len = all_string_container.back().size();
        
        xpdata_fix_t fix = {
            .id       = fix_name,
            .id_len   = fix_name_len,
            .coords   = {
                .lat = std::stod(splitted[0]),
                .lon = std::stod(splitted[1])
            },
        };
    
        xpdata->push_fix(std::move(fix));
    } catch(const std::invalid_argument &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_fix.dat:" << line_no << ": invalid parameter (failed str->int conversion)." << ENDL;
        return;
    } catch(const std::out_of_range &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_fix.dat:" << line_no << ": invalid parameter (out-of-range str->int conversion)." << ENDL;
        return;
    }
}

//**************************************************************************************************
// WORKER functions - APTs
//**************************************************************************************************
void DataFileReader::parse_apts_file() {
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::badbit);
    
    std::string filename = xplane_directory + APT_FILE_PATH;
    LOG << logger_level_t::INFO << "[DataFileReader] Trying to open " << filename << "..." << ENDL;
    ifs.open(filename, std::ifstream::in);
    
    std::string line;
    int line_no = 0;
    ssize_t cur_seek = 0;
    while (!ifs.eof() && std::getline(ifs, line) && !this->stop) {
        if (line.size() > 0 and line[0] != 'I' and (line[0] != '9' or line[1] != '9')) {
            parse_apts_file_line(line_no, cur_seek, line);
        }
        line_no++;
        cur_seek = ifs.tellg();
    }

    ifs.close();
    LOG << logger_level_t::INFO << "[DataFileReader] Total lines read from " << filename << ": " << line_no << ENDL;
}

void DataFileReader::parse_apts_file_line(int line_no, ssize_t seek_pos, const std::string &line) {
    auto splitted = str_explode(line, ' ');
    if (splitted.size() < 1) {
        return;     // Empty line
    }
    if (splitted[0] == "1") {
        parse_apts_file_header(line_no, seek_pos, splitted);
    }
    else if (splitted[0] == "100") {
        parse_apts_file_runway(line_no, splitted);
    }
    
}

void DataFileReader::parse_apts_file_header(int line_no, ssize_t seek_pos, const std::vector<std::string> &splitted) {

    if (splitted.size() < 6) {
        LOG << logger_level_t::WARN << "[DataFileReader] apt.dat:" << line_no << ": invalid nr. parameters (airport)." << ENDL;
        return;     // Invalid airport
    }

    try {
        all_string_container.emplace_back(str_implode(splitted.begin()+5, splitted.end(), " "));
        const char* full_name = all_string_container.back().c_str();
        int full_name_len = all_string_container.back().size();

        all_string_container.push_back(splitted[4]);
        const char* icao_name = all_string_container.back().c_str();
        int icao_name_len = all_string_container.back().size();
        
        int altitude = std::stoi(splitted[1]);

        xpdata_apt_t apt = {
            .id       = icao_name,
            .id_len   = icao_name_len,
            .full_name= full_name,
            .full_name_len = full_name_len,
            .altitude = std::stoi(splitted[1]),
            .rwys = nullptr,
            .rwys_len = 0,
            .pos_seek = seek_pos
        };

        xpdata->push_apt(std::move(apt));
        
    } catch(const std::invalid_argument &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] apt.dat:" << line_no << ": invalid parameter (failed str->int conversion)." << ENDL;
        return;
    } catch(const std::out_of_range &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] apt.dat:" << line_no << ": invalid parameter (out-of-range str->int conversion)." << ENDL;
        return;
    }

}
void DataFileReader::parse_apts_file_runway(int line_no, const std::vector<std::string> &splitted) {
    if (splitted.size() < 22) {
        LOG << logger_level_t::WARN << "[DataFileReader] apt.dat:" << line_no << ": invalid nr. parameters (runway)." << ENDL;
        return;     // Something invalid here
    }
    
    try {
        int rwy_surface = std::stoi(splitted[2]);
        if (rwy_surface != 1 && rwy_surface != 2 && rwy_surface != 14 && rwy_surface != 15) {
            return; // Not asphalt, Not concrete, Not snow, Not transparent (custom scenery)
        }
        
        int name_len = splitted[8].size();
        int s_name_len = splitted[17].size();
        
        xpdata_apt_rwy_t rwy = {
            .name= {splitted[8][0], 
                    name_len > 1 ? splitted[8][1] : '\0',
                    name_len > 2 ? splitted[8][2] : '\0',
                    '\0'
                   },
            .sibl_name = { splitted[17][0], 
                           s_name_len > 1 ? splitted[17][1] : '\0',
                           s_name_len > 2 ? splitted[17][2] : '\0',
                           '\0'
                         },

            .coords   = {
                .lat = std::stod(splitted[9]),
                .lon = std::stod(splitted[10])
            },
            .sibl_coords = {
                .lat = std::stod(splitted[18]),
                .lon = std::stod(splitted[19])
            },
            
            .width = std::stod(splitted[1]),
            .surface_type = rwy_surface,
            .has_ctr_lights = splitted[5] == "1"
        };
        
        xpdata->push_apt_rwy(std::move(rwy));
    } catch(const std::invalid_argument &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] apt.dat:" << line_no << ": invalid parameter (failed str->int conversion)." << ENDL;
        return;
    } catch(const std::out_of_range &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] apt.dat:" << line_no << ": invalid parameter (out-of-range str->int conversion)." << ENDL;
        return;
    }
    
}

} // namespace avionicsbay
