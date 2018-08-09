//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GLAPIDefs.h $
/// \brief  This file contains definitions for GL API Functions

#ifndef _GLAPI_DEFS_H_
#define _GLAPI_DEFS_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

#include "GLFunctionEnumDefs.h"

#include <TSingleton.h>

namespace gl {
	/// Group IDs for OpenGL APIs
	enum class APIGroup
	{
		Unknown = 0,
		Base = 0x1,
	};

	/// a type representing a set of GLAPIGroup values
	typedef unsigned int GLAPIGroups;

	/// Singleton class with helpers for dealing with GL function types
	class APIDefs : public TSingleton<APIDefs>
	{
		friend class TSingleton<APIDefs>;
	public:
		/// Gets the OpenGL API function name for the specified GL_FUNC_TYPE
		/// \param type the function type whose function name is requested
		/// \return the OpenGL API function name for the specified GL_FUNC_TYPE
		const QString& GetOpenGLAPIString(FuncType type);

		/// Gets the groups for the specified function type
		/// \param type the function type whose groups are requested
		/// \return the groups for the specified function type
		APIGroup GetGLAPIGroup(FuncType type);

		/// Gets the GL_FUNC_TYPE value for the given function name
		/// \param name the function name whose GL_FUNC_TYPE value is requested
		/// \return the GL_FUNC_TYPE value for the given function name
		FuncType ToGLAPIType(QString name);

		/// Gets the group name from the given GLAPIGroup value
		/// \param group the GLAPIGroup whose name is requested
		/// \return the group name from the given GLAPIGroup value
		const QString GroupToString(APIGroup group);

		/// Is this an OpenGL function string:
		/// \param apiFunctionName the name of the function
		/// \return true iff the string represents an OpenGL function
		bool IsGLAPI(const QString& apiFunctionName) const { return m_APIStrings.contains(apiFunctionName); };

		/// Append the list of all OpenGL functions to the list:
		/// \param list[out] the list to append the OpenGL functions for
		void AppendAllGLFunctionsToList(QStringList& list) { list << m_APIStrings; };


	private:
		/// Initializes the static instance of the GLAPIDefs class
		APIDefs();

		QStringList                  m_APIStrings;  ///< list of OpenGL function names
		QMap<QString, FuncType>      m_APIMap;     ///< map of function names to GL_FUNC_TYPE value
		QMap<FuncType, APIGroup>     m_APIGroupMap;    ///< map of GL_FUNC_TYPE to group id
	};
};

#endif // _GLAPI_DEFS_H_

