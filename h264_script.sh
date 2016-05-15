#!/bin/sh
#####################################################################
# go.sh
# Generated by bench-run.pl
# /home/students/adku7025/benchmarks/bin/bench-run.pl --bench spec-cpu2006:int:464.h264ref:train --build base --prefix pin -t obj-intel64/checkpoint1A_Gag.so -o h264_gag.out -l 10000000000 -- --copy output.out --log LOG
#####################################################################

#####################################################################
# Creating logfile
date >> LOG
#####################################################################

#####################################################################
# benchmark: spec-cpu2006/int/464.h264ref input: train 0
#####################################################################
# log file update
echo -n "Started : spec-cpu2006/int/464.h264ref input: train 0 at " >> LOG 
date >> LOG
# setup
ln -s /home/students/adku7025/benchmarks/src/spec-cpu2006/int/464.h264ref/builds/base/464.h264ref /home/students/adku7025/checkpoint-1A/464.h264ref
ln -s -f -n --target-directory=/home/students/adku7025/checkpoint-1A /home/students/adku7025/benchmarks/src/spec-cpu2006/int/464.h264ref/data/train/input/foreman_train_encoder_baseline.cfg /home/students/adku7025/benchmarks/src/spec-cpu2006/int/464.h264ref/data/all/input/foreman_qcif.yuv /home/students/adku7025/benchmarks/src/spec-cpu2006/int/464.h264ref/data/all/input/leakybucketrate.cfg
# setup experiment files
# run benchmark
pin -t obj-intel64/checkpoint1A_Gag.so -o h264_gag.out -l 10000000000 -- /home/students/adku7025/checkpoint-1A/464.h264ref -d foreman_train_encoder_baseline.cfg > foreman_train_baseline_encodelog.out 2> foreman_train_baseline_encodelog.err 
# verify benchmark
# post-copy benchmark
cp /home/students/adku7025/checkpoint-1A/output.out /home/students/adku7025/benchmarks/src/spec-cpu2006/int/464.h264ref/builds/base/output.out;
# run custom script
# clean benchmark
rm -f /home/students/adku7025/checkpoint-1A/464.h264ref
rm -r -f foreman_train_encoder_baseline.cfg foreman_qcif.yuv leakybucketrate.cfg data.txt foreman_qcif.264 foreman_train_baseline_encodelog.err foreman_train_baseline_encodelog.out foreman_train_baseline_leakybucketparam.cfg log.dat stats.dat
# clean experiment files
# log file update
echo -n "Finished: spec-cpu2006/int/464.h264ref input: train 0 at " >> LOG 
date >> LOG
#####################################################################
# Finishing logfile
date >> LOG
#####################################################################

