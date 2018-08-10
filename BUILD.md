CodeXL Build Instructions
===========================

## Windows

### [Prerequisites](#prerequisites)
* [Latest AMD Radeon Software](http://support.amd.com/en-us/download)
* [Microsoft Visual Studio 2015 Community Edition or higher + Update 3](https://visualstudio.microsoft.com/vs/older-downloads/)
* [Windows 10 SDK Version 10.0.10586.0](https://developer.microsoft.com/en-us/windows/downloads/sdk-archive)
* [Java Development Kit - JDK](http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html)
* [Python 2.7 or newer](https://www.python.org/downloads/)
* [Android ADB](https://developer.android.com/studio/releases/platform-tools)

> CodeXL requires JDK for CPU Profiling support of Java applications and the build may fail on Linux machines if JDK is not found in the system.

> LPGPU<sup>2</sup> CodeXL uses ADB to communicate with Android devices in order to start or stop the Remote Agent that runs on the application. ADB is not strictly required but it greatly simplifies the profiling experience.

### Building CodeXL on Windows

* Download and install the required software as listed in [Prerequisites](#prerequisites).
* Clone the LPGPU<sup>2</sup> CodeXL repository.
    * `https://github.com/codeplaysoftware/LPGPU2-CodeXL.git`
* Open the solution with **Visual Studio 2015**: `LPGPU2-CodeXL\CodeXL\AllProjectsBuild\CodeXL-AllProjects.sln`
  * In the Solution Explorer, right-click on the `CodeXL-AllProjects` solution and select `Properties`.
    * Open `Common Properties > Startup Project`.
    * Select the *Single startup project* option and set the project to `AMDCodeXL`.
    * Click `OK`.
* Build 32 bit configuration in *Release* mode.
  * You *must build CodeXL for the Win32 platform before you can build it for the x64 platform*.
    * The Win32 build creates `CodeXL(-d).lib`, which is required for both Win32 and x64 CodeXL to run.
* Build 64 bit configuration
* If nothing goes wrong, CodeXL binaries will be located in `LPGPU2-CodeXL\CodeXL\Output\Release\bin\CodeXL.exe`.

##### Building the experimental ComputeCpp Profiler
* Download **ComputeCpp** from [Codeplay's developer website](https://developer.codeplay.com/computecppce/latest/overview)
* Extract the content in the folder `LPGPU2-CodeXL/Common/Lib/Ext/ComputeCpp`.
* Rename the extracted folder to `ComputeCpp-CE-0.9.1-Win64`.
* Double-click the script `SetupComputeCpp.bat`.
* This will generate a file called `ComputeCpp-Global.props` that is required to build CodeXL with SYCL Profiling capabilites.


##### Enabling build of CCVGCodeCoverage
The build of the **CCVGCodeCoverage** execution mode is disabled by default. In order to enabled it, perform the following steps:
* Go to the folder ```CodeXL\Components\LPGPU2_CCVG\CCVGCodeCoveragePlugin```
* Double click on the script ```Enable_CCVGBuild.bat```
* Build the solution again

##### Installing CodeXL Drivers
After a successful build of CodeXL solution, you can install the drivers that CodeXL uses to power profile PC applications.
> This step is optional for profiling LPGPU<sup>2</sup> Android applications.
* Go to the folder `CodeXL\Components\CodeXLDrivers\Windows\Scripts`
* Run the script `CodeXLInstallDriver_Standalone.bat`

#### Linux

##### Preface

* CodeXL is available in 32bit and 64bit flavours. We only support 64bit.
* These build steps were performed on clean/new Ubuntu 16.04 64bit LTS machine.
* If you are using a virtual Linux machine ensure the disk image size is at least 48GB.
* CodeXL uses the SCons build system on Linux.
* CodeXL require GCC version 4.8.5 or higher.

##### Prerequisites
> We only support Ubuntu 16.04 64bit

- Make sure the following packages are installed:

```
sudo apt install python2.7
sudo apt install adb
sudo apt install opencl-headers
sudo apt install gcc-multilib g++-multilib
sudo apt install libglu1-mesa-dev mesa-common-dev libgtk2.0-dev
sudo apt install zlib1g-dev libx11-dev
sudo apt install scons
sudo apt install libjpeg9-dev
sudo apt install libfltk1.3-dev
sudo apt install openjdk-8-jdk-headless
```

- Make sure the environment variable `JAVA_HOME` is defined and pointing a to a valid JDK installation. If `JAVA_HOME` is not defined, the build will fail.

##### Building CodeXL on Linux
* `cd LPGPU2-CodeXL/CodeXL/Util/linux`
* `./buildCodeXLFullLinuxProjects CXL_build=release LPGPU2_BUILD_CCVG=True -j8`
* If nothing goes wrong, CodeXL binaries will be located in `LPGPU2-CodeXL/Output_x86_64/bin`

##### Building the experimental ComputeCpp Profiler
* Download **ComputeCpp** from [Codeplay's developer website](https://developer.codeplay.com/computecppce/latest/overview)
* Extract the content in the folder `LPGPU2-CodeXL/Common/Lib/Ext/ComputeCpp`.
* SCons will take care of finding ComputeCpp as long as it is extracted in the specified location.

##### Optional build of the Desktop GPU Backends:
> In order to use the prototype of [ComputeCpp](https://www.codeplay.com/products/computesuite/computecpp) SYCL Profiler on Linux, this step must be performed:
* `cd LPGPU2-CodeXL/Components/GpuProfiling/Build`
* `./backend_build.sh skip-32bit-build skip-hsaprofiler`

##### Build Switches
* all SCons general switches, like -c for clean , more info at http://scons.org/doc/HTML/scons-man.html
* __-j__ specify the number of concurrent jobs (-j6).
* __LPGPU2\_BUILD\_CCVG=[True|False]__ - Enables or disables the build of the LPGPU2_CCVG project. By default the build is disabled.
* __CXL\_build=[debug|release]__ - build type. If not stated default value is release.
* __CXL\_build\_verbose=1__ - verbose output
* __CXL\_boost\_lib\_dir=[path to boost libraries]__ - override the bundled boost libraries with files in given path
* __CXL\_boost\_include\_dir=[path to boost headers]__ - override the bundled boost headers with files in given path
* __CXL\_hsa=[true|false]__ - define if to build HSA parts. If not stated default value is false (skip HSA)
* __-c__ - performs a "clean" of all build targets.
* When executing the backend\_build.sh script, the following switches are supported:
    * __debug__: performs a debug build
    * __skip-32bitbuild__: skips building the 32-bit binaries
    * __skip-oclprofiler__: skips building the OpenCL profiler binaries
    * __skip-hsaprofiler__: skips building the HSA profiler binaries. If building on a system where you don't have HSA/ROCR header files, use this switch to avoid errors due to missing header files
    * __hsadir 'dir'__: by default, when building the HSA Profiler binaries, the build scripts will look for the HSA/ROCR headers under /opt/rocm/hsa.  You can override this location using the "hsadir" switch.
    * __boostlibdir 'dir'__: by default, the build scripts will look for the boost libs in Common/Lib/Ext/Boost/boost_1_59_0/lib/RHEL6.  You can override the location this location using the "boostlibdir" switch.
    * __quick__ or __incremental__: performs an incremental build (as opposed to a from-scratch build)
    * __clean__: performs a "clean" of all build targets, removing all intermediate and final output files
* __LPGPU2\_BUILD\_ANDROID\_PROTOCOL=[True|False]__ - Disable/Enable the
building of the LPGPU2 remote device protocol and GUI. _Default:_ __True__

##### Installing CodeXL Drivers
The power profiling driver is installed using the `CodeXLPwrProfDriver.sh` script, which is located at `/CodeXL/Components/PowerProfiling/Backend/AMDTPowerProfilingDrivers/Linux`.
The script must be run with root privileges and accepts the following arguments:
* `install`
* `uninstall`

> **Note**: When script will copy the ```CodeXLPwrProfDriverSource.tar.gz``` file to the ```/usr/src/amdtPwrProf-<version>``` directory, where it will be compiled.

##### Important

> Before attempting to compile CodeXL if not on a clean machine be sure to clean out any previous builds of CodeXL especially if you have tried to build CodeXL with GCC versions not specified below. Calling `buildCodeXLFullLinuxProjects -c` to clean the build is not enough to clear out previously built libraries or object files. Likely the files will be used and not rebuilt. Manually remove the `Output_x68_64` and recompile everything.