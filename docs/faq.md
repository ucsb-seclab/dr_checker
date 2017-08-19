# DR.CHECKER FAQ #

### The `run_all.py` script throws errors, what should I do? ###

![Sample LLVM Build Error](https://github.com/ucsb-seclab/dr_checker/blob/master/docs/builderror.png)
Yes, it is common to see `run_all.py` throwing errors during llvm build stage.  These errors are because, `llvm` fails to compile `.S` files. As most of these files are kernel internal, we need to worry about them.

tl;dr its fine.
