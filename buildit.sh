#!/bin/bash

CPU=${CPU:-"000"}			# assign defaults if not set
DEBUG=${DEBUG:-"yes"}

echo "\$CPU=$CPU"
echo "\$DEBUG=$DEBUG"

FVDI_DIR=fvdi

FT2_DIR=${FVDI_DIR}/modules/ft2
BPL_DIR=${FVDI_DIR}/drivers/bitplane
RAD_DIR=${FVDI_DIR}/drivers/radeon
FBE_DIR=${FVDI_DIR}/drivers/firebee
ENG_DIR=${FVDI_DIR}/engine
BIT_DIR=${FVDI_DIR}/drivers/16_bit

TARGET_DIR=../fvdi_test

rm -f bitplane.sys
rm -f radeon.sys
rm -f firebee.sys
rm -f fvdi_gnu.prg

(cd ${FT2_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make clean) || exit
(cd ${FT2_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make -j) || exit

(cd ${ENG_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make clean) || exit
(cd ${ENG_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make -j) || exit

(cd ${BPL_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make clean) || exit
(cd ${BPL_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make -j) || exit

(cd ${BIT_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make clean) || exit
(cd ${BIT_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make -j) || exit


#(cd ${RAD_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make clean) || exit
#(cd ${RAD_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make -j) || exit

#(cd ${FBE_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make clean) || exit
#(cd ${FBE_DIR}; DEBUG=${DEBUG} CPU=${CPU} M68K_ATARI_MINT_CROSS=yes make -j) || exit

cp ${BPL_DIR}/bitplane.sys .
cp ${ENG_DIR}/fvdi_gnu.prg .
#cp ${RAD_DIR}/radeon.sys .
#cp ${FBE_DIR}/firebee.sys .
cp ${BIT_DIR}/16_bit.sys .

#m68k-atari-mint-strip bitplane.sys
#m68k-atari-mint-strip fvdi_gnu.prg

# copy files to test directory if existing and if CPU is supported by hatari
#if [ ${CPU} = '020' -o ${CPU} = '000' ]; then
#    cp bitplane.sys ${TARGET_DIR}/gemsys
#    cp fvdi_gnu.prg ${TARGET_DIR}/auto
#fi

