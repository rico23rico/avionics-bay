local initialized = false;
local ffi = require("ffi")


local PATH_LIBRARY = "./build/libavionicsbay.so"
local PATH_XPLANE  = "/usr/share/X-Plane 11/"
local PATH_LOGFILE = "/tmp/"

local AvionicsBay = {}

local function convert_cifp_array(rawdata, cifp_arr)
    if rawdata then
        return {
            data = cifp_arr.data,
            len  = cifp_arr.len
        }
    end
    
    to_return = {}
    for i=1,cifp_arr.len do
        local new_dat =  {
            proc_name   = ffi.string(cifp_arr.data[i-1].proc_name,  cifp_arr.data[i-1].proc_name_len),
            trans_name  = ffi.string(cifp_arr.data[i-1].trans_name,  cifp_arr.data[i-1].trans_name_len),
            legs = {}
        }
        
        for j=1,cifp_arr.data[i-1].legs_len do
            local l = cifp_arr.data[i-1].legs[j-1]
            table.insert(new_dat.legs, {
                leg_name = ffi.string(l.leg_name, l.leg_name_len),
                turn_direction = l.turn_direction,
                leg_type = l.leg_type,
                radius = l.radius,
                theta = l.theta,
                rho = l.rho,
                outb_mag = l.outb_mag,
                rte_hold = l.rte_hold,
                outb_mag_in_true = l.outb_mag_in_true,
                rte_hold_in_time = l.rte_hold_in_time,
                cstr_alt_type = l.cstr_alt_type,
                cstr_altitude1 = l.cstr_altitude1,
                cstr_altitude2 = l.cstr_altitude2,
                cstr_speed_type = l.cstr_speed_type,
                cstr_speed = l.cstr_speed,
                center_fix = ffi.string(l.center_fix, l.center_fix_len)
            })
        end
        
        table.insert(to_return, new_dat)
    end
    return to_return
end
    
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
        print(nav_array.navaids[1].category, nav_array.navaids[1].bearing)
        
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
--    while not AvionicsBay.c.xpdata_is_ready() do
--    end
    print("READY")

    AvionicsBay.c.load_cifp("LIML")
    print("WAIT CIFP")
    while not AvionicsBay.c.is_cifp_ready() do
    end
    print("READY CIFP")
    a = AvionicsBay.c.get_cifp("LIML")
    print("NR SIDS: " .. a.sids.len)
    print("NR STARS: " ..  a.stars.len)
    print("NR APPS: " .. a.apprs.len)
    print("NR CIFP RWYS: " .. a.rwys.len)

    print(a.rwys.data[0].loc_ident, a.rwys.data[0].loc_ident_len)

    local x = convert_cifp_array(false, a.sids)
    local x = convert_cifp_array(false, a.stars)
    local x = convert_cifp_array(false, a.apprs)

    if true then
	    return
    end
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

    print("Random MORA: " .. AvionicsBay.c.get_mora(45.52, 10.23))


    AvionicsBay.c.terminate()
    
end


load_avionicsbay()
