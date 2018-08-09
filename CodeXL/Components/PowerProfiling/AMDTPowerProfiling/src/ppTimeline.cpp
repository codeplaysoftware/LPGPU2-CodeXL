#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <QMouseEvent>

#include <AMDTPowerProfiling/src/ppTimeline.h>

ppTimeline::ppTimeline(QWidget* parent, const Qt::WindowFlags flags)
    : acTimeline(parent, flags)
{
    m_pHScrollBar = nullptr;
}

void ppTimeline::mouseMoveEvent(QMouseEvent* event)
{
    emit mouseMove(event->x());    

    // Uncomment this to enable mouse drag
    //acTimeline::mouseMoveEvent(event);
}

void ppTimeline::wheelEvent(QWheelEvent* event)
{
    GT_UNREFERENCED_PARAMETER(event);

    // Uncomment this to enable mouse zoom with the wheel
    //acTimeline::wheelEvent(event);
}

void ppTimeline::mouseReleaseEvent(QMouseEvent* event)
{
    acTimeline::mouseReleaseEvent(event);
    setPivot(-1);
}
