/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <vulkan/vulkan.h>

#include <string.h> // memcpy
#include <string.h> // strcmp

#include "vulkan_dispatch.h"
#include "export.h"
#include "logger.h"

#define VK_LAYER_API_VERSION VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION)

namespace {

  VkResult GetLayerProperties(const uint32_t count, const VkLayerProperties *layer_properties, uint32_t *pCount,
                              VkLayerProperties *pProperties)
  {
    uint32_t copy_size;

    if (pProperties == NULL || layer_properties == NULL)
    {
      *pCount = count;
      return VK_SUCCESS;
    }

    copy_size = *pCount < count ? *pCount : count;
    memcpy(pProperties, layer_properties, copy_size * sizeof(VkLayerProperties));
    *pCount = copy_size;
    if (copy_size < count)
    {
      return VK_INCOMPLETE;
    }

    return VK_SUCCESS;
  }

  // VkResult GetExtensionProperties(const uint32_t count, const VkExtensionProperties *layer_extensions, uint32_t *pCount,
  //                                 VkExtensionProperties *pProperties)
  // {
  //   uint32_t copy_size;

  //   if (pProperties == NULL || layer_extensions == NULL)
  //   {
  //     *pCount = count;
  //     return VK_SUCCESS;
  //   }

  //   copy_size = *pCount < count ? *pCount : count;
  //   memcpy(pProperties, layer_extensions, copy_size * sizeof(VkExtensionProperties));
  //   *pCount = copy_size;
  //   if (copy_size < count)
  //   {
  //     return VK_INCOMPLETE;
  //   }

  //   return VK_SUCCESS;
  // }

  const VkLayerProperties LayerProperties =
    {
      "VK_LAYER_LPGPU2_interposer", VK_LAYER_API_VERSION, 1, "LPGPU2 Interposer Layer",
    };

  // const VkExtensionProperties InstanceExtensions[] = {{VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION}};

}; // anon namespace


extern "C" {

VKAPI_ATTR VkResult DLL_EXPORT
vkEnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties)
{
  return GetLayerProperties(1, &LayerProperties, pCount, pProperties);
}

VKAPI_ATTR VkResult DLL_EXPORT
vkEnumerateDeviceLayerProperties(VkPhysicalDevice /*physicalDevice*/, uint32_t *pCount, VkLayerProperties *pProperties)
{
  return GetLayerProperties(1, &LayerProperties, pCount, pProperties);
}

VKAPI_ATTR VkResult DLL_EXPORT
vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount, VkExtensionProperties * /*pProperties*/)
{
  if(pLayerName && (0 == strcmp(pLayerName, LayerProperties.layerName)))
  {
    /* we dont have any instance extensions */
    *pCount = 0;
    return VK_SUCCESS;
    // return GetExtensionProperties(1, InstanceExtensions, pCount, pProperties);
  }

  return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR VkResult DLL_EXPORT
vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                   const char *pLayerName, uint32_t *pCount,
                                   VkExtensionProperties *pProperties)
{
  if(pLayerName && (0 == strcmp(pLayerName, LayerProperties.layerName)))
  {
    /* we dont have any physical device extensions */
    *pCount = 0;
    return VK_SUCCESS;
  }

  // assert(physicalDevice);
  return GetDispatchTable(physicalDevice)
    ->EnumerateDeviceExtensionProperties(physicalDevice, NULL, pCount, pProperties);
}

}; // extern C
