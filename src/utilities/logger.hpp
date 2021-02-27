#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace xpfiles {

typedef enum class logger_level_e {
    DEBUG=0,
    INFO,
    NOTICE,
    WARN,
    ERROR,
    CRIT,
    ALERT
} logger_level_t;

extern class LoggerCloser {
} ENDL;

class Logger
{
public:

    void set_min_lvl(logger_level_t lvl) noexcept {
        this->min_lvl = lvl;
    }

    template<typename T>
    Logger &operator<<(T msg) {
        curr_msg += std::to_string(msg);
        return *this;
    }

    Logger();
    ~Logger() noexcept;

private:

    Logger(const Logger&) = delete;
    Logger& operator=(Logger const&) = delete;

    logger_level_t min_lvl = logger_level_t::DEBUG;

    void save_log(logger_level_t level, const std::string &message) noexcept;

    std::string curr_msg;   // Do not need synchronization at this stage
    logger_level_t curr_level;   // Do not need synchronization at this stage

};

template<>
inline Logger &Logger::operator<<<const std::string &>(const std::string &msg) {
    curr_msg += msg;
    return *this;
}
template<>
inline Logger &Logger::operator<<<std::string>(std::string msg) {
    curr_msg += msg;
    return *this;
}
template<>
inline Logger &Logger::operator<<<const char*>(const char* msg) {
    curr_msg += msg;
    return *this;
}

template<>
inline Logger &Logger::operator<<<char>(char c) {
    curr_msg += c;
    return *this;
}

template<>
inline Logger &Logger::operator<<<logger_level_t>(logger_level_t level) {
    this->curr_level = level;
    return *this;
}

template<>
inline Logger &Logger::operator<<<LoggerCloser>(LoggerCloser) {
    this->save_log(this->curr_level, this->curr_msg);
    return *this;
}

} // namespace simnodes

#endif // LOGGER_H
