#ifndef __LPGPUPOWERMODELSDESCRIPTORS
#define __LPGPUPOWERMODELSDESCRIPTORS

#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>


struct LPGPU2_POWER_MODELING_API lpgpuCounterDescriptor {

    lpgpuCounterDescriptor(uint32_t _counterId, gtString _name, gtString _description, gtString _modelMappedName) 
    : counterId(_counterId), name(_name), description(_description), modelMappedName(_modelMappedName) {};

    uint32_t counterId;
    gtString name;
    gtString description;
    gtString modelMappedName;
};

struct LPGPU2_POWER_MODELING_API lpgpuParameterDescriptor {

    lpgpuParameterDescriptor(gtString _name, gtString _type, gtString _value) 
    : name(_name), type(_type), value(_value) {};

    gtString name;
    gtString type;
    gtString value;
};

struct LPGPU2_POWER_MODELING_API lpgpuDeviceDescriptor {
    lpgpuDeviceDescriptor(uint32_t _deviceId, gtString _name, gtString _description) 
    : deviceId(_deviceId), name(_name), description(_description) {};
    
    uint32_t deviceId;
    gtString name;
    gtString description;
    gtVector<lpgpuCounterDescriptor> availableCounters;
};


struct LPGPU2_POWER_MODELING_API lpgpuPowerModelDescriptor {
    lpgpuPowerModelDescriptor(gtString _modelUuid, gtString _name, gtString _description, gtString _longDescription, lpgpuDeviceDescriptor _device) 
    : modelUuid(_modelUuid), name(_name),  description(_description), longDescription(_longDescription),  device(_device) {};
    
    gtString modelUuid;
    gtString name;
    gtString description;
    gtString longDescription;

    lpgpuDeviceDescriptor device;
    gtVector<lpgpuParameterDescriptor> parameters;
    gtVector<lpgpuCounterDescriptor> requiredCounters;

};

#endif // __LPGPUPOWERMODELSDESCRIPTORS