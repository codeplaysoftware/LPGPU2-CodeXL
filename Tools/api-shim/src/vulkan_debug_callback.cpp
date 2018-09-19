/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <vulkan/vulkan.h>

#include <malloc.h>
#include <cassert>
#include <string.h>
#include "lpgpu2_api.h"
#include "logger.h"

#define MAX_MESSAGE_LENGTH  400
static VkDebugReportCallbackEXT debugReportCallback;
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
                                   VkDebugReportFlagsEXT msgFlags,
                                   VkDebugReportObjectTypeEXT /*objType*/,
                                   uint64_t /*srcObject*/, size_t /*location*/,
                                   int32_t msgCode, const char * pLayerPrefix,
                                   const char * pMsg, void * /*pUserData*/ )
{
   LPGPU2_ANNOTATION annotation;
   char message[MAX_MESSAGE_LENGTH] = {0};
   annotation.dwType = LPGPU2_ANNOTATION_TYPE_KHR_DEBUG;
   if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
   {
     snprintf(message, MAX_MESSAGE_LENGTH, "[%s] Code %i : %s", pLayerPrefix, msgCode, pMsg );
   }
   else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
   {
     snprintf(message, MAX_MESSAGE_LENGTH,"WARNING: [%s] Code %i : %s", pLayerPrefix, msgCode, pMsg);
   }
   else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
   {
     snprintf(message, MAX_MESSAGE_LENGTH,"PERFORMANCE WARNING: [%s] Code %i : %s", pLayerPrefix, msgCode, pMsg);
   }
   else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
   {
     snprintf(message, MAX_MESSAGE_LENGTH,"INFO: [%s] Code %i : %s", pLayerPrefix, msgCode, pMsg);
   }
   else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
   {
     snprintf(message, MAX_MESSAGE_LENGTH,"DEBUG: [%s] Code %i : %s", pLayerPrefix, msgCode, pMsg);
   }
   annotation.dwString = message;
   lpgpu2_AnnotateCapture(&annotation);
   // Returning false tells the layer not to stop when the event occurs, so
   // they see the same behavior with and without validation layers enabled.
   return VK_FALSE;
}

void EnableVulkanDebugCallback(VkInstance instance)
{
  LPGPU2_ANNOTATION annotation;
  char message[MAX_MESSAGE_LENGTH] = {0};
  annotation.dwType = LPGPU2_ANNOTATION_TYPE_KHR_DEBUG;
  // Get the instance extension count
  uint32_t inst_ext_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &inst_ext_count, nullptr);

  // Enumerate the instance extensions
  VkExtensionProperties* inst_exts =
    (VkExtensionProperties *)malloc(inst_ext_count * sizeof(VkExtensionProperties));

  vkEnumerateInstanceExtensionProperties(nullptr, &inst_ext_count, inst_exts);

  const char * enabled_inst_exts[16] = {};
  uint32_t enabled_inst_ext_count = 0;

  // Make sure the debug report extension is available
  for (uint32_t i = 0; i < inst_ext_count; i++)
  {
    if (strcmp(inst_exts[i].extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
    {
      enabled_inst_exts[enabled_inst_ext_count++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    }
  }

  if (enabled_inst_ext_count == 0)
  {
    snprintf(message, MAX_MESSAGE_LENGTH, "Vulkan Extensions Unsupported");
    annotation.dwString = message;
    lpgpu2_AnnotateCapture(&annotation);
    return;
  }

  // Pass the instance extensions into vkCreateInstance
  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.enabledExtensionCount = enabled_inst_ext_count;
  instance_info.ppEnabledExtensionNames = enabled_inst_exts;

  PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
  PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

  vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)
    vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
  vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
    vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

  assert(vkCreateDebugReportCallbackEXT);
  assert(vkDestroyDebugReportCallbackEXT);

  // Create the debug callback with desired settings
  if (vkCreateDebugReportCallbackEXT)
  {
    VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;
    debugReportCallbackCreateInfo.sType =
      VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    debugReportCallbackCreateInfo.pNext = NULL;
    debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
      VK_DEBUG_REPORT_WARNING_BIT_EXT |
      VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debugReportCallbackCreateInfo.pfnCallback = DebugReportCallback;
    debugReportCallbackCreateInfo.pUserData = NULL;

    vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCreateInfo,
                                   nullptr, &debugReportCallback);
    snprintf(message, MAX_MESSAGE_LENGTH,"vkCreateDebugReportCallbackEXT Set");
    annotation.dwString = message;
    lpgpu2_AnnotateCapture(&annotation);
  }
  else
  {
    snprintf(message, MAX_MESSAGE_LENGTH,"vkCreateDebugReportCallbackEXT Unsupported");
    annotation.dwString = message;
    lpgpu2_AnnotateCapture(&annotation);
  }

}

void DestroyVulkanDebugCallback(VkInstance instance)
{
  PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
  vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
          vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
  if (vkDestroyDebugReportCallbackEXT)
  {
    vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, nullptr);
  }
}
