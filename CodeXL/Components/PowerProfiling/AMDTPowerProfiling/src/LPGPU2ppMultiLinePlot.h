
#ifndef LPGPU2_PP_MULTILINEPLOT_H_INCLUDE
#define LPGPU2_PP_MULTILINEPLOT_H_INCLUDE

#include <AMDTPowerProfiling/src/ppMultiLinePlot.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// Forward declarations
namespace lpgpu2 {
  enum class PPFnStatus;
}

class PP_API LPGPU2ppMultiLinePlot : public ppMultiLinePlot
{
    Q_OBJECT

// Methods:
public:
    explicit LPGPU2ppMultiLinePlot(ppSessionController *vpSessionController);
    virtual ~LPGPU2ppMultiLinePlot();

    lpgpu2::PPFnStatus Initialise();

    lpgpu2::PPFnStatus AddRangeHighlight(gtUInt32 vRegionId, double vRangeStart, double vRangeEnd, const QColor &vColor);
    lpgpu2::PPFnStatus ClearHighlights();

// Signals:
signals:
    void RegionClicked(gtUInt32 vRegionId);

// Methods:
private slots:
    void OnRegionSelected(bool vbSelected);
    void OnYAxisRangeChanged(const QCPRange &newRange);

// Typedef:
private:
    using MapHightlightToRegionId_t = QMap<QCPItemRect*, gtUInt32>;

// Attributes
private:
    static constexpr int pp_defaultAlphaValue = 80;
    static constexpr int pp_defaultDarkerFactor = 100;

    MapHightlightToRegionId_t   m_hightlightRegionToIdMap;
};

#endif // LPGPU2_PP_MULTILINEPLOT_H_INCLUDE