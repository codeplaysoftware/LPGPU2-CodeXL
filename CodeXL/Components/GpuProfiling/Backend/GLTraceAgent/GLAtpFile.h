#pragma once

#include "../Common/FileUtils.h"
#include "APIInfoManagerBase.h"
#include "GLAPIInfo.h"
#include "../sprofile/AtpFile.h"
#include <sstream>
#include "../Common/ATPFileUtils.h"
#include "../Common/StringUtils.h"

typedef std::map<osThreadId, std::vector<GLAPIInfo*> > GLAPIInfoMap;


class GLAtpFilePart : public IAtpFilePart, public IAtpFilePartParser, public BaseParser<GLAPIInfo>
{
public:
	/// Constructor
	/// \param config Config object
	GLAtpFilePart(const Config& config, bool shouldReleaseMemory = false) : IAtpFilePart(config, shouldReleaseMemory) {
		m_strPartName = "GL";
		m_sections.push_back("GL Timestamp Output");
	}

	/// Parse input stream
	/// \param in Input stream
	/// \return True if succeeded
	bool Parse(std::istream& in, std::string& outErrorMsg) override;

	void WriteHeaderSection(SP_fileStream& sout) override;
	bool WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const std::string& strPID) override;

	bool ParseHeader(const std::string& strKey, const std::string& strVal) override
	{
		SP_UNREFERENCED_PARAMETER(strKey);
		SP_UNREFERENCED_PARAMETER(strVal);
		return false;
	}
};
