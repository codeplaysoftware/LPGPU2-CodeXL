#include <AMDTPowerProfilingMidTier/include/LPGPU2_BaseDataPacketParser.h>
#include <utility>
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <cassert>

namespace lpgpu2 {

/// @brief Interface for classes which consume data. Implemented using the
///        NVI idiom
/// @param bp Handle to the bytes parser
/// @return RC::Success if successful, RC::Failure if unsuccessful
PPFnStatus BaseDataPacketParser::ConsumeData(BytesParser &bp)
{
  return ConsumeDataImpl(bp);
}

/// @brief Interface for classes which flush data. Implemented using the
///        NVI idiom
/// @return RC::Success if successful, RC::Failure if unsuccessful
PPFnStatus BaseDataPacketParser::FlushData()
{
  return FlushDataImpl();
}

} // namespace lpgpu2
