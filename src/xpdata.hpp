#ifndef XPDATA_H
#define XPDATA_H

#ifdef _WIN32
#define EXPORT_DLL __declspec( dllexport )
#else
#define EXPORT_DLL
#endif

#include "utilities/logger.hpp"
#include "data_types.hpp"

#include <algorithm>
#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

extern "C" { EXPORT_DLL void terminate(void); }

namespace avionicsbay {

extern std::shared_ptr<Logger> get_logger() noexcept;
extern std::pair<double, double> get_acf_cur_pos() noexcept;

class XPData {

public:
    XPData() : is_ready(false) {
        this->logger = get_logger();
        
        // Init vector capacities
        fixes_all.reserve(200000);
        apts_all.reserve(30000);
    }
    
    virtual ~XPData() {
        *this->logger << STARTL << logger_level_t::DEBUG << "[XPData] Terminating..." << ENDL;

    }

    void set_is_ready(bool is_ready) noexcept { this->is_ready = is_ready; }
    bool get_is_ready() const        noexcept { return this->is_ready; }

/**************************************************************************************************/
/** NAVAIDS **/
/**************************************************************************************************/
    void push_navaid(xpdata_navaid_t &&navaid) noexcept;
    void flag_navaid_coupled() noexcept;
    void index_navaids_by_name() noexcept;
    void index_navaids_by_freq() noexcept;
    void index_navaids_by_coords() noexcept;

    std::pair<const xpdata_navaid_t* const*, size_t> get_navaids_by_name(xpdata_navaid_type_t type, const std::string &name) const noexcept;
    std::pair<const xpdata_navaid_t* const*, size_t> get_navaids_by_freq(xpdata_navaid_type_t type, unsigned int freq) const noexcept;
    std::pair<const xpdata_navaid_t* const*, size_t> get_navaids_by_coords(xpdata_navaid_type_t type, double lat, double lon) const noexcept;

/**************************************************************************************************/
/** FIXES **/
/**************************************************************************************************/
    void push_fix(xpdata_fix_t &&fix) noexcept;
    void index_fixes_by_name() noexcept;
    void index_fixes_by_coords() noexcept;

    std::pair<const xpdata_fix_t* const*, size_t> get_fixes_by_name(const std::string &name) const noexcept;
    std::pair<const xpdata_fix_t* const*, size_t> get_fixes_by_coords(double lat, double lon) const noexcept;

/**************************************************************************************************/
/** APT **/
/**************************************************************************************************/
    void push_apt(xpdata_apt_t &&apt) noexcept;
    void push_apt_rwy(xpdata_apt_rwy_t &&rwy) noexcept;
    void index_apts_by_name() noexcept;
    void index_apts_by_coords() noexcept;

    std::pair<xpdata_apt_t* const*, size_t> get_apts_by_name(const std::string &name) const noexcept;
    std::pair<const xpdata_apt_t* const*, size_t> get_apts_by_coords(double lat, double lon) const noexcept;

    void update_nearest_airport() noexcept;
    const xpdata_apt_t* get_nearest_airport() noexcept {
        std::lock_guard<std::mutex> lk(mx_nearest_airport);
        return this->nearest_airport;
    }
    
/**************************************************************************************************/
/** APT - details **/
/**************************************************************************************************/
    void push_apt_taxi(xpdata_apt_t *apt, int color, const std::vector<xpdata_apt_node_t> &nodes) noexcept;
    void push_apt_line(xpdata_apt_t *apt, int color, const std::vector<xpdata_apt_node_t> &nodes) noexcept;
    void push_apt_bound(xpdata_apt_t *apt, int color, const std::vector<xpdata_apt_node_t> &nodes) noexcept;
    void push_apt_hole(xpdata_apt_t *apt, int color, const std::vector<xpdata_apt_node_t> &nodes) noexcept;

    void push_apt_gate(xpdata_apt_t *apt, xpdata_apt_gate_t &&gate) noexcept;

    void push_apt_route_taxi(const xpdata_apt_t *apt, xpdata_apt_route_t && route) noexcept;
    void push_apt_route_id(const xpdata_apt_t *apt, int id, xpdata_coords_t && coords) noexcept;
    
    xpdata_coords_t get_route_point(long cur_seek, int id) const {
        return apts_details_ruotes_id.at(cur_seek).at(id);
    }
    
    void allocate_apt_details(xpdata_apt_t *apt) noexcept;
    void finalize_apt_details(xpdata_apt_t *apt) noexcept;
    
/**************************************************************************************************/
/** MORAs **/
/**************************************************************************************************/
    void push_mora(int16_t lat_idx, int16_t lon_idx, uint16_t value) noexcept;
    uint16_t get_mora(double lat, double lon) const noexcept;
    
private:
    std::shared_ptr<Logger> logger;
    std::atomic<bool> is_ready;
    
    const xpdata_apt_t *nearest_airport = nullptr; // can be nullptr at any time
    std::mutex mx_nearest_airport;

    xpdata_apt_node_array_t *last_pushed_node_array = nullptr;

/**************************************************************************************************/
/** NAVAIDS **/
/**************************************************************************************************/
    std::map<xpdata_navaid_type_t, std::vector<xpdata_navaid_t>> navaids_all;
    std::map<xpdata_navaid_type_t, std::unordered_map<std::string, std::vector<xpdata_navaid_t*>>> navaids_name;
    std::map<xpdata_navaid_type_t, std::unordered_map<unsigned int, std::vector<xpdata_navaid_t*>>> navaids_freq;
    std::map<xpdata_navaid_type_t, std::map<std::pair<int, int>, std::vector<xpdata_navaid_t*>>> navaids_coords;

/**************************************************************************************************/
/** FIXES **/
/**************************************************************************************************/
    std::vector<xpdata_fix_t> fixes_all;
    std::unordered_map<std::string, std::vector<xpdata_fix_t*>> fixes_name;
    std::map<std::pair<int, int>, std::vector<xpdata_fix_t*>> fixes_coords;

/**************************************************************************************************/
/** APT **/
/**************************************************************************************************/
    std::vector<xpdata_apt_t> apts_all;
    std::unordered_map<long, std::vector<xpdata_apt_rwy_t>> apts_rwy_all; // This uses the airport seek in the
                                                                          // file as index: it's for sure unique
    std::unordered_map<std::string, std::vector<xpdata_apt_t*>> apts_name;
    std::map<std::pair<int, int>, std::vector<xpdata_apt_t*>> apts_coords;

/**************************************************************************************************/
/** APT - details **/
/**************************************************************************************************/
    std::list<xpdata_apt_details_t> apts_details_all;   // List is important here, because the
                                                        // container is not static after initialization
    std::unordered_map<long, std::vector<xpdata_apt_gate_t>> apts_details_gates;

    std::unordered_map<long, std::vector<xpdata_apt_node_array_t>> apts_details_pavements_arrays;
    std::unordered_map<long, std::vector<xpdata_apt_node_array_t>> apts_details_linear_feature_arrays;
    std::unordered_map<long, std::vector<xpdata_apt_node_array_t>> apts_details_boundaries_arrays;

    std::list<std::vector<xpdata_apt_node_t>> apts_details_nodes_all;
    std::list<xpdata_apt_node_array_t> apts_details_holes_all;   // List is important here, because the
                                                                 // container is not static after initialization

    std::unordered_map<long, std::vector<xpdata_apt_route_t>> apts_details_ruotes_arrays;
    std::unordered_map<long, std::unordered_map<int, xpdata_coords_t>> apts_details_ruotes_id;
    
/**************************************************************************************************/
/** MORA **/
/**************************************************************************************************/
    std::map<std::pair<int16_t, int16_t>, uint16_t> moras;
    
};


} // namespace avionicsbay


#endif
