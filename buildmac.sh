#!/bin/sh

args=`getopt cu $*`
if [ $? != 0 ]
then
   echo 'Usage: $0 [-c] [-u]'
   exit 2
fi

set -- $args

for i ; do case "$i" in
   -c)
	CLEAN=true
	shift;;
   -u)
	UPLOAD=true
	shift;;
   --)
	shift; break;;
  esac
done

VERSION=0.10

XCODEDIR=./xcode
VOLNAME=freej
BUILD=build/Release/freej.app
BGPIC=../dmgbg.png

TMPFILE=/tmp/freejtmp.dmg
MNTPATH=/tmp/mnt/

APPNAME=$(basename $BUILD)

#####
cd $XCODEDIR

if [ -n "$CLEAN" ]; then
xcodebuild clean || exit
fi
xcodebuild ||exit

#####

BUILDNUMBER=`grep BUILD_NUMBER config.h | awk '{print $3}'`
if [ -n "$BUILDNUMBER" ]; then
  VERSION=${VERSION}.${BUILDNUMBER}
fi
DMGFILE=/tmp/$VOLNAME-$VERSION.dmg
echo $DMGFILE

#roll a DMG

mkdir -p $MNTPATH
if [ -e $TMPFILE -o -e $DMGFILE -o ! -d $MNTPATH ]; then
  echo "could not make DMG. (file exists?!)"
  exit;
fi

hdiutil create -megabytes 200 $TMPFILE
DiskDevice=$(hdid -nomount "${TMPFILE}" | grep Apple_HFS | cut -f 1 -d ' ')
newfs_hfs -v "${VOLNAME}" "${DiskDevice}"

mount -t hfs "${DiskDevice}" "${MNTPATH}"

cp -r ${BUILD} ${MNTPATH}/
mkdir ${MNTPATH}/.background
BGFILE=$(basename $BGPIC)
cp -vi ${BGPIC} ${MNTPATH}/.background/${BGFILE}
#ln -s /Applications ${MNTPATH}/Applications

echo '
   tell application "Finder"
     tell disk "'${VOLNAME}'"
	   open
	   set current view of container window to icon view
	   set toolbar visible of container window to false
	   set statusbar visible of container window to false
	   set the bounds of container window to {400, 200, 800, 440}
	   set theViewOptions to the icon view options of container window
	   set arrangement of theViewOptions to not arranged
	   set icon size of theViewOptions to 64
	   set background picture of theViewOptions to file ".background:'${BGFILE}'"
	   make new alias file at container window to POSIX file "/Applications" with properties {name:"Applications"}
	   set position of item "'${APPNAME}'" of container window to {100, 100}
	   set position of item "Applications" of container window to {310, 100}
	   close
	   open
	   update without registering applications
	   delay 5
	   eject
     end tell
   end tell
' | osascript

sync

# Umount the image
umount "${DiskDevice}"
hdiutil eject "${DiskDevice}"

# Create a read-only version, use zlib compression
hdiutil convert -format UDZO "${TMPFILE}" -imagekey zlib-level=9 -o "${DMGFILE}"

# Delete the temporary files
rm $TMPFILE
rmdir $MNTPATH

if [ -n "$UPLOAD" ];then
  scp $DMGFILE rg42.org:/var/sites/inout/docroot/software/
fi
