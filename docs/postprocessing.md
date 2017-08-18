DR.CHECKER results post processing
===================
## Convert to source annotated jsons
We provide a script that converts the DR.CHECKER jsons into source annotated jsons. 
The script to use is `helper_scripts/pp_scripts/pp_jsons.py`

```
python pp_jsons.py --help
usage: pp_jsons.py [-h] [-d DR_JSONS] [-o OUTPUT_JSONS] [-k KERNEL_SRC_DIR]
                   [-r ORIGINAL_SRC_DIR] [-n NEW_SRC_DIR]

Script that converts DR.CHECKER LLVM based jsons to jsons containing source
code info.

optional arguments:
  -h, --help           show this help message and exit
  -d DR_JSONS          Destination directory where all the DR.CHECKER jsons
                       should be read from.
  -o OUTPUT_JSONS      Destination directory where all the post-processed
                       jsons should be stored.
  -k KERNEL_SRC_DIR    Absolute path to the kernel sources.
  -r ORIGINAL_SRC_DIR  Path to the original kernel sources, that should be
                       replaced with alternate source dir.
  -n NEW_SRC_DIR       Path to the new kernel sources, that should be used
                       instead or original src dir.

```

Examples:
```
python pp_jsons.py -d ~/mediatek_kernel/dr_checker_out -o ~/mediatek_kernel/post_processed_jsons
[*]  Provided DR.CHECKER json dir: /home/drchecker/mediatek_kernel/dr_checker_out
[*]  Provided Output dir: /home/drchecker/mediatek_kernel/post_processed_jsons
[*]  Provided Kernel source dir: None
[*]  Provided original source dir: None
[*]  Provided alternate source dir: None
[*]  Processing all jsons: 123  in multiprocessing mode
[*]  Total time: 145.0116510391235  seconds.
```

