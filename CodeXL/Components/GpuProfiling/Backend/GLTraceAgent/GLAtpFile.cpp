#include "GLAtpFile.h"

bool GLAtpFilePart::Parse(std::istream & in, std::string & outErrorMsg) {

    bool bError = false;
    bool bTSStart = false;

    std::string strProgressMessage = "Parsing trace data...";
    ErrorMessageUpdater errorMessageUpdater(outErrorMsg, this);

    do
    {
        if (m_shouldStopParsing)
        {
            break;
        }

        //read in initial line
        std::string line;
        READLINE(line)

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

        //first number after section header is the thread id
        osThreadId tid = 0;
        bool ret = StringUtils::Parse(line, tid);

        if (!ret)
        {
            Log(logERROR, "Failed to parse thread ID, Unexpected data in input file.\n");
            return false;
        }

        //line after thread id is number of api calls for that thread
        READLINE(line)
        unsigned int apiNum = 0;
        ret = StringUtils::Parse(line, apiNum);

        if (!ret)
        {
            Log(logERROR, "Failed to parse thread number, Unexpected data in input file.\n");
            return false;
        }

        int apiIndex = -1;

        for (std::vector<IParserListener<GLAPIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end(); it++)
        {
            if ((*it) != NULL)
            {
                (*it)->SetAPINum(tid, apiNum);
            }
        }

        //parse #apiNum trace lines and create apiInfo objects
        for (unsigned int i = 0; i < apiNum && !m_shouldStopParsing; i++) {

            apiIndex++;
            READLINE(line)

            ReportProgress(strProgressMessage, i, apiNum);

            // timestamp
            GLAPIInfo* pAPIInfo = new GLAPIInfo;

            if (line.empty())
            {
                continue;
            }

            // ATP API line example:
            //fxn name		start ts			end ts
            //glGenVertexArrays         9232381192968       9232411229944

            // First find the API name (required for the API info object allocation):
            std::stringstream ss(line);
            ss >> pAPIInfo->m_strName;
            CHECK_SS_ERROR(ss)
            ss >> pAPIInfo->m_ullStart;
            CHECK_SS_ERROR(ss)
            ss >> pAPIInfo->m_ullEnd;
            CHECK_SS_ERROR(ss)
		
            pAPIInfo->m_uiSeqID = i;
            pAPIInfo->m_uiDisplaySeqID = apiIndex;

            //send pAPIInfo to summarizer to generate API timeline item
            for (std::vector<IParserListener<GLAPIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end() && !m_shouldStopParsing; it++)
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

    } while (!in.eof());

    return true;

}

void GLAtpFilePart::WriteHeaderSection(SP_fileStream & sout) { SP_UNREFERENCED_PARAMETER(sout); return; }

bool GLAtpFilePart::WriteContentSection(SP_fileStream & sout, const std::string & strTmpFilePath, const std::string & strPID)
{
    SP_UNREFERENCED_PARAMETER(strTmpFilePath);
    SP_UNREFERENCED_PARAMETER(strPID);

    std::string glOutPath(FileUtils::GetTempFragFilePath());
    glOutPath.append("/gltrace" TMP_TIME_STAMP_EXT);

    std::ifstream glDumpFile(glOutPath.c_str());
    SpAssertRet(glDumpFile) false;
    sout << "=====GL Timestamp Output=====\n";
    std::string line;

    while (glDumpFile.good())
    {
        std::getline(glDumpFile, line);
#ifdef _WIN32
        auto wline = std::wstring(line.begin(), line.end());
        sout << wline << std::endl;
#else
        sout << line << std::endl;
#endif

    }

    return true;
}
