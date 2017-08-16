## Running DR.CHECKER on individual entry points.
Instead of running DR.CHECKER on the drivers, You can just run DR.CHECKER on individual entry points.

First, While running `run_all.py`, pass `-ski` so that it does not run DR.CHECKER on all the drivers.

Next, Refer the entry point configuration i.e., `entry_point_out.txt`, and identify the entry point you want to run DR.CHECKER on.
Consider that you have following entried in your `entry_point_out.txt`:
```
FileRead:hidraw_read:/home/drchecker/33.2.A.3.123/llvm_bc_out/drivers/hid/llvm_link_final/final_to_check.bc
FileWrite:hidraw_write:/home/drchecker/33.2.A.3.123/llvm_bc_out/drivers/hid/llvm_link_final/final_to_check.bc
IOCTL:hidraw_ioctl:/home/drchecker/33.2.A.3.123/llvm_bc_out/drivers/hid/llvm_link_final/final_to_check.bc
```

Say, you want to run DR.CHECKER on `hidraw_ioctl`.
```
cd <repo_dir>/llvm_analysis/MainAnalysisPasses/build_dir/SoundyAliasAnalysis
# running the dr.checker pass
opt -load ./libSoundyAliasAnalysis.so -dr_checker -toCheckFunction="hidraw_ioctl" -functionType="IOCTL" -outputFile="hidraw_ioctl.drcheck.json" /home/drchecker/33.2.A.3.123/llvm_bc_out/drivers/hid/llvm_link_final/final_to_check.bc
```
After the above command finishes, all warnings will be in the file: `hidraw_ioctl.drcheck.json`.
