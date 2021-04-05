#ifndef CIFP_PARSER_H
#define CIFP_PARSER_H

#include "utilities/logger.hpp"
#include "xpdata.hpp

namespace avionicsbay {

class CIFPParser {
public:
    CIFPParser(const std::string & xplane_directory);

private:
    std::shared_ptr<Logger> logger;
    std::shared_ptr<XPData> xpdata;
    std::string xplane_directory;
};


} // namespace avionicsbay

#endif

