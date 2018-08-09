// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief The SYCL ATP File Parser.
///
/// SYCLAtpFilePart implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include "SYCLAtpFile.h"
#include <codexl_profiling_api.h>

// STL:
#include <iostream>
#include <unordered_map>

/// @brief Class Constructor. Calls the base constructors and set the file part
///        this class represents as "SYCL"
/// @param[vIn] vConfig  The ATP configuration object.
/// @param[vIn] vbShouldReleaseMemory    true = Should release the memory used by this class upon destruction,
///                                     false = Should keep the memory used after destruction.
SYCLAtpFilePart::SYCLAtpFilePart(const Config &vConfig, bool vbShouldReleaseMemory /* = false */)
    : IAtpFilePart{ vConfig, vbShouldReleaseMemory }
    , IAtpFilePartParser{}
    , BaseParser<SYCLAPIInfo>{}
{
    m_strPartName = "SYCL";
    m_sections.push_back("SYCL Timestamp Output");
}

/// @brief                   Parses the stream as a SYCL ATP file.
/// @param[in]  in           The stream to read the ATP file.
/// @param[out] vOutErrorMsg The error message in case something went wrong.
/// @warning                 The stream must be called "in" because this method
///                          uses the macro READLINE defined in one of the
///                          base class headers.
bool SYCLAtpFilePart::Parse(std::istream &in, std::string &vOutErrorMsg)
{
    auto bError = false;
    auto bTSStart = false;
    auto bReadQueueInfo = false;

    std::cerr << "Started parsing SYCL..." << std::endl;

    std::string strProgressMessage = "Parsing trace data...";
    ErrorMessageUpdater errorMessageUpdater(vOutErrorMsg, this);

    do
    {
        if (m_shouldStopParsing)
        {
            break;
        }

        //read vIn initial line
        std::string line;
        READLINE(line);

        //ignore blank lines
        if (line.length() == 0)
        {
            continue;
        }

        if (line[0] == '=')
        {
            if (!bTSStart)
            {
                bTSStart = true;

                // read thread id
                ReadLine(in, line);
            }
        }

        // If it's not already been read, read the Queue information:
        if (!bReadQueueInfo)
        {
          unsigned int queueCount = 0;
          bool ret = StringUtils::Parse(line, queueCount);
          if (!ret)
          {
            Log(logERROR, "Failed to parse queue entry count.\n");
            return false;
          }
          for (unsigned int i = 0; i < queueCount; i++)
          {
            // read Queue ID
            READLINE(line);
            unsigned int queueID = 0;
            ret = StringUtils::Parse(line, queueID);
            if (!ret)
            {
                Log(logERROR, "Failed to parse queue ID\n");
                return false;
            }

            // read category count
            READLINE(line);
            unsigned int categoryCount = 0;
            ret = StringUtils::Parse(line, categoryCount);
            if (!ret)
            {
                Log(logERROR, "Failed to parse queue category count\n");
                return false;
            }
            std::map<int, unsigned int> mapCategoryDepth;
            for (unsigned int j = 0; j < categoryCount; j++)
            {
              READLINE(line);
              std::stringstream ss(line);
              int categoryId;
              int categoryDepth;
              ss >> categoryId;
              ss >> categoryDepth;
              mapCategoryDepth[categoryId] = categoryDepth;

              for (auto it = m_listenerList.begin(); it != m_listenerList.end(); it++)
              {
                if ((*it) != NULL)
                {
                    (*it)->SetAPIQueueDepth(queueID, mapCategoryDepth);
                }
              }
            }
          }
          bReadQueueInfo = true;
          ReadLine(in, line);
        }

        //first number after section header is the thread id
        osThreadId tid = 0;
        bool ret = StringUtils::Parse(line, tid);

        if (!ret)
        {
            std::cerr << "Unexpected data1" << std::endl;
            Log(logERROR, "Failed to parse thread ID, Unexpected data vIn input file.\n");
            return false;
        }

        //line after thread id is number of api calls for that thread
        READLINE(line);
        unsigned int apiNum = 0;
        ret = StringUtils::Parse(line, apiNum);

        if (!ret)
        {
            std::cerr << "Unexpected data2" << std::endl;
            Log(logERROR, "Failed to parse thread number, Unexpected data vIn input file.\n");
            return false;
        }

        // Parse maximum nesting depth per category:
        READLINE(line);
        unsigned int categoryDepthCount = 0;
        ret = StringUtils::Parse(line, categoryDepthCount);
        if (!ret)
        {
          Log(logERROR, "Failed to parse category depth count, Unexpected data.\n");
          return false;
        }

        std::map<int, unsigned int> mapCategoryDepth;
        for (unsigned int i = 0; i < categoryDepthCount; i++)
        {
          READLINE(line);
          std::stringstream ss(line);
          int categoryId;
          int categoryDepth;
          ss >> categoryId;
          ss >> categoryDepth;
          mapCategoryDepth[categoryId] = categoryDepth;
        }
        // -- -- --

        int apiIndex = -1;

        for (auto it = m_listenerList.begin(); it != m_listenerList.end(); it++)
        {
            if ((*it) != NULL)
            {
                (*it)->SetAPINum(tid, apiNum);
                (*it)->SetAPIDepth(tid, mapCategoryDepth);
            }
        }

        //parse #apiNum trace lines and create apiInfo objects
        for (unsigned int i = 0; i < apiNum && !m_shouldStopParsing; i++)
        {
            apiIndex++;
            READLINE(line);
            ReportProgress(strProgressMessage, i, apiNum);

            // timestamp
            SYCLAPIInfo* pAPIInfo = new SYCLAPIInfo;

            if (line.empty())
            {
                continue;
            }

            // ATP API line example:
            // start ts			       end ts				       thread    nest level  <event specific data>
            // 9232381192968       9232411229944       592	     3           .....................

            // Read in the first info that is common to all API entries:
            std::stringstream ss(line);

            ss >> pAPIInfo->m_ullStart;
            CHECK_SS_ERROR(ss);
            ss >> pAPIInfo->m_ullEnd;
            CHECK_SS_ERROR(ss);
            ss >> pAPIInfo->m_tid;
            CHECK_SS_ERROR(ss);
            ss >> pAPIInfo->m_nestLevel;
            CHECK_SS_ERROR(ss);
            ss >> pAPIInfo->m_queueNestLevel;
            CHECK_SS_ERROR(ss);

            // Use the remain data in the string stream to construct a new SYCL
            // Profile event object of the relevant type:
            pAPIInfo->m_syclProfileEvent = SYCLProfiler::Event::CreateFromATPText(ss);

            pAPIInfo->m_strName = pAPIInfo->m_syclProfileEvent->EventName();

            pAPIInfo->m_uiSeqID = i;
            pAPIInfo->m_uiDisplaySeqID = apiIndex;

            //send pAPIInfo to summarizer to generate API timeline item
            for (auto it = m_listenerList.begin(); it != m_listenerList.end() && !m_shouldStopParsing; it++)
            {
                if (((*it) != NULL) && (pAPIInfo != NULL))
                {
                    (*it)->OnParse(pAPIInfo, m_shouldStopParsing);
                }
                else
                {
                    SpBreak("pAPIInfo == NULL");
                }
            }

        }

    }
    while (!in.eof());

    return true;

}

/// @brief The SYCL file part has no header for its section,
///        so this method does nothing.
/// @param[in] vSOut Not used.
void SYCLAtpFilePart::WriteHeaderSection(SP_fileStream &vSOut)
{
    SP_UNREFERENCED_PARAMETER(vSOut);
}

/// @brief  Reads from the temporary sycltrace.tsamp file and writes into the specified
///         stream in order to consolidate the final ATP file.
/// @param[in]  vSOut The output stream to write the SYCL trace part of the ATP file.
/// @param[in]  vStrTmpFilePath The temporary file path where the SYCL ATP file was written.
/// @param[in]  vStrPID The PID of the process as a string.
/// @returns    bool    Always returns true.
/// @warning    vStrTmpFilePath and vStrPID are not being used right now.
bool SYCLAtpFilePart::WriteContentSection(SP_fileStream &vSOut, const std::string &vStrTmpFilePath, const std::string &vStrPID)
{
    SP_UNREFERENCED_PARAMETER(vStrTmpFilePath);
    SP_UNREFERENCED_PARAMETER(vStrPID);

    std::string syclOutPath(FileUtils::GetTempFragFilePath());
    syclOutPath.append("/sycltrace" TMP_TIME_STAMP_EXT);
    std::ifstream syclDumpFile(syclOutPath.c_str());
    // SpAssertRet(syclDumpFile);
    vSOut << "=====SYCL Timestamp Output=====\n";
    std::string line;

    while (syclDumpFile.good())
    {
        std::getline(syclDumpFile, line);
#ifdef _WIN32
        auto wline = std::wstring(line.begin(), line.end());
        vSOut << wline << std::endl;
#else
        vSOut << line << std::endl;
#endif

    }
    return true;
}

/// @brief              The SYCL file part has no header for its section,
///                     so this method does nothing.
/// @returns   bool     Always return false.
/// @param[in] vStrKey  Not used.
/// @param[in] vStrVal  Not used.
bool SYCLAtpFilePart::ParseHeader(const std::string &vStrKey, const std::string &vStrVal)
{
    SP_UNREFERENCED_PARAMETER(vStrKey);
    SP_UNREFERENCED_PARAMETER(vStrVal);
    return false;
}
