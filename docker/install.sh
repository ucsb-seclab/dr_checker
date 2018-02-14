#!/bin/bash
source /etc/bash_completion
mkdir ~/dr_checker
cd ~/dr_checker
git clone git@github.com:ucsb-seclab/dr_checker.git gitrepo
cd gitrepo
# setup
cd helper_scripts
python setup_drchecker.py -o /home/drchecker/dr_checker/drcheckerdeps
echo 'export LLVM_ROOT=/home/drchecker/dr_checker/drcheckerdeps/llvm/build' >> ~/.bashrc
echo 'export PATH=$LLVM_ROOT/bin:/home/drchecker/dr_checker/drcheckerdeps/sparse:$PATH' >> ~/.bashrc
# setup virtualization
mkvirtualenv drchecker_venv
workon drchecker_venv
cd ~/dr_checker/gitrepo/visualizer
pip install -r ./server/requirements.txt
curl -sL https://deb.nodesource.com/setup_8.x | sudo -E bash -
sudo apt-get -y install nodejs
sudo apt-get -y install npm
npm install -g serve
npm --prefix ./client install ./client
npm --prefix ./client run build ./client
