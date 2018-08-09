// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppMultiSourceCodeView.h
///
/// @brief Separate tab view to display source code using QScintilla as the
///         the base component. Will be expanded, this is a proof of concept.
///
/// ppMultiSourceCodeView definitions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/LPGPU2ppMultiSourceCodeView.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationComponents/Include/acSourceCodeView.h>
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// Local:
#include <AMDTPowerProfiling/src/ppSessionController.h>
#include <AMDTPowerProfiling/src/LPGPU2ppSourceCodeView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppSourceCodeWithSplitView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/src/LPGPU2ppInfoWidget.h>

#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAPIDefs.h>
#include <LPGPU2Database/LPGPU2FeedbackEngine/LPGPU2_db_FeedbackEngine.h>

// Qt:
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

using lpgpu2::PPFnStatus;

/// @brief Ctor. No work is done here
/// @param pParent The session view which owns this one
/// @param pSessionController The PP session controller
LPGPU2ppMultiSourceCodeView::LPGPU2ppMultiSourceCodeView(ppSessionView* pParent,
  ppSessionController* pSessionController) :
    QWidget{ nullptr }
    , m_pSessionController(pSessionController)
    , m_pParentView(pParent)
{
}

/// @brief Dtor. Normally no work is done here, but because of how Qt works
///        we need to make sure that Shutdown was called anyway.
LPGPU2ppMultiSourceCodeView::~LPGPU2ppMultiSourceCodeView()
{
  Shutdown();
}

/// @brief Initialise the object
/// @return PPFnStatus Success/failure enum value
PPFnStatus LPGPU2ppMultiSourceCodeView::Initialise()
{
    if (!m_bIsInitialised)
    {
        InitViewLayout();
        InitShaders();
        InitCounters();
        InitFeedbackScripts();

        const auto bHasShaders = m_pShaderComboBox->count() > 0;
        const auto bHasCounters = m_pCounterComboBox->count() > 0;
        const auto bHasScripts = m_pScriptComboBox->count() > 0;

        m_pShaderMatchOptionsWidget->setVisible(bHasShaders && bHasCounters && bHasScripts);
        m_pShaderComboBox->setVisible(bHasShaders);
        m_pShaderComboLabel->setVisible(bHasShaders);
    }    

    m_bIsInitialised = true;

    return PPFnStatus::success;
}

/// @brief Deinitialise the object
/// @return PPFnStatus Success/failure enum value
PPFnStatus LPGPU2ppMultiSourceCodeView::Shutdown()
{
  // Qt takes care of deleting the Qt objects on the heap so we do not need to
  // to delete them ourselves

  m_openTabsByName.clear();

  return PPFnStatus::success;
}

/// @brief Slot called when the user selects a shader from the shader selection combo box.
/// @param[in]  vShaderIndex    The combo box index as emitted by the signal 
///                             QComboBox::currentItemChanged(int) and QComboBox::activated(int)
void LPGPU2ppMultiSourceCodeView::OnShaderSelected(int vShaderIndex)
{
    GT_IF_WITH_ASSERT(m_pShaderComboBox != nullptr)
    {
        const auto shaderIdVariant = m_pShaderComboBox->itemData(vShaderIndex);
        const auto shaderIdPair = shaderIdVariant.value<QPair<gtUInt64, gtUInt64>>();
        const auto shaderId = shaderIdPair.first;
        const auto shaderApiId = shaderIdPair.second;
        
        gtVector<lpgpu2::db::ShaderTrace> shaderTraces;
        if (m_pSessionController->GetProfilerBL().GetShaderTraces(shaderTraces))
        {
            const auto stIt = std::find_if(shaderTraces.begin(), shaderTraces.end(), [=](const lpgpu2::db::ShaderTrace &st)
            {
                return st.m_shaderId == shaderId && st.m_apiId == static_cast<lpgpu2::db::EShaderTraceAPIId>(shaderApiId);
            });

            if (stIt != shaderTraces.end())
            {
                // Update the shader view with the new information                
                OpenAndDisplayShaderSlot(stIt->m_frameNum, stIt->m_drawNum);
            }
        }
    }    
}

/// @brief      Slot called when the user clicks to execute the feedback script for the shader_match.
/// @warning    Emits the signal FeedbackExecuted()
void LPGPU2ppMultiSourceCodeView::OnRunShaderFeedbackScript()
{
    GT_IF_WITH_ASSERT(m_pShaderComboBox != nullptr && m_pCounterComboBox != nullptr && m_pScriptComboBox != nullptr && m_pSessionController != nullptr)
    {
        // Read the data from the combo boxes.
        const auto shaderIdVariant = m_pShaderComboBox->currentData();
        const auto shaderIdPair = shaderIdVariant.value<QPair<gtUInt64, gtUInt64>>();
        const auto shaderId = shaderIdPair.first;
        const auto shaderApiId = shaderIdPair.second;

        const auto script = m_pScriptComboBox->currentText();
        const auto counter = m_pCounterComboBox->currentData().toInt();
        
        auto &profilerBL = m_pSessionController->GetProfilerBL();
        
        // Creates a feedback engine ptr
        std::unique_ptr<lpgpu2::db::LPGPU2FeedbackEngine> feedbackEngine;
        GT_IF_WITH_ASSERT(profilerBL.GetFeedbackEngineForSession(feedbackEngine))
        {
            // TODO: Create a list with all the FeedbackOptions that need to be invoked.

            // Builds the option to invoke the shader_match script
            lpgpu2::db::FeedbackOption shaderFeedbackOption;
            shaderFeedbackOption.functionName = PP_STR_LPGPU2_ShaderMatch_FeedbackScriptName;
            shaderFeedbackOption.functionArg = { static_cast<gtInt32>(shaderId), static_cast<gtInt32>(shaderApiId), counter };

            osFilePath scriptFullPath{ osFilePath::OS_CODEXL_BINARIES_PATH };
            scriptFullPath.appendSubDirectory(PP_STR_FeedbackEngineScriptsFolder_W);
            scriptFullPath.setFileName(acQStringToGTString(script));

            // Invoke the feedback engine
            if (feedbackEngine->Run(scriptFullPath.asString(), { shaderFeedbackOption }) == lpgpu2::db::success)
            {
                // TODO: Remove this
                // Generate some random data for testing the feedback engine trigger.
                // profilerBL.SimulateShaderCounterMatchScript(shaderId, shaderApiId, counter);
                
                // temp
                gtVector<lpgpu2::db::ShaderTrace> shaderTraces;
                if (profilerBL.GetShaderTraces(shaderTraces))
                {
                    const auto stIt = std::find_if(shaderTraces.begin(), shaderTraces.end(), [=](const lpgpu2::db::ShaderTrace &st)
                    {
                        return st.m_shaderId == shaderId && st.m_apiId == static_cast<lpgpu2::db::EShaderTraceAPIId>(shaderApiId);
                    });

                    if (stIt != shaderTraces.end())
                    {
                        // Update the shader view with the new information                
                        OpenAndDisplayShaderSlot(stIt->m_frameNum, stIt->m_drawNum);
                    }
                }
            }            
        }       
    }    
}

/// @brief Initialise the layout of the widget by creating widget objects
/// @return PPFnStatus Success/failure enum value
PPFnStatus LPGPU2ppMultiSourceCodeView::InitViewLayout()
{
  QVBoxLayout *pMainLayout = nullptr;
  // Create and set the layout
  LPGPU2PPNewQtWidget(&pMainLayout, this);
  pMainLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(pMainLayout);
  setVisible(false);

  QHBoxLayout *pShaderSelectionLayout = nullptr;
  LPGPU2PPNewQtWidget(&pShaderSelectionLayout, nullptr);

  LPGPU2PPNewQtWidget(&m_pShaderMatchOptionsWidget, this);
  QHBoxLayout *pShaderMatchOptionsLayout = nullptr;
  LPGPU2PPNewQtWidget(&pShaderMatchOptionsLayout, m_pShaderMatchOptionsWidget);
  
  LPGPU2PPNewQtWidget(&m_pShaderComboLabel, PP_STR_LPGPU2_ShaderMatch_ShaderLabel, this);
  LPGPU2PPNewQtWidget(&m_pShaderComboBox, this);

  QLabel *pSelectCounterLabel = nullptr;
  LPGPU2PPNewQtWidget(&pSelectCounterLabel, PP_STR_LPGPU2_ShaderMatch_CounterLabel, m_pShaderMatchOptionsWidget);
  LPGPU2PPNewQtWidget(&m_pCounterComboBox, m_pShaderMatchOptionsWidget);

  QLabel *pSelectScriptLabel = nullptr;
  LPGPU2PPNewQtWidget(&pSelectScriptLabel, PP_STR_LPGPU2_ShaderMatch_FeedbackScriptLabel, m_pShaderMatchOptionsWidget);
  LPGPU2PPNewQtWidget(&m_pScriptComboBox, m_pShaderMatchOptionsWidget);

  LPGPU2PPNewQtWidget(&m_pRunScriptButton, PP_STR_LPGPU2_ShaderMatch_CalculateStatsButton, m_pShaderMatchOptionsWidget);

  m_pShaderComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  m_pCounterComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  m_pScriptComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  pShaderMatchOptionsLayout->addWidget(pSelectCounterLabel);
  pShaderMatchOptionsLayout->addWidget(m_pCounterComboBox);
  pShaderMatchOptionsLayout->addWidget(pSelectScriptLabel);
  pShaderMatchOptionsLayout->addWidget(m_pScriptComboBox);
  pShaderMatchOptionsLayout->addWidget(m_pRunScriptButton);  

  pShaderSelectionLayout->addStretch();
  pShaderSelectionLayout->addWidget(m_pShaderComboLabel);
  pShaderSelectionLayout->addWidget(m_pShaderComboBox);

  pShaderSelectionLayout->addWidget(m_pShaderMatchOptionsWidget);  

  connect(m_pShaderComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnShaderSelected(int)));
  connect(m_pShaderComboBox, SIGNAL(activated(int)), this, SLOT(OnShaderSelected(int)));
  connect(m_pRunScriptButton, &QPushButton::clicked, this, &LPGPU2ppMultiSourceCodeView::OnRunShaderFeedbackScript);

  pMainLayout->addLayout(pShaderSelectionLayout);

  QHBoxLayout *pInfoWidgetLayout = nullptr;
  LPGPU2PPNewQtWidget(&pInfoWidgetLayout, nullptr);
  
  // Create the info widget  
  LPGPU2PPNewQtLPGPU2Widget(&m_pInfoWidget, PP_STR_LPGPU2InfoNoSourceTitle, PP_STR_LPGPU2InfoNoSourceMsg, this);
  
  pInfoWidgetLayout->addStretch();
  pInfoWidgetLayout->addWidget(m_pInfoWidget);
  pInfoWidgetLayout->addStretch();
  
  pMainLayout->addLayout(pInfoWidgetLayout);

  // Create tab widget used to hold all the source code views
  LPGPU2PPNewQtWidget(&m_pTabWidgdet, this);
  m_pTabWidgdet->setTabsClosable(true);
  m_pTabWidgdet->setVisible(false);
  pMainLayout->addWidget(m_pTabWidgdet);  

  m_pTabWidgdet->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  m_pInfoWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  // Connect the timeline view bubbles with the opening of source files
  connect(m_pTabWidgdet, &acTabWidget::tabCloseRequested, this, &LPGPU2ppMultiSourceCodeView::OnTabCloseRequestedSlot);

  return PPFnStatus::success;
}

/// @brief  Get all shaders available in the current session and populate the combo box for the user to select it.
/// @return PPFnStatus Success/failure enum value.
lpgpu2::PPFnStatus LPGPU2ppMultiSourceCodeView::InitShaders()
{
    GT_IF_WITH_ASSERT(m_pShaderComboBox != nullptr && m_pSessionController != nullptr)
    {
        auto &profilerBL = m_pSessionController->GetProfilerBL();
        
        gtVector<lpgpu2::db::ShaderTrace> shaderTraces;    
        if(profilerBL.GetShaderTraces(shaderTraces))
        {
            for (const auto &shader : shaderTraces)
            {
                const QPair<gtUInt64, gtUInt64> shaderId{ shader.m_shaderId, static_cast<gtUInt64>(shader.m_apiId) };
                m_pShaderComboBox->addItem(QString{ "Shader %0:%1" }.arg(shader.m_frameNum).arg(shader.m_drawNum), QVariant::fromValue(shaderId));
            }
        }
    }

    return PPFnStatus::success;
}

/// @brief  Get all the counter available in the current session and populate the combo box for the user to select it.
/// @return PPFnStatus Success/failure enum value.
lpgpu2::PPFnStatus LPGPU2ppMultiSourceCodeView::InitCounters()
{
    auto bReturn = PPFnStatus::failure;

    GT_IF_WITH_ASSERT(m_pCounterComboBox != nullptr && m_pSessionController != nullptr)
    {
        auto &profilerBL = m_pSessionController->GetProfilerBL();
     
        // Get the list of counters from both databases
        gtMap<int, AMDTPwrCounterDesc*> counterDetails;
        if (profilerBL.GetAllSessionCountersDescription(counterDetails))
        {
            for (const auto& counterDetailPair : counterDetails)
            {
                const auto *counterDetail = counterDetailPair.second;
                
                const auto counterName = QString{ "%0 - %1" }.arg(counterDetail->m_counterID).arg(counterDetail->m_name);
                m_pCounterComboBox->addItem(counterName, QVariant::fromValue(counterDetail->m_counterID));
            }            
        }    

        // Need to delete AMDTPwrCounterDesc*
        for (const auto& counterDetailsPair : counterDetails)
        {
            if (counterDetailsPair.second != nullptr)
            {
                delete counterDetailsPair.second;
            }
        }

        bReturn = PPFnStatus::success;
    }

    return bReturn;
}

/// @brief  Get all the feedback scripts that can execute the shader_counter_match feedback and populates the combo box for the user to select one.
/// @return PPFnStatus Success/failure enum value.
lpgpu2::PPFnStatus LPGPU2ppMultiSourceCodeView::InitFeedbackScripts()
{
    auto bReturn = PPFnStatus::failure;    

    GT_IF_WITH_ASSERT(m_pScriptComboBox != nullptr && m_pSessionController != nullptr)
    {
        auto& profilerBL = m_pSessionController->GetProfilerBL();
        
        std::unique_ptr<lpgpu2::db::LPGPU2FeedbackEngine> feedbackEnginePtr;
        if (profilerBL.GetFeedbackEngineForSession(feedbackEnginePtr))
        {
            // Set the feedback engine callback to the output Window.
            auto *pAppCommands = afApplicationCommands::instance();
            if (pAppCommands != nullptr)
            {
                pAppCommands->ClearInformationView();
                feedbackEnginePtr->SetOutputCallbackFunc([=](const std::string &str)
                {
                    pAppCommands->AddStringToInformationView(QString::fromStdString(str));
                });
            }

            gtVector<gtString> avaiableScripts;
            feedbackEnginePtr->GetAvailableFeedbackScripts(avaiableScripts);

            for (const auto &script : avaiableScripts)
            {
                // For each script, check if the shader feedback script is available
                osFilePath scriptFullPath{ osFilePath::OS_CODEXL_BINARIES_PATH };
                scriptFullPath.appendSubDirectory(PP_STR_FeedbackEngineScriptsFolder_W);
                scriptFullPath.setFileName(script);

                gtVector<lpgpu2::db::AvailableFeedback> feedbackOptions;
                if (feedbackEnginePtr->GetAvailableFeedbackOptions(scriptFullPath, feedbackOptions) == lpgpu2::db::success)
                {
                    // Look for the options to run the shader match
                    const auto availableFeedbackIt = std::find_if(feedbackOptions.begin(), feedbackOptions.end(), [](const lpgpu2::db::AvailableFeedback &feedback)
                    {
                        return feedback.scriptFunction.find(PP_STR_LPGPU2_ShaderMatch_FeedbackScriptName) != -1;
                    });

                    // If we have the required feedback:
                    if (availableFeedbackIt != feedbackOptions.end())
                    {
                        m_pScriptComboBox->addItem(acGTStringToQString(script));
                    }                    
                }                
            }            

            bReturn = PPFnStatus::success;
        }
    }

    return bReturn;
}

/// @brief Open an shader
/// @param asmText The text of the ASM shader
/// @param asmDescription The description (title) for the ASM shader
/// @param stats The statistics to display besides the ASM shader
/// @return PPFnStatus Success/failure enum value
lpgpu2::PPFnStatus LPGPU2ppMultiSourceCodeView::OpenASM(const gtString &asmText,
    const gtString &asmDescription,
    const gtVector<lpgpu2::db::ShaderAsmStat> &stats)
{
  auto it = m_openTabsByName.find(
      acGTStringToQString(asmText));

  if (it != m_openTabsByName.end())
  {
    m_pTabWidgdet->setCurrentIndex(it.value());
    return PPFnStatus::success;
  }

  // Hide the info widget:
  m_pInfoWidget->setVisible(false);
  m_pTabWidgdet->setVisible(true);

  // Create the source code view
  lpgpu2::ppSrcCodeWithSplitView *pSourceCodeView = nullptr;
  LPGPU2PPNewQtLPGPU2Widget(&pSourceCodeView, this);

  auto status = pSourceCodeView->DisplayASMText(asmText, asmDescription, stats, {});

  if (status == PPFnStatus::failure)
  {
    return PPFnStatus::failure;
  }

  // Add the source code view to the tab widget
  auto newIndex = m_pTabWidgdet->addTab(pSourceCodeView,
    acGTStringToQString(asmDescription));
  m_pTabWidgdet->setCurrentIndex(newIndex);

  m_openTabsByName[acGTStringToQString(asmDescription)] = newIndex;

  setVisible(true);

  return PPFnStatus::success;

}


/// @brief Open a high-level shader with its related ASM code
/// @param text The text of the higher-level shader
/// @param shaderFileName The filename of the higher-level shader
/// @param asmText The text of the ASM shader
/// @param asmDescription The description (title) for the ASM shader
/// @param stats The statistics to display besides the ASM shader
/// @return PPFnStatus Success/failure enum value
PPFnStatus LPGPU2ppMultiSourceCodeView::OpenShaderWithASM(const gtString &text,
      const osFilePath& shaderFileName,
      const gtString &asmText,
      const gtString &asmDescription,
      const gtVector<lpgpu2::db::ShaderAsmStat> &stats,
      const gtVector<lpgpu2::db::ShaderToAsm> &vShaderToAsm)
{
  auto status = OpenShaderNoASM(text, shaderFileName);

  if (status == PPFnStatus::failure)
  {
    return PPFnStatus::failure;
  }

  auto *pWidget = m_pTabWidgdet->currentWidget();
  if (pWidget == nullptr)
  {
    return PPFnStatus::failure;
  }

  auto *pSourceCodeView = qobject_cast<lpgpu2::ppSrcCodeWithSplitView*>(pWidget);
  if (pSourceCodeView == nullptr)
  {
    return PPFnStatus::failure;
  }

  return pSourceCodeView->DisplayASMText(asmText, asmDescription, stats, vShaderToAsm);
}

/// @brief Open a high-level shader
/// @param text The text of the higher-level shader
/// @param shaderFileName The filename of the higher-level shader
/// @return PPFnStatus Success/failure enum value
lpgpu2::PPFnStatus LPGPU2ppMultiSourceCodeView::OpenShaderNoASM(
    const gtString &shaderText,
      const osFilePath& shaderFileName)
{
  auto it = m_openTabsByName.find(
      acGTStringToQString(shaderFileName.asString()));

  if (it != m_openTabsByName.end())
  {
    m_pTabWidgdet->setCurrentIndex(it.value());
    return PPFnStatus::success;
  }

  // Hide the info widget:
  m_pInfoWidget->setVisible(false);
  m_pTabWidgdet->setVisible(true);

  // Create the source code view
  lpgpu2::ppSrcCodeWithSplitView *pSourceCodeView = nullptr;
  LPGPU2PPNewQtLPGPU2Widget(&pSourceCodeView, this);

  auto status = pSourceCodeView->DisplayMainTextAsFile(shaderText, shaderFileName);

  if (status == PPFnStatus::failure)
  {
    return PPFnStatus::failure;
  }

  // Add the source code view to the tab widget
  gtString shaderName;
  shaderFileName.getFileName(shaderName);
  auto newIndex = m_pTabWidgdet->addTab(pSourceCodeView,
    acGTStringToQString(shaderName));
  m_pTabWidgdet->setCurrentIndex(newIndex);

  m_openTabsByName[acGTStringToQString(shaderName)] = newIndex;

  setVisible(true);

  return PPFnStatus::success;
}

///// @brief Opens a binary shader by performing SPIR-V disassembly on the data.
///// @param shaderBin  Vector of bytes containing the shader data.
///// @param shaderFileName The filename of the binary shader
//lpgpu2::PPFnStatus LPGPU2ppMultiSourceCodeView::OpenShaderBinary(const std::vector<gtUByte> &shaderBin,
//  const osFilePath& shaderFileName)
//{
//  //gtString shaderDisassembly;
//  //const auto status = lpgpu2::SPIRVDisassemble(shaderBin, shaderDisassembly);
//
//  //if (status == PPFnStatus::failure)
//  //{
//  //  return PPFnStatus::failure;
//  //}
//
//  return PPFnStatus::success;
//}

/// @brief Slot called to open the 
/// @param frameNum The frame number the shader is located at.
///  @param drawNum  The draw number the shader is located at.
void LPGPU2ppMultiSourceCodeView::OpenAndDisplayShaderSlot(gtUInt32 frameNum, gtUInt32 drawNum)
{
  GT_IF_WITH_ASSERT(m_pSessionController != nullptr)
  {
      auto &profilerBL = m_pSessionController->GetProfilerBL();

      gtVector<lpgpu2::db::ShaderTrace> shaderTraces;
      if (profilerBL.GetShaderTracesInFrameRange(frameNum, frameNum, shaderTraces))
      {
          for (const auto& shader : shaderTraces)
          {
              if (shader.m_drawNum == drawNum)
              {
                  const auto shaderNameStr = QString{ "Shader %0:%1" }.arg(
                      QString::number(frameNum), QString::number(drawNum));

                  const auto shaderNameAsFilePath = osFilePath{ acQStringToGTString(shaderNameStr) };

                  if (shader.m_asmCode.isEmpty())
                  {
                      // TODO use different function calls to show the text depending on whether
                      // the higher level shader code is available or not, and on whether the ASM
                      // shader code is available or not
                      if (shader.m_srcCode.isEmpty() && shader.m_binary.size() > 0)
                      {
                        //const auto status = OpenShaderBinary(shader.m_binary,
                        //                    shaderNameAsFilePath);
                        //GT_ASSERT(status == PPFnStatus::success);
                      }
                      else
                      {
                        const auto status = OpenShaderNoASM(shader.m_srcCode,
                                            shaderNameAsFilePath);
                        GT_ASSERT(status == PPFnStatus::success);
                      }
                  }
                  else
                  {
                      gtVector<lpgpu2::db::ShaderAsmStat> shaderAsmStats;
                      profilerBL.GetShaderAsmStats(shader.m_shaderId,
                                                   shaderAsmStats);

                      gtVector<lpgpu2::db::ShaderToAsm> shaderToAsm;
                      profilerBL.GetShaderToAsmMappings(shader.m_shaderId, shaderToAsm);

                      // TODO use different function calls to show the text depending on whether
                      // the higher level shader code is available or not, and on whether the ASM
                      // shader code is available or not
                      const auto asmNameStr = QString{ "ASM of %0" }.arg(shaderNameStr);
                      const auto status = OpenShaderWithASM(shader.m_srcCode,
                                                      shaderNameAsFilePath,
                                                      shader.m_asmCode,
                                                      acQStringToGTString(asmNameStr),
                                                      shaderAsmStats,
                                                      shaderToAsm);
                      GT_ASSERT(status == PPFnStatus::success);

                  }

                  // Inform listeners that the tab was opened successfully
                  emit SourceSet();
              }
          }
      }
  }
}

/// @brief Called by the tab widget when the user tries to close a tab
/// @see   This function does not use the PPFnStatus return value idiom
///        because it needs to interface with Qt
/// @param index The index of the tab in the tab widget
void LPGPU2ppMultiSourceCodeView::OnTabCloseRequestedSlot(int index)
{
  auto *pWidget = m_pTabWidgdet->widget(index);
  if (pWidget == nullptr)
  {
    return;
  }
  auto *pSourceCodeView = qobject_cast<lpgpu2::ppSrcCodeWithSplitView*>(pWidget);
  if (pSourceCodeView == nullptr)
  {
    return;
  }

  m_openTabsByName.remove(
      acGTStringToQString(pSourceCodeView->GetMainTextFileName()));
  m_pTabWidgdet->removeTab(index);

  if (m_pTabWidgdet->count() == 0)
  {
      m_pInfoWidget->show();
  }
}
