from base_component import *
import os
from multiprocessing import Pool, cpu_count
import json


class ComputeWarningStats(Component):
    """
        Component which computes the stats of warnings.
    """

    def __init__(self, value_dict):
        soundy_analysis_out = None
        output_stats_csv = None
        if 'soundy_analysis_out' in value_dict:
            soundy_analysis_out = value_dict['soundy_analysis_out']
        if 'total_warning_stats' in value_dict:
            output_stats_csv = value_dict['total_warning_stats']

        self.soundy_analysis_out = soundy_analysis_out
        self.output_stats_csv = output_stats_csv

    def setup(self):
        if not os.path.isdir(self.soundy_analysis_out):
            return "Provided Soundy Analysis out path:" + str(self.soundy_analysis_out) + " does not exist."

        return None

    def perform(self):
        log_info("Invoking Warnings Stats Computation.")
        ret_val = _run_multi_warnings_json(self.soundy_analysis_out, self.output_stats_csv)
        log_success("Warning stats of all entry points is at:", self.output_stats_csv)
        return ret_val

    def get_name(self):
        return "ComputeWarningStats"

    def is_critical(self):
        # Yes, this component is critical.
        return False


def get_warning_type(warn_json):
    warn_type = None
    if 'warn_data' in warn_json:
        warn_type = warn_json['warn_data']['by'].split()[0]
    return warn_type


def _read_warning_json(json_file):
    fp = open(json_file, "r")
    fp_cont = fp.read()
    fp.close()
    total_warnings = 0
    ep_name = os.path.splitext(os.path.basename(json_file))[0]
    warning_dict = {}
    warning_type_set = set()
    try:
        if len(fp_cont) > 0:
            json_obj = json.loads(fp_cont)
            if "num_contexts" in json_obj:
                # default json
                if 'all_contexts' in json_obj:
                    all_cont_obj = json_obj['all_contexts']
                    for curr_context in all_cont_obj:
                        if 'warnings' in curr_context:
                            for curr_warn in curr_context['warnings']:
                                total_warnings += 1
                                curr_warn_type = get_warning_type(curr_warn)
                                if curr_warn_type is not None:
                                    warning_type_set.add(curr_warn_type)
                                    if curr_warn_type not in warning_dict:
                                        warning_dict[curr_warn_type] = 0
                                    warning_dict[curr_warn_type] += 1
            elif "num_instructions" in json_obj:
                # instruction json
                if 'all_instrs' in json_obj:
                    all_cont_obj = json_obj['all_instrs']
                    for curr_context in all_cont_obj:
                        if 'warnings' in curr_context:
                            for curr_warn in curr_context['warnings']:
                                total_warnings += 1
                                curr_warn_type = get_warning_type(curr_warn)
                                if curr_warn_type is not None:
                                    warning_type_set.add(curr_warn_type)
                                    if curr_warn_type not in warning_dict:
                                        warning_dict[curr_warn_type] = 0
                                    warning_dict[curr_warn_type] += 1
    except Exception as e:
        pass

    warning_dict['total'] = total_warnings
    return ep_name, warning_dict, warning_type_set


def _run_multi_warnings_json(output_dir, output_csv_file):
    all_json_files = []
    for curr_file in os.listdir(output_dir):
        file_path = os.path.join(output_dir, curr_file)
        if os.path.isfile(file_path) and file_path.endswith('json'):
            all_json_files.append(file_path)
    p = Pool(cpu_count())
    ret_vals = p.map(_read_warning_json, all_json_files)
    all_warning_types = set()
    for _, _, warn_type in ret_vals:
        warn_type_set = set(warn_type)
        all_warning_types.update(warn_type_set)
    all_warning_types = list(all_warning_types)
    header = "EntryPointName,Total"
    for curr_warn_type in all_warning_types:
        header += "," + curr_warn_type
    fp = open(output_csv_file, "w")
    fp.write(header + "\n")
    warnings_total = {}
    for ep_name, warn_stat, _ in ret_vals:
        curr_line = ep_name + "," + str(warn_stat['total'])
        if 'total' not in warnings_total:
            warnings_total['total'] = 0
        warnings_total['total'] += warn_stat['total']
        for wn_type in all_warning_types:
            warn_no = 0
            if wn_type not in warnings_total:
                warnings_total[wn_type] = 0
            if wn_type in warn_stat:
                warn_no = warn_stat[wn_type]
            warnings_total[wn_type] += warn_no
            curr_line += "," + str(warn_no)
        fp.write(curr_line + "\n")
    footer = "TOTAL," + str(warnings_total['total'])
    for wn_type in all_warning_types:
        footer += "," + str(warnings_total[wn_type])
    fp.write(footer + "\n")
    return True

