#include "triangulator.hpp"

#include "utilities/earcut.hpp"

#include <iostream>

namespace avionicsbay {

const std::vector<xpdata_coords_t> empty_vector;

const std::vector<xpdata_coords_t> & Triangulator::triangulate(const xpdata_apt_node_array_t* array) {
    if (array == nullptr) {    // Uh?
        return empty_vector;
    }
    
    if (array->nodes_len == 0) {    // Uh?
        return empty_vector;
    }

    if (previous_data.count(array) > 0) {
        return previous_data.at(array);
    }
    
    using Coord = double;
    using N = uint_fast16_t;
    using Point = std::array<Coord, 2>;
    std::vector<std::vector<Point>> full_dataset;

    std::vector<Point> real_polygon;
    for (int i=0; i < array->nodes_len; i++) {
        real_polygon.push_back({{array->nodes[i].coords.lat, array->nodes[i].coords.lon}});
    }
    
    full_dataset.push_back(real_polygon);
    
    xpdata_apt_node_array_t *curr_hole = array->hole;
    while (curr_hole != nullptr) {
        std::vector<Point> hole_polygon;
        for (int i=0; i < curr_hole->nodes_len; i++) {
            hole_polygon.push_back({curr_hole->nodes[i].coords.lat, curr_hole->nodes[i].coords.lon});
        }
        
        full_dataset.push_back(std::move(hole_polygon));
        
        curr_hole = curr_hole->hole;
    }

    std::vector<N> indices = mapbox::earcut<N>(full_dataset);
    
    for (auto i : indices) {
        
        if (i < array->nodes_len) {
        
            previous_data[array].push_back(array->nodes[i].coords);
        
        } else {
            i = i - array->nodes_len;
            xpdata_apt_node_array_t *curr_hole = array->hole;
            while (i >= curr_hole->nodes_len) {
                i = i - curr_hole->nodes_len;
                curr_hole = curr_hole->hole;
            }
            
            previous_data[array].push_back(curr_hole->nodes[i].coords);
        }
    
        
    }

    return previous_data[array];
}

} // avionicsbay
