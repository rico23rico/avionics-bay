#ifndef API_H
#define API_H

#ifdef _WIN32
#define EXPORT_DLL __declspec( dllexport )
#else
#define EXPORT_DLL
#endif

#include "data_types.hpp"

extern "C" {
    EXPORT_DLL xpdata_navaid_array_t get_navaid_by_name  (xpdata_navaid_type_t, const char*);
    EXPORT_DLL xpdata_navaid_array_t get_navaid_by_freq  (xpdata_navaid_type_t, unsigned int);
    EXPORT_DLL xpdata_navaid_array_t get_navaid_by_coords(xpdata_navaid_type_t, double, double, bool);
    EXPORT_DLL bool xpdata_is_ready(void);
}

namespace avionicsbay {
    void api_init();
}

#endif // API_H
