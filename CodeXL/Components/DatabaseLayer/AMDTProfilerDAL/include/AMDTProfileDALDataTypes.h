//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTProfileDALDataTypes.h
///
//==================================================================================

#ifndef _AMDTPROFILEDALDATATYPES_H_
#define _AMDTPROFILEDALDATATYPES_H_

//++AT:LPGPU2
// Imports typedefs integers
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
//--AT:LPGPU2

// This is only used between DAL & Adapter
struct PPSampleData
{
//++AT:LPGPU2
// Type aliases
  using BlobVec = gtVector<gtUByte>;

// Attributes
  gtInt64 m_quantizedTime = 0;
  gtInt32 m_counterID = 0;
  double  m_sampleValue = 0.0;
  BlobVec m_blob;

// Methods
  // Ctor
  PPSampleData(gtInt64 quantizedTime, gtInt32 counterID, double sampleValue) :
        m_quantizedTime{quantizedTime},
        m_counterID{counterID},
        m_sampleValue{sampleValue},
        m_blob{} {}
  PPSampleData(gtInt64 quantizedTime, gtInt32 counterID, double sampleValue,
      const BlobVec &blob) :
        m_quantizedTime{quantizedTime},
        m_counterID{counterID},
        m_sampleValue{sampleValue},
        m_blob{blob} {}
  PPSampleData(gtInt64 quantizedTime, gtInt32 counterID, double sampleValue,
      BlobVec &&blob) :
        m_quantizedTime{quantizedTime},
        m_counterID{counterID},
        m_sampleValue{sampleValue},
        m_blob{std::move(blob)} {}

//--AT:LPGPU2
};


#endif //_AMDTPROFILEDALDATATYPES_H_
