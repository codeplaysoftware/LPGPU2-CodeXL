#
# The MIT License
#
# Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
# For conditions of distribution and use, see the accompanying COPYING file.
#
#
export ANDROID_SDK_PATH=~/Android/Sdk
export ANDROID_NDK_PATH=~/Android/Sdk/ndk-bundle
export PATH=$ANDROID_SDK_PATH/platform-tools:$PATH
export PATH=$ANDROID_NDK_PATH:$PATH
export ANDROID_HOME=$ANDROID_SDK_PATH
export ANDROID_NDK_HOME=$ANDROID_NDK_PATH

if [ -z "$JAVA_HOME" ]
then
  export PATH=$PATH:/usr/lib/jvm/jdk1.8.0_112/bin:/usr/lib/jvm/jdk1.8.0_112/db/bin:/usr/lib/jvm/jdk1.8.0_112/jre/bin
  export JAVA_HOME=/usr/lib/jvm/jdk1.8.0_112
fi

# we use VULKAN_SDK_PATH for vk_layer.h. However it should be part of the android ndk as we wont know
# if the loader lib and vk_layer.h are compatible.
export VULKAN_SDK_PATH=$ANDROID_SDK_PATH/ndk-bundle/sources/third_party/vulkan/src/include/
