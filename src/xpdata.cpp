#include "xpdata.hpp"


namespace xpfiles {

void XPData::push_navaid(xpdata_navaid_t &&navaid) noexcept {
    navaids_all[navaid.type].push_back(std::move(navaid));
}

void XPData::index_navaids_by_name() noexcept {

    for(int i=0; i < navaids_all.size(); i++) {
        for(int j=0; j < navaids_all[i].size(); j++) {
            auto element_ptr = &navaids_all[i][j];
        
            auto id_str = std::string(element_ptr->id);

            if (navaids_name[element_ptr->type].count(id_str) == 1) {
                navaids_name[element_ptr->type].at(id_str).push_back(element_ptr);
            } else {
                navaids_name[element_ptr->type][id_str].push_back(element_ptr);
            }
        }
    }
}

std::pair<const xpdata_navaid_t* const*, size_t> XPData::get_navaids_by_name(xpdata_navaid_type_t type, std::string name) const noexcept {
    try {
        const auto & element = this->navaids_name.at(type).at(name);
        return std::pair<const xpdata_navaid_t* const*, size_t> (element.data(), element.size());
    } catch(...) {
        return std::pair<const xpdata_navaid_t* const*, size_t> (nullptr, 0);
    }
}

} // namespace xpfiles
