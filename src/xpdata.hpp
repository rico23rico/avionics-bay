#ifndef XPDATA_H
#define XPDATA_H

#include "utilities/logger.hpp"
#include "data_types.hpp"

#include <algorithm>
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace avionicsbay {

extern std::shared_ptr<Logger> get_logger() noexcept;

class XPData {

public:
    XPData() : is_ready(false) {
        this->logger = get_logger();
        
        // Init vector capacities
        fixes_all.reserve(200000);
        apts_all.reserve(30000);
    }

    void set_is_ready(bool is_ready) noexcept { this->is_ready = is_ready; }
    bool get_is_ready() const        noexcept { return this->is_ready; }

/**************************************************************************************************/
/** NAVAIDS **/
/**************************************************************************************************/
    void push_navaid(xpdata_navaid_t &&navaid) noexcept;
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



private:
    std::shared_ptr<Logger> logger;
    std::atomic<bool> is_ready;

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
};


} // namespace avionicsbay


#endif
