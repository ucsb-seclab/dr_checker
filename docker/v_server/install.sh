#!/bin/bash
source /etc/bash_completion
mkdir /dr_checker
cd /dr_checker
git clone https://github.com/ucsb-seclab/dr_checker.git gitrepo
cd /dr_checker/gitrepo/visualizer
pip install -r ./server/requirements.txt