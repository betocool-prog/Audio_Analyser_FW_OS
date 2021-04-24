#!/bin/bash
#Script used to generate nanopb and python proto files

#export PATH="/home/betocool/protobuf-3.9.0/src:$PATH"
#export PYTHONPATH=":$PYTHONPATH"

protoc -oanalyser.pb --python_out=../python analyser.proto
#protoc --python_out=../python analyser.proto
python3.7 /home/betocool/nanopb-0.4.4-linux-x86/generator/nanopb_generator.py --verbose analyser.pb
