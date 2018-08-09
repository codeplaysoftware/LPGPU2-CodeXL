
//++TLRS: LPGPU2: Renaming the file
#ifndef __AFOPENCLDEVICEINFORMATIONCOLLECTOR_H
#define __AFOPENCLDEVICEINFORMATIONCOLLECTOR_H

// Infra:
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTimer.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>

//++TLRS: LPGPU2: Adding enum to select the kind of device to query info.
enum class EnumDeviceType;
//--TLRS: LPGPU2: Adding enum to select the kind of device to query info.

/// \class afDeviceInformationCollector is a thread used to run afSystemInformationCommand's CollectAllOpenCLDevicesInformation which might take a while
class afDeviceInformationCollector : protected osThread
{
public:
    //++TLRS: LPGPU2: Changing the constructor to receive the device type enum
    explicit afDeviceInformationCollector(EnumDeviceType deviceType);
    //--TLRS: LPGPU2: Changing the constructor to receive the device type enum
    virtual ~afDeviceInformationCollector();

    bool StartCollectingInfo();
    bool& IsActive();
    void StopCollectingInfo();
    bool GetDeviceInformation(gtList<gtList<gtString>>& deviceInfo);

protected:
    // Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination() {};

private:
    //++TLRS: LPGPU2: Adding enum to select the kind of device to query info.
    EnumDeviceType m_deviceType;
    //--TLRS: LPGPU2: Adding enum to select the kind of device to query info.
    bool m_isActive;
    bool m_dataCollectedOk;
    gtList<gtList<gtString>> m_devicesInfoData;  
};

#endif
//--TLRS: LPGPU2: Renaming the file
