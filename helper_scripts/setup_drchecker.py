"""
This script clones and setups llvm and friends in the provided folder.
"""

import argparse
from multiprocessing import cpu_count
import os
import sys


def log_info(*args):
    log_str = "[*] "
    for curr_a in args:
        log_str = log_str + " " + str(curr_a)
    print log_str


def log_error(*args):
    log_str = "[!] "
    for curr_a in args:
        log_str = log_str + " " + str(curr_a)
    print log_str


def log_warning(*args):
    log_str = "[?] "
    for curr_a in args:
        log_str = log_str + " " + str(curr_a)
    print log_str


def log_success(*args):
    log_str = "[+] "
    for curr_a in args:
        log_str = log_str + " " + str(curr_a)
    print log_str


def execute_command(cmd):
    if os.system(cmd) != 0:
        log_error("Command {} failed".format(cmd))
        exit(1)

LLVM_GIT_HUB_BASE = "https://github.com/llvm/llvm-project.git"
SPARSE_URL = "git://git.kernel.org/pub/scm/devel/sparse/sparse.git"


def setup_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('-b', action='store', dest='target_branch',
                        default='release/3.8.x',
                        help='Branch (i.e. version) of the LLVM to setup. Default: release/3.8.x')

    parser.add_argument('-o', action='store', dest='output_folder',
                        help='Folder where everything needs to be setup.')

    return parser


def usage():
    log_error("Invalid Usage.")
    log_error("Run: python ", __file__, "--help", ", to know the correct usage.")
    sys.exit(-1)


def main():
    arg_parser = setup_args()
    parsed_args = arg_parser.parse_args()
    # step 1: Setup common dictionary
    reps_to_setup = ['clang', 'compiler-rt', 'libcxx', 'libcxxabi', 'openmp']
    if parsed_args.output_folder is None:
        usage()
    sparse_dir = os.path.join(parsed_args.output_folder, "sparse")
    base_output_dir = os.path.join(parsed_args.output_folder, "llvm")
    target_branch = parsed_args.target_branch
    log_info("Preparing setup in:", parsed_args.output_folder)
    backup_dir = os.getcwd()
    if not os.path.exists(sparse_dir):
        log_info("Cloning sparse")
        execute_command('mkdir -p ' + sparse_dir)
        execute_command("git clone " + SPARSE_URL + " " + sparse_dir)
    os.chdir(sparse_dir)
    execute_command("make")
    os.chdir(backup_dir)
    log_success("Successfully built sparse.")
    if not os.path.exists(base_output_dir):
        log_info("Cloning LLVM")
        execute_command('mkdir -p ' + str(base_output_dir))
        git_clone_cmd = "git clone " + LLVM_GIT_HUB_BASE + " -b " + \
                        str(target_branch) + " " + base_output_dir
        log_info("Setting up llvm.")
        execute_command(git_clone_cmd)

        log_success("Cloned all the repositories.")

    build_dir = os.path.join(base_output_dir, "build")
    log_info("Trying to build in ", build_dir)
    if not os.path.exists(build_dir):
        execute_command('mkdir -p ' + build_dir)
    os.chdir(build_dir)
    execute_command('cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS=\'{}\' ../llvm'.format(";".join(reps_to_setup)))
    multi_proc_count = cpu_count()
    if multi_proc_count > 0:
        log_info("Building in multiprocessing mode on ", multi_proc_count, " cores.")
        execute_command('cmake --build . -- -j' + str(multi_proc_count))
    else:
        log_info("Building in single core mode.")
        execute_command('cmake --build .')
    log_success("Build Complete.")
    print ""
    log_success("Add following lines to your .bashrc")
    print("export LLVM_ROOT=" + os.path.abspath(build_dir))
    print("export PATH=$LLVM_ROOT/bin:" + os.path.abspath(sparse_dir) + ":$PATH")
    print ""
    log_success("After adding the above lines to .bashrc.\nPlease run: source ~/.bashrc on the "
                "terminal for the changes to take effect.")
    print ""
    log_success("Setup Complete.")
    os.chdir(backup_dir)


if __name__ == "__main__":
    main()
