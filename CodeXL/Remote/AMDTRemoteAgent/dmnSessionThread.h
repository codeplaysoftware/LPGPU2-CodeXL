//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnSessionThread.h
///
//==================================================================================

#ifndef __dmnSessionThread_h
#define __dmnSessionThread_h

#include <functional>
#include <memory>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtSet.h>


// Local:
#include <AMDTRemoteAgent/Public Include/dmnDefinitions.h>
#include <AMDTRemoteAgent/dmnUtils.h>
//++AT:LPGPU2
#include <AMDTRemoteAgent/dmnLPGPU2ProjectBuildDefinitions.h>
#if (LPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN == LPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN_YES)
#include <AMDTRemoteAgent/dmnPowerBackendAdapter.h>
#endif
//--AT:LPGPU2

//++KB: LPGPU2
struct ChunkHeader {
	uint32_t magicNumber;
	uint32_t uuid;
	uint32_t size;
};

struct TimelineRecord {
	uint64_t id;
	uint64_t startTime;
	uint64_t endTime;
	uint64_t frame;
	uint64_t draw;
	uint64_t apiID;
	uint64_t poffset;
	uint64_t plength;
};

struct FB_Record {
	uint64_t frame_no;
	uint64_t draw_no;
	uint64_t x_res;
	uint64_t y_res;
	uint64_t bpp;
	uint64_t fb_size;
	uint64_t type;
	uint64_t format;
};

struct AnnotateRecord {
	uint64_t frame_no;
	uint64_t draw_no;
	uint64_t type;
	uint64_t cpu_time;
	uint64_t strlen;
	//uchar[0..strlen];
};

struct CallstackTrace {
	void** current;
	void** end;
};

struct CallstackHeader {
	uint64_t apiID;
	uint64_t id;
	uint64_t frame_no;
	uint64_t draw_no;
	uint64_t startTime;
	uint64_t records;
};

struct CallstackRecord {
	uint64_t dli_fbase;      // Base address at which shared object is loaded
	uint64_t dli_saddr;      // Exact address of symbol named in dli_sname
	uint64_t dli_fname_len;  // Length of Pathname of shared object that contains address
	uint64_t dli_sname_len;  // Length of Name of symbol whose definition overlaps addr
};
//--KB: LPGPU2

// Represents a thread that is being spawned by
// the remote agent to handle a connection with a single client.
class dmnSessionThread :
    public osThread
{
public:
    dmnSessionThread(osTCPSocketServerConnectionHandler* pConnHandler,
                     const gtString& threadName, bool syncTermination = false);
    ~dmnSessionThread(void);

    // Prints a message to the console about
    // the disconnection of the most recent client.
    void notifyUserAboutDisconnection();

    // Terminates the running session.
    void terminateSession();
    static void KillDependantProcesses();

protected:
    // The threads' entry point.
    virtual int entryPoint() override;

private:
    void CleanupProcessLeftOvers(const REMOTE_OPERATION_MODE mode) const;
    void LaunchProfiler(bool& isTerminationRequired);
    bool LaunchGraphicsBeckendServer();

    unsigned short GetGraphicServerPortFromArgs(const gtString& fixedCmdLineArgs) const;

    bool GetCapturedFrames();
    bool GetCapturedFramesByTime();
    bool GetCapturedFrameData();
    bool DeleteFrameAnalysisSession();
    bool GetRemoteFile();
    void TerminateWholeSession(bool& isTerminationRequired, int& ret);
    bool LaunchRds();

    /// Write the data for the requested frame after extracting it from the server file system
    /// \param frameIndex the frame index
    /// \param projectName the project name
    /// \param sessionName the session name
    void WriteFrameFilesData(int frameIndex, const gtString& projectName, const gtString& sessionName);

    /// Sends a frame analysis file content to the CodeXL client
    /// \param filePath the local file path
    /// \return true iff the file data was sent successfully
    bool SendFrameAnalysisFileData(const osFilePath& filePath);

    bool ReadFile(const osFilePath& filePath, const bool isBinary, gtVector<gtByte>& pBuffer, gtUInt32& fileSize) const;

    typedef std::function<bool(osFilePath)> FilePathFilter;
    bool CreateCapturedFramesInfoFile(const gtString& projectName, const  FilePathFilter& sessionFilterFunc, const FilePathFilter& frameFilterFunc);
    void BuildFrameCaptureInfoNode(const gtList<osFilePath>& framesFiles, TiXmlElement* frameElement) const;
    bool GetCurrentUserFrameAnalysisFolder(osFilePath& buffer) const;
    void AddFADependantProcessToTerminationList(const osFilePath& serverPath, const gtString& arguments) const;


    bool GetDaemonCXLVersion();
    void GetDaemonPlatform();
    bool TerminateGraphicsBeckendServerSession();
    bool TerminateProfilingSession();
    bool TerminateDebuggingSession();

    /// General utilities

    /// Check if a requested process is running
    bool IsProcessRunning();

    /// Kill a running process by it's name
    bool KillRunningProcess();
    
    /// Returns true if HSA enabled 
    bool IsHSAEnabled();

    bool IsSYCLEnabled();

    bool ValidateAppPaths();

private:
    osTCPSocketServerConnectionHandler* m_pConnHandler;
    osProcessId m_rdsProcId;
    osProcessId m_sProfProcId;
    osProcessId m_sGraphicsProcId;
    osProcessId m_systemInfoProcId; // !< The process id of the system information process.

//++AT:LPGPU2
#if (LPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN == LPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN_YES)
    //++TLRS: LPGPU2: (NOTE CF:LPGPU2 this might need moved above the #if)
    //--TLRS: LPGPU2:
    dmnPowerBackendAdapter m_powerBackendAdapter;
#endif
//--AT:LPGPU2

    bool m_isForcedTerminationRequired;

    // No copy.
    dmnSessionThread(const dmnSessionThread& other);
    const dmnSessionThread& operator=(const dmnSessionThread& other);

    bool CreateProcess(REMOTE_OPERATION_MODE mode, const gtString& cmdLineArgs, std::vector<osEnvironmentVariable>& envVars, const osFilePath& filePath = osFilePath(), const osFilePath& dirPath = osFilePath());

    /// Check if a server process exists, and kill it
    /// \param serverPath the server full path
    static void KillServerExistingProcess(const osFilePath& serverPath);

    bool terminateProcess(REMOTE_OPERATION_MODE mode);
    void releaseResources();
    /// A map containingg the list of supported extensions for frame analysis data files,
    /// with mapping to a boolean - true iff the file extension contain binary data
    static gtMap<gtString, bool> m_sFrameAnalysisFileExtensionToBinaryfileTypeMap;

    static gtSet<gtString> m_ProcessNamesTerminationSet;
};


#endif // __dmnSessionThread_h
