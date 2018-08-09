// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Define globals to be used to support the Android profile mode
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_ANDROIDPACKETGLOBALS_H
#define LPGPU2_ANDROIDPACKETGLOBALS_H

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// clang-format off
const gtSizeType CRP_CHUNK_HEADER_LENGHT = 8;

// Remote protocol tokens
enum class RemProtoToken : gtUInt32 {
  CRP_ALIVE                   = 0,
  CRP_GET_DEVICE_INFO         = 1,
  CRP_RAGENT_INFO             = 2,
  CRP_SEND_COLLECTION_OPTIONS = 3,
  CRP_START_COLLECTION        = 4,
  CRP_STOP_COLLECTION         = 5,
  CRP_FLUSH                   = 6,
  CRP_GET_SUPPORTED_APPS      = 7,
  CRP_CLEAN_STATE             = 8,
  CRP_DATA_PACKET             = 9,
  CRP_DISCONNECT              = 100,

  // Response codes
  CRP_RESPONSE_ACK            = 99,
  CRP_RESPONSE_NACK           = 98,
  CRP_RESPONSE_STOP           = 97,
  END_OF_ENUM
}; // enum class RemProtoToken

// Types of data chunks which can be sent
enum class DataChunkType: gtUInt32 {
  NONE        = 0x0,
  TRACE       = 0x3000,
  COUNTER     = 0x3001,
  STACK       = 0x3002,
  PARAMS      = 0x3003,
  HINTS       = 0x3004,
  ANNOTATIONS = 0x3005,
  TEXTURE     = 0x3006,
  GPUTIMER    = 0x3007,
  TRACKING    = 0x3008,
  GATOR       = 0x3009,
  SHADER      = 0x300A,
  COUNTERV2   = 0x300B,
  MEGACHUNK   = 0x300C,
  SHADER_META = 0x300E,
  DATA_STOP   = 0x4000,
  END_OF_ENUM
}; // enum class DataChunkType

// Possible flags for a data chunk
enum class DataChunkFlags: gtUInt32 {
  NORMAL          = 0,
  ZLIB_COMPRESSED = 1,
  END_OF_ENUM
}; // enum class DataChunkType

enum class ShaderMetaDataType: gtUInt32 {
  ASSEMBLY    = 0,
  BINARY      = 1,
  DBG         = 2,
  END_OF_ENUM
}; // enum class ShaderMetaType

/// @brief simple utility function to get the gtUInt32 version of a remote
///        protocol token
gtUInt32 toGtUI32(RemProtoToken tok);
/// @brief simple utility function to get the gtUInt32 version of a chunk
///        type
gtUInt32 toGtUI32(DataChunkType type);

/// @brief    Represents the header to be used in the SAMS Android protocol
///           in any packet exchanged between the two parties (CXL and the
///           device).
/// @warning  None.
/// @date     06/09/2017.
/// @author   Alberto Taiuti.
struct ChunkHeader {
// Methods:
  ChunkHeader()
    : id(RemProtoToken::CRP_RESPONSE_NACK), payloadSize(0U) {}

// Attributes:
  RemProtoToken id;
  gtUInt32 payloadSize;

}; // struct ChunkHeader

const gtSizeType CRP_CHUNK_HEADER_SIZE = sizeof(ChunkHeader);

/// @brief    Used to hash the enum classes in an std::unordered_map
/// @warning  None.
/// @date     19/09/2017.
/// @author   Alberto Taiuti.
struct enum_hash
{
  template <typename T>
  inline
  typename std::enable_if<std::is_enum<T>::value, std::size_t>::type
  operator ()(T const value) const
  {
    return static_cast<std::size_t>(value);
  }
}; // struct enum_hash

/// @brief Create a coupling between API calls and their bitfield value to be
///        used by the RAgent
struct FrameTerminatorApiCalls {
  enum FrameTerminatorApiCall
  {
    FT_GL_CLEAR            = 1 << 0,
    FT_GL_FLUSH            = 1 << 1,
    FT_GL_FINISH           = 1 << 2,
    FT_EGL_SWAPBUFFERS     = 1 << 6,
    FT_CL_FLUSH            = 1 << 10,
    FT_CL_FINISH           = 1 << 11,
    FT_CL_WAITFOREVENTS    = 1 << 12,
    FT_VK_QUEUEPRESENTKHR  = 1 << 15,
  };
};

/// @brief Annotation types
enum class AnnotationTypes {
  LPGPU2_USER_USER          = 0,
  LPGPU2_USER_KHR_DEBUG     = 1,
  LPGPU2_USER_SHIM          = 2,
  LPGPU2_USER_LIMITATION    = 3,
  LPGPU2_SUGGESTION         = 10,
  LPGPU2_INFORMATION        = 11,
  LPGPU2_ISSUE              = 12,
  LPGPU2_ERROR              = 13,
  LPGPU2_GENERAL            = 14,
  LPGPU2_LIMITATION         = 15,
  LPGPU2_GLOBAL_SUGGESTION  = 30,
  LPGPU2_GLOBAL_INFORMATION = 31,
  LPGPU2_GLOBAL_ISSUE       = 32,
  LPGPU2_GLOBAL_ERROR       = 33,
  LPGPU2_GLOBAL_GENERAL     = 34,
  LPGPU2_GLOBAL_LIMITATION  = 35
};
// Utilities to refer to the entries in the annotation types list which start
// and end the range of a type of annotations
const auto kUserAnnotationStartID = AnnotationTypes::LPGPU2_USER_USER;
const auto kUserAnnotationEndID   = AnnotationTypes::LPGPU2_USER_LIMITATION;
const auto kGlobalAnnotationStartID = AnnotationTypes::LPGPU2_GLOBAL_SUGGESTION;
const auto kGlobalAnnotationEndID   = AnnotationTypes::LPGPU2_GLOBAL_LIMITATION;


// clang-format on

#endif // LPGPU2_ANDROIDPACKETGLOBALS_H
