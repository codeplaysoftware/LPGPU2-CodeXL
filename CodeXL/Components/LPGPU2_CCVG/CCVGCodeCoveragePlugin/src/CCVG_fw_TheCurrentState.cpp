// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTheCurrentState implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// TinyXml:
#include <tinyxml.h>

// Framework:
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGAppWrapper.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGProjectBuildDefinitions.h>

namespace ccvg {
namespace fw {

// Instantiations:
bool                                      CCVGFWTheCurrentState::ms_bIsCCVGBusy = true;
bool                                      CCVGFWTheCurrentState::ms_bProjOpen = false;  
CCVGFWTheCurrentStateProjectSettings*     CCVGFWTheCurrentState::ms_pProjSetting = nullptr;
CCVGFWTheCurrentStateProjectData*         CCVGFWTheCurrentState::ms_pProjData = nullptr;
gtString                                  CCVGFWTheCurrentState::ms_xmlGlobalStateName = L"CCVGGlobalState";
CCVGFWTheCurrentState::SettingsXMLTagIDs  CCVGFWTheCurrentState::ms_xmlSettingTagIDs;
CCVGFWTheCurrentState::SettingsConst      CCVGFWTheCurrentState::ms_settingsDefault =
{
  false  // bCcvgAllProjectsEnabledFlag
};

/// @brief  Class constructor. No work is done here by this class. 
CCVGFWTheCurrentState::NotifyOnSettingChangedCcvgAllProjectsEnabledFlag::NotifyOnSettingChangedCcvgAllProjectsEnabledFlag()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGFWTheCurrentState::NotifyOnSettingChangedCcvgAllProjectsEnabledFlag::~NotifyOnSettingChangedCcvgAllProjectsEnabledFlag()
{
}

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentState::SettingsXMLTagIDs::SettingsXMLTagIDs()
: ccvgAllProjectsEnabledFlag(L"CCVGAllPRojectsEnabledFlag")
{
}

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentState::Settings::Settings()
: bCcvgAllProjectsEnabledFlag(false)
{
}

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentState::Settings::Settings(bool vbAllProjectsEnabledFlag)
: bCcvgAllProjectsEnabledFlag(vbAllProjectsEnabledFlag)
{
}

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentState::SettingsConst::SettingsConst()
{
}

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentState::SettingsConst::SettingsConst(bool vbAllProjectsEnabledFlag)
: Settings(vbAllProjectsEnabledFlag)
{
}

/// @brief Class constructor. Copy the CCVG default operational parameter values
///        to the user entered equivalent parameters.
CCVGFWTheCurrentState::CCVGFWTheCurrentState()
: m_bDoNotNotifySettingChangedCcvgAllProjectsEnabledFlag(false)
{
  SettingsWorkingResetToDefault();
}

/// @brief Class destructor. Tidy up release resources used by *this instance.
CCVGFWTheCurrentState::~CCVGFWTheCurrentState()
{
}

/// @brief  Retrieve the ID of the computer CodeXL is running on now.
/// @return gtString& String text ID.
const gtString& CCVGFWTheCurrentState::GetComputerIDCurrent() const
{
  return m_computerIDCurrent;
}

/// @brief  Retrieve the ID of the user currently running CodeXL now.
/// @return gtString& String text ID.
const gtString& CCVGFWTheCurrentState::GetUserIDCurrent() const
{
  return m_userIDCurrent;
}

/// @brief     Set the ID of the computer CodeXL is running on now.
/// @param[in] vCurrentPCId Text ID.
void CCVGFWTheCurrentState::SetComputerIDCurrent(const gtString &vCurrentPCId)
{
  m_computerIDCurrent = vCurrentPCId;
}

/// @brief     Set the ID of the user using CodeXL at the moment.
/// @param[in] vUserPCId Text ID.
void CCVGFWTheCurrentState::SetUserIDCurrent(const gtString &vUserPCId)
{
  m_userIDCurrent = vUserPCId;
}

/// @brief  Static function. Retrieve the CCVG XML tag IDs for all operational 
///         parameter values data structure.
/// @return SettingsXMLTagIDs& Data structure of XML tag IDs.
const CCVGFWTheCurrentState::SettingsXMLTagIDs& CCVGFWTheCurrentState::GetSettingsXMLTagIDs()
{
  return ms_xmlSettingTagIDs;
}

/// @brief  Static function. Retrieve the CCVG default operational parameter values
///         data structure.
/// @return Settings& Data structure containing default settings.
const CCVGFWTheCurrentState::SettingsConst& CCVGFWTheCurrentState::GetSettingsDefault()
{
  return ms_settingsDefault;
}

/// @brief Retrieve the CCVG user operational parameter values
///        data structure.
/// @return SettingsWorking& Data structure containing user defined settings.
const CCVGFWTheCurrentState::Settings& CCVGFWTheCurrentState::GetSettingsWorking() const
{
  return m_settingsWorking;
}

/// @brief     Update the CCVG user operational parameter values data structure.
/// @param[in] vUpdateSettings = proposed new setting values
/// @return    bool True = ok updated new settings, false = one or more settings 
///            invalid.
status CCVGFWTheCurrentState::SetSettingsWorking(const CCVGFWTheCurrentState::Settings &vUpdateSettings)
{
  gtString errMsg;
  if (!ChkSettingsWorkingValid(vUpdateSettings, errMsg))
  {
    return failure;
  }

  bool bNotifySettingChangedCcvgAllProjectsEnabledFlag = false;
  if (m_settingsWorking.bCcvgAllProjectsEnabledFlag != vUpdateSettings.bCcvgAllProjectsEnabledFlag)
  {
    bNotifySettingChangedCcvgAllProjectsEnabledFlag = true;
  }

  m_settingsWorking.bCcvgAllProjectsEnabledFlag = vUpdateSettings.bCcvgAllProjectsEnabledFlag;

  if (bNotifySettingChangedCcvgAllProjectsEnabledFlag)
  {
    NotifySettingChangedCcvgAllProjectsEnabledFlag();
  }

  return success;
}

/// @brief      Check the CCVG global parameter values are valid. 
/// @param[in]  vSettings The parameters to validate.
/// @param[out] vInvalidMessageStr Append error message(s).
/// @return     bool True = all settings valid, false = one or more invalid.
bool CCVGFWTheCurrentState::ChkSettingsWorkingValid(const Settings &vSettings, gtString &vInvalidMessageStr)
{
  GT_UNREFERENCED_PARAMETER(vSettings);
  GT_UNREFERENCED_PARAMETER(vInvalidMessageStr);

  const bool bEnableAcrossAllProjs = GetSettingsWorking().GetCcvgAllProjectsEnabledFlag();
  if (!bEnableAcrossAllProjs)
  {
    // May be we do not need anymore testing
    return true;
  }
  
  bool bValid = true;

  // Do nothing - nothing to check

  return bValid;
}

/// @brief Reset the CCVG user operational parameter values to their default
///        values.
void CCVGFWTheCurrentState::SettingsWorkingResetToDefault()
{
  bool bNotifySettingChangedCcvgAllProjectsEnabledFlag = false;
  if (m_settingsWorking.bCcvgAllProjectsEnabledFlag != ms_settingsDefault.bCcvgAllProjectsEnabledFlag)
  {
    bNotifySettingChangedCcvgAllProjectsEnabledFlag = true;
  }

  m_settingsWorking.bCcvgAllProjectsEnabledFlag = ms_settingsDefault.bCcvgAllProjectsEnabledFlag;

  if (bNotifySettingChangedCcvgAllProjectsEnabledFlag)
  {
    NotifySettingChangedCcvgAllProjectsEnabledFlag();
  }
}

/// @brief  Package initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWTheCurrentState::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  CCVGFWTheCurrentStateProjectSettings *pSetting = new (std::nothrow) CCVGFWTheCurrentStateProjectSettings;
  if (pSetting == nullptr)
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_TheCurrentState_err_fail_init));
  }
  ms_pProjSetting = pSetting;

  CCVGFWTheCurrentStateProjectData *pPrjData = new (std::nothrow) CCVGFWTheCurrentStateProjectData;
  if (pPrjData == nullptr)
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_TheCurrentState_err_fail_init));
  }
  ms_pProjData = pPrjData;

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWTheCurrentState::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  // Tidy up
  delete ms_pProjSetting;
  ms_pProjSetting = nullptr;
  delete ms_pProjData;
  ms_pProjData = nullptr;

  m_bBeenShutdown = true;

  return success;
}

/// @brief  Determine if current CodeXL execution mode is the CCVG mode or
///         another is operational at this time
/// @return bool True = yes CCVG mode, false = another mode.
bool CCVGFWTheCurrentState::IsInCCVGExecutionMode()
{
  const bool bIsCCVGModeActive = afExecutionModeManager::instance().isActiveMode(CCVGRSRC(kIDS_CCVG_STR_execmode_name));
  return bIsCCVGModeActive;
}

/// @brief  Determine if CCVG is ready to do work. It may be that resources or
///         or hardware was not available on DLL/library load and
///         initialisation.
/// @return bool True = yes ready, false = cannot do anything useful.
bool CCVGFWTheCurrentState::IsCCVGReadyToWork()
{
#if LPGPU2_CCVG_COMPILE_IN
  return CCVGAppWrapper::GetIsLoadEnabled();
#else
  return false;
#endif // LPGPU2_CCVG_COMPILE_IN
}

/// @brief  Determine if CCVG busy doing some task at the moment. 
/// @return bool True = yes busy, false = idling.
bool CCVGFWTheCurrentState::IsCCVGBusy()
{
  return ms_bIsCCVGBusy;
}

/// @brief  Set state indicating CCVG is busy or not doing some task at the 
///         moment. 
/// @param[in] bool True = yes busy, false = now not busy.
void CCVGFWTheCurrentState::SetCCVGIsBusy(bool vbYesBusy)
{
  ms_bIsCCVGBusy = vbYesBusy;
}

/// @brief  Determine if there is a CodeXL project (implying a CCVG
///         project) currently open at the moment. The alternative
///         to find out is a project is open is to call 
///         afProjectManager::instance().currentProjectSettings().
///         projectName(). If the name returned is empty it indicates that
///         that there is not project open.
/// @return bool True = yes open, false = closed.
bool CCVGFWTheCurrentState::IsProjectOpen()
{    
  return ms_bProjOpen;
}

/// @brief  Set state indicating there is a CodeXL project (implying a CCVG
///         project) is currently open at the moment. When set to closed the 
///         current project settings is set to default and the project data
///         is cleaned out
/// @param[in] bool True = yes open, false = closed.
void CCVGFWTheCurrentState::SetProjectIsOpen(bool vbYesOpen)
{
  ms_bProjOpen = vbYesOpen;
  if (!vbYesOpen)
  {
    // Reset the project state to nothing no project
    GetProjectData().ItemsClear();
    GetProjectSetting().SettingsWorkingResetToDefault();
  }
}

/// @brief  Retrieve the object that takes care of the current CCVG project 
///         settings in use. The settings are one to one with a current 
///         project.
/// @return CCVGFWTheCurrentStateProjectSettings& Current settings object. 
CCVGFWTheCurrentStateProjectSettings& CCVGFWTheCurrentState::GetProjectSetting()
{
  return *ms_pProjSetting;
}

/// @brief  Retrieve the object that takes care of the current CCVG project 
///         data. 
/// @return CCVGFWTheCurrentStateProjectData& Current data manager, 
CCVGFWTheCurrentStateProjectData& CCVGFWTheCurrentState::GetProjectData()
{
  return *ms_pProjData;
}

/// @brief  static function. Retrieve the XML section title/ node name for
///         the CCVG global state setting.
/// @return gtString& Current data manager, 
const gtString& CCVGFWTheCurrentState::GetXMLSectionTitle()
{
  return ms_xmlGlobalStateName;
}

/// @brief      Save CCVG global settings into a XML string. 
/// @param[out] vParamsAsXMLString XML formatted settings string.
/// @return     status True = success, false = failure.
status CCVGFWTheCurrentState::GetXMLGlobalSettings(gtString &vParamsAsXMLString)
{
  CCVGFWTheCurrentStateSetIsBusy CCVGIsBusy;

  const SettingsXMLTagIDs &xmlTag(GetSettingsXMLTagIDs());
  const Settings &rCurrentSettings(GetSettingsWorking());
  const wchar_t *pXMLNode = ms_xmlGlobalStateName.asCharArray();
  vParamsAsXMLString.appendFormattedString(L"<%ls>", pXMLNode);
  afUtils::addFieldToXML(vParamsAsXMLString, xmlTag.GetCcvgAllProjectsEnabledFlag(), rCurrentSettings.GetCcvgAllProjectsEnabledFlag());
  vParamsAsXMLString.appendFormattedString(L"</%ls>", pXMLNode);

  return success;
}

/// @brief      Load CCVG global settings. Read the XML string extracting CCVG 
///             working global settings.
/// @param[in]  vParamsAsXMLString XML formatted data string.
/// @return     status True = success, false = failure.
status CCVGFWTheCurrentState::SetGlobaljSettings(const gtString &vParamsAsXMLString)
{
  CCVGFWTheCurrentStateSetIsBusy CCVGIsBusy;

  TiXmlNode *pTpNode = nullptr;
  bool bOk = FindXMLNode(vParamsAsXMLString, ms_xmlGlobalStateName, pTpNode);
  if ((bOk == success) && (pTpNode != nullptr))
  {
    Settings currentSettings;
    const SettingsXMLTagIDs &xmlTag(GetSettingsXMLTagIDs());
    bool bCcvgAllProjectsEnabledFlag;
    afUtils::getFieldFromXML(*pTpNode, xmlTag.GetCcvgAllProjectsEnabledFlag(), bCcvgAllProjectsEnabledFlag);
    currentSettings.SetCcvgAllProjectsEnabledFlag(bCcvgAllProjectsEnabledFlag);
    bOk = SetSettingsWorking(currentSettings);
    
    delete pTpNode;
  }
  
  return bOk;
}

/// @brief      Search the XML DOM data string for a specific node by name. 
///             A node is created on finding a node with the matching name
///             and past back out. The called is responsible for deleting the
///             node.
/// @param[in]  vXMLString The XML data to search for specific node.
/// @param[in]  vNodeName The name of the node to find.
/// @param[out] vpNodeDeleteMe New TiXmlNode object or NULL on failure.
/// @return     status True = success, false = failure.
status CCVGFWTheCurrentState::FindXMLNode(const gtString &vXMLString, const gtString &vNodeName, TiXmlNode *&vpNodeDeleteMe)
{
  bool bOk = failure;

  vpNodeDeleteMe = nullptr;
  const QString stateAsQtXML = acGTStringToQString(vXMLString);
  const QByteArray projectAsQtXMLAsUTF8 = stateAsQtXML.toUtf8();
  TiXmlNode *pElement = new (std::nothrow) TiXmlElement(vNodeName.asASCIICharArray());
  if (pElement != nullptr)
  {
    bOk = success;
    pElement->Parse(projectAsQtXMLAsUTF8.data(), 0, TIXML_DEFAULT_ENCODING);
    gtString tpNodeTitle;
    tpNodeTitle.fromASCIIString(pElement->Value());
    if (vNodeName == tpNodeTitle.asCharArray())
    {
      vpNodeDeleteMe = pElement;
    }
  }

  return bOk;
}

/// @brief      Save project data and settings into a XML string. 
/// @param[out] vProjectAsXMLString XML formatted settings string.
/// @return     status True = success, false = failure.
status CCVGFWTheCurrentState::GetXMLProjSettingsAndProjDataString(gtString &vProjectAsXMLString)
{
  CCVGFWTheCurrentStateSetIsBusy CCVGIsBusy;

  const wchar_t *pExtnXMLTopNode = ms_pProjSetting->GetExtnName().asCharArray();
  vProjectAsXMLString.appendFormattedString(L"<%ls>", pExtnXMLTopNode);
  bool bOk = ms_pProjSetting->GetXMLDataString(vProjectAsXMLString);
  bOk = bOk && ms_pProjData->GetXMLDataString(vProjectAsXMLString);
  vProjectAsXMLString.appendFormattedString(L"</%ls>", pExtnXMLTopNode);

  return bOk;
}

/// @brief      Load project data and settings from an existing project. Read the 
///             XML string extracter CCVG working data and settings.
/// @param[in]  vProjectAsXMLString XML formatted data string.
/// @return     status True = success, false = failure.
status CCVGFWTheCurrentState::SetProjSettingsAndProjDataFromXMLString(const gtString &vProjectAsXMLString)
{
  CCVGFWTheCurrentStateSetIsBusy CCVGIsBusy;

  const gtString &rExtnCCVGXMLNodeID = CCVGFWTheCurrentStateProjectSettings::GetExtnName();
  TiXmlNode *pTpNode = nullptr;
  bool bOk = FindXMLNode(vProjectAsXMLString, rExtnCCVGXMLNodeID, pTpNode);
  if ((bOk == success) && (pTpNode != nullptr))
  {
    bOk = ms_pProjSetting->SetDataFromXMLString(vProjectAsXMLString, *pTpNode);
    
    const bool bCCVGEnabledForCurrentPrj = GetProjectSetting().GetSettingsWorking().GetCcvgProjectEnabledFlag();
    if (bOk && bCCVGEnabledForCurrentPrj)
    {
      bOk = ms_pProjData->SetDataFromXMLString(vProjectAsXMLString, *pTpNode);
    }
    delete pTpNode;
  }

  return bOk;
}

/// @brief Register with *this class for notifications of when global CodeXL 
///        setting has been changed.
/// @param[in] NotifyOnSettingChangedCcvgAllProjectsEnabledFlag& Object to notify.
/// @return status success = registration ok, false = error no registration.
status CCVGFWTheCurrentState::RegisterForNotificationOnSettingChangedCcvgAllProjectsEnabledFlag(const NotifyOnSettingChangedCcvgAllProjectsEnabledFlag &vObj)
{
  m_vecNotifySettingChangedCcvgAllProjectsEnabledFlag.push_back(const_cast<NotifyOnSettingChangedCcvgAllProjectsEnabledFlag *>(&vObj));
  return success;
}

/// @brief Notify registered listeners that some part the project setting has 
///        changed. Internally to *this object it can disable the notification 
///        for example on object destruction.
/// @return status success = all notifications ok, false = one or more 
///        stated they failed talk of carrying out the notification.
status CCVGFWTheCurrentState::NotifySettingChangedCcvgAllProjectsEnabledFlag()
{
  if (m_bDoNotNotifySettingChangedCcvgAllProjectsEnabledFlag)
  {
    return success;
  }

  CCVGuint errCount = 0;
  for (NotifyOnSettingChangedCcvgAllProjectsEnabledFlag *pObj : m_vecNotifySettingChangedCcvgAllProjectsEnabledFlag)
  {
    errCount += (pObj->OnNotifySettingChangedCcvgAllProjectsEnabledFlag() == success) ? 0 : 1;
  }

  return ((errCount == 0) ? success : failure);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/// @brief Class constructor.
CCVGFWTheCurrentStateSetIsBusy::CCVGFWTheCurrentStateSetIsBusy()
{
  CCVGFWTheCurrentState::Instance().SetCCVGIsBusy(true);
}

/// @brief Class destructor.
CCVGFWTheCurrentStateSetIsBusy::~CCVGFWTheCurrentStateSetIsBusy()
{
  CCVGFWTheCurrentState::Instance().SetCCVGIsBusy(false);
}

} // namespace fw
} // namespace ccvg
