// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFrameDetailView.h
///
/// LPGPU2ppFrameDetailView declarations.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved

#ifndef __LPGPU2PPFRAMETIMELINE_H
#define __LPGPU2PPFRAMETIMELINE_H

#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>

class LPGPU2ppFrameTimeline final : public acTimeline
{
    Q_OBJECT
    
public:
    LPGPU2ppFrameTimeline(QWidget* parent, Qt::WindowFlags flags = 0);

    void updateGrid() override;
};

#endif // __LPGPU2PPFRAMETIMELINE_H
