#include "xpdata.hpp"

#include <cmath>

#define LOG *this->logger << STARTL

#define DEG_TO_RAD(x) (x*M_PI/180.)
#define RAD_TO_DEG(x) (x*180./M_PI)

static_assert(5 == DEG_TO_RAD(RAD_TO_DEG(5)));
static_assert(0 == DEG_TO_RAD(0));
static_assert(M_PI == DEG_TO_RAD(180));

namespace avionicsbay {

static int last_navaid_type = 0;

static double GC_distance_km(double lat1, double lon1, double lat2, double lon2) {
    //This function returns great circle distance between 2 points.
    //Found here: http://bluemm.blogspot.gr/2007/01/excel-formula-to-calculate-distance.html

    double distance = acos(cos(DEG_TO_RAD(90.-lat1))*cos(DEG_TO_RAD(90.-lat2))+
                      sin(DEG_TO_RAD(90.-lat1))*sin(DEG_TO_RAD(90.-lat2))*cos(DEG_TO_RAD(lon1-lon2)))
                      * (6378000./1000.);

    return distance;
}

/**************************************************************************************************/
/** NAVAIDS **/
/**************************************************************************************************/
void XPData::push_navaid(xpdata_navaid_t &&navaid) noexcept {
    last_navaid_type = navaid.type;
    navaids_all[navaid.type].push_back(std::move(navaid));
}

void XPData::flag_navaid_coupled() noexcept {
    navaids_all[last_navaid_type].back().is_coupled_dme = true;
}

void XPData::index_navaids_by_name() noexcept {

    LOG << logger_level_t::DEBUG << "[XPData] Indexing NAVAIDS by name [type_nr=" << navaids_all.size() << ']' << ENDL;

    for(int i=0; i < navaids_all.size(); i++) {
        for(int j=0; j < navaids_all[i].size(); j++) {
            auto element_ptr = &navaids_all[i][j];
        
            auto id_str = std::string(element_ptr->id);

            if (navaids_name[element_ptr->type].count(id_str) == 1) {
                navaids_name[element_ptr->type].at(id_str).push_back(element_ptr);
            } else {
                navaids_name[element_ptr->type][id_str].push_back(element_ptr);
            }
        }
    }
}

void XPData::index_navaids_by_freq() noexcept {

    LOG << logger_level_t::DEBUG << "[XPData] Indexing NAVAIDS by freq [type_nr=" << navaids_all.size() << ']' << ENDL;

    for(int i=0; i < navaids_all.size(); i++) {
        for(int j=0; j < navaids_all[i].size(); j++) {
            auto element_ptr = &navaids_all[i][j];
        
            auto freq = element_ptr->frequency;

            if (navaids_freq[element_ptr->type].count(freq) == 1) {
                navaids_freq[element_ptr->type].at(freq).push_back(element_ptr);
            } else {
                navaids_freq[element_ptr->type][freq].push_back(element_ptr);
            }
        }
    }
}

void XPData::index_navaids_by_coords() noexcept {

    LOG << logger_level_t::DEBUG << "[XPData] Indexing NAVAIDS by coords [type_nr=" << navaids_all.size() << ']' << ENDL;

    for(int i=0; i < navaids_all.size(); i++) {
        for(int j=0; j < navaids_all[i].size(); j++) {
            auto element_ptr = &navaids_all[i][j];
        
            int lat = static_cast<int>(element_ptr->coords.lat);
            int lon = static_cast<int>(element_ptr->coords.lon);

            lat = lat - (lat % 4);
            lon = lon - (lon % 4);

            auto lat_lon_pair = std::pair<int, int>(lat, lon);

            if (navaids_coords[element_ptr->type].count(lat_lon_pair) == 1) {
                navaids_coords[element_ptr->type].at(lat_lon_pair).push_back(element_ptr);
            } else {
                navaids_coords[element_ptr->type][lat_lon_pair].push_back(element_ptr);
            }
        }
    }
}


std::pair<const xpdata_navaid_t* const*, size_t> XPData::get_navaids_by_name(xpdata_navaid_type_t type, const std::string &name) const noexcept {
    try {
        const auto & element = this->navaids_name.at(type).at(name);
        return std::pair<const xpdata_navaid_t* const*, size_t> (element.data(), element.size());
    } catch(...) {
        return std::pair<const xpdata_navaid_t* const*, size_t> (nullptr, 0);
    }
}

std::pair<const xpdata_navaid_t* const*, size_t> XPData::get_navaids_by_freq(xpdata_navaid_type_t type, unsigned int freq) const noexcept {
    try {
        const auto & element = this->navaids_freq.at(type).at(freq);
        return std::pair<const xpdata_navaid_t* const*, size_t> (element.data(), element.size());
    } catch(...) {
        return std::pair<const xpdata_navaid_t* const*, size_t> (nullptr, 0);
    }
}

std::pair<const xpdata_navaid_t* const*, size_t> XPData::get_navaids_by_coords(xpdata_navaid_type_t type, double d_lat, double d_lon) const noexcept {

    int lat = static_cast<int>(d_lat);
    int lon = static_cast<int>(d_lon);

    lat = lat - (lat % 4);
    lon = lon - (lon % 4);
    
    auto ctr_pair = std::pair<int, int> (lat, lon);

    try {
        const auto & element = this->navaids_coords.at(type).at(ctr_pair);
        return std::pair<const xpdata_navaid_t* const*, size_t> (element.data(), element.size());
    } catch(...) {
        return std::pair<const xpdata_navaid_t* const*, size_t> (nullptr, 0);
    }
}

/**************************************************************************************************/
/** FIXES **/
/**************************************************************************************************/
void XPData::push_fix(xpdata_fix_t &&fix) noexcept {
    fixes_all.push_back(std::move(fix));
}

void XPData::index_fixes_by_name() noexcept {

    LOG << logger_level_t::DEBUG << "[XPData] Indexing FIXES by name [total=" << fixes_all.size() << ']' << ENDL;

    for(int i=0; i < fixes_all.size(); i++) {
        auto element_ptr = &fixes_all[i];
    
        auto id_str = std::string(element_ptr->id);

        if (fixes_name.count(id_str) == 1) {
            fixes_name.at(id_str).push_back(element_ptr);
        } else {
            fixes_name[id_str].push_back(element_ptr);
        }

    }
}

void XPData::index_fixes_by_coords() noexcept {

    LOG << logger_level_t::DEBUG << "[XPData] Indexing FIXES by coords [total=" << fixes_all.size() << ']' << ENDL;

    for(int i=0; i < fixes_all.size(); i++) {

        auto element_ptr = &fixes_all[i];
    
        int lat = static_cast<int>(element_ptr->coords.lat);
        int lon = static_cast<int>(element_ptr->coords.lon);

        lat = lat - (lat % 4);
        lon = lon - (lon % 4);

        auto lat_lon_pair = std::pair<int, int>(lat, lon);

        if (fixes_coords.count(lat_lon_pair) == 1) {
            fixes_coords.at(lat_lon_pair).push_back(element_ptr);
        } else {
            fixes_coords[lat_lon_pair].push_back(element_ptr);
        }

    }
}

std::pair<const xpdata_fix_t* const*, size_t> XPData::get_fixes_by_name(const std::string &name) const noexcept {
    try {
        const auto & element = this->fixes_name.at(name);
        return std::pair<const xpdata_fix_t* const*, size_t> (element.data(), element.size());
    } catch(...) {
        return std::pair<const xpdata_fix_t* const*, size_t> (nullptr, 0);
    }
}

std::pair<const xpdata_fix_t* const*, size_t> XPData::get_fixes_by_coords(double d_lat, double d_lon) const noexcept {

    int lat = static_cast<int>(d_lat);
    int lon = static_cast<int>(d_lon);

    lat = lat - (lat % 4);
    lon = lon - (lon % 4);
    
    auto ctr_pair = std::pair<int, int> (lat, lon);

    try {
        const auto & element = this->fixes_coords.at(ctr_pair);
        return std::pair<const xpdata_fix_t* const*, size_t> (element.data(), element.size());
    } catch(...) {
        return std::pair<const xpdata_fix_t* const*, size_t> (nullptr, 0);
    }
}

/**************************************************************************************************/
/** APT **/
/**************************************************************************************************/
void XPData::push_apt(xpdata_apt_t &&apt) noexcept {
    apts_all.push_back(std::move(apt));
}

void XPData::push_apt_rwy(xpdata_apt_rwy_t &&rwy) noexcept {
    apts_rwy_all[apts_all.back().pos_seek].emplace_back(std::move(rwy));
}

void XPData::index_apts_by_name() noexcept {
    LOG << logger_level_t::DEBUG << "[XPData] Indexing APTS by name [total=" << apts_all.size() << ']' << ENDL;

    for(int i=0; i < apts_all.size(); i++) {
        auto element_ptr = &apts_all[i];
    
        auto id_str = std::string(element_ptr->id);

        if (apts_name.count(id_str) == 1) {
            apts_name.at(id_str).push_back(element_ptr);
        } else {
            apts_name[id_str].push_back(element_ptr);
        }

    }
}

void XPData::index_apts_by_coords() noexcept {

    LOG << logger_level_t::DEBUG << "[XPData] Indexing APTS by coords [total=" << apts_all.size() << ']' << ENDL;

    for(int i=0; i < apts_all.size(); i++) {

        auto element_ptr = &apts_all[i];
    
        if (apts_rwy_all.count(element_ptr->pos_seek) == 0) {
            // An airport with no runways?
            continue;
        }

        // Let's compute the airport center coordinates as a centroid of all the middle points
        // of the runways
        double d_lat=0, d_lon=0;
        const auto & rwys_vector = apts_rwy_all.at(element_ptr->pos_seek);
        int nr_rwys = rwys_vector.size();
        for (int i=0; i < rwys_vector.size(); i++) {
            auto curr_runway = rwys_vector[i];
            d_lat += (curr_runway.coords.lat + curr_runway.sibl_coords.lat)/2;
            d_lon += (curr_runway.coords.lon + curr_runway.sibl_coords.lon)/2;
        }
        d_lat /= nr_rwys;
        d_lon /= nr_rwys;

        element_ptr->apt_center.lat = d_lat;
        element_ptr->apt_center.lon = d_lon;
        element_ptr->rwys = rwys_vector.data();
        element_ptr->rwys_len = rwys_vector.size();

        int lat = static_cast<int>(d_lat);
        int lon = static_cast<int>(d_lon);

        lat = lat - (lat % 4);
        lon = lon - (lon % 4);

        auto lat_lon_pair = std::pair<int, int>(lat, lon);

        if (apts_coords.count(lat_lon_pair) == 1) {
            apts_coords.at(lat_lon_pair).push_back(element_ptr);
        } else {
            apts_coords[lat_lon_pair].push_back(element_ptr);
        }

    }
}

std::pair<const xpdata_apt_t* const*, size_t> XPData::get_apts_by_name(const std::string &name) const noexcept {
    try {
        const auto & element = this->apts_name.at(name);
        return std::pair<const xpdata_apt_t* const*, size_t> (element.data(), element.size());
    } catch(...) {
        return std::pair<const xpdata_apt_t* const*, size_t> (nullptr, 0);
    }
}
std::pair<const xpdata_apt_t* const*, size_t> XPData::get_apts_by_coords(double d_lat, double d_lon) const noexcept {
    int lat = static_cast<int>(d_lat);
    int lon = static_cast<int>(d_lon);

    lat = lat - (lat % 4);
    lon = lon - (lon % 4);
    
    auto ctr_pair = std::pair<int, int> (lat, lon);

    try {
        const auto & element = this->apts_coords.at(ctr_pair);
        return std::pair<const xpdata_apt_t* const*, size_t> (element.data(), element.size());
    } catch(...) {
        return std::pair<const xpdata_apt_t* const*, size_t> (nullptr, 0);
    }
}


void XPData::update_nearest_airport() noexcept {

    auto acf_coords = get_acf_cur_pos();
    auto arpts = get_apts_by_coords(acf_coords.first, acf_coords.second);
    
    double min_distance = 99999999.;
    const xpdata_apt_t *min_arpt = nullptr;
    
    for (int i=0; i < arpts.second; i++) {
        double distance = GC_distance_km(arpts.first[i]->apt_center.lat, arpts.first[i]->apt_center.lon, acf_coords.first, acf_coords.second);
        
        if (distance < min_distance) {
            min_arpt = arpts.first[i];
            min_distance = distance;
        }
    }
    
    std::lock_guard<std::mutex> lk(mx_nearest_airport);
    this->nearest_airport = min_arpt;
    
}

} // namespace avionicsbay
