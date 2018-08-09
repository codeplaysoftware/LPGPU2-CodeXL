#ifndef __LPGPUPOWERMODEL
#define __LPGPUPOWERMODEL

#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2PowerModelsAPIDefs.h>

#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2PowerModelsDescriptors.h>

// LPGPU2Database
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAccessor.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>

#include <memory>

class LPGPU2_POWER_MODELING_API lpgpuPowerModel
{
public:
    lpgpuPowerModel(lpgpuPowerModelDescriptor descriptor, const gtString& dbPath ) : mDescriptor(descriptor), mDbPath(dbPath) {
        mpDbAdapter = std::make_shared<lpgpu2::db::LPGPU2DatabaseAdapter>();
        
        // NOTE: Initialise needs to be called after constructing the adapter object
        bool ret = mpDbAdapter->Initialise();

        if (ret)
        {
            ret &= mpDbAdapter->OpenDb(dbPath, AMDT_PROFILE_MODE_TIMELINE, false);
        }
    }

    // Pure virtual
    virtual void estimatePower() = 0;

protected:

    lpgpuPowerModelDescriptor mDescriptor;
    gtString mDbPath;
    std::shared_ptr<lpgpu2::db::LPGPU2DatabaseAdapter> mpDbAdapter;

};



#endif //__LPGPUPOWERMODEL
