#include "cifp_parser.hpp"

#include "constants.hpp"
#include "data_types.hpp"
#include "plugin.hpp"
#include "utilities/filesystem.hpp"

#include <cassert>
#include <chrono>
#include <list>

#define LOG *this->logger << STARTL

#define CIFP_FILE_DIR  "Resources/default data/CIFP/"

constexpr int F_ROW_TYPE = 1;
constexpr int F_NAME     = 2;
constexpr int F_TRANS    = 3;
constexpr int F_LEG_NAME = 4;

constexpr int F_LEG_TURN = 9;
constexpr int F_LEG_TYPE = 11;
constexpr int F_LEG_TDV  = 12;

constexpr int F_LEG_RECC_NAVAID = 13;


constexpr int F_LEG_RADIUS = 17;
constexpr int F_LEG_THETA  = 18;
constexpr int F_LEG_RHO    = 19;
constexpr int F_LEG_OB_MAG = 20;
constexpr int F_LEG_RTE_HOLD = 21;

constexpr int F_LEG_ALT_TYPE = 22;
constexpr int F_LEG_ALT1 = 23;
constexpr int F_LEG_ALT2 = 24;

constexpr int F_LEG_SPD_TYPE = 26;
constexpr int F_LEG_SPD = 27;
constexpr int F_LEG_ANGLE = 28;

constexpr int F_LEG_CTR_FIX = 30;

constexpr int RWY_ID = 0;
constexpr int RWY_HEIGHT = 3;
constexpr int RWY_LOC = 5;
constexpr int RWY_CAT = 6;

static std::list<std::string> all_string_container;

namespace avionicsbay {

extern std::vector<std::string> str_explode(std::string const & s, char delim);

CIFPParser::CIFPParser(const std::string &xplane_directory) : xplane_directory(xplane_directory) {

    this->logger = get_logger();
    
    assert(this->logger);

    LOG << logger_level_t::DEBUG << "Initializing CIFP Parser..." << ENDL;

}


void CIFPParser::perform_init_checks() {
    if (!is_a_directory(xplane_directory)) {
        throw std::runtime_error("Directory " + xplane_directory + " is not a directory or is not accessible.");
    }

    if (!is_a_directory(xplane_directory + '/' + CIFP_FILE_DIR)) {
        throw std::runtime_error("Directory " + xplane_directory + " is not a directory or is not accessible.");
    }

}

void CIFPParser::load_airport(const std::string &arpt_id) {
    this->ap_future = std::async(std::launch::async, &CIFPParser::task, this, arpt_id);
}

bool CIFPParser::is_ready() noexcept {
    if (! this->ap_future.valid()) {
        return true;  // If it's not valid, the thread has been never called or finished running
    }
    if (this->ap_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        // Loaded
        this->ap_future.get();
        return true;
    }
    return false;
}

void CIFPParser::task(const std::string &arpt_id) noexcept {


#if defined(__linux__)
    pthread_setname_np(pthread_self(), "CIFPParser");   // For debugging purposes
#endif

    try {
        parse_cifp_file(arpt_id);
    } 
    catch(const std::ifstream::failure &e) {
        LOG << logger_level_t::ERROR << "[CIFPParser] I/O exception: " << e.what() << ENDL;
        return;
    }
    catch(...) {
        LOG << logger_level_t::CRIT << "[CIFPParser] Unexpected exception." << ENDL;
        return;
    }

}

//**************************************************************************************************
// Single Field Parsing
//**************************************************************************************************

char compute_turn(const std::string &turn, const std::string &tdv) {
    if(turn.size() != 1 || tdv.size() != 1) {
        throw std::runtime_error("Invalid Turn/TDV.");
    }
    
    if (turn[0] == ' ') {
        return 'N'; // No turn
    }
    
    bool tdv_b = tdv[0] == 'Y';
    if (tdv_b) {    // Required turn
        return turn[0] + 1; // M, S, F
    } else {
        return turn[0]; // L, R, E
    }

}

uint8_t compute_spd_type(const std::string &spd_type) {
    if(spd_type.size() != 1) {
        throw std::runtime_error("Invalid SPD TYPE.");
    }
    
    switch(spd_type[0]) {
        case ' ':
        case '@':
            return NAV_CIFP_CSTR_SPD_AT;
        case '+':
            return NAV_CIFP_CSTR_SPD_ABOVE;
        case '-':
            return NAV_CIFP_CSTR_SPD_BELOW;
    }
    
    return NAV_CIFP_CSTR_SPD_NONE;
}

uint8_t get_leg_type(const std::string &leg_type) {
    if (leg_type == "IF") {
        return NAV_CIFP_TYPE_IF;
    } else if (leg_type == "TF") {
        return NAV_CIFP_TYPE_TF;
    } else if (leg_type == "CF") {
        return NAV_CIFP_TYPE_CF;
    } else if (leg_type == "DF") {
        return NAV_CIFP_TYPE_DF;
    } else if (leg_type == "FA") {
        return NAV_CIFP_TYPE_FA;
    } else if (leg_type == "FC") {
        return NAV_CIFP_TYPE_FC;
    } else if (leg_type == "FD") {
        return NAV_CIFP_TYPE_FD;
    } else if (leg_type == "FM") {
        return NAV_CIFP_TYPE_FM;
    } else if (leg_type == "CA") {
        return NAV_CIFP_TYPE_CA;
    } else if (leg_type == "CD") {
        return NAV_CIFP_TYPE_CD;
    } else if (leg_type == "CI") {
        return NAV_CIFP_TYPE_CI;
    } else if (leg_type == "CR") {
        return NAV_CIFP_TYPE_CR;
    } else if (leg_type == "RF") {
        return NAV_CIFP_TYPE_RF;
    } else if (leg_type == "AF") {
        return NAV_CIFP_TYPE_AF;
    } else if (leg_type == "VA") {
        return NAV_CIFP_TYPE_VA;
    } else if (leg_type == "VD") {
        return NAV_CIFP_TYPE_VD;
    } else if (leg_type == "VI") {
        return NAV_CIFP_TYPE_VI;
    } else if (leg_type == "VM") {
        return NAV_CIFP_TYPE_VM;
    } else if (leg_type == "VR") {
        return NAV_CIFP_TYPE_VR;
    } else if (leg_type == "PI") {
        return NAV_CIFP_TYPE_PI;
    } else if (leg_type == "HA") {
        return NAV_CIFP_TYPE_HA;
    } else if (leg_type == "HF") {
        return NAV_CIFP_TYPE_HF;
    } else if (leg_type == "HM") {
        return NAV_CIFP_TYPE_HM;
    } else {
        throw std::runtime_error("Invalid Leg type.");
    }
}

int safe_stoi(const std::string &str) {
    try {
        return std::stoi(str);
    } catch(...) {
        return 0;
    }
}

int safe_alt(const std::string &str, bool &is_fl) {
    is_fl = false;
    std::string copy_str = str;
    if (str[0] == 'F' && str[1] == 'L') {
        is_fl = true;
        copy_str = str.substr(2);
    }
    try {
        return std::stoi(copy_str);
    } catch(...) {
        return 0;
    }
}

uint8_t compute_alt_type(const std::string &alt_type) {
    if(alt_type.size() != 1) {
        throw std::runtime_error("Invalid ALT Type.");
    }
    
    switch(alt_type[0]) {
        case '+':
        case 'V':
            return NAV_CIFP_CSTR_ALT_ABOVE;
        case '-':
        case 'Y':
            return NAV_CIFP_CSTR_ALT_BELOW;
        case '@':
        case 'X':
        case ' ':
            return NAV_CIFP_CSTR_ALT_AT;
        case 'B':
            return NAV_CIFP_CSTR_ALT_ABOVE_BELOW;
        case 'C':
            return NAV_CIFP_CSTR_ALT_ABOVE_2ND;
        case 'G':
        case 'H':
        case 'I':
        case 'J':
            return NAV_CIFP_CSTR_ALT_GLIDE;
    }

    return NAV_CIFP_CSTR_ALT_NONE;
}

//**************************************************************************************************
// Parsing
//**************************************************************************************************
void CIFPParser::parse_cifp_file(const std::string &arpt_id) {
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::badbit);
    
    std::string filename = xplane_directory + '/' + CIFP_FILE_DIR + '/' + arpt_id + ".dat";
    LOG << logger_level_t::INFO << "[CIFPParser] Trying to open " << filename << "..." << ENDL;
    ifs.open(filename, std::ifstream::in);
    
    std::string line;
    int line_no = 0;
    while (!ifs.eof() && std::getline(ifs, line)) {
        if (line.size() > 0) {
            parse_cifp_file_line(arpt_id, line_no, line);
        }
        line_no++;
    }

    ifs.close();
    LOG << logger_level_t::INFO << "[CIFPParser] Total lines read from " << filename << ": " << line_no << ENDL;
    
    finalize_structures();
    
}

void CIFPParser::parse_cifp_file_line(const std::string &arpt_id, int line_no, const std::string &line) {
    auto splitted = str_explode(line, ',');
    if (splitted.size() < 1) {
        return;     // Empty line
    }
    
    try {
        if (splitted[0].rfind("SID:", 0) == 0) {
            int id = std::stoi(splitted[0].substr(4));
            this->parse_sid(arpt_id, id, splitted);
        }
        else if (splitted[0].rfind("STAR:", 0) == 0) {
            int id = std::stoi(splitted[0].substr(5));
            this->parse_star(arpt_id, id, splitted);
        }
        else if (splitted[0].rfind("APPCH:", 0) == 0) {
            int id = std::stoi(splitted[0].substr(6));
            this->parse_appch(arpt_id, id, splitted);
        }
        else if (splitted[0].rfind("PRDAT:", 0) == 0) {
            return; // Currently not implemented
        }
        else if (splitted[0].rfind("RWY:", 0) == 0) {
            int id = std::stoi(splitted[0].substr(6));
            this->parse_rwy(splitted[0].substr(6), id, splitted);
        }
    } catch(const std::runtime_error &err) {
        LOG << logger_level_t::ERROR << "[CIFPParser] Line " << line_no << " error: " << err.what() << ENDL;
    } catch(...) {
        LOG << logger_level_t::CRIT << "[CIFPParser] Line " << line_no << " unexpected error" << ENDL;
    }
}

int CIFPParser::create_new_cifp_data(std::unordered_map<std::string, std::vector<xpdata_cifp_data_t>> &vec_ref, const std::string &arpt_id, const std::vector<std::string> &splitted) {
    xpdata_cifp_data_t new_proc;
    
    new_proc.type = splitted[F_ROW_TYPE][0];
    
    all_string_container.push_back(splitted[F_NAME]);
    new_proc.proc_name = all_string_container.back().c_str();
    new_proc.proc_name_len = all_string_container.back().size();

    all_string_container.push_back(splitted[F_TRANS]);
    new_proc.trans_name = all_string_container.back().c_str();
    new_proc.trans_name_len = all_string_container.back().size();

    legs_array[legs_array_progressive] = {};
    new_proc._legs_arr_ref = legs_array_progressive++;

    vec_ref[arpt_id].push_back(new_proc);
    return vec_ref[arpt_id].size()-1;
}

void CIFPParser::parse_leg(xpdata_cifp_leg_t &new_leg, const std::vector<std::string> &splitted) {
    
    all_string_container.push_back(splitted[F_LEG_NAME]);
    new_leg.leg_name     = all_string_container.back().c_str();
    new_leg.leg_name_len = all_string_container.back().size();

    new_leg.turn_direction = compute_turn(splitted[F_LEG_TURN], splitted[F_LEG_TDV]);
    new_leg.leg_type       = get_leg_type(splitted[F_LEG_TYPE]);

    new_leg.radius   = safe_stoi(splitted[F_LEG_RADIUS]);
    new_leg.theta    = safe_stoi(splitted[F_LEG_THETA]);
    new_leg.rho      = safe_stoi(splitted[F_LEG_RHO]);
    new_leg.outb_mag = safe_stoi(splitted[F_LEG_OB_MAG]);
    new_leg.rte_hold = safe_stoi(splitted[F_LEG_RTE_HOLD]);
    
    new_leg.cstr_alt_type  = compute_alt_type(splitted[F_LEG_ALT_TYPE]);
    new_leg.cstr_altitude1 = safe_alt(splitted[F_LEG_ALT1], new_leg.cstr_altitude1_fl);
    new_leg.cstr_altitude2 = safe_alt(splitted[F_LEG_ALT2], new_leg.cstr_altitude2_fl);
    
    new_leg.cstr_speed = safe_stoi(splitted[F_LEG_SPD]);
    new_leg.cstr_speed_type = new_leg.cstr_speed == 0 ? NAV_CIFP_CSTR_SPD_NONE : compute_spd_type(splitted[F_LEG_SPD_TYPE]);
    
    new_leg.vpath_angle = -safe_stoi(splitted[F_LEG_ANGLE]);

    all_string_container.push_back(splitted[F_LEG_CTR_FIX]);
    new_leg.center_fix     = all_string_container.back().c_str();
    new_leg.center_fix_len = all_string_container.back().size();

    all_string_container.push_back(splitted[F_LEG_RECC_NAVAID]);
    new_leg.recomm_navaid     = all_string_container.back().c_str();
    new_leg.recomm_navaid_len = all_string_container.back().size();

    
}

void CIFPParser::parse_sid(const std::string &arpt_id, int id, const std::vector<std::string> &splitted) {
    if (splitted.size() < F_LEG_CTR_FIX+1) {
        return;     // Error line
    }

    auto index_str = arpt_id + ":" + splitted[F_NAME] + ":" + splitted[F_TRANS];  // For internal use only

    int index;
    if (data_sid_idx.find(index_str) == data_sid_idx.end()) {
        // Not yet seen
        index = create_new_cifp_data(data_sid, arpt_id, splitted);
        data_sid_idx[index_str] = index;
    } else {
        index = data_sid_idx.at(index_str);
    }

    int leg_array_id = data_sid.at(arpt_id).at(index)._legs_arr_ref;
    
    xpdata_cifp_leg_t new_leg;

    parse_leg(new_leg, splitted);

    legs_array[leg_array_id].push_back(std::move(new_leg));
}

void CIFPParser::parse_star(const std::string &arpt_id, int id, const std::vector<std::string> &splitted) {

    if (splitted.size() < F_LEG_CTR_FIX+1) {
        return;     // Error line
    }


    auto index_str = arpt_id + ":" + splitted[F_NAME] + ":" + splitted[F_TRANS];  // For internal use only


    int index;
    if (data_star_idx.find(index_str) == data_star_idx.end()) {
        // Not yet seen
        index = create_new_cifp_data(data_star, arpt_id, splitted);
        data_star_idx[index_str] = index;
    } else {
        index = data_star_idx.at(index_str);
    }

    int leg_array_id = data_star.at(arpt_id).at(index)._legs_arr_ref;
    
    xpdata_cifp_leg_t new_leg;

    parse_leg(new_leg, splitted);

    legs_array[leg_array_id].push_back(std::move(new_leg));

}

void CIFPParser::parse_appch(const std::string &arpt_id, int id, const std::vector<std::string> &splitted) {
    if (splitted.size() < F_LEG_CTR_FIX+1) {
        return;     // Error line
    }


    auto index_str = arpt_id + ":" + splitted[F_NAME] + ":" + splitted[F_TRANS];  // For internal use only
    
    int index;
    if (data_app_idx.find(index_str) == data_app_idx.end()) {
        // Not yet seen
        index = create_new_cifp_data(data_app, arpt_id, splitted);
        data_app_idx[index_str] = index;
    } else {
        index = data_app_idx.at(index_str);
    }

    int leg_array_id = data_app.at(arpt_id).at(index)._legs_arr_ref;
    
    xpdata_cifp_leg_t new_leg;

    parse_leg(new_leg, splitted);

    legs_array[leg_array_id].push_back(std::move(new_leg));
}

void CIFPParser::parse_rwy(const std::string &rwy_id, int id, const std::vector<std::string> &splitted) {
    if (splitted.size() < 9) {
        return;     // Error line
    }

    xpdata_cifp_rwy_data_t rwy;

    all_string_container.push_back(rwy_id);
    rwy.rwy_name     = all_string_container.back().c_str();
    rwy.rwy_name_len = all_string_container.back().size();

    rwy.ldg_threshold_alt = std::stoi(splitted[RWY_HEIGHT]);

    all_string_container.push_back(splitted[RWY_LOC]);
    rwy.loc_ident     = all_string_container.back().c_str();
    rwy.loc_ident_len = all_string_container.back().size();

    rwy.ils_category = splitted[RWY_CAT].size() > 0 ? splitted[RWY_CAT][0] : ' ';

    this->rwys_array.push_back(std::move(rwy));
}


void CIFPParser::finalize_structures() {

    /** SID **/
    for (auto apt_it = data_sid.begin(); apt_it != data_sid.end(); apt_it++) {
        for(auto sid_it = apt_it->second.begin(); sid_it != apt_it->second.end(); sid_it++) {
            int ref = sid_it->_legs_arr_ref;
            sid_it->legs = legs_array.at(ref).data();
            sid_it->legs_len = legs_array.at(ref).size();
        }
    }

    /** STAR **/
    for (auto apt_it = data_star.begin(); apt_it != data_star.end(); apt_it++) {
        for(auto star_it = apt_it->second.begin(); star_it != apt_it->second.end(); star_it++) {
            int ref = star_it->_legs_arr_ref;
            star_it->legs = legs_array.at(ref).data();
            star_it->legs_len = legs_array.at(ref).size();
        }
    }

    /** APPR **/
    for (auto apt_it = data_app.begin(); apt_it != data_app.end(); apt_it++) {
        for(auto app_it = apt_it->second.begin(); app_it != apt_it->second.end(); app_it++) {
            int ref = app_it->_legs_arr_ref;
            app_it->legs = legs_array.at(ref).data();
            app_it->legs_len = legs_array.at(ref).size();
        }
    }

}

xpdata_cifp_t CIFPParser::get_full_cifp(const char* name) {
    xpdata_cifp_t to_ret;
    try {
        to_ret.sids.data = data_sid.at(name).data();
        to_ret.sids.len = data_sid.at(name).size();
    } catch(...) {
        to_ret.sids.data = nullptr;
        to_ret.sids.len = 0;
    }
    
    try {
        to_ret.stars.data = data_star.at(name).data();
        to_ret.stars.len = data_star.at(name).size();
    } catch(...) {
        to_ret.stars.data = nullptr;
        to_ret.stars.len = 0;
    }

    try {
        to_ret.apprs.data = data_app.at(name).data();
        to_ret.apprs.len = data_app.at(name).size();
    } catch(...) {
        to_ret.apprs.data = nullptr;
        to_ret.apprs.len = 0;
    }

    try {
        to_ret.rwys.data = rwys_array.data();
        to_ret.rwys.len = rwys_array.size();
    } catch(...) {
        to_ret.rwys.data = nullptr;
        to_ret.rwys.len = 0;
    }

    return to_ret;
}

} // namespace avionicsbay
