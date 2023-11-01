#!/bin/bash
CWD=${PWD}
cd ${CWD} && test -d middleware/MTK/dspalg/clk_skew_protected && cd middleware/MTK/dspalg/clk_skew_protected&&make clean&&make&&make prebuilt_install&&make clean
cd ${CWD} && test -d middleware/airoha_protected/AAC_decoder/Xplorer/XT_lib_pic && cd middleware/airoha_protected/AAC_decoder/Xplorer/XT_lib_pic && make clean && make && make prebuilt_install && make clean
cd ${CWD} && test -d middleware/airoha_protected/compander/xtlib_pic && cd middleware/airoha_protected/compander/xtlib_pic && make clean && make && make prebuilt_install && make clean
cd ${CWD} && test -d middleware/airoha_protected/ECNR/xtlib_pic && cd middleware/airoha_protected/ECNR/xtlib_pic && make clean&&make && make prebuilt_install && make clean
cd ${CWD} && test -d middleware/airoha_protected/mSBC_encoder/mSBC_enc_XT_v13/xtlib_pic && cd middleware/airoha_protected/mSBC_encoder/mSBC_enc_XT_v13/xtlib_pic && make clean && make && make prebuilt_install && make clean
cd ${CWD} && test -d middleware/airoha_protected/PLC/PLC_XT_v32/xtlib_pic && cd middleware/airoha_protected/PLC/PLC_XT_v32/xtlib_pic && make clean && make && make prebuilt_install && make clean
cd ${CWD} && test -d middleware/airoha_protected/sampler_by2/xtlib &&  cd middleware/airoha_protected/sampler_by2/xtlib && make clean && make && make prebuilt_install && make clean
cd ${CWD} && test -d middleware/airoha_protected/SBC_decoder/SBC_decoder_Xtensa_X3in1_V02/xtlib_pic && cd middleware/airoha_protected/SBC_decoder/SBC_decoder_Xtensa_X3in1_V02/xtlib_pic && make clean&&make && make prebuilt_install&& make clean
cd ${CWD} && test -d middleware/airoha_protected/SBC_decoder/SBC_decoder_Xtensa_X3in1_V02/xtlib_pic && cd middleware/airoha_protected/SBC_decoder/SBC_decoder_Xtensa_X3in1_V02/xtlib_pic && make -f mSBC.mak clean&&make -f mSBC.mak && make -f mSBC.mak prebuilt_install&& make -f mSBC.mak clean
cd ${CWD} && test -d middleware/airoha_protected/SkewCtrl/xtlib_pic &&  cd middleware/airoha_protected/SkewCtrl/xtlib_pic && make clean && make && make prebuilt_install && make clean
