// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the Session Comparison View widget.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_SESSION_COMPARISON_VIEW_H_INCLUDE
#define LPGPU2_PP_SESSION_COMPARISON_VIEW_H_INCLUDE

// Qt:
#include <QWidget>

// For ExplorerSessionId
#include <AMDTSharedProfiling/inc/SessionExplorerDefs.h>

// Declarations:
namespace lpgpu2 {  enum class PPFnStatus; }
class QComboBox;
class QTableWidget;
class QPushButton;
class osFilePath;
class osDirectory;
class acCustomPlot;
class SessionTreeNodeData;
class ppSessionController;

/// @brief  Widget to display the comparison between two Power Profiling sessions.
///         It till display a statistics table for comparing counter data and
///         also a plot with the difference between the two sessions for the selected
///         counters.
/// @date   01/11/2017
/// @author Thales Sabino
// clang-format off
class LPGPU2ppSessionComparisonView : public QWidget
{
    Q_OBJECT
// Methods
public:
    explicit LPGPU2ppSessionComparisonView(ppSessionController *vpSessionController, QWidget *vpParent = nullptr);
    virtual ~LPGPU2ppSessionComparisonView();

    lpgpu2::PPFnStatus Initialise();
    lpgpu2::PPFnStatus Shutdown();

// Slots
private slots:    
    void OnSessionAdded(SessionTreeNodeData* pNewSessionData) const;
    void OnSessionRenamed(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir) const;
    void OnSessionDeleted(const ExplorerSessionId vSessionId, SessionExplorerDeleteType vDeleteType, bool& vrSessionDeleted) const;
    void OnComparisonTriggered();
    void OnStatisticsTableSelectionChanged();
   
// Methods
private:
    void AddSessionToComparisonList(SessionTreeNodeData *vpSessionData) const;

// Attributes
private:
    QComboBox *m_pSessionComboBox = nullptr;
    QPushButton *m_pComparePushButton = nullptr;
    QTableWidget *m_pStatisticsTable = nullptr;
    acCustomPlot *m_pDiffPlot = nullptr;

    SessionTreeNodeData *m_pSessionNodeData = nullptr;    
    ppSessionController *m_pSessionController = nullptr;

    ExplorerSessionId m_otherSessionId = SESSION_ID_ERROR;
};
// clang-format on

#endif // LPGPU2_PP_SESSION_COMPARISON_VIEW_H_INCLUDE
