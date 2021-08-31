#include "data_file_reader.hpp"

#include "utilities/filesystem.hpp"
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
#include <vector>

#define LOG *this->logger << STARTL

#define AWY_FILE_PATH  "Resources/default data/earth_awy.dat"
#define NAV_FILE_PATH  "Resources/default data/earth_nav.dat"
#define FIX_FILE_PATH  "Resources/default data/earth_fix.dat"
#define MORA_FILE_PATH "Resources/default data/earth_mora.dat"
#define HOLD_FILE_PATH "Resources/default data/earth_hold.dat"
#define APT_FILE_PATH "Resources/default scenery/default apt dat/Earth nav data/apt.dat"

#define NEAREST_APT_UPDATE_SEC 2

namespace avionicsbay {

static std::list<std::string> all_string_container;

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


void DataFileReader::request_apts_details(const std::string &id) noexcept {
    if (!xpdata->get_is_ready()) {
        return; // XPData not yet initialized
    }

    // Check if airport exists
    auto arpt = xpdata->get_apts_by_name(id);
    if (arpt.second == 0) {
        return; // Airport not found ?
    }

    // If it's not zero it should be 1 because airport id is unique

    std::lock_guard<std::mutex> lk(mx_apt_details);
    this->detail_arpt = arpt.first[0];
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
        xpdata->index_navaids_by_name();
        xpdata->index_navaids_by_freq();
        xpdata->index_navaids_by_coords();
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
        xpdata->index_fixes_by_name();
        xpdata->index_fixes_by_coords();
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
        xpdata->index_apts_by_name();
        xpdata->index_apts_by_coords();
    } 
    catch(const std::ifstream::failure &e) {
        LOG << logger_level_t::ERROR << "[DataFileReader] APT I/O exception: " << e.what() << ENDL;
        return;
    }
    catch(...) {
        LOG << logger_level_t::CRIT << "[DataFileReader] APT Unexpected exception." << ENDL;
        return;
    }

    try {
        parse_mora_file();
    } 
    catch(const std::ifstream::failure &e) {
        LOG << logger_level_t::ERROR << "[DataFileReader] MORA I/O exception: " << e.what() << ENDL;
        return;
    }
    catch(...) {
        LOG << logger_level_t::CRIT << "[DataFileReader] MORA Unexpected exception." << ENDL;
        return;
    }

    try {
        parse_hold_file();
        xpdata->index_holds();
    } 
    catch(const std::ifstream::failure &e) {
        LOG << logger_level_t::ERROR << "[DataFileReader] HOLD I/O exception: " << e.what() << ENDL;
        return;
    }
    catch(...) {
        LOG << logger_level_t::CRIT << "[DataFileReader] HOLD Unexpected exception." << ENDL;
        return;
    }

    try {
        parse_awy_file();
        xpdata->index_awys();
    } 
    catch(const std::ifstream::failure &e) {
        LOG << logger_level_t::ERROR << "[DataFileReader] AWY I/O exception: " << e.what() << ENDL;
        return;
    }
    catch(...) {
        LOG << logger_level_t::CRIT << "[DataFileReader] AWY Unexpected exception." << ENDL;
        return;
    }

    xpdata->set_is_ready(true);

    LOG << logger_level_t::INFO << "[DataFileReader] Data Ready." << ENDL;

    while(!this->stop) {
        xpdata->update_nearest_airport(); // No need synchronization for this

        std::unique_lock<std::mutex> lk(mx_apt_details);
        cv_apt_details.wait_for(lk, std::chrono::seconds(NEAREST_APT_UPDATE_SEC));

        xpdata_apt_t *temp_arpt=nullptr;

        // Do we need to update detailed info of an airport?
        if (this->detail_arpt != nullptr) {
            temp_arpt = this->detail_arpt;
            this->detail_arpt = nullptr;
        }

        lk.unlock();
        if (temp_arpt != nullptr) {
            parse_apts_details(temp_arpt);  // This may be very heavy, don't put this in the mutex
        }
    }
    
    xpdata->set_is_ready(false);
    
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
        if ((type < NAV_ID_NDB || type > NAV_ID_IM) && (type < NAV_ID_DME)) {
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
            .is_coupled_dme = false,
            .category = std::stoi(splitted[5]),
            .bearing  = static_cast<int>(std::stod(splitted[6]) * 1000)
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

void DataFileReader::parse_apts_details(xpdata_apt_t *arpt) {

    if (arpt->is_loaded_details) {
        return;  // Nothing to do
    }
    
    xpdata->allocate_apt_details(arpt); // Allocate and set the point of xpdata_apt_t

    
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::badbit);
    
    std::string filename = xplane_directory + APT_FILE_PATH;
    LOG << logger_level_t::INFO << "[DataFileReader] [Loading=" << arpt->id << "] Trying to open " << filename << "..." << ENDL;
    ifs.open(filename, std::ifstream::in);
    
    // Now we move the seek to the position of the airport we previously saved:
    ifs.seekg(arpt->pos_seek);
    
    std::string line;
    int line_no = 0;

    bool first_airport_header = false;
    while (!ifs.eof() && std::getline(ifs, line) && !this->stop) {
        if (parse_apts_details_line(arpt, line_no, line)) {
        
            // This is the logic to stop: when I encounter the first airport it's my airport (just
            // after the seek), then the next airport (and first_airport_header will be true),
            // I exit
            if (first_airport_header) {
                break;
            }
            first_airport_header = true;
        }
        line_no++;
    }

    ifs.close();
    LOG << logger_level_t::INFO << "[DataFileReader] Total lines read from " << filename << ": " << line_no << ENDL;
    
    xpdata->finalize_apt_details(arpt);  // Set the last pointers into the final struct and flag is_loaded_details
    
}

void DataFileReader::parse_apts_details_tower(xpdata_apt_t *arpt, const std::vector<std::string> &splitted) {
    if (splitted.size() < 3) {
        return;     //  Should not happen
    }

    double lat = std::stod(splitted[1]);
    double lon = std::stod(splitted[2]);
    
    arpt->details->tower_pos.lat = lat;
    arpt->details->tower_pos.lon = lon;
}

void DataFileReader::parse_apts_details_arpt_gate(xpdata_apt_t *arpt, const std::vector<std::string> &splitted) {
    if (splitted.size() < 7) {
        return;     // Invalid gate
    }

    if (splitted[4] != "gate") {
        return; // It means it's a runway, we are not interested in runways here.
    }

    all_string_container.emplace_back(splitted[6]);
    const char* gate_name = all_string_container.back().c_str();
    int gate_name_len = all_string_container.back().size();

    double lat = std::stod(splitted[1]);
    double lon = std::stod(splitted[2]);

    xpdata_apt_gate_t gate = {
        .name       = gate_name,
        .name_len   = gate_name_len,
        .coords     = { .lat=lat, .lon=lon }
    };

    xpdata->push_apt_gate(arpt, std::move(gate));
    
}

void DataFileReader::parse_apts_details_linear_start(const std::vector<std::string> &splitted) {
    if (splitted.size() < 3) {
        return;     // Invalid line
    }

    double lat = std::stod(splitted[1]);
    double lon = std::stod(splitted[2]);

    if (splitted.size() >= 4) {
        // We have also the color specified
        current_color = std::stoi(splitted[3]);
    }

    xpdata_apt_node_t node = {
        .coords     = { .lat=lat, .lon=lon },
        .is_bez = false,
    };

    this->curr_node_list.push_back(std::move(node));
}

void DataFileReader::parse_apts_details_beizer_start(const std::vector<std::string> &splitted) {
    if (splitted.size() < 5) {
        return;     // Invalid line
    }

    double lat = std::stod(splitted[1]);
    double lon = std::stod(splitted[2]);
    double c_lat = std::stod(splitted[3]);
    double c_lon = std::stod(splitted[4]);

    if (splitted.size() >= 6) {
        // We have also the color specified
        current_color = std::stoi(splitted[5]);
    }

    xpdata_apt_node_t node = {
        .coords     = { .lat=lat, .lon=lon },
        .is_bez = true,
        .bez_cp     = { .lat=c_lat, .lon=c_lon },
    };

    this->curr_node_list.push_back(std::move(node));
}

void DataFileReader::parse_apts_details_save(xpdata_apt_t *arpt) {

    if (apt_detail_status == ROW_NONE) {
        // This should not happen
        LOG << logger_level_t::CRIT << "[DataFileReader] Detailed parser: incongruent status on line-close." << ENDL;
        this->curr_node_list.clear();
        return;
    }


    // 2 - Store the vector in XPData - note, we need a copy here
    if (apt_detail_status == ROW_TAXI) {
        xpdata->push_apt_taxi(arpt, current_color, curr_node_list);
    } else if (apt_detail_status == ROW_LINE) {
        xpdata->push_apt_line(arpt, current_color, curr_node_list);
    } else if (apt_detail_status == ROW_BOUND) {
        xpdata->push_apt_bound(arpt, current_color, curr_node_list);
    } else if (apt_detail_status == ROW_HOLE) {
        xpdata->push_apt_hole(arpt, current_color, curr_node_list);
    }

    // 3 - Clear the vector and the color for the next path
    this->current_color = 0;
    this->curr_node_list.clear();
}

void DataFileReader::parse_apts_details_linear_close(xpdata_apt_t *arpt, const std::vector<std::string> &splitted) {

    // Add the last node to the vector
    this->parse_apts_details_linear_start(splitted);
    
    // And then save to XPData
    parse_apts_details_save(arpt); 
}

void DataFileReader::parse_apts_details_linear_end(xpdata_apt_t *arpt, const std::vector<std::string> &splitted) {
    this->parse_apts_details_linear_close(arpt, splitted);  // At present their are handled in the same way
}

void DataFileReader::parse_apts_details_beizer_close(xpdata_apt_t *arpt, const std::vector<std::string> &splitted) {

    // Add the last node to the vector
    this->parse_apts_details_beizer_start(splitted);
    
    // And then save to XPData
    parse_apts_details_save(arpt); 
}

void DataFileReader::parse_apts_details_beizer_end(xpdata_apt_t *arpt, const std::vector<std::string> &splitted) {
    this->parse_apts_details_beizer_close(arpt, splitted);  // At present their are handled in the same way
}


void DataFileReader::parse_apts_details_route_point(xpdata_apt_t *arpt, const std::vector<std::string> &splitted) {
    if (splitted.size() < 5) {
        return;     //  Should not happen
    }

    double lat   = std::stod(splitted[1]);
    double lon   = std::stod(splitted[2]);
    int route_id = std::stoi(splitted[4]);
    
    xpdata->push_apt_route_id(arpt, route_id, { .lat = lat, .lon = lon });
}

void DataFileReader::parse_apts_details_route_taxi(xpdata_apt_t *arpt, const std::vector<std::string> &splitted) {
    if (splitted.size() < 6) {
        return;     //  Should not happen
    }

    if (splitted[4] == "runway") {
        return;     // I'm not interested in runway routes
    }

    all_string_container.emplace_back(splitted[5]);
    const char* route_name = all_string_container.back().c_str();
    int route_name_len = all_string_container.back().size();

    xpdata_apt_route_t new_route = {
        .name = route_name,
        .name_len = route_name_len,
        .route_node_1 = std::stoi(splitted[1]),
        .route_node_2 = std::stoi(splitted[2])
    };

    xpdata->push_apt_route_taxi(arpt, std::move(new_route));
}

bool DataFileReader::parse_apts_details_line(xpdata_apt_t * arpt, int line_no, const std::string &line) {
    if (line.size() == 0) {
        return false;
    }

    auto splitted = str_explode(line, ' ');
    if (splitted.size() < 1) {
        return false;     // Empty line
    }
    
    const auto &id = splitted[0];
    
    try {
        
        if (id == "1") {
            return true; // Airport header
        }
        else if (id == "14") { // Airport tower
            parse_apts_details_tower(arpt, splitted);
        }
        else if (id == "110") { // Taxyways
            apt_detail_status = ROW_TAXI;
        } else if ( id == "120" ) { // Linear feature
            apt_detail_status = ROW_LINE;
        } else if ( id == "130" ) { // Linear feature
            apt_detail_status = ROW_BOUND;
        } else if ( id == "111" ) {
            parse_apts_details_linear_start(splitted);
        } else if ( id == "112" ) {
            parse_apts_details_beizer_start(splitted);
        } else if ( id == "113" ) {
            parse_apts_details_linear_close(arpt, splitted);
            apt_detail_status = ROW_HOLE;
        } else if ( id == "114" ) {
            parse_apts_details_beizer_close(arpt, splitted);
            apt_detail_status = ROW_HOLE;
        } else if ( id == "115" ) {
            parse_apts_details_linear_end(arpt, splitted);
            apt_detail_status = ROW_HOLE;
        } else if ( id == "116" ) {
            parse_apts_details_beizer_end(arpt, splitted);
            apt_detail_status = ROW_HOLE;
        } else if ( id == "1201" ) {
            parse_apts_details_route_point(arpt, splitted);
        } else if ( id == "1202" ) {
            parse_apts_details_route_taxi(arpt, splitted);
        } else if ( id == "1300" ) {
            parse_apts_details_arpt_gate(arpt, splitted);
        }

    } catch(const std::invalid_argument &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] apt.dat:" << line_no << ": invalid parameter (failed str->num conversion)." << ENDL;
        return false;
    } catch(const std::out_of_range &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] apt.dat:" << line_no << ": invalid parameter (out-of-range str->num conversion)." << ENDL;
        return false;
    }

    return false;
}


//**************************************************************************************************
// MORA
//**************************************************************************************************
void DataFileReader::parse_mora_file() {
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::badbit);
    
    std::string filename = xplane_directory + MORA_FILE_PATH;
    LOG << logger_level_t::INFO << "[DataFileReader] Trying to open " << filename << "..." << ENDL;
    ifs.open(filename, std::ifstream::in);
    
    std::string line;
    int line_no = 0;
    while (!ifs.eof() && std::getline(ifs, line) && !this->stop) {
        if (line.size() > 0 and line[0] != 'I' and (line[0] != '9' or line[1] != '9')) {
            parse_mora_line(line_no, line);
        }
        line_no++;
    }

    ifs.close();
    LOG << logger_level_t::INFO << "[DataFileReader] Total lines read from " << filename << ": " << line_no << ENDL;
}

void DataFileReader::parse_mora_line(int line_no, const std::string &line) {
    auto splitted = str_explode(line, ' ');
    if (splitted.size() < 3) {
        return;     // Empty or invalid line
    }

    try {

        auto lat = std::stoi(splitted[0]);  // THis is in +123 format
        auto lon = std::stoi(splitted[1]);  // THis is in +123 format

    for(int i=2; i < splitted.size(); i++) {
        auto mora_value = std::stoi(splitted[i]);  // THis is in +123 format
        xpdata->push_mora(lat, lon+i-2, mora_value);
    }

    } catch(const std::invalid_argument &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_mora.dat:" << line_no << ": invalid parameter (failed str->num conversion)." << ENDL;
        return;
    } catch(const std::out_of_range &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_mora.dat:" << line_no << ": invalid parameter (out-of-range str->num conversion)." << ENDL;
        return;
    }   
}


//**************************************************************************************************
// HOLDS
//**************************************************************************************************
void DataFileReader::parse_hold_file() {
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::badbit);
    
    std::string filename = xplane_directory + HOLD_FILE_PATH;
    LOG << logger_level_t::INFO << "[DataFileReader] Trying to open " << filename << "..." << ENDL;
    ifs.open(filename, std::ifstream::in);
    
    std::string line;
    int line_no = 0;
    while (!ifs.eof() && std::getline(ifs, line) && !this->stop) {
        if (line.size() > 0 && line[0] != 'I' && (line[0] != '9' or line[1] != '9') && line.rfind("11", 0) == std::string::npos) {
            parse_hold_line(line_no, line);
        }
        line_no++;
    }

    ifs.close();
    LOG << logger_level_t::INFO << "[DataFileReader] Total lines read from " << filename << ": " << line_no << ENDL;
}

void DataFileReader::parse_hold_line(int line_no, const std::string &line) {
    auto splitted = str_explode(line, ' ');
    if (splitted.size() < 11) {
        return;     // Empty or invalid line
    }

    try {
        // ID
        all_string_container.emplace_back(splitted[0]);
        const char* hold_id = all_string_container.back().c_str();
        int hold_id_len = all_string_container.back().size();

        // APT ID
        all_string_container.emplace_back(splitted[2]);
        const char* apt_id = all_string_container.back().c_str();
        int apt_id_len = all_string_container.back().size();

        uint8_t type_navaid = std::stoi(splitted[3]);
        char turn_direction = splitted[7][0];

        auto inbound_course= static_cast<uint16_t>(std::stof(splitted[4]) * 10);
        auto leg_time      = static_cast<uint16_t>(std::stof(splitted[5]) * 60);
        auto dme_value     = static_cast<uint16_t>(std::stof(splitted[6]) * 10);

        uint32_t alt_min  = std::stoi(splitted[8]);
        uint32_t alt_max  = std::stoi(splitted[9]);
        uint16_t hold_spd = std::stoi(splitted[10]);

        xpdata_hold_t new_hold = {
            .id         = hold_id,
            .id_len     = hold_id_len,
            .apt_id     = apt_id,
            .apt_id_len = apt_id_len,
            .navaid_type = type_navaid,
            .turn_direction = turn_direction,
    
            .inbound_course = inbound_course,
            .leg_time = leg_time,
            .dme_leg_length = dme_value,
            
            .max_altitude = alt_max,
            .min_altitude = alt_min,
            .holding_speed_limit = hold_spd
        };

        xpdata->push_hold(std::move(new_hold));

    } catch(const std::invalid_argument &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_hold.dat:" << line_no << ": invalid parameter (failed str->num conversion)." << ENDL;
        return;
    } catch(const std::out_of_range &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_hold.dat:" << line_no << ": invalid parameter (out-of-range str->num conversion)." << ENDL;
        return;
    }   
}

//**************************************************************************************************
// AWYs
//**************************************************************************************************
void DataFileReader::parse_awy_file() {
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::badbit);
    
    std::string filename = xplane_directory + AWY_FILE_PATH;
    LOG << logger_level_t::INFO << "[DataFileReader] Trying to open " << filename << "..." << ENDL;
    ifs.open(filename, std::ifstream::in);
    
    std::string line;
    int line_no = 0;
    while (!ifs.eof() && std::getline(ifs, line) && !this->stop) {
        if (line.size() > 2 && (line[0] != '9' or line[1] != '9') && line.rfind("11", 0) == std::string::npos) {
            parse_awy_line(line_no, line);
        }
        line_no++;
    }

    ifs.close();
    LOG << logger_level_t::INFO << "[DataFileReader] Total lines read from " << filename << ": " << line_no << ENDL;
}

void DataFileReader::parse_awy_line(int line_no, const std::string &line) {
    auto splitted = str_explode(line, ' ');
    if (splitted.size() < 11) {
        return;     // Empty or invalid line
    }

    std::string check = splitted[0] + "-" + splitted[3] +  "-" + splitted[10];
    if (check == this->prev_awy_double_entry) {
        return; // Double entry
    }

    prev_awy_double_entry = check;
    try {

        all_string_container.emplace_back(splitted[0]);
        const char* begin_wpt_id = all_string_container.back().c_str();
        int begin_wpt_id_len = all_string_container.back().size();

        uint8_t begin_wpt_type = std::stoi(splitted[2]);

        all_string_container.emplace_back(splitted[3]);
        const char* end_wpt_id = all_string_container.back().c_str();
        int end_wpt_id_len = all_string_container.back().size();

        uint8_t end_wpt_type   = std::stoi(splitted[5]);

        char direction = splitted[6][0];

        uint16_t base_alt = std::stoi(splitted[8]);
        uint16_t top_alt  = std::stoi(splitted[9]);

        auto awy_splitted = str_explode(splitted[10], '-');

        for (const auto &awy_id_s : awy_splitted) {

            all_string_container.emplace_back(awy_id_s);
            const char* awy_id = all_string_container.back().c_str();
            int awy_id_len = all_string_container.back().size();

            if (direction == 'N' || direction == 'F') {
                xpdata_awy_t awy = {
                    .id            = awy_id,
                    .id_len        = awy_id_len,

                    .start_wpt     = begin_wpt_id,
                    .start_wpt_len = begin_wpt_id_len,
                    .start_wpt_type= begin_wpt_type,

                    .end_wpt       = end_wpt_id,
                    .end_wpt_len   = end_wpt_id_len,
                    .end_wpt_type  = end_wpt_type,

                    .base_alt      = base_alt,
                    .top_alt       = top_alt
                };
                xpdata->push_awy(std::move(awy));
            }

            if (direction == 'N' || direction == 'B') {
                xpdata_awy_t awy = {
                    .id            = awy_id,
                    .id_len        = awy_id_len,

                    .start_wpt     = end_wpt_id,
                    .start_wpt_len = end_wpt_id_len,
                    .start_wpt_type= end_wpt_type,

                    .end_wpt       = begin_wpt_id,
                    .end_wpt_len   = begin_wpt_id_len,
                    .end_wpt_type  = begin_wpt_type,

                    .base_alt      = base_alt,
                    .top_alt       = top_alt
                };
                xpdata->push_awy(std::move(awy));
            }
        }


    } catch(const std::invalid_argument &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_awy.dat:" << line_no << ": invalid parameter (failed str->num conversion)." << ENDL;
        return;
    } catch(const std::out_of_range &e) {
        LOG << logger_level_t::WARN << "[DataFileReader] earth_awy.dat:" << line_no << ": invalid parameter (out-of-range str->num conversion)." << ENDL;
        return;
    }   
}


} // namespace avionicsbay
