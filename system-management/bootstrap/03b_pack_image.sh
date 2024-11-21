#!/bin/bash

RFS_DIR=/var/nfs/rfs
IMAGE_FILE=image.tar

if [ $UID -ne 0 ]; then
    echo "Please run as root ..."
    exit 1
fi

if [ $# -ge 1 ]; then
    IMAGE_FILE=$1
    exit 1
fi

echo "Packing RFS from $RFS_DIR into image $IMAGE_FILE."

tar -c -C $RFS_DIR -f $IMAGE_FILE .

