#!/bin/bash

TAR_NAME="CodeXL_v*.tar.gz"
VERSION_VER="2.0.0"
PACAKGENAME="codexl"
CONFIGURATION=""
BUILD_NUMBER=""

if [ "$#" -eq 4 ]; then
    TAR_NAME=$1
    VERSION_VER=$2
	CONFIGURATION=$3	
	BUILD_NUMBER=$4
	PACAKGENAME="lpgpu2-codexl"

	if [ "${BUILD_NUMBER}" != "" ]; then
		PACAKGENAME=${PACAKGENAME}_Build-${BUILD_NUMBER}
	fi
else
	TAR_NAME = "CodeXL_v*.tar.gz"
	VERSION_VER=$(find . -name 'CodeXL_v*.tar.gz' | awk -F"." '{ print $4}')
fi

BASE_VERSION=${VERSION_VER}
BASE_REVISION=
INTERNAL_VERSION=${BASE_VERSION}.${VERSION_VER}

if [ "${CONFIGURATION}" == "" ]; then
	VERSION=${BASE_VERSION}
else
	VERSION=${BASE_VERSION}_${CONFIGURATION}
fi

echo ${VERSION}
echo $TAR_NAME
# Change Package name if NDA or INTERNAL version
NDASET=$(find . -name 'CodeXL*.tar.gz' | grep -q "NDA"; [ $? -eq 0 ] && echo "nda")
INTERNALSET=$(find . -name 'CodeXL*.tar.gz' | grep -q "Internal"; [ $? -eq 0 ] && echo "internal")
if ! [ -z "$NDASET" ]; then
	PACAKGENAME="${PACAKGENAME}-${NDASET}"
elif ! [ -z "$INTERNALSET" ]; then
	PACAKGENAME="${PACAKGENAME}-${INTERNALSET}"
fi

# Remove old folders
rm -rf AMDExtractFolder
rm -rf opt
rm -rf usr

#Un-tar source files
echo "Unpack files" 

mkdir AMDExtractFolder
tar xf $TAR_NAME --strip 1 -C AMDExtractFolder
cp *.desktop AMDExtractFolder/

echo "Setting files and folder"
# Set Executables Icons
chmod +xw AMDExtractFolder/amdcodexlicon.desktop
chmod +xw AMDExtractFolder/amdremoteagenticon.desktop
echo "Version=${BASE_VERSION}" >> AMDExtractFolder/amdcodexlicon.desktop
echo "Exec=/opt/CodeXL_${VERSION}/CodeXL" >> AMDExtractFolder/amdcodexlicon.desktop
echo "Icon=/opt/CodeXL_${VERSION}/Images/ApplicationIcon_64.ico" >> AMDExtractFolder/amdcodexlicon.desktop
echo "Path=/opt/CodeXL_${VERSION}/" >> AMDExtractFolder/amdcodexlicon.desktop
echo "Version=${BASE_VERSION}" >> AMDExtractFolder/amdremoteagenticon.desktop
echo "Exec=/opt/CodeXL_${VERSION}/CodeXLRemoteAgent" >> AMDExtractFolder/amdremoteagenticon.desktop
echo "Icon=/opt/CodeXL_${VERSION}/Images/ApplicationIcon_64.ico" >> AMDExtractFolder/amdremoteagenticon.desktop 
echo "Path=/opt/CodeXL_${VERSION}/" >> AMDExtractFolder/amdremoteagenticon.desktop 

# Move files to relative destination
mkdir -p opt/CodeXL_${VERSION}
cp -r AMDExtractFolder/. opt/CodeXL_${VERSION}/
mkdir -p usr/share/CodeXL_${VERSION}/
mv opt/CodeXL_${VERSION}/examples/ usr/share/CodeXL_${VERSION}/examples/
ln -s /usr/share/CodeXL_${VERSION}/examples/ opt/CodeXL_${VERSION}/examples
mkdir -p usr/share/applications
cp AMDExtractFolder/amdcodexlicon.desktop usr/share/applications/amdcodexlicon.desktop
cp AMDExtractFolder/amdremoteagenticon.desktop usr/share/applications/amdremoteagenticon.desktop 

# Edit Debian package files with current version
echo "Setting debian package files"
chmod +w control
chmod +w postinst
chmod +w prerm
chmod +w changelog
sed -i "s/Version: /Version: ${BASE_VERSION}/g" control 
sed -i "s/CodeXL_[^/]*/CodeXL_${VERSION}/g" postinst
sed -i "s/CodeXL_[^/]*/CodeXL_${VERSION}/g" prerm
sed -i "s/()/(${VERSION})/g" changelog
#get build time
buildtime=$(date +"%a, %d %b %Y %H:%M:%S %z")
sed -i "s/com>/com> ${buildtime}/g" changelog
#Set package folder size
psize=$(du -s AMDExtractFolder/| sed 's/\s.*$//')
sed -i "s/Installed-Size: /Installed-Size: ${psize}/g" control

#Running FPM command
# Reference command - fpm -f -C ~/Downloads/FromRPM --workdir ~/Downloads/testrpmdeb/ --deb-custom-control ~/Downloads/debscripts/control --after-install ~/Downloads/debscripts/postinst --before-remove ~/Downloads/debscripts/prerm -s dir -t deb -n amdcodexl -v 1.8-9000 opt/ usr/
echo "Running fpm... (pack the directories to deb package)"
fpm -f -C . --deb-custom-control control --after-install postinst --before-remove prerm --deb-changelog changelog -s dir -t deb -n ${PACAKGENAME} -v ${BASE_VERSION} opt/ usr/
