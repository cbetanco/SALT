#!/bin/bash
# -------------------------------------------------------
#  find last created directory: format NNNN
# -------------------------------------------------------
lastdir=$(ls -d ASIC[0-9][0-9][0-9][0-9] | tail -1)
lastdirNum=${lastdir#ASIC}
# -------------------------------------------------------
#  create new sequential directory
# -------------------------------------------------------
newdirNum=$((++lastdirNum))
newDir=$(printf "ASIC%04u" $newdirNum)
#mkdir $newDir
./main $newDir $1
