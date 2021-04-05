#ifndef CIFP_PARSER_H
#define CIFP_PARSER_H

#include "utilities/logger.hpp"
#include "xpdata.hpp"

#include <future>

namespace avionicsbay {

class CIFPParser {
public:
    CIFPParser(const std::string & xplane_directory);

    void perform_init_checks();

    void load_airport(const std::string &arpt_id);

    bool is_ready() noexcept;

    void task(const std::string &arpt_id) noexcept;

private:
    std::string xplane_directory;

    std::shared_ptr<Logger> logger;
    std::shared_ptr<XPData> xpdata;
    std::future<void> ap_future;
};


} // namespace avionicsbay

#endif

