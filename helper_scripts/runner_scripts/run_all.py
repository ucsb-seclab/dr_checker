import argparse
from components import *
import time


def setup_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('-l', action='store', dest='llvm_bc_out',
                        help='Destination directory where all the generated bitcode files should be stored.')

    parser.add_argument('-a', action='store', dest='chipset_num',
                        help='Chipset number. Valid chipset numbers are: 1(mediatek)|2(qualcomm)|3(huawei)|4(samsung)',
                        type=int)
    parser.add_argument('-m', action='store', dest='makeout',
                        help='Path to the makeout.txt file.')

    parser.add_argument('-g', action='store', dest='compiler_name',
                        help='Name of the compiler used in the makeout.txt, '
                             'This is needed to filter out compilation commands. Ex: aarch64-linux-android-gcc')

    parser.add_argument('-n', action='store', dest='arch_num',
                        help='Destination architecture, 32 bit (1) or 64 bit (2).',
                        type=int)

    parser.add_argument('-o', action='store', dest='out', default=None,
                        help='Path to the out folder. This is the folder, which could be used as '
                             'output directory during compiling some kernels.')

    parser.add_argument('-k', action='store', dest='kernel_src_dir',
                        help='Base directory of the kernel sources.')

    parser.add_argument('-skb', action='store_true', dest='skip_llvm_build', default=False,
                        help='Skip LLVM Build (default: not skipped).')

    parser.add_argument('-skl', action='store_true', dest='skip_dr_linker', default=False,
                        help='Skip Dr Linker (default: not skipped).')

    parser.add_argument('-skp', action='store_true', dest='skip_parse_headers', default=False,
                        help='Skip Parsing Headers (default: not skipped).')

    parser.add_argument('-ske', action='store_true', dest='skip_entry_identifier', default=False,
                        help='Skip Entry point identification (default: not skipped).')

    parser.add_argument('-skI', action='store_true', dest='skip_generate_includes', default=False,
                        help='Skip Generate Includes (default: not skipped).')

    parser.add_argument('-ski', action='store_true', dest='skip_soundy_checker', default=False,
                        help='Skip Soundy Analysis (default: not skipped).')

    parser.add_argument('-f', action='store', dest='soundy_analysis_out',
                        help='Path to the output folder where the soundy analysis output should be stored.')

    return parser


def get_bin_path(bin_name):
    out_p = subprocess.check_output('which ' + bin_name, shell=True)
    return out_p.strip()


def main():
    arg_parser = setup_args()
    parsed_args = arg_parser.parse_args()
    arg_dict = dict()
    utils_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
    utils_dir = os.path.join(utils_dir, "../../llvm_analysis")
    ana_helpers = os.path.join(utils_dir, "AnalysisHelpers")
    passes_dir = os.path.join(utils_dir, "MainAnalysisPasses")
    # step 1: Setup common dictionary
    arg_dict['llvm_bc_out'] = parsed_args.llvm_bc_out
    arg_dict['dr_link_bin'] = os.path.join(ana_helpers, "Dr_linker/dr_linker")
    arg_dict['chipset_num'] = parsed_args.chipset_num
    arg_dict['makeout'] = parsed_args.makeout
    arg_dict['clangbin'] = get_bin_path('clang')
    arg_dict['compiler_name'] = parsed_args.compiler_name
    arg_dict['arch_num'] = parsed_args.arch_num
    arg_dict['out'] = parsed_args.out
    arg_dict['c2xml_bin'] = get_bin_path('c2xml')
    arg_dict['kernel_src_dir'] = parsed_args.kernel_src_dir
    arg_dict['ep_finder_bin'] = os.path.join(ana_helpers, "EntryPointIdentifier/entry_point_handler")
    arg_dict['opt_bin_path'] = get_bin_path('opt')
    arg_dict['soundy_analysis_so'] = os.path.join(passes_dir, "SoundyAliasAnalysis/libSoundyAliasAnalysis.so")
    arg_dict['soundy_analysis_out'] = parsed_args.soundy_analysis_out
    __add_temp_files(arg_dict)

    component_times = {}

    # set up all the components that need to run
    target_components = list()
    if not parsed_args.skip_llvm_build:
        target_components.append(LLVMBuild(arg_dict))
    if not parsed_args.skip_dr_linker:
        target_components.append(DriverLinker(arg_dict))
    if not parsed_args.skip_generate_includes:
        target_components.append(GenerateIncludes(arg_dict))
    if not parsed_args.skip_parse_headers:
        target_components.append(ParseHeaders(arg_dict))
    if not parsed_args.skip_entry_identifier:
        target_components.append(EntryPointIdentifier(arg_dict))
    if not parsed_args.skip_soundy_checker:
        target_components.append(SoundyAnalysisRunner(arg_dict))

    for curr_comp in target_components:
        component_name = curr_comp.get_name()
        log_info("Trying to Run Component:", component_name)
        ret_val = __run_component(curr_comp, component_times)
        if ret_val:
            log_success("Component:", component_name, " passed successfully.")
        else:
            log_error("Component:", component_name, " failed. Exiting.")

    log_info("Component Runtime information:")
    for curr_comp in component_times:
        log_info(curr_comp + ":" + str(component_times[curr_comp]) + " seconds.")


def __run_component(component_obj, component_times):
    """
        Run provided component.
        This function takes care of running setup, performing the component.
        It takes of ignoring the error, if the component is non-critical.
    :param component_obj: Component object to be run.
    :param component_times: Dictionary in which each components times 
    should be recorded.
    :return: True if component ran fine else False.
    """
    setup_msg = component_obj.setup()
    if setup_msg is None:
        log_success("Setup for component:", component_obj.get_name(), " complete")
        st_time = time.time()
        ret_val = component_obj.perform()
        total_time = time.time() - st_time
        component_times[component_obj.get_name()] = total_time
        if ret_val:
            log_success("Component:", component_obj.get_name(), " ran successfully.")
            return True
        else:
            log_warning("Component:", component_obj.get_name(), " failed.")
            # Ignore if the component is not critical.
            if not component_obj.is_critical():
                return True
    else:
        log_error("Setup failed for component:", component_obj.get_name(), ", with Error:", setup_msg)
    return False


def __add_temp_files(target_dict):
    """
        Add temp files that will be used by some components to put their output files.
    :param target_dict: target dictionary to which the file paths need to be added.
    :return: None
    """
    target_dict['entry_point_out'] = os.path.join(target_dict['llvm_bc_out'],  'entry_point_out.txt')
    target_dict['hdr_file_list'] = os.path.join(target_dict['llvm_bc_out'],  'hdr_file_config.txt')

if __name__ == "__main__":
    main()
