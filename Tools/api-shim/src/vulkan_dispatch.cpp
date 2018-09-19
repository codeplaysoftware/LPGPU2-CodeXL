/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include "vulkan_dispatch.h"

#include <unordered_map>
#include <mutex>

#include "logger.h"

namespace
{

  struct LayerData
  {
    VkInstance instance;
    VkLayerInstanceDispatchTable* instance_dispatch_table;
    VkLayerDispatchTable* device_dispatch_table;

    LayerData() : instance(NULL), instance_dispatch_table(NULL), device_dispatch_table(NULL)
    {
    }

    ~LayerData()
    {
      delete instance_dispatch_table;
      delete device_dispatch_table;
    }
  };

  typedef std::unordered_map<void *, LayerData> LayerMap;

  // @todo; this follows the examples but we don't really need a map here, or lock
  // in most instances. The typical case is probably one instance and one
  // device. Consider a list of fixed size arrays with locking only on adding
  // another array to the list.
  std::mutex map_lock;
  LayerMap data_map;

  LayerData* FindLayerData(void *data_key)
  {
    auto got = data_map.find(data_key);

    LayerData* ret = NULL;

    if(got != data_map.end())
    {
      ret = &(got->second);
    }

    return ret;
  }

  LayerData* CreateLayerData(void *data_key)
  {
    data_map[data_key] = LayerData();
    return &data_map[data_key];
  }

  void RemoveLayerData(void *data_key, LayerMap& map)
  {
    map.erase(data_key);
  }

  inline void* GetDispatchKey(const void *object)
  {
    // Its recommended to take a VKInstance/VkDevice and dereference the internal dispatch table as a key.
    // An VkInstance/VkDevice is otherwise an opaque handle.
    // In layer examples VkLayerDispatchTable is used as it is below for devices and instances
    return (void*) * (VkLayerDispatchTable **)object;
  }

}; // anonymous namespace

VkLayerInstanceDispatchTable* GetDispatchTable(VkInstance dispatchable)
{
  void* key = GetDispatchKey(dispatchable);

  std::lock_guard<std::mutex> lock(map_lock);

  if(LayerData* data = FindLayerData(key))
  {
    return data->instance_dispatch_table;
  }

  if(LayerData* data = CreateLayerData(key))
  {
    data->instance_dispatch_table = new VkLayerInstanceDispatchTable;
    data->instance = reinterpret_cast<VkInstance>(dispatchable);
    return data->instance_dispatch_table;
  }

  // otherwise
  Log(LogError, "Could not create instance dispatch table");
  return NULL;
}

VkLayerDispatchTable* GetDispatchTable(VkDevice dispatchable)
{
  void* key = GetDispatchKey(dispatchable);

  std::lock_guard<std::mutex> lock(map_lock);

  if(LayerData* data = FindLayerData(key))
  {
    return data->device_dispatch_table;
  }

  if(LayerData* data = CreateLayerData(key))
  {
    data->device_dispatch_table = new VkLayerDispatchTable;
    return data->device_dispatch_table;
  }

  // otherwise
  Log(LogError, "Could not create device dispatch table");
  return NULL;
}

VkInstance GetInstance(VkPhysicalDevice dispatchable)
{
  void* key = GetDispatchKey(dispatchable);
  std::lock_guard<std::mutex> lock(map_lock);

  if(LayerData* data = FindLayerData(key))
  {
    return data->instance;
  }

  if(LayerData* data = CreateLayerData(key))
  {
    data->instance_dispatch_table = new VkLayerInstanceDispatchTable;
    data->instance = reinterpret_cast<VkInstance>(dispatchable);
    return data->instance;
  }

  // otherwise
  Log(LogError, "Could not create instance dispatch table");
  return NULL;
}

void RemoveDispatchTable(VkInstance dispatchable)
{
  void* key = GetDispatchKey(dispatchable);
  std::lock_guard<std::mutex> lock(map_lock);
  return RemoveLayerData(key, data_map);
}

void RemoveDispatchTable(VkDevice dispatchable)
{
  void* key = GetDispatchKey(dispatchable);
  std::lock_guard<std::mutex> lock(map_lock);
  return RemoveLayerData(key, data_map);
}

VkLayerDispatchTable* GetDispatchTable(VkCommandBuffer dispatchable)
{
  return GetDispatchTable(reinterpret_cast<VkDevice>(dispatchable));
}

VkLayerDispatchTable* GetDispatchTable(VkQueue dispatchable)
{
  return GetDispatchTable(reinterpret_cast<VkDevice>(dispatchable));
}

VkLayerInstanceDispatchTable* GetDispatchTable(VkPhysicalDevice dispatchable)
{
  return GetDispatchTable(reinterpret_cast<VkInstance>(dispatchable));
}

VkLayerInstanceDispatchTable* GetDispatchTable(VkInstance* dispatchable)
{
  return GetDispatchTable(*dispatchable);
}

VkLayerDispatchTable* GetDispatchTable(VkDevice* dispatchable)
{
  return GetDispatchTable(*dispatchable);
}

