"""
List of all the settings of our server.
"""
import os

DEBUG = False
# allow running on external interfaces
RUN_REMOTE = True

# dr_checker's output folder
RESULTS_DIR = "/dockershare/outputjson" if os.path.exists("/dockershare/outputjson") else "/path/to/the/directory/containing/jsons"

# this flag indicates if the path of the file name in the jsons should be replaced.
# all paths that start with PATH_TO_BE_REPLACED will be replaced with SOURCECODE_DIR
REPLACE_KERNEL_SRC = False
# New source code directory, which should be used to replace the below path
SOURCECODE_DIR = "/path/to/the/new/kernel/srcs/which/should/be/used"
# kernel src path that should be replaced with SOURCECODE_DIR
PATH_TO_BE_REPLACED = "/path/of/the/kernel/srcs/which/needs/to/be/replaced"
