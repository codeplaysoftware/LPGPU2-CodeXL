/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef CONFIG_FIELD_LOOKUP_H
#define CONFIG_FIELD_LOOKUP_H

#include <map>
#include <string>
#include <stdlib.h> // strtol

#include "tinyxml2.h"

template <typename T>
class Optional
{
public:
  typedef void ( Optional::*bool_type )() const;

  Optional(): ok(false), value(T()) {}
  Optional(T val): ok(true), value(val) {}
  Optional(bool o, T val): ok(o), value(val) {}

  void Set(bool o, T val) {  ok = o, value = val; }

  T& Get() { return value; }

  T& operator*() { return value; }

  // safe bool idiom
  operator bool_type() const {
    return ok == true ? &Optional::this_type_does_not_support_comparisons : 0;
  }

private:
  bool ok;
  T value;
  void this_type_does_not_support_comparisons() const {}
};

typedef Optional<int> OptionalInt;
typedef Optional<std::string> OptionalString;

// Lookup for "Config name=x value=x" in CollectionDefinition
class ConfigFieldLookup {
public:
  void Read(tinyxml2::XMLElement* root)
  {
    if(root)
    {
      tinyxml2::XMLElement* config = root->FirstChildElement( "Config" );
      while(config)
      {
        if(const char*s = config->Attribute("Name"))
        {
          lut[std::string(s)] = config;
        }
        config = config->NextSiblingElement("Config");
      }
    }
  }

  tinyxml2::XMLElement* Find(const std::string& configName) const
  {
    tinyxml2::XMLElement* ret = NULL;
    ConfigLutIter iter = lut.find(configName);
    if(iter != lut.end())
    {
      ret = iter->second;
    }
    return ret;
  }

  OptionalInt FindInt(const std::string& configName) const
  {
    OptionalInt ret(false, 0);

    if(const char* s = FindValue(configName))
    {
      char *end;
      int n = strtol(s, &end, 0);
      if(end != s)
      {
        ret = OptionalInt(n);
      }

    }
    return ret;
  }

  OptionalString FindString(const std::string& configName) const
  {
    OptionalString ret;

    if(const char* s = FindValue(configName))
    {
      ret = OptionalString(s);
    }
    return ret;
  }


private:
  typedef std::map<const std::string, tinyxml2::XMLElement*> ConfigLut;
  typedef ConfigLut::const_iterator ConfigLutIter;

  ConfigLut lut;

  const char* FindValue(const std::string& configName) const
  {
    const char* ret = NULL;
    if(tinyxml2::XMLElement* elem = Find(configName))
    {
      if(const char*s = elem->Attribute("Value"))
      {
        ret = s;
      }
    }
    return ret;
  }


};


#endif // header
