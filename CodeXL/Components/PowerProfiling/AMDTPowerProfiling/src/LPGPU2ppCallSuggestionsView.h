// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the Call Suggestions View widget.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_CALL_SUGGESTIONS_VIEW_H_INCLUDE
#define LPGPU2_PP_CALL_SUGGESTIONS_VIEW_H_INCLUDE

// Qt:
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

// Declarations
namespace lpgpu2 { namespace db { struct Annotation; } }
class QTableWidget;
class QTableWidgetItem;
class LPGPU2ppInfoWidget;
class QTabWidget;
class QLineEdit;

/// @brief   Represents the widget used to display annotations related
///          to API calls performed in a power profiling session
/// @warning None.
/// @author  Thales Sabino.
/// @date    20/11/2017
// clang-format off
class PP_API LPGPU2ppCallSuggestionsView : public QWidget
{
    Q_OBJECT

// Methods
public:
    explicit LPGPU2ppCallSuggestionsView(QWidget *vpParent = nullptr);
    virtual ~LPGPU2ppCallSuggestionsView();

    lpgpu2::PPFnStatus Initialise();
    lpgpu2::PPFnStatus Shutdown();

    lpgpu2::PPFnStatus SetAnnotations(const gtVector<lpgpu2::db::Annotation> &vAnnotations);
    lpgpu2::PPFnStatus SetGlobalAnnotations(const gtVector<lpgpu2::db::Annotation> &vAnnotations);
    lpgpu2::PPFnStatus SetUserAnnotations(const gtVector<lpgpu2::db::Annotation> &vAnnotations);

// Signals
signals:
    void SuggestionSelected(gtUInt32 vAnnotationId);

// Slots
private slots:
    void OnTableItemActivated(QTableWidgetItem *vpTableItem);
    void OnFilterTextChanged(const QString& vText);

// Methods
private:
    lpgpu2::PPFnStatus InitialiseTableWidgetLayout(QTableWidget *pTable);
    lpgpu2::PPFnStatus SetAnnotationsForTableWidget(const gtVector<lpgpu2::db::Annotation> &vAnnotations, QTableWidget *pTable);

// Attributes
private:    
    QLineEdit    *m_pFilterLineEdit = nullptr;    
    QTableWidget *m_pLocalAnnotationsWidget  = nullptr;
    QTableWidget *m_pGlobalAnnotationsWidget = nullptr;
    QTableWidget *m_pUserAnnotationsWidget   = nullptr;
    QTabWidget   *m_pAnnotationsTab          = nullptr;
    LPGPU2ppInfoWidget *m_pInfoWidget        = nullptr;
    bool m_isLocalTableVisible               = true;
    bool m_isGlobalTableVisible              = true;
    bool m_isUserTableVisible                = true;
};
// clang-format on

#endif // LPGPU2_PP_CALL_SUGGESTIONS_VIEW_H_INCLUDE
