//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages the GL APIs for API tracing.
//==============================================================================

#ifndef _GL_API_INFO_H_
#define _GL_API_INFO_H_

#include "../Common/APIInfo.h"
#include "GLFunctionEnumDefs.h"

//------------------------------------------------------------------------------------
/// GL API Base class
//------------------------------------------------------------------------------------
class GLAPIInfo : public APIInfo
{
public:
	/// Virtual destructor
	virtual ~GLAPIInfo() {}

	unsigned int m_apiID = 0;         ///< CL API ID, defined in ../Common/CLFunctionEnumDefs.h
	gl::FuncType m_Type = gl::FuncType::Unknown;  ///< CL API Type
	std::string m_strComment;                     ///< Comment
}; 
#endif //_CL_API_INFO_H_
