#!/bin/bash
source /etc/bash_completion
mkdir /dr_checker
cd /dr_checker
git clone https://github.com/ucsb-seclab/dr_checker.git gitrepo
cd /dr_checker/gitrepo/visualizer
npm install -g serve
rm ./client/package-lock.json
npm --prefix ./client install ./client
npm --prefix ./client run build ./client