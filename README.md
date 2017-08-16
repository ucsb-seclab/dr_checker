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
To run DR.CHECKER on kernel drivers, we need to first convert them into llvm bitcode.
### Building kernel
First, we need to have a buildable kernel. Which means we should be able to compile the kernel using regular build setup. i.e., make.
We first capture the output of `make` command, from this output we extract the extract compilation command.
#### Generating output of `make` (or `makeout.txt`)
Just pass `V=1` and redirect the output to the file.
Example:
```
make V=1 O=out ARCH=arm64 > makeout.txt 2>&1
```
### Running DR.CHECKER analysis
There are several steps to run DR.CHECKER analysis, all these steps are wrapped in a single script `helper_scripts/runner_scripts/run_all.py`
How to run:
```
python run_all.py --help
usage: run_all.py [-h] [-l LLVM_BC_OUT] [-a CHIPSET_NUM] [-m MAKEOUT] [-g COMPILER_NAME] [-n ARCH_NUM] [-o OUT] [-k KERNEL_SRC_DIR] [-skb] [-skl] [-skp] [-ske] [-ski] [-f SOUNDY_ANALYSIS_OUT]

optional arguments:
  -h, --help            show this help message and exit
  -l LLVM_BC_OUT        Destination directory where all the generated bitcode files should be stored.
  -a CHIPSET_NUM        Chipset number. Valid chipset numbers are:
                        1(mediatek)|2(qualcomm)|3(huawei)|4(samsung)
  -m MAKEOUT            Path to the makeout.txt file.
  -g COMPILER_NAME      Name of the compiler used in the makeout.txt, This is
                        needed to filter out compilation commands. Ex: aarch64-linux-android-gcc
  -n ARCH_NUM           Destination architecture, 32 bit (1) or 64 bit (2).
  -o OUT                Path to the out folder. This is the folder, which
                        could be used as output directory during compiling
                        some kernels.
  -k KERNEL_SRC_DIR     Base directory of the kernel sources.
  -skb                  Skip LLVM Build (default: not skipped).
  -skl                  Skip Dr Linker (default: not skipped).
  -skp                  Skip Parsing Headers (default: not skipped).
  -ske                  Skip Entry point identification (default: not
                        skipped).
  -ski                  Skip Soundy Analysis (default: not skipped).
  -f SOUNDY_ANALYSIS_OUT    Path to the output folder where the soundy analysis output should be stored.

```
