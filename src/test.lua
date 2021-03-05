local initialized = false;
local ffi = require("ffi")

AvionicsBay = {}
    
local function expose_functions()

    AvionicsBay.is_initialized = function()
        return initialized
    end

    AvionicsBay.get_error = function()
        return ffi.string(AvionicsBay.c.get_error())
    end
    
    AvionicsBay.test = function()
        nav_array = AvionicsBay.c.get_navaid_by_name(3, "SRN");
        
        print(nav_array.len)
        print(nav_array.navaids[1].id_len)
        print(nav_array.navaids[1].full_name_len)
        print(ffi.string(nav_array.navaids[1].full_name, nav_array.navaids[1].full_name_len))
        
        nav_array = AvionicsBay.c.get_navaid_by_freq(3, 11370);
        print("NAVAID BY FREQ: " .. nav_array.len)
        print(ffi.string(nav_array.navaids[1].full_name, nav_array.navaids[1].full_name_len))
        
        nav_array = AvionicsBay.c.get_navaid_by_coords(3, 45.646801, 9.022850);
        print("NAVAID BY COORDS: " .. nav_array.len)
        print(ffi.string(nav_array.navaids[1].full_name, nav_array.navaids[1].full_name_len))
        
        fix_array = AvionicsBay.c.get_fixes_by_name("ROMEO");
        print("FIX BY NAME: " .. fix_array.len)
        print(fix_array.fixes[1].coords.lat, fix_array.fixes[1].coords.lon)

        fix_array = AvionicsBay.c.get_fixes_by_coords(45.646801, 9.022850);
        print("FIX BY COORDS: " ..fix_array.len)

        apt_array = AvionicsBay.c.get_apts_by_name("LIRF");
        print("APT BY NAME: " .. apt_array.len)
        print("  APT NAME: " .. ffi.string(apt_array.apts[0].full_name, apt_array.apts[0].full_name_len))
        print("  APT SEEK: " .. tonumber(apt_array.apts[0].pos_seek))
        print("  APT COORDS: " .. apt_array.apts[0].apt_center.lat .. " " .. apt_array.apts[0].apt_center.lon)
        print("  NR. RWY: " .. apt_array.apts[0].rwys_len)
        print("  RWY COORDS: " .. apt_array.apts[0].rwys[1].coords.lat .. " " .. apt_array.apts[0].rwys[1].coords.lon)
        print("  RWY NAME: " .. ffi.string(apt_array.apts[0].rwys[1].name) .. "/" .. ffi.string(apt_array.apts[0].rwys[1].sibl_name))
        
    end
end

local function load_avionicsbay()
        path = "./libavionicsbay.so"

    ffi.cdef[[
        typedef int xpdata_navaid_type_t;

        typedef struct xpdata_coords_t {
            double lat;
            double lon;
        } xpdata_coords_t;
        typedef struct xpdata_navaid_t {
            const char *id;         // e.g., SRN
            int id_len;
            const char *full_name;  // e.g., Saronno VOR
            int full_name_len;
            xpdata_navaid_type_t type; // Constants NAV_ID_* 
            xpdata_coords_t coords;
            int altitude;
            unsigned int frequency;
        } xpdata_navaid_t;

        typedef struct xpdata_navaid_array_t {
            const struct xpdata_navaid_t * const * navaids;
            int len;
        } xpdata_navaid_array_t;

        typedef struct xpdata_fix_t {
            const char *id;         // e.g., ROMEO
            int id_len;
            xpdata_coords_t coords;
        } xpdata_fix_t;

        typedef struct xpdata_fix_array_t {
            const struct xpdata_fix_t * const * fixes;
            int len;
        } xpdata_fix_array_t;

        typedef struct xpdata_apt_rwy_t {
            char name[4];
            char sibl_name[4];              // On the other head of the runway

            xpdata_coords_t coords;
            xpdata_coords_t sibl_coords;    // On the other head of the runway
            
            double width;
            int surface_type;
            bool has_ctr_lights;
            
        } xpdata_apt_rwy_t;

        typedef struct xpdata_apt_t {
            const char *id;         // e.g., LIRF
            int id_len;
            
            const char *full_name;  // e.g., Roma Fiumicino
            int full_name_len;
            
            int altitude;

            const xpdata_apt_rwy_t *rwys;
            int rwys_len;
            
            xpdata_coords_t apt_center;
            
            long pos_seek;   // For internal use only, do not modify this value
            
        } xpdata_apt_t;

        typedef struct xpdata_apt_array_t {
            const struct xpdata_apt_t * const * apts;
            int len;
        } xpdata_apt_array_t;

        bool initialize(const char*);
        const char* get_error(void);
        xpdata_navaid_array_t get_navaid_by_name(xpdata_navaid_type_t type, const char* name);
        xpdata_navaid_array_t get_navaid_by_freq  (xpdata_navaid_type_t, unsigned int);
        xpdata_navaid_array_t get_navaid_by_coords(xpdata_navaid_type_t, double, double);
        xpdata_fix_array_t get_fixes_by_name  (const char*);
        xpdata_fix_array_t get_fixes_by_coords(double, double);
        xpdata_apt_array_t get_apts_by_name  (const char*);
        xpdata_apt_array_t get_apts_by_coords(double, double);
        
        
        const xpdata_apt_t* get_nearest_apt();
        void set_acf_coords(double lat, double lon);
        bool xpdata_is_ready(void);
    ]]

    AvionicsBay.c = ffi.load(path)
    
    if AvionicsBay.c.initialize("/usr/share/X-Plane 11/") then
        initialized = true
        print("Initialized")
    else
        initialized = false
        print("not Initialized")
    end
    
    expose_functions()

    AvionicsBay.c.set_acf_coords(40, 9);

    print("WAIT")
    while not AvionicsBay.c.xpdata_is_ready() do
    end
    print("READY")

    print(AvionicsBay.test())
    
    while AvionicsBay.c.get_nearest_apt() == nil do
    end
    
    print(ffi.string(AvionicsBay.c.get_nearest_apt().id))
    
end


load_avionicsbay()
