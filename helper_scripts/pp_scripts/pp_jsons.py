import argparse
import multiprocessing
import os
import time
import json


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


def setup_args():
    parser = argparse.ArgumentParser(description="Script that converts DR.CHECKER LLVM based jsons to jsons "
                                                 "containing source code info.")

    parser.add_argument('-d', action='store', dest='dr_jsons',
                        help='Destination directory where all the DR.CHECKER jsons should be read from.')

    parser.add_argument('-o', action='store', dest='output_jsons',
                        help='Destination directory where all the post-processed jsons should be stored.')

    parser.add_argument('-k', action='store', dest='kernel_src_dir', default=None,
                        help='Absolute path to the kernel sources.')

    parser.add_argument('-r', action='store', dest='original_src_dir', default=None,
                        help='Path to the original kernel sources, that should be replaced with alternate source dir.')

    parser.add_argument('-n', action='store', dest='new_src_dir', default=None,
                        help='Path to the new kernel sources, that should be used instead or original src dir.')

    return parser


def read_file_line(lineno, given_file, k_src, o_src, a_src):
    """
        Read the source line from the provided llvm line info
    :param lineno: line number to read from.
    :param given_file: file path in the original json
    :param k_src: Absolute path to the kernel sources.
    :param o_src: Path to the original kernel sources, that should be replaced with alternate source dir.
    :param a_src: Path to the new kernel sources, that should be used instead or original src dir.
    :return: None
    """
    if lineno <= 0:
        return ""

    if o_src is not None and a_src is not None:
        if os.path.isabs(given_file):
            given_file = given_file.replace(o_src, a_src)

    if not os.path.isabs(given_file):
        if k_src is not None:
            given_file = os.path.join(k_src, given_file)

    if os.path.exists(given_file) and os.path.isfile(given_file):
        fp = open(given_file, "r")
        all_lines = fp.readlines()
        to_ret = ""
        if len(all_lines) >= lineno:
            to_ret = all_lines[lineno-1].strip()
        return to_ret
    return ""


def process_instr(instr_obj, k_src, o_src, a_src):
    """
        Process the instruction object.
    :param instr_obj: Current instruction object to process.
    :param k_src: Absolute path to the kernel sources.
    :param o_src: Path to the original kernel sources, that should be replaced with alternate source dir.
    :param a_src: Path to the new kernel sources, that should be used instead or original src dir.
    :return: None
    """

    line_no = -1
    if "lineno" in instr_obj:
        line_no = instr_obj["lineno"]
    if "instr_loc" in instr_obj:
        line_no = instr_obj["instr_loc"]

    instr_file = None
    if "file" in instr_obj:
        instr_file = instr_obj["file"]
    if "instr_file" in instr_obj:
        instr_file = instr_obj["instr_file"]

    target_instr = None
    if instr_file is not None and not instr_file.endswith(".h"):
        curr_l = read_file_line(line_no, instr_file, k_src, o_src, a_src)
        if curr_l:
            target_instr = curr_l

    if target_instr is not None:
        if "instr" in instr_obj:
            instr_obj["instr"] = target_instr


def process_warning(src_warnings_obj, k_src, o_src, a_src):
    """
        Process the current warning obj.
    :param src_warnings_obj: Warning obj to process.
    :param k_src: Absolute path to the kernel sources.
    :param o_src: Path to the original kernel sources, that should be replaced with alternate source dir.
    :param a_src: Path to the new kernel sources, that should be used instead or original src dir.
    :return: None
    """
    curr_data = src_warnings_obj["warn_data"]
    curr_line = curr_data["at_line"]
    if "at_file" in curr_data:
        curr_file = curr_data["at_file"]
        if not curr_file.endswith(".h"):
            curr_cont = read_file_line(curr_line, curr_file, k_src, o_src, a_src)
            curr_data["at"] = curr_cont

    for curr_ins in curr_data["inst_trace"]:
        process_instr(curr_ins, k_src, o_src, a_src)


def process_context(context_obj, k_src, o_src, a_src):
    """
        Context obj.
    :param context_obj: context info to process
    :param k_src: Absolute path to the kernel sources.
    :param o_src: Path to the original kernel sources, that should be replaced with alternate source dir.
    :param a_src: Path to the new kernel sources, that should be used instead or original src dir.
    :return: None
    """
    for curr_in in context_obj:
        process_instr(curr_in, k_src, o_src, a_src)


def process_all_context(src_context_obj, k_src, o_src, a_src, fp):
    """
        Process the current context object.
    :param src_context_obj: current object to process.
    :param k_src: Absolute path to the kernel sources.
    :param o_src: Path to the original kernel sources, that should be replaced with alternate source dir.
    :param a_src: Path to the new kernel sources, that should be used instead or original src dir.
    :param fp: Output file descriptor.
    :return: None
    """
    process_context(src_context_obj["context"], k_src, o_src, a_src)
    fp.write("{\"num_warnings\": " + str(src_context_obj["num_warnings"]) + ",\n")
    fp.write("\"context\":" + json.dumps(src_context_obj["context"]) + ",\n")
    fp.write("\"warnings\":[")
    add_comma = False
    for curr_warning in src_context_obj["warnings"]:
        if add_comma:
            fp.write(",\n")
        process_warning(curr_warning, k_src, o_src, a_src)
        fp.write(json.dumps(curr_warning))
        add_comma = True
    fp.write("]}")


def process_json((src_json, output_json, ker_src, orig_src, alter_src)):
    """
        Process the json

    :return: None
    """
    fp = open(src_json, "r")
    fp_cont = fp.read()
    fp.close()
    if len(fp_cont) > 0:
        json_obj = json.loads(fp_cont)
        if "all_contexts" in json_obj:
            fp = open(output_json, "w")
            all_contexts = json_obj["all_contexts"]
            fp.write("{\"all_contexts\":[\n")
            add_comma = False
            for curr_context in all_contexts:
                if add_comma:
                    fp.write(",\n")
                process_all_context(curr_context, ker_src, orig_src, alter_src, fp)
                # fp.write(json.dumps(curr_context))
                add_comma = True
            fp.write("]}")
            fp.close()


def main():
    arg_parser = setup_args()
    parsed_args = arg_parser.parse_args()

    kernel_dir = None
    if parsed_args.kernel_src_dir is not None:
        kernel_dir = os.path.realpath(parsed_args.kernel_src_dir)

    origianl_src_dir = None
    if parsed_args.original_src_dir is not None:
        origianl_src_dir = os.path.realpath(parsed_args.original_src_dir)

    alternate_src_dir = None
    if parsed_args.new_src_dir is not None:
        alternate_src_dir = os.path.realpath(parsed_args.new_src_dir)

    log_info("Provided DR.CHECKER json dir:", parsed_args.dr_jsons)
    log_info("Provided Output dir:", parsed_args.output_jsons)
    log_info("Provided Kernel source dir:", kernel_dir)
    log_info("Provided original source dir:", origianl_src_dir)
    log_info("Provided alternate source dir:", alternate_src_dir)

    # create output directory
    os.system('mkdir -p ' + parsed_args.output_jsons)

    all_tasks = []
    for curr_json in os.listdir(parsed_args.dr_jsons):
        c_fp = os.path.join(parsed_args.dr_jsons, curr_json)
        if os.path.isfile(c_fp) and curr_json.endswith(".json"):
            output_p = os.path.join(parsed_args.output_jsons, curr_json)
            all_tasks.append((c_fp, output_p, kernel_dir, origianl_src_dir, alternate_src_dir))

    log_info("Processing all jsons:", len(all_tasks), " in multiprocessing mode")
    p = multiprocessing.Pool()
    st = time.time()
    p.map(process_json, all_tasks)
    et = time.time() - st
    log_info("Total time:", et, " seconds.")


if __name__ == "__main__":
    main()
