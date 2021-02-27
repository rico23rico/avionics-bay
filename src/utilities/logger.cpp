#include "logger.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>

#define LOG_FILENAME "xpfiles-internal.log"
#define MAX_QUEUE_LOGS 1024

static std::ofstream log_file;
static std::mutex    mx_log_file;

namespace xpfiles {

LoggerCloser ENDL;

static const char* levels_to_STR(logger_level_t lev) {
    switch(lev) {
    case logger_level_t::ALERT:
        return "[ALERT] ";
    case logger_level_t::CRIT:
        return "[CRIT]  ";
    case logger_level_t::ERROR:
        return "[ERROR] ";
    case logger_level_t::WARN:
        return "[WARN]  ";
    case logger_level_t::NOTICE:
        return "[NOTICE]";
    case logger_level_t::INFO:
        return "[INFO]  ";
    case logger_level_t::DEBUG:
        return "[DEBUG] ";
    }

    return "[UNKN]  ";
}

static std::string time_in_HH_MM_SS_MMM()
{
    // From here: https://stackoverflow.com/questions/24686846/get-current-time-in-milliseconds-or-hhmmssmmm-format
    
    using namespace std::chrono;

    // get current time
    auto now = system_clock::now();

    // get number of milliseconds for the current second
    // (remainder after division into seconds)
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // convert to std::time_t in order to convert to std::tm (broken time)
    auto timer = system_clock::to_time_t(now);

    // convert to broken time
    std::tm bt = *std::localtime(&timer);

    std::ostringstream oss;

    oss << std::put_time(&bt, "%H:%M:%S"); // HH:MM:SS
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}


Logger::Logger()
{

    log_file.exceptions(~std::ios_base::goodbit);
    const std::string filename = LOG_FILENAME;
    // Try to open the log file
    log_file.open(filename, std::ios_base::app);
}

Logger::~Logger() noexcept {
    try {
        log_file.close();
    } catch(...) {}
}

void Logger::save_log(logger_level_t level, const std::string &message) noexcept {
    if(level >= min_lvl) {

        // Build the time string:
        std::string datetime = time_in_HH_MM_SS_MMM();

        std::string full_message = '[' + datetime + "] " +  levels_to_STR(level) + ' ' + message;

        {
            std::lock_guard<std::mutex> lk(mx_log_file);
            log_file << full_message << std::endl;
        }
    }
    curr_msg = "";
}


} // namespace yalp

