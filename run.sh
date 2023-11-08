#!/bin/bash

if [[ $1 = '-d' ]]; then
   qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -soundhw pcspk -S -gdb tcp::1234 
  #qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -S -gdb tcp::1234 
else
   qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -soundhw pcspk
  #qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512
fi
