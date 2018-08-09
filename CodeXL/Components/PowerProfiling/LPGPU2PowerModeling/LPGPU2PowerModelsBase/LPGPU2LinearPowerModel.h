#ifndef __LPGPULINEARPOWERMODEL
#define __LPGPULINEARPOWERMODEL

#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2PowerModel.h>

class LPGPU2_POWER_MODELING_API lpgpuLinearPowerModel : public lpgpuPowerModel 
{
public:

    lpgpuLinearPowerModel(lpgpuPowerModelDescriptor descriptor, const gtString& dbPath  ) : lpgpuPowerModel(descriptor, dbPath) {};

    virtual void estimatePower();


private:
    
};



#endif //__LPGPULINEARPOWERMODEL
