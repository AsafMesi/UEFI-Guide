#!/bin/bash

qemu-system-x86_64 \
  -bios /usr/share/OVMF/OVMF.fd \
  -drive file=fat.img,format=raw,if=virtio \
  -net none \
  -nographic
