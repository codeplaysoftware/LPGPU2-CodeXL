# LPGPU2 CodeXL

![LPGPU2 Logo](http://lpgpu.org/wp/wp-content/uploads/2016/01/logo_web.png)

## About LPGPU2

Low-Power Parallel Computing on GPUs 2 (LPGPU<sup>2</sup>) is a EU-funded research project into low powered graphics devices. It is the work of a specially formed consortium of four companies and one university, all from across the EU, who are collaborating to deliver advances in tools and applications for energy efficient use of mobile GPUs.

## About this directory

> This directory will, soon, be the home for the open source release of the work done by the LPGPU<sup>2</sup> consortium on top of CodeXL.

In particular it will contain the Android target components and test applications.

## System requirements
(Requires Ubuntu 16.04 or higher)

## Preparation for building

### Install Unzip
sudo apt-get install unzip 

### Install Git
sudo apt-get install git

### Installing Android Studio and components
1.	Download Android Studio 3.1.3 (June 8 2017) from https://developer.android.com  or https://developer.android.com/studio/archive.html
2.	Or you can try with the latest version, unfortunately sometimes Android Studio updates can break existing projects. 
2. Follow install instructions for linux to launch Android Studio, ensuring to install required libraries.
sudo unzip android-studio-ide-162.3764568-linux.zip -d /opt
sudo apt-get install libc6:i386 libncurses5:i386 libstdc++6:i386 lib32z1 libbz2-1.0:i386
May require a reboot after this if Android Studio hangs on start up.
3. Select Custom Install and install the default SDK platform, currently if not installed when you run studio.h it complains that no SDK installed and run set up wizard again.
4. From Android Studio select Configure -> SDK Manager and select the following:
SDK Android Platforms
•	7.0 API 24 rev 2
•	6.0 API 23 rev 3
•	5.0 API 21 rev 2
•	3.1 API 12 rev 3   (Latest version of NDK has dropped support for this.)
SDK Tools
•	GPU Debugging Tools
•	CMake
•	LLDB
•	Android Emulator 25.3.1
•	Android SDK Platform tools 25.0.4 - ADB tool
•	Android SDK tools 25.3.1
•	NDK 14.0.3770861
•	Google repository 44
5. Note on support Libraries for Gradle build Tool
There are a set of constraint libraries used by Gradle and these will be specific to a particular application. For instance, the Choreographer app (please see below) requires constraint-layout:1.0.0-alpha7. This is configured in the /Choreographer/choreographer-30fps/build.gradle file.  
Please ensure the appropriate support library is included.
6. Add path to studio.sh, path to Android tools, and path to Android Platform tools, to the PATH environment variable in .bashrc or, .profile etc.

### Install Python3
sudo apt-get install python3 python3-pip python3-pyside
sudo pip3 install pyinstaller
sudo pip3 install aiofiles
sudo pip3 install numpy
sudo pip3 install pyqtgraph
sudo pip3 install pypng
sudo apt-get install liblua5.2-dev
sudo pip3 install lupa
sudo pip3 install lxml

### Prepare Environment
sudo apt install ant 
From Git folder project installed to enter:
export ANDROID_HOME=<path to SDK>
./env-android.sh

### 'Shimify' apps
We call the process of inserting the Graphics API Shim into the apps 'shimifying'. To shimify the apps navigate to the Tools directory:
cd <your LPGPU2 git repo>/Tools
then run the shimifying script like this
./buildv3.py shimify_apps
This will take a few minutes, but is quite verbose so you will see lots of encouraging activity.
When it is finished it should say "Success! (No actions failed)"
Now you are ready to run your first app

### Run Your First App
Now that you have cloned the <your LPGPU2 git repo>/Tools repo and shimified all the apps, open the ReplayNative project in Android Studio. It is located in the <your LPGPU2 git repo>/Tools/test directory

Plug in your android device and allow your machine to connect to it by clicking "Allow" on the device, if asked.
Android Studio can take a little while to "warm up", but once the wheel has stopped spinning in the status bar at the bottom, you should have a clickable green arrow go button in the toolbar (at the top of the Android Studio window).
Click this go button and you should be prompted to select a device. This is in the form of a pop up with a list box containing the name of your device in it. Select the device and click OK.
Android Studio will begin building. After building the app, Android studio will report "Installing apk" in the status bar, and then "Launching app"
After a few more seconds, the app should begin to run on your device.
You can stop it using the red square stop button in the tool.
The app is installed on your device, so you can disconnect your device (pull the connecting cable out) and find the app called "LPGPU2 Carousel" in the app list on your device.
You can run it like any other app by clicking on it, and leave it by going to your home screen. 

Now that you are an expert at running apps (Well done, by the way!) You will want to profile it, and for that you will need CodeXL.
