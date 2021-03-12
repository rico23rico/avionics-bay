local initialized = false;
local ffi = require("ffi")


local PATH_LIBRARY = "./build/libavionicsbay.so"
local PATH_XPLANE  = "/usr/share/X-Plane 11/"
local PATH_LOGFILE = "/tmp/"

local AvionicsBay = {}
    
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
    ffi.cdef(require("avionicsbay_lua_include"))
    AvionicsBay.c = ffi.load(PATH_LIBRARY)
    
    if AvionicsBay.c.initialize(PATH_XPLANE, PATH_LOGFILE) then
        initialized = true
        print("Initialized")
    else
        initialized = false
        print("not Initialized")
    end
    
    expose_functions()

    AvionicsBay.c.set_acf_coords(32.149534655, -110.83512209);

    print("WAIT")
    while not AvionicsBay.c.xpdata_is_ready() do
    end
    print("READY")

    print(AvionicsBay.test())
    
    while AvionicsBay.c.get_nearest_apt() == nil do
    end
    
    print(ffi.string(AvionicsBay.c.get_nearest_apt().id))

    print("Loading details...")
    AvionicsBay.c.request_apts_details("LIRF");
    
    while not AvionicsBay.c.get_apts_by_name("LIRF").apts[0].is_loaded_details do
    end
    print("Loaded...")

    local airport = AvionicsBay.c.get_apts_by_name("LIRF").apts[0];
    print("Numer of pavements: " .. airport.details.pavements_len)
    print("Numer of linear features: " .. airport.details.linear_features_len)
    print("Numer of boundaries: " .. airport.details.boundaries_len)
    print("Numer of routes: " .. airport.details.routes_len)
    print("Numer of gates: " .. airport.details.gates_len)
    
    for i=0,10 do
        print("Pavements[".. i .."], color: " .. airport.details.pavements[i].color)
        print("Pavements[".. i .."], nr. nodes: " .. airport.details.pavements[i].nodes_len)
        print("Pavements[".. i .."], has_hole: " .. (airport.details.pavements[i].hole ~= nil and "YES" or "NO"))
        local triangles = AvionicsBay.c.triangulate(airport.details.pavements[i])
        print("Total triangles: " .. triangles.points_len)
    end
    print("Random route name: " .. ffi.string(airport.details.routes[2].name))
    print("Random route lat: " .. AvionicsBay.c.get_route_pos(airport, airport.details.routes[2].route_node_1).lat)

    AvionicsBay.c.terminate()
    
end


load_avionicsbay()
