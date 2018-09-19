/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <stdio.h>
#include <stdlib.h> // strtol
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "config.h"
#include "logger.h"
#include "string_bitfield.h"
#include "filesystem.h"
#include "gpu_timing.h"

using namespace tinyxml2;

extern int CommandCount;
extern char const * CommandNames[];

extern char **environ;

const char* Config::DATA_DIRECTORY = "DataDirectory";
const char* Config::DCAPI_PATH = "DcApiPath";
const char* Config::GL_PATH = "GlPath";
const char* Config::EGL_PATH = "EglPath";
const char* Config::GLES2_PATH = "Gles2Path";
const char* Config::GLES3_PATH = "Gles3Path";
const char* Config::CL_PATH = "ClPath";

extern const char* BUILD_DATE;

namespace {

  class HexString {
  public:
    HexString() {}

    char* str(uint32_t id)
    {
      snprintf(_str, 11, "0x%X", id);
      return _str;
    }

  private:
    char _str[11];  // '0x34567890_'
  };

  bool IsWriteablePath(const char*path, std::string& error)
  {
    bool ret = false;
    std::string fn = PathJoin(path, std::string("test.write.file"));
    FILE* file = fopen(fn.c_str() , "w");
    if(file)
    {
      fclose(file);
      remove(fn.c_str());
      ret = true;
      error = "";
    }
    else
    {
      error = std::string(strerror(errno));
      Log(LogError, "Could not open for writeable test:%s %d %s\n", fn.c_str(), errno, strerror(errno));
    }
    return ret;
  }

  FILE* FirstRegularFile(const char*path, bool isLibrary, std::string& full_path, std::string& error)
  {
    FILE* ret = NULL;
    DIR *dirp = NULL;
    struct dirent *direntp = NULL;
    struct stat stat_buf;

    error = "";

    if ((dirp = opendir(path)) == NULL)
    {
      error = std::string(strerror(errno));
    }
    else
    {
      while((direntp = readdir(dirp)) != NULL)
      {
        std::string name(direntp->d_name);
        full_path = PathJoin(path, name);

        if(isLibrary)
        {
          if(full_path.size() > 3)
          {
            if( !(name[0] == 'l' &&
                  name[1] == 'i' &&
                  name[2] == 'b' &&
                  name[name.size()-1] == 'o' &&
                  name[name.size()-2] == 's' &&
                  name[name.size()-3] == '.'))
            {
              continue;
            }
          }
        }

        if(stat(full_path.c_str(), &stat_buf)==-1)
        {
          error = std::string(strerror(errno));
        }
        else
        {
          if(!S_ISDIR(stat_buf.st_mode)) // @todo a dir isnt a regular file... or is it?
          {
            if(S_ISREG(stat_buf.st_mode))
            {
              ret = fopen(full_path.c_str() , "r");
              break; // OUT after first regular file;
            }
          }
        }
      }
    }

    if(NULL == ret)
    {
      full_path = "";
      error = "Cannot find file to open";
    }

    return ret;
  }

  void WriteXmlCheckPath(XMLDocument& doc, XMLElement& appendTo, const char*path)
  {
    std::string error;
    std::string full_path;

    if(XMLElement* m = doc.NewElement("Path"))
    {
      appendTo.InsertEndChild( m );

      m->SetAttribute("Name", path);

      bool writeable = IsWriteablePath(path, error);
      m->SetAttribute("Writeable", writeable);
      m->SetAttribute("WriteError", error.c_str());

      FILE* file = FirstRegularFile(path, false, full_path, error);
      bool readable = file != NULL;
      if(file)
      {
        fclose(file);
        m->SetAttribute("ReadFile", full_path.c_str());
      }
      m->SetAttribute("Readable", readable);
      m->SetAttribute("ReadError", error.c_str());

      file = FirstRegularFile(path, true, full_path, error);
      if(file)
      {
        fclose(file);
        m->SetAttribute("Library", full_path.c_str());
      }
      m->SetAttribute("LibraryOpenError", error.c_str());

    }
  }

  void WriteXmlEnvironment(XMLDocument& doc, XMLElement& appendTo)
  {
    int count = 0;

    while(environ[count] != NULL)
    {
      if(XMLElement* m = doc.NewElement("Env"))
      {
        appendTo.InsertEndChild( m );
        m->SetAttribute("Name", environ[count]);
        m->SetAttribute("Value", environ[count]); // @todo this was getenv(environ[count]) on android and wasn't failing??
        count++;
      }
    }
  }

  std::string ReadAll(FILE* file)
  {
    std::string ret;
    char buffer[256];
    if(file)
    {
      while(!(feof(file) || ferror(file)))
      {
        if(char *pStr = fgets(buffer, sizeof(buffer), file))
        {
          ret += pStr;
        }
      }
    }
    return ret;
  }

  std::string ReadProperty(const std::string& name)
  {
    std::string ret;
    std::string command = "getprop " + name;
    if(FILE* file = popen(command.c_str(), "r"))
    {
      ret = ReadAll(file);
      pclose(file);
    }
    return ret;
  }


  void WriteXmlAndroidProperty(XMLDocument& doc, XMLElement& appendTo, const std::string& xml_tag, const std::string& property)
  {
    if(XMLElement* elem = doc.NewElement("Property"))
    {
      appendTo.InsertEndChild( elem );

      elem->SetAttribute("Name", xml_tag.c_str());
      elem->SetAttribute("Value", ReadProperty(property).c_str());
    }
  }

  void WriteXmlMachine(XMLDocument& doc, XMLElement& appendTo)
  {
    std::string name("<unknown>");
#ifdef __ANDROID__
    name = ReadProperty("ro.product.model");
#endif // __ANDROID__

    if(XMLElement* m = doc.NewElement("Machine"))
    {
      appendTo.InsertEndChild( m );
      m->SetAttribute("Name", name.c_str());
      m->SetAttribute("Int", (int)(sizeof(unsigned int)));
      m->SetAttribute("Long", (int)(sizeof(long)));
      int endian = 1;
      m->SetAttribute("Endian", (int) (((char *)&endian)[0]));

      WriteXmlCheckPath(doc, *m, "/data/local/tmp/");
      WriteXmlCheckPath(doc, *m, "/tmp/");
      WriteXmlCheckPath(doc, *m, "/sdcard/");
      WriteXmlCheckPath(doc, *m, "/mnt/sdcard/");
      WriteXmlCheckPath(doc, *m, "/system/");
      WriteXmlCheckPath(doc, *m, "/system/_lpgpu2");
      WriteXmlCheckPath(doc, *m, "/system/lib/");
      WriteXmlCheckPath(doc, *m, "/vendor/lib");
      WriteXmlCheckPath(doc, *m, "/system/vendor/lib/");

#ifdef __ANDROID__
      // all props
      if(XMLElement* elem = doc.NewElement("Properties"))
      {
        m->InsertEndChild( elem );
        elem->SetText(ReadProperty("").c_str());
      }

      // pulled out props
      WriteXmlAndroidProperty(doc, *m, "ro.product.model",  "ro.product.model");
      WriteXmlAndroidProperty(doc, *m, "ro.product.cpu.abi", "ro.product.cpu.abi");
      WriteXmlAndroidProperty(doc, *m, "ro.product.device", "ro.product.device");
      WriteXmlAndroidProperty(doc, *m, "ro.product.name", "ro.product.name");

#endif // __ANDROID__
    }
  }


  void WriteXmlProcess(XMLDocument& doc, XMLElement& appendTo)
  {
    if(XMLElement* m = doc.NewElement("Process"))
    {
      appendTo.InsertEndChild( m );
      m->SetAttribute("pid", (int)getpid());
      WriteXmlEnvironment(doc, *m);

      if(XMLElement* elem = doc.NewElement("CommandLine"))
      {
        m->InsertEndChild( elem );

        elem->SetAttribute("Name", "/proc/self/cmdline");

        if(FILE *file = fopen("/proc/self/cmdline", "rb"))
        {
          std::string data = ReadAll(file);
          fclose(file);
          elem->SetText(data.c_str());
        }
      }
    }
  }

  void WriteXmlTargetCharacteristics(XMLDocument& doc)
  {
    if(XMLElement* target = doc.NewElement("Target"))
    {
      target->SetAttribute("Build", BUILD_DATE);

      // always write machine info
      WriteXmlMachine(doc, *target);

      // always write process info
      WriteXmlProcess(doc, *target);

      WriteXmlCommandParameters(doc, *target);

      doc.InsertEndChild(target);
    }
  }

  void WriteXmlTargetCharacteristics(const std::string& filename)
  {
    XMLDocument doc;

    if(XMLDeclaration* decl = doc.NewDeclaration())
    {
      doc.InsertEndChild(decl);
    }

    WriteXmlTargetCharacteristics(doc);

    std::string fn( filename + ".tmp" );
    if(doc.SaveFile(fn.c_str()))
    {
      Log(LogError,"Error: Cannot save %s\n", fn.c_str());
    }

    // use rename so that when 'filename' appears it is complete.
    rename(fn.c_str(), filename.c_str());
  }


  XMLNode *XMLDeepCopy( XMLNode *src, XMLDocument* allocatorDoc )
  {
    XMLNode *current = src->ShallowClone( allocatorDoc );
    for( XMLNode *child=src->FirstChild(); child; child=child->NextSibling() )
    {
      current->InsertEndChild( XMLDeepCopy( child, allocatorDoc ) );
    }

    return current;
  }

} // anon namespace


Config::Config()
{
  Reset();
}

void Config::Reset()
{
  countersNeeded = false;
  timelineNeeded = false;
  compressionNeeded = false;
  frameCapture = false;
  explicitControl = false;
  explicitTerminator = false;
  dumpCallstack = false;
  gpuTimingMode = 0;
  frameTerminators = 0;
  counterBatchSize = 0;
  timelineBatchSize = 0;
  startFrame = -1;
  stopFrame = -1;
  startDraw = -1;
  stopDraw = -1;
  collection = NULL;
}

bool Config::Parsed(const std::string& filename)
{
  Reset();

  XMLError e = doc.LoadFile(filename.c_str());

  if(XML_SUCCESS == e)
  {
    collection = doc.FirstChildElement( "Collection" );

    // log level isnt set yet so can only LogPrint
    if(collection)
    {
      LogPrint("Loaded: %s\n", filename.c_str());
    }
    else
    {
      LogPrint("Error: Collection element not found in configuration (%d:'%s')\n", e, filename.c_str());
    }
  }
  else
  {
    LogPrint("Error(%d:%s): Could not parse configuration file '%s'\n", e, doc.ErrorName(), filename.c_str());
  }

  return (XML_SUCCESS == e);
}

std::string Config::QueryLogLevel()
{
  static const std::string LogLevelName("LogLevel");

  std::string ret;

  if(!collection)
  {
    Log(LogInfo,"Error: CollectionDefinition.xml is not loaded\n");
  }
  else
  {
    tinyxml2::XMLElement* config = collection->FirstChildElement( "Config" );
    while(config)
    {
      if(const char*s = config->Attribute("Name"))
      {
        if(const char*v = config->Attribute("Value"))
        {
          if(std::string(s) == LogLevelName)
          {
            ret =  std::string(v);
          }
        }
      }
      config = config->NextSiblingElement("Config");
    }
  }

  return ret;
}

void Config::Read()
{
  if(!collection)
  {
    Log(LogError, "Error: CollectionDefinition.xml is not loaded\n");
  }
  else
  {
    configLut.Read(collection);

    if(OptionalInt max = configLut.FindInt("CounterBatchSize"))
    {
      counterBatchSize = max.Get();
    }

    if(OptionalInt max = configLut.FindInt("TimelineBatchSize"))
    {
      timelineBatchSize = max.Get();
    }

    XMLElement* timeline = collection->FirstChildElement( "Timeline" );
    if(timeline)
    {
      int enable;
      if( XML_SUCCESS == timeline->QueryIntAttribute( "Enable", &enable) )
      {
        if(enable)
        {
          timelineNeeded = true;
        }
      }
    }

    XMLElement* compressed = collection->FirstChildElement( "Compressed" );
    if(compressed)
    {
      int enable;
      if( XML_SUCCESS == compressed->QueryIntAttribute( "Enable", &enable) )
      {
        if(enable)
        {
          compressionNeeded = true;
        }
      }
    }

    XMLElement* frame = collection->FirstChildElement( "FrameCapture" );
    if(frame)
    {
      int enable;
      if( XML_SUCCESS == frame->QueryIntAttribute( "Enable", &enable) )
      {
        if(enable)
        {
          frameCapture = true;
        }
      }
    }

    XMLElement* explct = collection->FirstChildElement( "ExplicitControl" );
    if(explct)
    {
      int enable;
      if( XML_SUCCESS == explct->QueryIntAttribute( "Enable", &enable) )
      {
        if(enable)
        {
          explicitControl = true;
        }
      }
    }

    XMLElement* explctT = collection->FirstChildElement( "ExplicitFrameTerminator" );
    if(explctT)
    {
      int enable;
      if( XML_SUCCESS == explctT->QueryIntAttribute( "Enable", &enable) )
      {
        if(enable)
        {
          explicitTerminator = true;
        }
      }
    }

    XMLElement* dumpcs = collection->FirstChildElement( "DumpCallstack" );
    if(dumpcs)
    {
      int enable;
      if( XML_SUCCESS == dumpcs->QueryIntAttribute( "Enable", &enable) )
      {
        if(enable)
        {
          dumpCallstack = true;
        }
      }
    }

    XMLElement* gputim = collection->FirstChildElement( "GpuTimingMode" );
    if(gputim)
    {
      // QueryIntAttribute doesnt seem to handle hex
      const char* s = gputim->Attribute( "Mode" );
      if(NULL == s)
      {
        Log(LogError,"Error: GpuTimingMode has no Mode. \n");
      }
      else
      {
        if ( strcmp(s, GPU_TIMING_DISJOINT_FRAME_STRING) == 0 )
        {
          gpuTimingMode = GPU_TIMING_MODE_FRAME_DISJOINT;
        }
        else if ( strcmp(s, GPU_TIMING_DRAW_CALL_STRING) == 0 )
        {
          gpuTimingMode = GPU_TIMING_MODE_DRAW;
        }
        else
        {
          Log(LogError, "Error unknown GpuTimingMode: %s \n", s);
        }
      }
    }

    XMLElement* term = collection->FirstChildElement( "FrameTerminators" );
    if(term)
    {
      // QueryIntAttribute doesnt seem to handle hex
      const char* s = term->Attribute( "Flags" );
      if(NULL == s)
      {
        Log(LogError,"Error: FrameTerminators has no Flags. \n");
      }
      else
      {
        frameTerminators = strtol(s, NULL, 0);
      }
    }

    if(XMLElement* elem = collection->FirstChildElement( "Process" ))
    {
      if(const char*v = elem->Attribute("Name"))
      {
        processName = std::string(v);
      }
    }

    if(XMLElement* elem = collection->FirstChildElement( "API" ))
    {
      if(const char*v = elem->Attribute("Name"))
      {
        apiNames = std::string(v);
      }
    }

    XMLElement* crange = collection->FirstChildElement( "CollectionRange" );
    if(crange)
    {
      if( const char*v = crange->Attribute("StartFrame" ) )
      {
        startFrame = atoi(v);
      }
      else {
        startFrame = -1;
      }

      if( const char*v = crange->Attribute("StopFrame" ) )
      {
        stopFrame = atoi(v);
      }
      else {
        stopFrame = -1;
      }

      if( const char*v = crange->Attribute( "StartDraw" ) )
      {
        startDraw = atoi(v);
      }
      else {
        startDraw = -1;
      }

      if( const char*v = crange->Attribute( "StopDraw" ) )
      {
        stopDraw = atoi(v);
      }
      else {
        stopDraw = -1;
      }

    }
  }
}

std::string Config::GetPidFilename()
{
  std::string filename("lpgpu2.pid");

  if(Optional<std::string> directory = configLut.FindString(Config::DATA_DIRECTORY))
  {
    filename = PathJoin(*directory, filename);
  }

  if(OptionalString s = configLut.FindString("PidFileName"))
  {
    filename = *s;
  }

  return filename;
}

void Config::WritePid()
{
  std::string filename(GetPidFilename());


  if(filename.size())
  {
    pid_t pid = getpid();
    Log(LogInfo, "Trying to write: %s < %d\n", filename.c_str(), pid);

    FILE *file = fopen(filename.c_str(), "w");
    size_t nbytes = 0;
    if(file)
    {
      char buffer[20];
      int length = snprintf(buffer, sizeof(buffer), "%d", pid);
      nbytes = fwrite(buffer, 1, length, file);
      fclose(file);
    }

    if(0 == nbytes)
    {
      Log(LogError, "Error: Cannot write pid to file %s\n", filename.c_str());
    }
    else
    {
      Log(LogInfo, "         Wrote PID: %s:%d\n", filename.c_str(), pid);
    }
  }
}

void Config::WriteTargetCharacteristics(void)
{
  std::string targetXMLFilename("TargetCharacteristics.xml");

  if(XMLElement* elem = configLut.Find("TargetCharacteristicsFilename"))
  {
    if(const char*s = elem->Attribute("value"))
    {
      targetXMLFilename = std::string(s);
    }
  }

  if(OptionalString directory = configLut.FindString(Config::DATA_DIRECTORY))
  {
    targetXMLFilename = PathJoin(*directory, targetXMLFilename);
  }
  else
  {
    Log(LogError, "No data directory set for target characteristics?");
  }

  if(targetXMLFilename.size())
  {
    LogPrint("Trying to write: '%s'", targetXMLFilename.c_str());
  }
  else
  {
    Log(LogError, "Trying to write target XML with empty filename?");
  }

  WriteXmlTargetCharacteristics(targetXMLFilename);
}

std::string Config::DeviceInfoXml(const std::string& additional_xml)
{
  XMLDocument doc;
  if(XMLDeclaration* decl = doc.NewDeclaration())
  {
    doc.InsertEndChild(decl);
  }

  WriteXmlTargetCharacteristics(doc);

  XMLDocument additional_doc;

  if(additional_xml.size())
  {
    if(XML_SUCCESS == additional_doc.Parse(additional_xml.c_str()))
    {
      if(XMLElement* android = additional_doc.FirstChildElement("Android"))
      {
        if(XMLElement* directories = android->FirstChildElement("Directories"))
        {
          XMLElement* directory = directories->FirstChildElement("Directory");
          while(directory)
          {
            if(const char* dir = directory->Attribute("Value"))
            {
              std::string error;
              directory->SetAttribute("Writeable", IsWriteablePath(dir, error));
              directory->SetAttribute("Error", error.c_str());
            }
            directory = directory->NextSiblingElement();
          }
        }

        if(XMLNode* e = XMLDeepCopy(android, &doc))
        {
          doc.RootElement()->InsertEndChild(e);
        }
      }
    }
    else
    {
      Log(LogError,"Error: Cannot parse additional xml %s\n", additional_xml.c_str());
    }
  }
  else
  {
    Log(LogError,"Error: Cannot parse additional xml which is empty?\n");
  }

  XMLPrinter printer;

  doc.Print( &printer );

  return std::string(printer.CStr());
}

pid_t Config::ReadPid()
{
  std::string filename(GetPidFilename());
  return ReadPidFile(filename);
}

pid_t Config::ReadPidFile(const std::string& filename)
{
  pid_t pid = 0;

  if(FILE *file = fopen(filename.c_str(), "r"))
  {
    char buffer[20];
    if(size_t size = fread(buffer,  1, sizeof(buffer)-1, file))
    {
      buffer[size] = 0;
      pid = static_cast<pid_t>(atoi(&buffer[0]));
    }
    else
    {
      Log(LogInfo, "Read PID failed. Read zero bytes from '%s'", filename.c_str());
    }
  }
  else
  {
    Log(LogInfo, "Read PID failed. Missing file '%s'", filename.c_str());
  }

  return pid;
}

void Config::WriteClientTargetCharacteristics(XMLDocument& doc)
{
  if(XMLElement* target = doc.NewElement("Target"))
  {
    target->SetAttribute("Build", BUILD_DATE);

    // always write machine info
    WriteXmlMachine(doc, *target);

    // always write process info
    WriteXmlProcess(doc, *target);

    WriteXmlCommandParameters(doc, *target);

    doc.InsertEndChild(target);
  }
}

void Config::SetOverridingFunctions(void *pointerArray[], size_t size)
{
  if( collection )
  {
    XMLElement* names = collection->FirstChildElement( "CommandNames" );
    if(!names)
    {
      Log(LogInfo, "CollectionDefinition has no Command override block\n");
    }
    else
    {
      // "shim" is the default, "passthrough" sets all to the passthrough funcs
      const std::string passthrough("passthrough");
      const std::string parameters("recordParameters");
      const char* s = names->Attribute("DefaultOverride");
      if(s && (std::string(s) == passthrough))
      {
        Log(LogInfo, "Default command override for all: %s\n", s);
        for(size_t i =0; i < static_cast<size_t>(CommandCount); i++)
        {
          std::string funcName(passthrough);
          funcName += "_";
          funcName += std::string(CommandNames[i]);
          if(void* ptr = dlsym(RTLD_DEFAULT, funcName.c_str()))
          {
            if(i >= size)
            {
              Log(LogError, "Command override ID > Number of known commmands? %d > %d\n", i, size);
            }
            else
            {
              pointerArray[i] = ptr;
            }
          }
          else
          {
            Log(LogError, "Could not override %s with %s (%d)\n", CommandNames[i], funcName.c_str(), i);
          }
        }
      }
      else if(s && (std::string(s) == parameters))
      {
        Log(LogInfo, "Default command override for all: %s\n", s);
        for(size_t i =0; i < static_cast<size_t>(CommandCount); i++)
        {
          std::string funcName(parameters);
          funcName += "_";
          funcName += std::string(CommandNames[i]);
          if(void* ptr = dlsym(RTLD_DEFAULT, funcName.c_str()))
          {
            if(i >= size)
            {
              Log(LogError, "Command override ID > Number of known commmands? %d > %d\n", i, size);
            }
            else
            {
              pointerArray[i] = ptr;
            }
          }
          else
          {
            Log(LogError, "Could not override %s with %s (%d)\n", CommandNames[i], funcName.c_str(), i);
          }
        }
      }
      else
      {
        Log(LogInfo, "No default function override set '%s'", names->Value());
      }

      // commands can be individually overriden
      XMLElement* command = names->FirstChildElement( "Command" );
      while(command)
      {
        int id;
        do {
          // QueryIntAttribute doesnt seem to handle hex
          const char* s = command->Attribute( "Id" );
          if(NULL == s)
          {
            Log(LogError,"A command override has no Id?\n");
            break;
          }

          char *end;
          id = strtol(s, &end, 0);
          if(end == s)
          {
            Log(LogError,"Command override Id was not a number? '%s'\n", s);
            break;
          }

          if(id >= static_cast<int>(size))
          {
            Log(LogError, "Command override ID > Number of known commmands? %d > %d\n", id, size);
            break;
          }

          s = command->Attribute( "Name" );
          if(NULL == s)
          {
            Log(LogError,"A command override has no Name?\n");
            break;
          }

          if(0 != strcmp(s, CommandNames[id]))
          {
            Log(LogError,"A command override ID does not match (Different interposer build?)\n");
            break;
          }

          s = command->Attribute( "Override" );
          if(NULL != s)
          {
            if(void* ptr = dlsym(RTLD_DEFAULT, s))
            {
              pointerArray[id] = ptr;
              Log(LogVerbose, "Overrode %s with %s (%d)\n", CommandNames[id], s, id);
            }
            else
            {
              Log(LogError, "Could not override %s with %s (%d)\n", CommandNames[id], s, id);
            }
          }

          break;
        } while(1);

        command = command->NextSiblingElement();
      } // while(command)

    } // if(names)

  } // if(collection)

} // SetOverridingFunctions(..)
