#!/bin/bash

#change to your bochs path that contains executable files.
BOCHS_BASE="/home/ice/gm/tools/bochs-2.6.11/image"
BOCHS="${BOCHS_BASE}/bin/bochs"
GPOS_BASE="${PWD}"

${BOCHS} -f ${GPOS_BASE}/utils/bochsrc
