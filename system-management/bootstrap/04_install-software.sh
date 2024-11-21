#!/bin/bash

PACKAGES="systemd-timesyncd dbus vim locales basic-startup-conf-sd256-bookworm server-pm-control"

if [ $UID -ne 0 ]; then
    echo "Please run as root ..."
    exit 1
fi

echo "Installing userland software ..."

echo "Setting time read from time.txt"

timeToSet=$(cat time.txt)
date -s "${timeToSet}"

echo "Installing package repo list files."
cp apt-sources/*.list /etc/apt/sources.list.d/ || exit 1

echo "Updating package cache to take care for new lists"
apt-get update || exit 1

echo "Installing packages: ${PACKAGES}"
apt-get -y install ${PACKAGES} || exit 1

# rfs modifications
echo "Removing files from RFS ..."
files2remove=$(cat ./files-to-remove-from-rfs.conf)
rm -r ${files2remove}


# config files
echo "Installing configuration files ..."
cp -r patch_dir/* / || exit 1

echo "Done"
echo "Reboot to finish setup ..."
