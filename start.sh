#!/bin/bash

i2cset -y 0 0x74 2 0x00

i2cset -y 0 0x74 3 0x00

i2cset -y 0 0x74 6 0x05

i2cset -y 0 0x74 7 0x00

i2cset -y 0 0x74 2 0x0A

i2cset -y 0 0x74 7 0x00

i2cset -y 0 0x74 3 0x88

#./main power
./main reset reset reset_fpga
echo SUCCESS!
echo END
