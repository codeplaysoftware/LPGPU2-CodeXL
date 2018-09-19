#!/usr/bin/python3
#
# The MIT License
#
# Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
# For conditions of distribution and use, see the accompanying COPYING file.
#
#
import sys
import subprocess
import inspect
import os
import os.path
import zipfile
import tarfile
import tempfile
import shutil
import glob
import re

from os.path import realpath, join, expandvars, expanduser

USAGE = """
LPGPU2 collector build tool
e.g.
  ./build.py              ; prints help
  ./build.py all          ; builds all - interposer,dcapi,executable and zip
  ./build.py all_targets  ; build all target binaries - interposer, dcapi etc
  ./build.py clean        ; removes built files

  Accepts a series of actions run serially so

  ./build.py clean all    ; will first run 'clean' then run 'all'

  Actions can be parameterised with command line arguments containing '=' after
  the action they apply to.

  ./build.py android_generate_api vulkan=true

  Sticky parameters can be set with 'set' and apply for all subsequent actions

  ./build.py set abi=armeabi-v7a android_interposer android_dcapi_test

"""

ACTIONS = {}
SCRIPT_PATH, SCRIPT_NAME = os.path.split(realpath(sys.argv[0]))
USER_ACTIONS = " ".join(sys.argv[1:])

CPP_STD_LIBRARY = "gnustl_shared"

#
#
#
def count_quotes(s):
    pos = s.find('"',0)
    if pos < 0 or pos >= len(s):
        return 0
    else:
        c = 0
        while pos >= 0 and pos < len(s):
            if pos > 0:
                if s[pos-1] != '\\':
                    c += 1
            else:
                c += 1
            pos= s.find('"', pos+1)

        return c

def split_handling_quotes(string):
    split = string.split()
    ret = []
    quotes = 0
    for s in split:
        c = count_quotes(s)

        if quotes:
            ret[-1] += " "
            ret[-1] += s
            quotes += c
            if not (quotes & 0x1):
                quotes = 0
            continue

        if c == 0:
            ret.append(s)
        elif c & 0x1:
            ret.append(s)
            quotes = c
        else:
            ret.append(s)

    return ret
#
#
#
def run(cmd):
    returncode = 0

    try:
        subprocess.check_call(split_handling_quotes(cmd))
    except subprocess.CalledProcessError as error:
        returncode = error.returncode

    return returncode

#
#
#
def run_until_error(cmd_list):
    assert isinstance(cmd_list, type([])), "must be a list and not a single command"
    for cmd in cmd_list:
        print("Build:", repr(cmd))
        error = run(cmd)
        if error:
            print("Stopping on Error (", repr(error), "):", repr(cmd))
            sys.exit(error)

#
#
#
def truthy_as_integer_string(string):
    if string in ["1", "True", "true"]:
        return "1"
    else:
        return "0"

#
#
#
def fullpath(name):
    return realpath(expandvars(expanduser(name)))

#
#
#
class change_directory:
    def __init__(self, directory):
        self.directory = directory
        self.original = os.getcwd()
    def __enter__(self):
        print("Build: " + repr("cd " + self.directory))
        os.chdir(self.directory)
        return self
    def __exit__(self, typ, value, traceback):
        print("Build: Return " + repr("cd " + self.original))
        os.chdir(self.original)

class TemporaryDirectory:
    def __init__(self):
        self.directory = tempfile.mkdtemp() # [suffix=''[, prefix='tmp'[, dir=None]]])
        print("Build: Create temporary directory " + self.directory)
    def __del__(self):
        print("Build: Remove temporary directory " + self.directory)
        shutil.rmtree(self.directory, ignore_errors=True)

class Config:
    def __init__(self):
        self.global_config = {}
        self.local_config = {}

    def __getitem__(self, name):
        # local overrides
        try:
            return self.local_config[name]
        except KeyError:
            pass
        # then global
        try:
            return self.global_config[name]
        except KeyError:
            pass

        # then none not KeyError so this works 'arg_dict["api"] or "armeabi-v7a"
        return None

CONFIG = Config()

#
#
#
def do_set(config):
    """Set options globaly
"""
    config.global_config.update(config.local_config)

#
#
#
def do_android_generate_api(config):
    """Generate source from API XML files. Optional arguments are {abi} {platform} {vulkan}.
ie android_interposer abi=armeabi-v7a
   android_interposer abi=armabi-v7a platform=android-23 vulkan=True
"""
    build_directory = realpath("./build/api-shimv2/")

    os.environ["BUILD_DIRECTORY"] = build_directory

    abi = config["abi"] or "armeabi-v7a"
    os.environ["ABI"] = abi

    platform = config["platform"] or "android-9"
    os.environ["PLATFORM"] = platform

    os.environ["CPP_STD_LIBRARY"] = config["CPP_STD_LIBRARY"] or CPP_STD_LIBRARY

    os.environ["GENERATE_VULKAN"] = truthy_as_integer_string(config["vulkan"]) or "0"

    if os.environ["GENERATE_VULKAN"] == "1":
        if int(platform.split("-")[1]) < 23:
            print("Vulkan build Needs abi >= 23")
            exit(1)

    with change_directory("./api-shim"):
        run_until_error(["./generate-api.sh"])

#
#
#
def do_android_interposer(config):
    """Build android mali interposer. Optional arguments are {ABI} {PLATFORM} {LOCATION} {LINK}.
ie android_interposer abi=armeabi-v7a
   android_interposer abi=armeabi-v7a platform=android-9
   android_interposer abi=armeabi-v7a platform=android-9 link=explicit
"""
    build_directory = realpath("./build/api-shimv2/")

    os.environ["BUILD_DIRECTORY"] = build_directory

    abi = config["abi"] or "armeabi-v7a"
    os.environ["ABI"] = abi

    platform = config["platform"] or "android-9"
    os.environ["PLATFORM"] = platform

    location_root = config["location_root"] or "./build/targets/interposer"

    location = config["location"] or join(location_root, platform, abi)

    os.environ["CPP_STD_LIBRARY"] = config["CPP_STD_LIBRARY"] or CPP_STD_LIBRARY

    build_link = config["link"] or "implicit"
    if build_link == "explicit":
        os.environ["INTERPOSER_CFLAGS"] = "-DINTERPOSER_EXPLICITLY_LINKED_FROM_APP"

    with change_directory("./api-shim"):
        run_until_error(["./build-android-interposer.sh"])

    run_until_error(["mkdir -p %s" % location])

    with change_directory(location):
        for filename in ["libGles2Shim.so",
                         "lib%s.so" % os.environ["CPP_STD_LIBRARY"]]:
            run_until_error(["cp " + join(build_directory, "libs", abi, filename) + " ."])

#
#
#
def do_android_vulkan(config):
    """Build android mali interposer. Optional arguments are {ABI} {PLATFORM} {LOCATION} {LINK}.
ie android_vulkan abi=armeabi-v7a
   android_vulkan abi=armeabi-v7a platform=android-9
   android_vulkan abi=armeabi-v7a platform=android-9 vukan_sdk_path=
"""
    build_directory = realpath("./build/api-shimv2/")

    os.environ["BUILD_DIRECTORY"] = build_directory

    abi = config["abi"] or "armeabi-v7a"
    os.environ["ABI"] = abi

    platform = config["platform"] or "android-23" # 23 is the start of vulkan
    os.environ["PLATFORM"] = platform

    location_root = config["location_root"] or "./build/targets/vulkan"

    # 'lib' helps when using 'aapt add' to add the file to the apk
    location = config["location"] or join(location_root, platform, "lib", abi)

    os.environ["CPP_STD_LIBRARY"] = config["CPP_STD_LIBRARY"] or "gnustl_static" # CPP_STD_LIBRARY

    os.environ["VULKAN_LAYER"] = "1"

    with change_directory("./api-shim"):
        run_until_error(["./build-android-interposer.sh"])

    run_until_error(["mkdir -p %s" % location])

    with change_directory(location):
        for filename in ["libVkLayer_lpgpu2.so"]:
                         #"lib%s.so" % os.environ["CPP_STD_LIBRARY"]]:
            run_until_error(["cp " + join(build_directory, "libs", abi, filename) + " ."])

#
#
#
def do_android_dcapi_test(config):
    """Build android dcapi test library. Optional arguments are {ABI} {PLATFORM} {LOCATION}.
ie android_dcapi_test abi=armeabi-v7a
   android_dcapi_test abi=armeabi-v7a platform=android-9
"""
    build_directory = realpath("./build/libdcapitest/")

    os.environ["BUILD_DIRECTORY"] = build_directory

    abi = config["abi"] or "armeabi-v7a"
    os.environ["ABI"] = abi

    platform = config["platform"] or "android-9"
    os.environ["PLATFORM"] = platform

    location_root = config["location_root"] or "./build/targets/dcapi"

    location = config["location"] or join(location_root, platform, abi)

    os.environ["CPP_STD_LIBRARY"] = config["CPP_STD_LIBRARY"] or CPP_STD_LIBRARY

    with change_directory("./test/dcapi"):
        run_until_error(["./build.sh",
                        ])

    run_until_error(["mkdir -p %s" % location])

    with change_directory(location):
        run_until_error(["cp " + join(build_directory, "libs", abi, "libDCAPITest.so") + " .",
                         ])

#
#
#
def do_shimify_apps(config):
    """Build glbench test app with explicitly linked Interposer
"""
## First, build and push the shim for the non-vulkan apps.

    jnilibs = "build/shim_builds/jniLibs"

    platform_abi_tuples=[("android-23", "armeabi-v7a"),
			    ("android-21", "arm64-v8a"),
			    ("android-14", "x86")]

    for platform, abi in platform_abi_tuples:
        run_until_error([
            "mkdir -p %s" % jnilibs,
            ])

        shim_location = join(jnilibs, abi)

        execute_actions("set abi=%s platform=%s location=%s " % (abi, platform, shim_location) +\
		"android_generate_api "
		"android_interposer link=explicit "
		)

    # copy to target
    run_until_error([
        "cp -r ./build/shim_builds/jniLibs test/ReplayNative/Replay/src/main",
        "cp -r ./build/shim_builds/jniLibs test/GLGlobeUber/Globe/src/main",
        "cp -r ./build/shim_builds/jniLibs test/OpenCLTest/app/src/main",
        ])


## Second, build and push the shim for Vulkan apps and SpinDigital Vulkan base app

    # vulkan starts at 23 and only supported for armeabi-v7a or arm64-v8a
    vulkan_abi_tuples=[ ("android-23", "armeabi-v7a"),
		        ("android-23", "arm64-v8a") ]

    jnilibs = "build/shim_vulkan_builds/jniLibs"

    for platform, abi in vulkan_abi_tuples:

        shim_location = join(jnilibs, abi)

        execute_actions("set abi=%s platform=%s location=%s CPP_STD_LIBRARY=gnustl_static " % (abi, platform, shim_location) +\
                    "android_generate_api vulkan=1 "
                    "android_vulkan "
                    "".format(platform=platform, abi=abi)
                    )

    # copy to targets
    run_until_error(["cp -r ./build/shim_vulkan_builds/jniLibs test/VulkanApps/app/src/main"])


## Third, build and push dcapi to the ragent.

    jnilibs = "build/dcapi_builds/jniLibs"

    platform_abi_tuples=[("android-23", "armeabi-v7a"),
			    ("android-21", "arm64-v8a"),
			    ("android-14", "x86")]

    for platform, abi in platform_abi_tuples:
        run_until_error([
            "mkdir -p %s" % jnilibs,
            ])

        shim_location = join(jnilibs, abi)

        execute_actions("set abi=%s platform=%s location=%s " % (abi, platform, shim_location) +\
		"android_dcapi_test "
		)

    # copy to target
    run_until_error([
        "cp -r ./build/dcapi_builds/jniLibs remote/LPGPU2RAgent/app/src/main",
        "cp ./dcapi/DCAPI.h remote/LPGPU2RAgent/app/src/main/cpp"
        ])

## Fourth, modify and push Client Code.

    with open('./test/Client/shim2ify.h') as f:
      shimify = f.read()

    with open('./test/Client/LPGPU2RMessengerInterface.java') as f:
      messenger = f.read()

    with open('./test/Client/ShimActivity.java') as f:
      activity = f.read()

    with open('./test/Client/LPGPU2DataPacket.java') as f:
      data = f.read()

## ReplayNative
    with open('./test/ReplayNative/Replay/src/main/cpp/shim2ify.h', 'w') as f:
      f.write(shimify.replace('Package_Name', 'org_lpgpu_replaynative'))

    with open('./test/ReplayNative/Replay/src/main/java/org/lpgpu/replaynative/LPGPU2RMessengerInterface.java', 'w') as f:
      f.write(messenger.replace('Package_Name', 'org.lpgpu.replaynative'))

    with open('./test/ReplayNative/Replay/src/main/java/org/lpgpu/replaynative/ShimActivity.java', 'w') as f:
      act = activity.replace('Activity_Name', 'NativeActivity')
      f.write(act.replace('Package_Name', 'org.lpgpu.replaynative'))

    with open('./test/ReplayNative/Replay/src/main/java/org/lpgpu/replaynative/LPGPU2DataPacket.java', 'w') as f:
      f.write(data.replace('Package_Name', 'org.lpgpu.replaynative'))


## GLGlobeUber
    with open('./test/GLGlobeUber/Globe/src/main/cpp/shim2ify.h', 'w') as f:
      f.write(shimify.replace('Package_Name', 'org_lpgpu_globe'))

    with open('./test/GLGlobeUber/Globe/src/main/java/org/lpgpu/globe/LPGPU2RMessengerInterface.java', 'w') as f:
      f.write(messenger.replace('Package_Name', 'org.lpgpu.globe'))

    with open('./test/GLGlobeUber/Globe/src/main/java/org/lpgpu/globe/ShimActivity.java', 'w') as f:
      act = activity.replace('Activity_Name', 'NativeActivity')
      f.write(act.replace('Package_Name', 'org.lpgpu.globe'))

    with open('./test/GLGlobeUber/Globe/src/main/java/org/lpgpu/globe/LPGPU2DataPacket.java', 'w') as f:
      f.write(data.replace('Package_Name', 'org.lpgpu.globe'))

## OpenCLTest
    with open('./test/OpenCLTest/app/src/main/cpp/shim2ify.h', 'w') as f:
      f.write(shimify.replace('Package_Name', 'com_example_lpgpu2_opencltest'))

    with open('./test/OpenCLTest/app/src/main/java/com/example/lpgpu2/opencltest/LPGPU2RMessengerInterface.java', 'w') as f:
      f.write(messenger.replace('Package_Name', 'com.example.lpgpu2.opencltest'))

    with open('./test/OpenCLTest/app/src/main/java/com/example/lpgpu2/opencltest/ShimActivity.java', 'w') as f:
      act = activity.replace('Activity_Name', 'AppCompatActivity')
      f.write(act.replace('Package_Name', 'com.example.lpgpu2.opencltest'))

    with open('./test/OpenCLTest/app/src/main/java/com/example/lpgpu2/opencltest/LPGPU2DataPacket.java', 'w') as f:
      f.write(data.replace('Package_Name', 'com.example.lpgpu2.opencltest'))

## VulkanApps
    with open('./test/VulkanApps/app/src/main/cpp/shim2ify.h', 'w') as f:
      f.write(shimify.replace('Package_Name', 'org_lpgpu_vulkanapps'))

    with open('./test/VulkanApps/app/src/main/java/org/lpgpu/vulkanapps/LPGPU2RMessengerInterface.java', 'w') as f:
      f.write(messenger.replace('Package_Name', 'org.lpgpu.vulkanapps'))

    with open('./test/VulkanApps/app/src/main/java/org/lpgpu/vulkanapps/ShimActivity.java', 'w') as f:
      act = activity.replace('Activity_Name', 'NativeActivity')
      act = act.replace('import android.support.v7.app.AppCompatActivity;', '')
      act = act.replace('libGles2Shim', 'libVkLayer_lpgpu2')
      act = act.replace('startCollecting(shimPath);', '// startCollecting(shimPath);')
      act = act.replace('initShim(shimPath);', 'initShim(shimPath);\n        startCollecting(shimPath);')
      f.write(act.replace('Package_Name', 'org.lpgpu.vulkanapps'))

    with open('./test/VulkanApps/app/src/main/java/org/lpgpu/vulkanapps/LPGPU2DataPacket.java', 'w') as f:
      f.write(data.replace('Package_Name', 'org.lpgpu.vulkanapps'))

## Fith, push the lpgpu2_api header file.
    # The openCL test uses the lpgpu2 api (FrameTerminator)
    run_until_error([
        "cp -r ./api-shim/src/lpgpu2_api.h ./test/ReplayNative/Replay/src/main/cpp/",
        "cp -r ./api-shim/src/lpgpu2_api.h ./test/GLGlobeUber/Globe/src/main/cpp/",
        "cp -r ./api-shim/src/lpgpu2_api.h ./test/OpenCLTest/app/src/main/cpp/",
        "cp -r ./api-shim/src/lpgpu2_api.h ./test/VulkanApps/app/src/main/cpp/",
        ])
#
#
#
def arg_list_to_dictionary(*args):
    d = {}
    i = 0
    if not args:
        return d

    while 1:
        a = args[i]
        # if spaces ie 'name = value'
        if a == "=":
            print('''No spaces allowed around '=' ie 'name=value' or 'name="a value"' ''')
            print("Stopping")
            exit(1)

        if "=" in a:
            k, v = a.split("=")
            if v.startswith('"') and not v.endswith('"'):
                while 1:
                    i += 1
                    v = v + " " + args[i]
                    if args[i].endswith('"'):
                        break

            if v.startswith('"') and v.endswith('"'):
                d[k] = v[1:-1]
            else:
                d[k] = v
        else:
            d[a] = True

        i += 1

        if i >= len(args):
            break

    return d

#
#
#
def execute_actions(action_cmdline):
    global ACTIONS
    print("Build: Command Line:", repr(action_cmdline))
    args = []
    next_action = None
    for arg in action_cmdline.split():
        if arg == "--help" or arg == "-h":
            arg = "help"
        if arg in ACTIONS:
            # either an action name
            if next_action:
                print("Build: Action:", repr(next_action.__name__), "Args:", repr(args))
                CONFIG.local_config = arg_list_to_dictionary(*args)
                next_action(CONFIG)
                next_action = None
                args = []

            next_action = ACTIONS[arg]
        else:
            if "=" not in arg:
                print("Unknown build action ", repr(arg), "Quiting.")
                print(" Build Actions are words (see '%s help')" % sys.argv[0])
                print(" Action parameters have '=' without spaces ie '%s android_interposer abi=armeabi-v7a'" % sys.argv[0])
                print("Stopping")
                exit(1)
            else:
                # or an argument to the action name
                args.append(arg)


    if next_action:
        print("Build: Action:", repr(next_action.__name__), "Args:", repr(args))
        CONFIG.local_config = arg_list_to_dictionary(*args)
        next_action(CONFIG)

#
#
#
if __name__ == "__main__":

    def is_action_function(obj):
        return inspect.isfunction(obj) and obj.__name__.startswith("do_")

    ACTIONS = {name[3:]: obj for name, obj in globals().items() if is_action_function(obj)}

    execute_actions("shimify_apps")

    print("Build: Success! (no actions failed)")
