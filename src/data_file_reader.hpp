#ifndef DATA_FILE_READER_H
#define DATA_FILE_READER_H

#include "utilities/logger.hpp"
#include "xpdata.hpp"

#include <string>
#include <thread>

namespace xpfiles {

class DataFileReader {
public:

    DataFileReader(const std::string & xplane_directory);

    void worker() noexcept;

private:
    std::shared_ptr<Logger> logger;
    std::shared_ptr<XPData> xpdata;
    std::string xplane_directory;
    
    std::thread my_thread;
    
    
    void perform_init_checks();

    void parse_navaids_file();
    void parse_navaids_file_line(int line_no, const std::string &line);

};

}

#endif
