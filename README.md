# DR.CHECKER : A Soundy Analysis for Linux Kernel Drivers
This repo contains all the sources, including setup scripts.
### Tested on
Ubuntu >= 14.04.5 LTS

## Setup
Our implementation is based on LLVM, specifically LLVM 3.8. We also need tools like `c2xml` to parse headers.
We have created a single script, which downloads and builds all the required tools.
```
cd helper_scripts
python setup_drchecker.py --help
usage: setup_drchecker.py [-h] [-b TARGET_BRANCH] [-o OUTPUT_FOLDER]

optional arguments:
  -h, --help        show this help message and exit
  -b TARGET_BRANCH  Branch (i.e version) of the LLVM to setup. Default:
                    release_38 e.g., release_38
  -o OUTPUT_FOLDER  Folder where everything needs to be setup.

```
Example:
```
python setup_drchecker.py -o drchecker_deps
```
To complete the setup you also need modifications to your local `PATH` environment variable. The setup script will give you exact changes you need to do.
## Building
This depends on the successful completion of [Setup](#markdown-header-setup).
We have a single script that builds everything, you are welcome.
```
cd llvm_analysis
./build.sh
```
## Running
This depends on the successful completion of [Build](#markdown-header-building).
### Building kernel
### Running dr_checker analysis
