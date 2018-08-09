#ifndef __LPGPUPOWERMODELSCONFIGMANAGER_H
#define __LPGPUPOWERMODELSCONFIGMANAGER_H

#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

#include <AMDTPowerProfilingMidTier/include/LPGPU2ppPowerModels.h>
#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2PowerModel.h>


class LPGPU2_POWER_MODELING_API lpgpuPowerModelsConfigManager {

public:

    static lpgpuPowerModelsConfigManager& instance() {
        // C++11 ensures that local static variables are only initialised once even in the presence of concurrency.
        // Concerning freeing memory. No need because we want our singleton to live for the entire life of program
        // and gets destroyed on program exit.
        static lpgpuPowerModelsConfigManager* mInstance = new lpgpuPowerModelsConfigManager();
        return *mInstance;
    }

    osFilePath getConfigFilePath();
    gtVector<lpgpuPowerModelDescriptor> getSupportedPowerModels(const gtString& deviceName);


private:

    // Private Constructor
    lpgpuPowerModelsConfigManager();

    // Xml file path and doc root element
    osFilePath mConfigXmlFilePath;
    lpgpu2::PowerModels mPowerModelsInstance;
    bool m_bPowerModelsInitialiased = false;

    // Helper functions
    lpgpuDeviceDescriptor createDeviceDesciptor(const lpgpu2::DeviceElement& deviceElement);
    lpgpuPowerModelDescriptor createModelDesciptor(const lpgpu2::PwrModelElement& modelElement, const lpgpu2::DeviceElement& deviceElement,
                                                                              const lpgpu2::PPMDeviceElement& ppmDeviceElement);


};

#endif //__LPGPUPOWERMODELSCONFIGMANAGER_H
