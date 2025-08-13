#!/bin/bash
sudo insmod snfs.ko
sudo mkdir /mnt/sn
sudo mount -t snfs "TKN" /mnt/sn
