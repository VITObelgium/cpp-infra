#!/bin/bash

export LD_LIBRARY_PATH=/tools/hdf5/1.8.8-7/x86_64/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=../../../depend/apache-log4cxx-0.10.0/build/lib:$LD_LIBRARY_PATH
./opaq --cnf opaq-config.xml

