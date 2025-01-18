#!/bin/bash
sudo insmod vtfs.ko
sudo mkdir /mnt/vt
sudo mount -t vtfs "TODO" /mnt/vt
