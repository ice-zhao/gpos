BS_OEMName:         .ascii  "WB. YANG"    /* OEM String, 8 bytes required */
BPB_BytsPerSec:     .2byte  512           /* Bytes per sector */
BPB_SecPerCluster:  .byte   1             /* Sector per cluster */
BPB_ResvdSecCnt:    .2byte  1             /* Reserved sector count */
BPB_NumFATs:        .byte   2             /* Number of FATs */
BPB_RootEntCnt:     .2byte  224           /* Root entries count */
BPB_TotSec16:       .2byte  2880          /* Total sector number */
BPB_Media:          .byte   0xf0          /* Media descriptor */
BPB_FATSz16:        .2byte  9             /* FAT size(sectors) */
BPB_SecPerTrk:      .2byte  18            /* Sector per track */
BPB_NumHeads:       .2byte  2             /* Number of magnetic heads */
BPB_HiddSec:        .4byte  0             /* Number of hidden sectors */
BPB_TotSec32:       .4byte  0             /* If TotSec16 equal 0, this works */
BS_DrvNum:          .byte   0             /* Driver number of interrupt 13 */
BS_Reserved1:       .byte   0             /* Reserved */
BS_BootSig:         .byte   0x29          /* Boot signal */
BS_VolID:           .4byte  0             /* Volume ID */
BS_VolLab:          .ascii  "Solrex 0.01" /* Volume label, 11 bytes required */
BS_FileSysType:     .ascii  "FAT12   "    /* File system type, 8 bytes required */

.equ    RootDirSectors,  14        /* Root directory sector count */
.equ    SecNoOfRootDir,  19        /* 1st sector of root directory */
.equ    SecNoOfFAT1,     1         /* 1st sector of FAT1 */
.equ    DeltaSecNo,      17        /* BPB_(RsvdSecCnt+NumFATs*FATSz) -2 */
