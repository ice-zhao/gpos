#!/bin/bash

BOCHS_BASE="/home/ice/gm/tools/bochs-2.6.11/image"
BOCHS="${BOCHS_BASE}/bin/bochs"
GPOS_BASE="/home/ice/gm/projects/GPOS"

${BOCHS} -f ${GPOS_BASE}/utils/bochsrc
