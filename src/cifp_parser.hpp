#ifndef CIFP_PARSER_H
#define CIFP_PARSER_H

#include "utilities/logger.hpp"
#include "data_types.hpp"

#include <algorithm>
#include <future>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
    
namespace avionicsbay {

class CIFPParser {
public:
    CIFPParser(const std::string & xplane_directory);

    void perform_init_checks();

    void load_airport(const std::string &arpt_id);

    bool is_ready() noexcept;

    xpdata_cifp_t get_full_cifp(const char* name);

private:

    std::string xplane_directory;

    std::shared_ptr<Logger> logger;

    std::future<void> ap_future;

    std::unordered_map<std::string, std::vector<xpdata_cifp_data_t>> data_sid;    // key = airport id
    std::unordered_map<std::string, int> data_sid_idx;    // key = airport id:sid:trans ; value = data_sid index
    std::unordered_map<std::string, std::vector<xpdata_cifp_data_t>> data_star;    // key = airport id
    std::unordered_map<std::string, int> data_star_idx;    // key = airport id:sid:trans ; value = data_sid index
    std::unordered_map<std::string, std::vector<xpdata_cifp_data_t>> data_app;    // key = airport id
    std::unordered_map<std::string, int> data_app_idx;    // key = airport id:sid:trans ; value = data_sid index
    
    int legs_array_progressive=0;
    std::unordered_map<int, std::vector<xpdata_cifp_leg_t>> legs_array;

    void task(const std::string &arpt_id) noexcept;
    void parse_cifp_file(const std::string &arpt_id);
    void parse_cifp_file_line(const std::string &arpt_id, int line_no, const std::string &line);

    void parse_sid(const std::string &arpt_id, int line_no, const std::vector<std::string> &splitted);
    void parse_star(const std::string &arpt_id, int line_no, const std::vector<std::string> &splitted);
    void parse_appch(const std::string &arpt_id, int line_no, const std::vector<std::string> &splitted);

    void parse_leg(xpdata_cifp_leg_t &new_leg, const std::vector<std::string> &splitted);

    void finalize_structures();

    int create_new_cifp_data(std::unordered_map<std::string, std::vector<xpdata_cifp_data_t>> &vec_ref, const std::string &arpt_id, const std::vector<std::string> &splitted);

};


} // namespace avionicsbay

#endif

