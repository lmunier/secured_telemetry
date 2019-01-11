#!/bin/bash
# This file is used to set some limitations on the cpu and memory

#lm071218.1350

sudo cgcreate -a $USER:$USER -t $USER:$USER -g memory,cpu:test_pixhawk
cgset -r memory.limit_in_bytes=$((256*1024)) test_pixhawk
cgset -r cpu.cfs_quota_us=$((60000)) test_pixhawk
cgset -r cpu.cfs_period_us=$((1000000)) test_pixhawk
