//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GLAPIDefs.cpp $
/// \version $Revision: #14 $
/// \brief  This file contains definitions for GL API Functions

#include <qtIgnoreCompilerWarnings.h>

#ifdef _WIN32
#pragma warning(push, 1)
#endif
#include <QtCore>
#include <QtWidgets>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include <AMDTBaseTools/Include/gtAssert.h>

#include "GLAPIDefs.h"

using namespace gl;

APIDefs::APIDefs()
{
	m_APIStrings <<
		"glBegin" <<
		"glEnd" <<
		"";

	assert(m_APIStrings.length() - 1 == static_cast<int>(FuncType::Unknown));

	m_APIMap.insert(m_APIStrings[static_cast<int>(FuncType::GlBegin)], FuncType::GlBegin);
	m_APIMap.insert(m_APIStrings[static_cast<int>(FuncType::GlEnd)], FuncType::GlEnd);

	m_APIGroupMap.insert(FuncType::GlBegin, APIGroup::Base);
	m_APIGroupMap.insert(FuncType::GlEnd, APIGroup::Base);

}

const QString& APIDefs::GetOpenGLAPIString(FuncType type)
{
	if (type > FuncType::Unknown)
	{
		type = FuncType::Unknown;
	}

	return m_APIStrings[static_cast<size_t>(type)];
}

APIGroup APIDefs::GetGLAPIGroup(FuncType type)
{
	APIGroup retVal = APIGroup::Unknown;

	if (m_APIGroupMap.contains(type))
	{
		retVal = m_APIGroupMap[type];
	}

	return retVal;
}

FuncType APIDefs::ToGLAPIType(QString name)
{
	FuncType retVal = FuncType::Unknown;

	if (m_APIMap.contains(name))
	{
		retVal = m_APIMap[name];
	}

	return retVal;
}

const QString APIDefs::GroupToString(APIGroup group)
{
	switch (group)
	{
	case APIGroup::Base: return "Base";
	default:
		return "Other";
	}
}


