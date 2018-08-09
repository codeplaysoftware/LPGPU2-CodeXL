//==================================================================================
// Copyright (c) 2011 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppTimeline.h
///
//==================================================================================

#ifndef _PPTIMELINE_H_
#define _PPTIMELINE_H_

#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>

class ppTimeline : public acTimeline
{
    Q_OBJECT
    
public:
    ppTimeline(QWidget* parent, const Qt::WindowFlags flags = 0);

    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;       

signals:
    void mouseMove(int);
    void mouseRelease(int);
};

#endif
