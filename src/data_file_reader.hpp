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

private:
    std::atomic<bool> stop;
    std::atomic<bool> running;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<XPData> xpdata;
    std::string xplane_directory;
    
    std::thread my_thread;
    
    
    void perform_init_checks();

    void parse_navaids_file();
    void parse_navaids_file_line(int line_no, const std::string &line);

    void parse_fixes_file();
    void parse_fixes_file_line(int line_no, const std::string &line);

    void parse_apts_file();
    void parse_apts_file_line(int line_no, ssize_t seek_pos, const std::string &line);
    void parse_apts_file_header(int line_no, ssize_t seek_pos, const std::vector<std::string> &splitted);
    void parse_apts_file_runway(int line_no, const std::vector<std::string> &splitted);

    std::mutex mx_apt_details;
    std::condition_variable cv_apt_details;

};

}

#endif
