#ifndef TRIANGULATOR_H
#define TRIANGULATOR_H

#include "data_types.hpp"

#include <unordered_map>
#include <vector>

namespace avionicsbay {

class Triangulator {

public:
    const std::vector<xpdata_coords_t> & triangulate(const xpdata_apt_node_array_t*);

private:
    std::unordered_map<const xpdata_apt_node_array_t*, std::vector<xpdata_coords_t>> previous_data;

};


} // avionicsbay
#endif

