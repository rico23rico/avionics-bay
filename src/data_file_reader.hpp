#ifndef DATA_FILE_READER_H
#define DATA_FILE_READER_H

#include "utilities/logger.hpp"

#include <string>
#include <thread>

namespace xpfiles {

class DataFileReader {
public:

    DataFileReader(const std::string & xplane_directory);

    void worker() noexcept;

private:
    std::shared_ptr<Logger> logger;
    std::string xplane_directory;
    
    std::thread my_thread;
};

}

#endif
