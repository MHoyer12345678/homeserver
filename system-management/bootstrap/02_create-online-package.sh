#!/bin/bash

RFS_DIR=/var/nfs/rfs
ONLINE_PKG_DIR=$RFS_DIR/root/online-package

APT_SRCS_DST_DIR=$ONLINE_PKG_DIR/apt-sources
APT_SRCS_FILES=files/*.list

PATCH_FILES_DST_DIR=$ONLINE_PKG_DIR/patch_dir
PATCH_FILES=patch_dir/*

ADDITIONAL_FILES="files/files-to-remove-from-rfs.conf"

TIMESTAMP_DST=$ONLINE_PKG_DIR/time.txt

ONLINE_SCRIPTS="04_install-software.sh"

if [ $UID -ne 0 ]; then
    echo "Please run as root ..."
    exit 1
fi

echo "---------------- Putting Online Package into bootstrap RFS -----------------------------------"
echo "Online package dir: $ONLINE_PKG_DIR"
echo "----------------------------------------------------------------------------------------------"

echo "Going to empty existing online package in 5 secs"

sleep 5

# preparing online package dir
echo "Emptying existing package"

rm -r $ONLINE_PKG_DIR

echo "Creating new package"
mkdir $ONLINE_PKG_DIR || exit 1

# installing apt sources configs
echo "Adding apt sources to package ..."
mkdir -p $APT_SRCS_DST_DIR || exit 1
cp $APT_SRCS_FILES $APT_SRCS_DST_DIR/ || exit 1

# installing files to patch the rfs
echo "Adding files to patch to package ..."
mkdir -p $PATCH_FILES_DST_DIR || exit 1
cp -r $PATCH_FILES $PATCH_FILES_DST_DIR/ || exit 1

# installing online scripts
echo "Adding online scripts ..."
cp $ONLINE_SCRIPTS $ONLINE_PKG_DIR/ || exit 1

# copying additional files to online package
echo "Adding additional files ..."
cp $ADDITIONAL_FILES  $ONLINE_PKG_DIR/ || exit 1

echo "Adding time stamp to online package ..."
date -R > $TIMESTAMP_DST || exit 1

echo "Done"
