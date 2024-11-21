#!/bin/bash

DEBIAN_DISTRIBUTION=bookworm
DEBIAN_MIRROR=http://ftp2.de.debian.org/debian/
DEBIAN_COMPONENTS=main,contrib,non-free,non-free-firmware
KEY_RING=/usr/share/keyrings/debian-archive-bookworm-stable.gpg

ARCHITECTURE=armhf

KNL_PKG_DIR=../kernel/kpkg
BOOT_CNF_DIR=./boot-conf-files
RFS_DIR=/var/nfs/rfs

PACKAGES=systemd,bash,udev,openssh-server,ifupdown,kmod,net-tools,isc-dhcp-client,psmisc

HOSTNAME=homeserver-smpc

ROOT_PW_SHADOW_KEY='$y$j9T$nx8MkZ1DU9We5nn9ThGWT.$oMNeo0.k1.Jrpt4WaVVwFsB4pc.Ik0pJ5v0yETDlOQ8'
USER_NAME='joe'
USER_PW_SHADOW_KEY='$y$j9T$4Vsw/2c8J.WjZA5GVAnWS0$/ptrldtg9YWqMlEOQKeekoFmmZVuYLGXPMBmLu5BAN1'


if [ $UID -ne 0 ]; then
    echo "Please run as root ..."
    exit 1
fi

if [ ! -d $KNL_PKG_DIR ]; then
    echo "No kernel package found here: $KNL_PKG_DIR"
    echo "Please create kernel package using knl_0x tools."
    exit 1
fi

if [ ! -d $RFS_DIR ]; then
    echo "Please create a writable directory for the generated RFS: $RFS_DIR"
    exit 1
fi

echo "---------------- Creating minimal debian RFS for bootstrapping -------------------------------"
echo "Debian distro: $DEBIAN_DISTRIBUTION"
echo "Debian mirror: $DEBIAN_MIRROR"
echo "distro components: $DEBIAN_COMPONENTS" 
echo "Architecture: $ARCHITECTURE"
echo "Packages: $PACKAGES"
echo "Kernel package: $KNL_PKG_DIR"
echo "RFS directory: $RFS_DIR"
echo "----------------------------------------------------------------------------------------------"
echo
echo "Starting in 5 secs ..."

#sleep 5

#create rfs
echo "Removing everything from RFS directory ..."
rm -r $RFS_DIR/*
echo "Creating RFS ..."
debootstrap --keyring $KEY_RING --variant minbase --components $DEBIAN_COMPONENTS --include=$PACKAGES --arch=$ARCHITECTURE $DEBIAN_DISTRIBUTION $RFS_DIR $DEBIAN_MIRROR || exit 1

#installing kernel modules
echo "Installing kernel modules ..."
mkdir $RFS_DIR/usr/lib/modules 
tar -C $RFS_DIR/usr/lib/modules -x -f $KNL_PKG_DIR/modules.tgz

echo "Installing kernel image ..."
mkdir $RFS_DIR/boot
cp $KNL_PKG_DIR/image/zImage $RFS_DIR/boot/

echo "Installing device trees ..."
mkdir $RFS_DIR/boot/dtb 
cp $KNL_PKG_DIR/dtbs/* $RFS_DIR/boot/dtb

echo "Installing boot files ..."
cp $BOOT_CNF_DIR/* $RFS_DIR/boot


echo "Doing some modifications in the RFS ..."

#create symlink to systemd
ln -s /lib/systemd/systemd $RFS_DIR/sbin/init || exit 1

#set default root password
sed "s/root:\*/root:${ROOT_PW_SHADOW_KEY}/" $RFS_DIR/etc/shadow > $RFS_DIR/etc/shadow_x || exit 1
mv $RFS_DIR/etc/shadow_x $RFS_DIR/etc/shadow || exit 1

#add user 
mkdir $RFS_DIR/home/${USER_NAME} || exit 1
chown 1000:1000  $RFS_DIR/home/${USER_NAME} || exit 1
chmod 755 $RFS_DIR/home/${USER_NAME} || exit 1
echo "${USER_NAME}:x:1000:1000::/home/${USER_NAME}:/bin/bash" >> $RFS_DIR/etc/passwd || exit 1

#set user std password
echo "${USER_NAME}:${USER_PW_SHADOW_KEY}:17994:0:99999:7:::" >> $RFS_DIR/etc/shadow || exit 1

#install public key file for ssh
mkdir $RFS_DIR/home/${USER_NAME}/.ssh
cp files/id_rsa.pub $RFS_DIR/home/${USER_NAME}/.ssh/authorized_keys
chown 1000:1000 $RFS_DIR/home/${USER_NAME}/.ssh
chmod 700 $RFS_DIR/home/${USER_NAME}/.ssh
chown 1000:1000 $RFS_DIR/home/${USER_NAME}/.ssh/authorized_keys
chmod 600 $RFS_DIR/home/${USER_NAME}/.ssh/authorized_keys

#install .profile to user home
cp files/profile $RFS_DIR/home/${USER_NAME}/.profile
chown 1000:1000 $RFS_DIR/home/${USER_NAME}/.profile

#installing some configuration files
cp files/interfaces $RFS_DIR/etc/network/ || exit 1
#disabling ethernet renaming to stay w/ classical eth0
ln -s /dev/null $RFS_DIR/etc/udev/rules.d/80-net-setup-link.rules

#setting hostname
echo "${HOSTNAME}" > $RFS_DIR/etc/hostname

echo "Created RFS successfully in folder: $RFS_DIR"
