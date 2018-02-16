#!/bin/bash
source /etc/bash_completion
mkdir /dr_checker
cd /dr_checker
git clone https://github.com/ucsb-seclab/dr_checker.git gitrepo
cd gitrepo/helper_scripts
python setup_drchecker.py -o /dr_checker/drcheckerdeps
echo 'export LLVM_ROOT=/dr_checker/drcheckerdeps/llvm/build' >> ~/.bashrc
echo 'export PATH=$LLVM_ROOT/bin:/dr_checker/drcheckerdeps/sparse:$PATH' >> ~/.bashrc
source ~/.bashrc
cd /dr_checker/gitrepo/llvm_analysis
./build.sh
