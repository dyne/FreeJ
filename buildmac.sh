#!/bin/sh
VERSION=0.10
DMGFILE=/tmp/flowmixer-$VERSION.dmg
echo $DMGFILE

BUILD=build/Release/flowmixer.app

cd xcode

#xcodebuild clean && \
xcodebuild \
||exit

#roll a DMG
TMPFILE=/tmp/freejtmp.dmg
MNTPATH=/tmp/mnt/
VOLNAME=FlowMixer

mkdir -p $MNTPATH
if [ -e $TMPFILE -o -e $DMGFILE -o ! -d $MNTPATH ]; then
  echo "could not make DMG.."
  exit;
fi

hdiutil create -megabytes 100 $TMPFILE
DiskDevice=$(hdid -nomount "${TMPFILE}" | grep Apple_HFS | cut -f 1 -d ' ')
newfs_hfs -v "${VOLNAME}" "${DiskDevice}"
mount -t hfs "${DiskDevice}" "${MNTPATH}"

cp -r ${BUILD} ${MNTPATH}/
ln -s /Applications ${MNTPATH}/Applications

# Umount the image
umount "${DiskDevice}"
hdiutil eject "${DiskDevice}"

# Create a read-only version, use zlib compression
hdiutil convert -format UDZO "${TMPFILE}" -o "${DMGFILE}"

# Delete the temporary files
rm $TMPFILE
rmdir $MNTPATH

if [ -n "$1" ];then
  scp $DMGFILE rg42.org:/var/sites/inout/docroot/software/
fi
