// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTheCurrentStateProjectSettings implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Std:
#include <memory> // for std::unique_ptr

// TinyXml:
#include <tinyxml.h>

// Framework:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateCurrentPCUser.h>

namespace ccvg {
namespace fw {

// Instantiations:
gtString                                                CCVGFWTheCurrentStateProjectSettings::ms_xmlProjExtnName = L"LPGPU2CCVGCodeCoveragePrj"; // *** Do not change as will break all previous projects ***
gtString                                                CCVGFWTheCurrentStateProjectSettings::ms_xmlProjSettingsName = L"ProjSettings";
gtString                                                CCVGFWTheCurrentStateProjectSettings::ms_ccvgFilesExtnsValidCharsNonAlpha = L".";
CCVGFWTheCurrentStateProjectSettings::SettingsXMLTagIDs CCVGFWTheCurrentStateProjectSettings::ms_xmlSettingTagIDs;
CCVGFWTheCurrentStateProjectSettings::SettingsConst     CCVGFWTheCurrentStateProjectSettings::ms_settingsDefault =
{
  false,      // ccvgProjectEnabledFlag
  L"",        // ccvgFolderDirResult
  L"",        // ccvgFolderDirSource
  L"",        // ccvgFolderDirDriver
  L"",        // ccvgFolderDirExecuteable
  L".ccvgr",  // ccvgFilesExtnResult
  L".kc",     // ccvgFilesExtnSource
  L".ccvgd",  // ccvgFilesExtnDriver
  true,       // ccvgDriverFilesMandatoryFlag
  L".ccvge",  // ccvgFilesExtnExecuteable
  L"",        // previousPCId
  L""         // previousUserId
};

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentStateProjectSettings::SettingsXMLTagIDs::SettingsXMLTagIDs()
: ccvgProjectEnabledFlag(L"CCVGPRojectEnabledFlag")
, ccvgFolderDirResult(L"ResultsDir")
, ccvgFolderDirSource(L"SourceDir")
, ccvgFolderDirDriver(L"DriverDir")
, ccvgFolderDirExecuteable(L"DriverExecuteable")
, ccvgFilesExtnResult(L"ResultsFileExtn")
, ccvgFilesExtnSource(L"SourceFileExtn")
, ccvgFilesExtnDriver(L"DriverFileExtn")
, ccvgDriverFilesMandatoryFlag(L"DriverGrpBxEnabledFlag")
, ccvgFilesExtnExecuteable(L"ExecuteableFileExtn")
, previousPCId(L"PreviousPCId")
, previousUserId(L"PreviousUserId")
{
}

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentStateProjectSettings::Settings::Settings()
: ccvgDriverFilesMandatoryFlag(true)
, previousPCId(L"")
, previousUserId(L"")
{
}

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentStateProjectSettings::Settings::Settings(bool vbProjectEnabledFlag,
                                                         wchar_t *vpFolderDirResult,
                                                         wchar_t *vpFolderSource, 
                                                         wchar_t *vpFolderDirDriver, 
                                                         wchar_t *vpFolderDirExecuteable,
                                                         wchar_t *vpFolderExtnResult,
                                                         wchar_t *vpFolderExtnSource, 
                                                         wchar_t *vpFolderExtnDriver, 
                                                         bool vbDriverFilesMandatory, 
                                                         wchar_t *vpFolderExtnExecuteable,
                                                         gtString vPreviousPCId,
                                                         gtString vPreviousUserId)
: ccvgProjectEnabledFlag(vbProjectEnabledFlag)
, ccvgFolderDirResult(vpFolderDirResult)
, ccvgFolderDirSource(vpFolderSource)
, ccvgFolderDirDriver(vpFolderDirDriver)
, ccvgFolderDirExecuteable(vpFolderDirExecuteable)
, ccvgFilesExtnResult(vpFolderExtnResult)
, ccvgFilesExtnSource(vpFolderExtnSource)
, ccvgFilesExtnDriver(vpFolderExtnDriver)
, ccvgDriverFilesMandatoryFlag(vbDriverFilesMandatory)
, ccvgFilesExtnExecuteable(vpFolderExtnExecuteable)
, previousPCId(vPreviousPCId)
, previousUserId(vPreviousUserId)
{
}

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentStateProjectSettings::SettingsConst::SettingsConst()
{
}

/// @brief Class constructor. No work is done here by this class.
CCVGFWTheCurrentStateProjectSettings::SettingsConst::SettingsConst(bool vbProjectEnabledFlag,
                                                                   wchar_t *vpFolderDirResult,
                                                                   wchar_t *vpFolderSource,
                                                                   wchar_t *vpFolderDirDriver,
                                                                   wchar_t *vpFolderDirExecuteable,
                                                                   wchar_t *vpFolderExtnResult,
                                                                   wchar_t *vpFolderExtnSource,
                                                                   wchar_t *vpFolderExtnDriver,
                                                                   bool vbDriverFilesMandatory, 
                                                                   wchar_t *vpFolderExtnExecuteable,
                                                                   gtString vPreviousPCId,
                                                                   gtString vPreviousUserId)
: Settings(vbProjectEnabledFlag, vpFolderDirResult, vpFolderSource, vpFolderDirDriver, vpFolderDirExecuteable,
           vpFolderExtnResult, vpFolderExtnSource, vpFolderExtnDriver, vbDriverFilesMandatory, vpFolderExtnExecuteable,
           vPreviousPCId, vPreviousUserId)
{
}

/// @brief Class constructor. Copy the CCVG default operational parameter values
///        to the user entered equivalent parameters.
CCVGFWTheCurrentStateProjectSettings::CCVGFWTheCurrentStateProjectSettings()
: m_bIsUserAndPCTheSameAsPreviousUseOfTheCodeXLProject(false)
, m_bUserEnabledProjForThisComputer(false)
{
  SettingsWorkingResetToDefault();
}

/// @brief Class destructor. Tidy up release resources used by *this instance.
CCVGFWTheCurrentStateProjectSettings::~CCVGFWTheCurrentStateProjectSettings()
{
}

/// @brief Record the user has enabled the CCVG project settings GUI Groupbox
///        for editing (but not necessarily changed any settings) at least once
///        for a CodeXL session.
/// @param[in] vbYes True = Enabled, false = not yet.
void CCVGFWTheCurrentStateProjectSettings::SetUserEditedProjectSettingsOnThisComputer(bool vbYes)
{
  m_bUserEnabledProjForThisComputer = vbYes;
}

/// @brief Retrieve the flag indciating the user has enabled the CCVG project 
///        settings GUI Groupbox for editing (but not necessarily changed any
///        settings) at least once for a CodeXL session.
/// @return bool True = Enabled, false = not yet.
bool CCVGFWTheCurrentStateProjectSettings::GetUserEditedProjectSettingsOnThisComputer() const
{
  return m_bUserEnabledProjForThisComputer;
}

/// @brief Retrieve the flag stating that the user or PC is not the same as 
///        the CodeXL project was used previously. This is likely because the
///        project file has been copied or moved to a PC with is different
///        configuration.
/// @return bool True  = Same user and PC, false = different user or PC. 
bool CCVGFWTheCurrentStateProjectSettings::IsUserAndPCTheSameAsPreviousUseOfTheCodeXLProject() const
{
  return m_bIsUserAndPCTheSameAsPreviousUseOfTheCodeXLProject;
}

/// @brief Set the flag stating that the user or PC is the same as 
///        the CodeXL project was used previously.
void CCVGFWTheCurrentStateProjectSettings::SetUserAndPCTheSameAsPreviousUseOfTheCodeXLProject()
{
  m_bIsUserAndPCTheSameAsPreviousUseOfTheCodeXLProject = true;
}

/// @brief Retrieve the ID of the computer CodeXL was running on previously
///        for that last project.
/// @return gtString& String text ID.
const gtString& CCVGFWTheCurrentStateProjectSettings::GetComputerIDPrevious() const
{
  return m_settingsWorking.GetPreviousPCId();
}

/// @brief Retrieve the ID of the user running CodeXL previously for
///        that last project.
/// @return gtString& String text ID.
const gtString& CCVGFWTheCurrentStateProjectSettings::GetUserIDPrevious() const
{
  return m_settingsWorking.GetPreviousUserId();
}

/// @brief Compare the current and previous computer ID and user ID
///        to see if they are both the same. The project is running on  the
///        same computer and by the same user. This flag is cleared once the
///        user has the opportunity to edit the CCVG project settings and
///        confirm by clicking ok.
void CCVGFWTheCurrentStateProjectSettings::DetermineProjectMovedPCOrUser()
{
  CCVGFWTheCurrentState &rGlobalState(CCVGFWTheCurrentState::Instance());

  // Get current user and computer ID
  CCVGFWTheCurrentStateCurrentPCUser state;
  const gtString computerID(state.GetComputerIDCurrent());
  rGlobalState.SetComputerIDCurrent(computerID);
  const gtString userID(state.GetUserIDCurrent());
  rGlobalState.SetUserIDCurrent(userID);
  const gtString& prevPCId(GetComputerIDPrevious());
  const gtString& prevUserId(GetUserIDPrevious());

  m_bIsUserAndPCTheSameAsPreviousUseOfTheCodeXLProject = ((computerID == prevPCId) &&
                                                          (userID == prevUserId));
}

/// @brief Found out if the project just loaded is a new and different project
///        to the previous project loaded.
/// @return bool True = yes new different project, false = same project.
bool CCVGFWTheCurrentStateProjectSettings::IsNewDifferentProject() 
{
  const gtString &projRightNow(afProjectManager::instance().currentProjectSettings().projectName());
  const bool bDifferenProj = m_projectNameCurrent != projRightNow;
  m_projectNameCurrent = projRightNow;
  return bDifferenProj;
}

/// @brief Static function. Retrieve the CCVG XML tag IDs for all operational 
///        parameter values data structure.
/// @return SettingsXMLTagIDs& Data structure of XML tag IDs.
const CCVGFWTheCurrentStateProjectSettings::SettingsXMLTagIDs& CCVGFWTheCurrentStateProjectSettings::GetSettingsXMLTagIDs()
{
  return ms_xmlSettingTagIDs;
}

/// @brief Static function. Retrieve the CCVG default operational parameter values
///        data structure.
/// @return Settings& Data structure containing default settings.
const CCVGFWTheCurrentStateProjectSettings::SettingsConst& CCVGFWTheCurrentStateProjectSettings::GetSettingsDefault()
{
  return ms_settingsDefault;
}

/// @brief Retrieve the CCVG user operational parameter values
///        data structure.
/// @return SettingsWorking& Data structure containing user defined settings.
const CCVGFWTheCurrentStateProjectSettings::Settings& CCVGFWTheCurrentStateProjectSettings::GetSettingsWorking() const
{
  return m_settingsWorking;
}

/// @brief Update the CCVG user operational parameter values data structure.
/// @param[in] vUpdateSettings = proposed new setting values
/// @return    bool True = ok updated new settings, false = one or more settings 
///            invalid.
status CCVGFWTheCurrentStateProjectSettings::SetSettingsWorking(const CCVGFWTheCurrentStateProjectSettings::Settings &vUpdateSettings)
{
  gtString errMsg;
  if(!ChkSettingsWorkingValid(vUpdateSettings, errMsg))
  {
    return failure;
  }

  m_settingsWorking.ccvgProjectEnabledFlag = vUpdateSettings.ccvgProjectEnabledFlag;
  m_settingsWorking.ccvgFolderDirResult = vUpdateSettings.ccvgFolderDirResult;
  m_settingsWorking.ccvgFolderDirSource = vUpdateSettings.ccvgFolderDirSource;
  m_settingsWorking.ccvgFolderDirDriver = vUpdateSettings.ccvgFolderDirDriver;
  m_settingsWorking.ccvgFolderDirExecuteable = vUpdateSettings.ccvgFolderDirExecuteable;
  m_settingsWorking.ccvgFilesExtnResult = vUpdateSettings.ccvgFilesExtnResult;
  m_settingsWorking.ccvgFilesExtnSource = vUpdateSettings.ccvgFilesExtnSource;
  m_settingsWorking.ccvgFilesExtnDriver = vUpdateSettings.ccvgFilesExtnDriver;
  m_settingsWorking.ccvgDriverFilesMandatoryFlag = vUpdateSettings.ccvgDriverFilesMandatoryFlag;
  m_settingsWorking.ccvgFilesExtnExecuteable = vUpdateSettings.ccvgFilesExtnExecuteable;
  m_settingsWorking.previousPCId = vUpdateSettings.previousPCId;
  m_settingsWorking.previousUserId = vUpdateSettings.previousUserId;

  return success;
}

/// @brief Reset the CCVG user operational parameter values to their default
///        values.
void CCVGFWTheCurrentStateProjectSettings::SettingsWorkingResetToDefault()
{
  m_settingsWorking.ccvgProjectEnabledFlag = ms_settingsDefault.ccvgProjectEnabledFlag;
  m_settingsWorking.ccvgFolderDirResult = ms_settingsDefault.ccvgFolderDirResult;
  m_settingsWorking.ccvgFolderDirSource = ms_settingsDefault.ccvgFolderDirSource;
  m_settingsWorking.ccvgFolderDirDriver = ms_settingsDefault.ccvgFolderDirDriver;
  m_settingsWorking.ccvgFolderDirExecuteable = ms_settingsDefault.ccvgFolderDirExecuteable;
  m_settingsWorking.ccvgFilesExtnResult = ms_settingsDefault.ccvgFilesExtnResult;
  m_settingsWorking.ccvgFilesExtnSource = ms_settingsDefault.ccvgFilesExtnSource;
  m_settingsWorking.ccvgFilesExtnDriver = ms_settingsDefault.ccvgFilesExtnDriver;
  m_settingsWorking.ccvgDriverFilesMandatoryFlag = ms_settingsDefault.ccvgDriverFilesMandatoryFlag;
  m_settingsWorking.ccvgFilesExtnExecuteable = ms_settingsDefault.ccvgFilesExtnExecuteable;
  m_settingsWorking.previousPCId = ms_settingsDefault.previousPCId;
  m_settingsWorking.previousUserId = ms_settingsDefault.previousUserId;
}

/// @brief  Static function. Retrieve the CCVG project extension name or XML 
///         top node ID. Used in the XML DOM node identification for CCVG 
///         project information.
/// @return gtString& Wide char text.
const gtString& CCVGFWTheCurrentStateProjectSettings::GetExtnName()
{
  return ms_xmlProjExtnName;
}

/// @brief  Static function. Retrieve the CCVG project settings extension name.
/// @return gtString& Wide char text.
const gtString& CCVGFWTheCurrentStateProjectSettings::GetExtnNameValidChars()
{
  return ms_ccvgFilesExtnsValidCharsNonAlpha;
}

/// @brief Check the CCVG user operational parameter values are valid. If 
///        seeting ccvgProjectEnabledFlag is not enabled, the CCVG plugin
///        features are disabled. This is not to nag users who are not 
///        interested in the CCVG plugin usage.
/// @param[in]  vSettings The parameters to validate.
/// @param[out] vInvalidMessageStr Append error message(s).
/// @return     bool True = all settings valid, false = one or more invalid.
bool CCVGFWTheCurrentStateProjectSettings::ChkSettingsWorkingValid(const Settings &vSettings, gtString &vInvalidMessageStr)
{
  bool bValid = true;

  const bool bEnableAcrossAllProjs = CCVGFWTheCurrentState::Instance().GetSettingsWorking().GetCcvgAllProjectsEnabledFlag();
  if (!bEnableAcrossAllProjs)
  {
    // Global setting not to use CCVG execution mode
    // So no need to check validity.
    return bValid;
  }
  
  if (!vSettings.ccvgProjectEnabledFlag)
  {
    // See brief
    return bValid;
  }

  CCVGuint lineCnt = vInvalidMessageStr.isEmpty() ? 0 : 1;
  {
    const wchar_t *pWord = L"Results";
    if (!IsValidDirPath(vSettings.ccvgFolderDirResult))
    {
      vInvalidMessageStr += (lineCnt++ > 0) ? L"\n" : gtString();
      bValid = false;
      vInvalidMessageStr.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFolderPath), pWord);
    }
    if (!IsValidFileExtn(vSettings.ccvgFilesExtnResult))
    {
      lineCnt++;
      bValid = false;
      vInvalidMessageStr.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFileExtn), pWord);
    }
  }

  {
    const wchar_t *pWord = L"Source";
    if (!IsValidDirPath(vSettings.ccvgFolderDirSource))
    {
      vInvalidMessageStr += (lineCnt++ > 0) ? L"\n" : gtString();
      bValid = false;
      vInvalidMessageStr.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFolderPath), pWord);
    }
    if (!IsValidFileExtn(vSettings.ccvgFilesExtnSource))
    {
      lineCnt++;
      bValid = false;
      vInvalidMessageStr.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFileExtn), pWord);
    }
  }

  {
    const wchar_t *pWord = L"Executeable";
    if (!IsValidDirPath(vSettings.ccvgFolderDirExecuteable))
    {
      vInvalidMessageStr += (lineCnt++ > 0) ? L"\n" : gtString();
      bValid = false;
      vInvalidMessageStr.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFolderPath), pWord);
    }
    if (!IsValidFileExtn(vSettings.ccvgFilesExtnExecuteable))
    {
      lineCnt++;
      bValid = false;
      vInvalidMessageStr.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFileExtn), pWord);
    }
  }

  if (vSettings.ccvgDriverFilesMandatoryFlag)
  {
    const wchar_t *pWord = L"Driver";
    if (!IsValidDirPath(vSettings.ccvgFolderDirDriver))
    {
      vInvalidMessageStr += (lineCnt > 0) ? L"\n" : gtString();
      bValid = false;
      vInvalidMessageStr.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFolderPath), pWord);
    }
    if (!IsValidFileExtn(vSettings.ccvgFilesExtnDriver))
    {
      lineCnt++;
      bValid = false;
      vInvalidMessageStr.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_fw_ProjectSettings_err_invalidFileExtn), pWord);
    }
  }

  return bValid;
}

/// @brief Check the file extension is a valid extension. An extension must be
///        at least 2 charactors long and the first charactor is a '.'.
/// @param[in]  vExtn The file extension text.
/// @return     bool True = valid, false = invalid.
bool CCVGFWTheCurrentStateProjectSettings::IsValidFileExtn(const gtString &vExtn) const
{
  const int len = vExtn.length();
  if ((len < 2) || (len > OS_MAX_PATH))
  {
    return false;
  }

  return const_cast<gtString &>(vExtn).isAlnum(ms_ccvgFilesExtnsValidCharsNonAlpha);
}

/// @brief Check the file directory path is not emplty and is valid location.
/// @param[in]  vPath The file directory path.
/// @return     bool True = valid, false = invalid.
bool CCVGFWTheCurrentStateProjectSettings::IsValidDirPath(const gtString &vPath) const
{
  const int len = vPath.length();
  if (len == 0)
  {
    return false;
  }

  const QDir dir(acGTStringToQString(vPath));
  return dir.exists();
}

/// @brief      Save project data and settings into a XML string. 
/// @param[out] vProjectAsXMLString XML formatted settings string.
/// @return     status True = success, false = failure.
status CCVGFWTheCurrentStateProjectSettings::GetXMLDataString(gtString &vProjectAsXMLString)
{
  CCVGFWTheCurrentState &rGlobalState(CCVGFWTheCurrentState::Instance());
  const SettingsXMLTagIDs &xmlTag(GetSettingsXMLTagIDs());
  const Settings &rCurrentSettings(GetSettingsWorking());

  const wchar_t *pXMLNodeName = ms_xmlProjSettingsName.asCharArray();
  vProjectAsXMLString.appendFormattedString(L"<%ls>", pXMLNodeName);
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetCcvgProjectEnabledFlag(), rCurrentSettings.GetCcvgProjectEnabledFlag());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetCcvgFolderDirResult(), rCurrentSettings.GetCcvgFolderDirResult());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetCcvgFolderDirSource(), rCurrentSettings.GetCcvgFolderDirSource());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetCcvgFolderDirDriver(), rCurrentSettings.GetCcvgFolderDirDriver());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetCcvgFolderDirExecuteable(), rCurrentSettings.GetCcvgFolderDirExecuteable());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetCcvgFilesExtnResult(), rCurrentSettings.GetCcvgFilesExtnResult());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetCcvgFilesExtnSource(), rCurrentSettings.GetCcvgFilesExtnSource());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetCcvgFilesExtnDriver(), rCurrentSettings.GetCcvgFilesExtnDriver());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetCcvgDriverFilesMandatoryFlag(), rCurrentSettings.GetCcvgDriverFilesMandatoryFlag());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetCcvgFilesExtnExecuteable(), rCurrentSettings.GetCcvgFilesExtnExecuteable());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetPreviousPCId(), rGlobalState.GetComputerIDCurrent());
  afUtils::addFieldToXML(vProjectAsXMLString, xmlTag.GetPreviousUserId(), rGlobalState.GetUserIDCurrent());
  vProjectAsXMLString.appendFormattedString(L"</%ls>", pXMLNodeName);

  return success;
}

/// @brief      Load project data and settings from an existing project. Read the 
///             XML string extracter CCVG working data and settings.
/// @param[in]  vProjectAsXMLString XML formatted data string.
/// @param[in]  vMainNode CCVG project XML top node.
/// @return     bool True = success, false = failure.
status CCVGFWTheCurrentStateProjectSettings::SetDataFromXMLString(const gtString &vProjectAsXMLString, const TiXmlNode &vMainNode)
{
  GT_UNREFERENCED_PARAMETER(vProjectAsXMLString);
  status bOk = true;

  bool bFound = false;
  for (const TiXmlElement *pNode = vMainNode.FirstChildElement(); pNode != nullptr; pNode = pNode->NextSiblingElement())
  {
    const gtString nodeName(gtString().fromASCIIString(pNode->Value()));
    if (nodeName == ms_xmlProjSettingsName)
    {
      Settings currentSettings;
      const SettingsXMLTagIDs &xmlTag(GetSettingsXMLTagIDs());
      bool ccvgProjectEnabledFlag;
      gtString ccvgFolderDirResult;
      gtString ccvgFolderDirSource;
      gtString ccvgFolderDirDriver;
      gtString ccvgFolderDirExecuteable;
      gtString ccvgFilesExtnResult;
      gtString ccvgFilesExtnSource;
      gtString ccvgFilesExtnDriver;
      bool ccvgDriverFilesMandatoryFlag;
      gtString ccvgFilesExtnExecuteable;
      gtString previousPCId;
      gtString previousUserId;
      TiXmlElement *pN = const_cast<TiXmlElement *>(pNode);
      afUtils::getFieldFromXML(*pN, xmlTag.GetCcvgProjectEnabledFlag(), ccvgProjectEnabledFlag);
      afUtils::getFieldFromXML(*pN, xmlTag.GetCcvgFolderDirResult(), ccvgFolderDirResult);
      afUtils::getFieldFromXML(*pN, xmlTag.GetCcvgFolderDirSource(), ccvgFolderDirSource);
      afUtils::getFieldFromXML(*pN, xmlTag.GetCcvgFolderDirDriver(), ccvgFolderDirDriver);
      afUtils::getFieldFromXML(*pN, xmlTag.GetCcvgFolderDirExecuteable(), ccvgFolderDirExecuteable);
      afUtils::getFieldFromXML(*pN, xmlTag.GetCcvgFilesExtnResult(), ccvgFilesExtnResult);
      afUtils::getFieldFromXML(*pN, xmlTag.GetCcvgFilesExtnSource(), ccvgFilesExtnSource);
      afUtils::getFieldFromXML(*pN, xmlTag.GetCcvgFilesExtnDriver(), ccvgFilesExtnDriver);
      afUtils::getFieldFromXML(*pN, xmlTag.GetCcvgDriverFilesMandatoryFlag(), ccvgDriverFilesMandatoryFlag);
      afUtils::getFieldFromXML(*pN, xmlTag.GetCcvgFilesExtnExecuteable(), ccvgFilesExtnExecuteable);
      afUtils::getFieldFromXML(*pN, xmlTag.GetPreviousPCId(), previousPCId);
      afUtils::getFieldFromXML(*pN, xmlTag.GetPreviousUserId(), previousUserId);
      currentSettings.SetCcvgProjectEnabledFlag(ccvgProjectEnabledFlag);
      currentSettings.SetCcvgFolderDirResult(ccvgFolderDirResult);
      currentSettings.SetCcvgFolderDirSource(ccvgFolderDirSource);
      currentSettings.SetCcvgFolderDirDriver(ccvgFolderDirDriver);
      currentSettings.SetCcvgFolderDirExecuteable(ccvgFolderDirExecuteable);
      currentSettings.SetCcvgFilesExtnResult(ccvgFilesExtnResult);
      currentSettings.SetCcvgFilesExtnSource(ccvgFilesExtnSource);
      currentSettings.SetCcvgFilesExtnDriver(ccvgFilesExtnDriver);
      currentSettings.SetCcvgDriverFilesMandatoryFlag(ccvgDriverFilesMandatoryFlag);
      currentSettings.SetCcvgFilesExtnExecuteable(ccvgFilesExtnExecuteable);
      currentSettings.SetPreviousPCId(previousPCId);
      currentSettings.SetPreviousUserId(previousUserId);
      bOk = SetSettingsWorking(currentSettings);
      bFound = true;

      // Now determine if the  project has been moved to a new PC (different 
      // configuration) or user changed (different mounted drives)
      DetermineProjectMovedPCOrUser();
      
      break;
    }
  }

  bOk = bFound;
  
  return bOk;
}

} // namespace fw
} // namespace ccvg
