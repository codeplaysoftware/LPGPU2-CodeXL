
#ifndef __LPGPUPOWERMODELSFACTORY_H
#define __LPGPUPOWERMODELSFACTORY_H

#include <memory>

#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2PowerModel.h>
#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2LinearPowerModel.h>


/**
 * This class uses the singleton design pattern
 *
 * This implements the engine factory responsible for creating engines.
 */
class LPGPU2_POWER_MODELING_API lpgpuPowerModelsFactory {
public:

    // Singleton instance
    static lpgpuPowerModelsFactory &instance();

    // Factory method
    static std::shared_ptr<lpgpuPowerModel> createPowerModel(lpgpuPowerModelDescriptor& modelDescriptor, const gtString& dbPath);
};

#endif //__LPGPUPOWERMODELSFACTORY_H