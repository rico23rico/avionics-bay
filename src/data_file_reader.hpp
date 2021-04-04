#ifndef DATA_FILE_READER_H
#define DATA_FILE_READER_H

#include "utilities/logger.hpp"
#include "xpdata.hpp"

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

namespace avionicsbay {

class DataFileReader {
public:

    DataFileReader(const std::string & xplane_directory);
    virtual ~DataFileReader() {
        this->worker_stop();   // This is too late, but better than nothing
        this->my_thread.join();
    }

    void worker() noexcept;
    void worker_stop() noexcept {
        this->stop = true;
        cv_apt_details.notify_one();
    }
    
    bool is_worker_running() {
        return this->running;
    }

    void request_apts_details(const std::string &id) noexcept;


private:
    std::atomic<bool> stop;
    std::atomic<bool> running;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<XPData> xpdata;
    std::string xplane_directory;
    
    xpdata_apt_t *detail_arpt = nullptr;    // The airport to load the details
    
    std::thread my_thread;

    std::mutex mx_apt_details;
    std::condition_variable cv_apt_details;

    static constexpr int ROW_NONE = 0;
    static constexpr int ROW_TAXI = 1;
    static constexpr int ROW_LINE = 2;
    static constexpr int ROW_BOUND = 3;
    static constexpr int ROW_HOLE = 4;
    
    int apt_detail_status = ROW_NONE;
    bool apt_detail_is_in_hole = false;
    int current_color = 0;
    std::vector<xpdata_apt_node_t> curr_node_list;

    void perform_init_checks();

    void parse_navaids_file();
    void parse_navaids_file_line(int line_no, const std::string &line);

    void parse_fixes_file();
    void parse_fixes_file_line(int line_no, const std::string &line);

    void parse_apts_file();
    void parse_apts_file_line(int line_no, ssize_t seek_pos, const std::string &line);
    void parse_apts_file_header(int line_no, ssize_t seek_pos, const std::vector<std::string> &splitted);
    void parse_apts_file_runway(int line_no, const std::vector<std::string> &splitted);
    void parse_apts_details(xpdata_apt_t *detail_arpt);
    bool parse_apts_details_line(xpdata_apt_t *, int line_no, const std::string &line); // Returns true if airport header found
    void parse_apts_details_tower(xpdata_apt_t *detail_arpt, const std::vector<std::string> &splitted);
    void parse_apts_details_linear_start(const std::vector<std::string> &splitted);
    void parse_apts_details_beizer_start(const std::vector<std::string> &splitted);
    void parse_apts_details_linear_close(xpdata_apt_t *arpt, const std::vector<std::string> &splitted);
    void parse_apts_details_beizer_close(xpdata_apt_t *arpt, const std::vector<std::string> &splitted);
    void parse_apts_details_linear_end(xpdata_apt_t *arpt, const std::vector<std::string> &splitted);
    void parse_apts_details_beizer_end(xpdata_apt_t *arpt, const std::vector<std::string> &splitted);
    void parse_apts_details_route_point(xpdata_apt_t *arpt, const std::vector<std::string> &splitted);
    void parse_apts_details_route_taxi(xpdata_apt_t *arpt, const std::vector<std::string> &splitted);
    void parse_apts_details_arpt_gate(xpdata_apt_t *arpt, const std::vector<std::string> &splitted);

    void parse_apts_details_save(xpdata_apt_t *arpt);
    
    void parse_mora_file();
    void parse_mora_line(int line_no, const std::string &line);

};

}

#endif
