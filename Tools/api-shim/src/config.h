/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef XML_CONFIG_HPP
#define XML_CONFIG_HPP

#include <string>
#include <map>

#include "tinyxml2.h"
#include "config_field_lookup.h"
#include "logger.h"

class Config
{
public:
  typedef int32_t (*SET_COUNTER_SET_CALLBACK)(uint32_t csid, uint32_t enable_disable);
  typedef int32_t (*SET_COUNTER_CALLBACK)(uint32_t csid, uint32_t id, uint32_t enable_disable);
  ConfigFieldLookup configLut;

  static const char* DATA_DIRECTORY;
  static const char* DCAPI_PATH;
  static const char* GL_PATH;
  static const char* EGL_PATH;
  static const char* GLES2_PATH;
  static const char* GLES3_PATH;
  static const char* CL_PATH;

  bool countersNeeded;
  bool timelineNeeded;
  bool compressionNeeded;
  bool frameCapture;
  bool explicitControl;
  bool explicitTerminator;
  bool dumpCallstack;

  uint64_t gpuTimingMode;
  uint64_t frameTerminators;

  int counterBatchSize;
  int timelineBatchSize;

  std::string processName;
  std::string apiNames;

  int startFrame;
  int stopFrame;
  int startDraw;
  int stopDraw;

  Config();
  void Reset(void);
  bool Parsed(const std::string& xml_filename);

  // Only query the log level without loading other settings
  std::string QueryLogLevel();

  // Read settings
  void Read();

  void WritePid();
  pid_t ReadPid();

  pid_t ReadPidFile(const std::string& filename);

  void WriteTargetCharacteristics(void);
  void WriteClientTargetCharacteristics(XMLDocument& doc);
  std::string DeviceInfoXml(const std::string& additional_xml);

  void SetOverridingFunctions(void *pointerArray[], size_t size);

private:
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement* collection;

  std::string GetPidFilename();
};

#endif // header
