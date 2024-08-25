#!/bin/bash

#change to your bochs path that contains executable files.
BOCHS_BASE=""
BOCHS="${BOCHS_BASE}bochs"
GPOS_BASE="${PWD}"

${BOCHS} -f ${GPOS_BASE}/utils/bochsrc
