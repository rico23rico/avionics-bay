#ifndef XPDATA_H
#define XPDATA_H

namespace xpfiles {

class XPData {

public:
    XPData() : is_ready(is_ready) {
    
    }

    void set_is_ready(bool is_ready) noexcept { this->is_ready = is_ready; }
    bool get_is_ready() const        noexcept { return this->is_ready; }

private:
    bool is_ready;

};


} // namespace xpfiles


#endif
