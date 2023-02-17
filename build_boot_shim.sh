#!/bin/bash

cd BootShim
make UEFI_BASE=0x11808000 UEFI_SIZE=0x00200000
cd ..
