#include <cassert>
#include <memory>
#include <string>

#include "plugin.hpp"
#include "utilities/logger.hpp"

#include "wmm_interface.hpp"
#include "wmm/EGM9615.h"
#include "wmm/GeomagnetismHeader.h"

#define LOG *logger << STARTL


#define WMM_COF_FILE "plugins/avionicsbay/WMM.COF"

namespace avionicsbay {

    static MAGtype_Ellipsoid Ellip;
    static MAGtype_MagneticModel * MagneticModels[1], *TimedMagneticModel;
    std::shared_ptr<Logger> logger;


    double get_declination(double lat, double lon, unsigned short year) {
        MAGtype_CoordGeodetic CoordGeodetic;
        CoordGeodetic.phi = lat;
        CoordGeodetic.lambda = lon;
        CoordGeodetic.HeightAboveGeoid = 0;
        CoordGeodetic.UseGeoid = 0;

        MAGtype_Date UserDate;
        UserDate.DecimalYear = year;

        MAGtype_CoordSpherical CoordSpherical;
        MAGtype_GeoMagneticElements GeoMagneticElements, Errors;
        MAG_GeodeticToSpherical(Ellip, CoordGeodetic, &CoordSpherical);
        MAG_TimelyModifyMagneticModel(UserDate, MagneticModels[0], TimedMagneticModel); /* Time adjust the coefficients, Equation 19, WMM Technical report */
        MAG_Geomag(Ellip, CoordSpherical, CoordGeodetic, TimedMagneticModel, &GeoMagneticElements); /* Computes the geoMagnetic field elements and their time change*/
        MAG_CalculateGridVariation(CoordGeodetic, &GeoMagneticElements);

        return GeoMagneticElements.Decl;
    }

    bool init_wmm_interface(std::string plane_path) {
        logger = get_logger();
        
        assert(logger);

        LOG << logger_level_t::DEBUG << "Initializing WMM Interface..." << ENDL;


        MAGtype_Geoid Geoid;
        int epochs = 1;

        std::string filename = plane_path + "/" + WMM_COF_FILE;
        LOG << logger_level_t::DEBUG << "Opening " << filename << "..." << ENDL;
        if(!MAG_robustReadMagModels(const_cast<char*>(filename.c_str()), &MagneticModels, epochs)) {
            LOG << logger_level_t::CRIT << "[WMM] MAG_robustReadMagModels failed." << ENDL;
            return false;
        }    

        int NumTerms, Flag = 1, nMax = 0;
        if(nMax < MagneticModels[0]->nMax) {
            nMax = MagneticModels[0]->nMax;
        }
        NumTerms = ((nMax + 1) * (nMax + 2) / 2);
        TimedMagneticModel = MAG_AllocateModelMemory(NumTerms); /* For storing the time modified WMM Model parameters */
        if(MagneticModels[0] == NULL || TimedMagneticModel == NULL)
        {
            LOG << logger_level_t::CRIT << "[WMM] MAG_AllocateModelMemory failed." << ENDL;
            return false;
        }
        MAG_SetDefaults(&Ellip, &Geoid); /* Set default values and constants */

        /* Set EGM96 Geoid parameters */
        Geoid.GeoidHeightBuffer = GeoidHeightBuffer;
        Geoid.Geoid_Initialized = 1;
        Geoid.UseGeoid = 0;

        LOG << logger_level_t::DEBUG << "[WMM] Ready." << ENDL;

        return true;
    }

}

