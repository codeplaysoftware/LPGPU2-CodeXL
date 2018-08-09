
#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2PowerModelsConfigManager.h>

// Private Constructor
lpgpuPowerModelsConfigManager::lpgpuPowerModelsConfigManager() {

    bool retVal = mConfigXmlFilePath.SetInstallRelatedPath(osFilePath::OS_CODEXL_BINARIES_PATH);
    GT_IF_WITH_ASSERT(retVal)
    {
        // TODO change filepath and extension constants
        mConfigXmlFilePath.appendSubDirectory(L"LPGPU2_PowerModels");
        mConfigXmlFilePath.setFileName(L"LPGPU2PowerModelsConfigSimp");
        //mConfigXmlFilePath.setFileName(L"lpgpuPowerModelsConfig");
        //mConfigXmlFilePath.setFileName(L"LPGPU2PowerModels");
        mConfigXmlFilePath.setFileExtension(L"xml");

        if (mConfigXmlFilePath.exists())
        {
            // Parse the config file
            m_bPowerModelsInitialiased = mPowerModelsInstance.init( mConfigXmlFilePath.asString() ) == lpgpu2::PPFnStatus::success;
        }
    }
}

osFilePath lpgpuPowerModelsConfigManager::getConfigFilePath() {
    return mConfigXmlFilePath;
}

gtVector<lpgpuPowerModelDescriptor> lpgpuPowerModelsConfigManager::getSupportedPowerModels(const gtString& deviceName) {

    gtVector<lpgpuPowerModelDescriptor> pwDesciptors;

    if (m_bPowerModelsInitialiased)
    {
        // Get the the supported devices
        auto& supportedDevices = mPowerModelsInstance.GetElements().GetDevices();
        auto& supportedModels = mPowerModelsInstance.GetElements().GetPwrModels();

        for (const auto& device : supportedDevices) {

            // Search for the passed device name
            if (device.GetName().compare(deviceName) == 0) {

                // Search all supported model using the supplied device ID
                for (const auto& model : supportedModels) {
                    for(const auto& modelSupportedDevice : model.GetDevices()) {

                        if (device.GetId() == modelSupportedDevice.GetId()) {

                            // lpgpuPowerModelDescriptor descriptor(model.GetUUID(), model.GetName(), model.GetDescription(), model.GetLongDescription(),
                            //                                     createDeviceDesciptor(device)
                            // );

                            pwDesciptors.push_back(createModelDesciptor(model, device, modelSupportedDevice) );
                        }
                    }
                }
                return pwDesciptors;
            }
        }
    }

    return pwDesciptors;
}

// Private functions
lpgpuDeviceDescriptor lpgpuPowerModelsConfigManager::createDeviceDesciptor(const lpgpu2::DeviceElement& deviceElement) {

    gtVector<lpgpuCounterDescriptor> availableCounters;

    for ( auto& counterElement : deviceElement.GetCounterElemsVec()) {
        availableCounters.push_back( lpgpuCounterDescriptor(counterElement.GetId(), counterElement.GetName(), counterElement.GetDescription(),
                                                            counterElement.GetModelMappedName() ));
    }

    lpgpuDeviceDescriptor deviceDescriptor(deviceElement.GetId(), deviceElement.GetName(), deviceElement.GetDescription());
    deviceDescriptor.availableCounters = availableCounters;

    return deviceDescriptor;
}

lpgpuPowerModelDescriptor lpgpuPowerModelsConfigManager::createModelDesciptor(const lpgpu2::PwrModelElement& modelElement,
                                                                              const lpgpu2::DeviceElement& deviceElement,
                                                                              const lpgpu2::PPMDeviceElement& ppmDeviceElement) {

    lpgpuPowerModelDescriptor modelDescriptor(modelElement.GetUUID(), modelElement.GetName(), modelElement.GetDescription(),
                                              modelElement.GetLongDescription(),  createDeviceDesciptor(deviceElement));




    // Get all required counters
    gtVector<lpgpuCounterDescriptor> requiredCounters;
    for (auto& countElement : ppmDeviceElement.GetRequiredCounters()) {

        // Search and add the device counter with the corresponding required counter ID.
        for (auto& deviceCounter : modelDescriptor.device.availableCounters) {
            if (countElement == deviceCounter.counterId)
                requiredCounters.push_back(deviceCounter);
        }
    }
    modelDescriptor.requiredCounters = requiredCounters;

    // Get all the parameters
    gtVector<lpgpuParameterDescriptor> parameters;
    for ( auto& paramElement : ppmDeviceElement.GetParameterElemsVec()) { // change this
        parameters.push_back(lpgpuParameterDescriptor(paramElement.GetName(), paramElement.GetType(), paramElement.GetValue()));
    }
    modelDescriptor.parameters = parameters;

    return modelDescriptor;

}
