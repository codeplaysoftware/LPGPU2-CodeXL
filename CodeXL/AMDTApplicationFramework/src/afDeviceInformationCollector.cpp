//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afDeviceInformationCollector.cpp
///
//==================================================================================
#include "../Include/afDeviceInformationCollector.h"

// infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// local
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>
#include <AMDTApplicationFramework/Include/afDeviceInformationCollector.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// ---------------------------------------------------------------------------
afDeviceInformationCollector::afDeviceInformationCollector(EnumDeviceType deviceType)
    : osThread(L"DeviceInfoCollector")
    , m_deviceType(deviceType)
    , m_isActive(false)
    , m_dataCollectedOk(false)
{
}

// ---------------------------------------------------------------------------
afDeviceInformationCollector::~afDeviceInformationCollector()
{
}

// ---------------------------------------------------------------------------
int afDeviceInformationCollector::entryPoint()
{
    int retVal = 0;
    afSystemInformationCommand sysInfoCmd;

    switch (m_deviceType)
    {
        case EnumDeviceType::kOpenCL:
            m_dataCollectedOk = sysInfoCmd.CollectAllOpenCLDevicesInformation(m_devicesInfoData);
            break;
        case EnumDeviceType::kSYCL:
            m_dataCollectedOk = sysInfoCmd.CollectAllSYCLDevicesInformation(m_devicesInfoData);
            break;
        default:
            break;
    }   

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"m_dataCollectedOk = %d, m_devicesInfoData.size = %d", m_dataCollectedOk, m_devicesInfoData.size());

    m_isActive = false;

    return retVal;
}

// ---------------------------------------------------------------------------
bool afDeviceInformationCollector::StartCollectingInfo()
{
    bool retVal = false;
    m_dataCollectedOk = false;
    m_isActive = false;
    m_devicesInfoData.clear();

    if (osThread::execute())
    {
        m_dataCollectedOk = true;
        m_isActive = true;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool& afDeviceInformationCollector::IsActive()
{
    return m_isActive;
}

// ---------------------------------------------------------------------------
void afDeviceInformationCollector::StopCollectingInfo()
{
    if (m_isActive)
    {
        m_dataCollectedOk = false;
        // Terminate thread
        osThread::terminate();
        // Clear data as the thread didn't finish it's work
        m_devicesInfoData.clear();
        m_isActive = false;
    }
}

// ---------------------------------------------------------------------------
bool afDeviceInformationCollector::GetDeviceInformation(gtList<gtList<gtString>> &deviceInfo)
{
    bool retVal = false;
    deviceInfo.clear();

    if (!m_isActive && m_dataCollectedOk)
    {
        deviceInfo = m_devicesInfoData;
        retVal = true;
    }

    return retVal;
}