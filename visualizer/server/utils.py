"""
Module wit all the utility functions useful for the application
"""
import json
import os

def isContextAnalysisPresent(config, analysis_name):
    """
    Check if the analysys specified in analysis_name
    has produced some result for CONTEXT
    :param config: configuration of the flask app
    :param analysis_name: name of the analysis we want to check
    """
    analysis_by_context_file_path = os.path.join(
        config["RESULTS_DIR"],
        analysis_name
    )
    if os.path.exists(analysis_by_context_file_path):
        content_context_analysis = ""
        with open(analysis_by_context_file_path, "r") as result_file:
            content_context_analysis = result_file.read()
            try:
                json_data = json.loads(content_context_analysis)
                if json_data["num_contexts"] != 0:
                    return True
            except Exception:
                return False
    return False

def isInstructionAnalysisPresent(config, analysis_name):
    """
    Check if the analysys specified in analysis_name
    has produced some result for INSTRUCTION
    :param config: configuration of the flask app
    :param analysis_name: name of the analysis we want to check
    """
    analysis_by_instruction_file_path = os.path.join(
        config["RESULTS_DIR"],
        analysis_name
    )
    if os.path.exists(analysis_by_instruction_file_path):
        content_instruction_analysis = ""
        with open(analysis_by_instruction_file_path, "r") as result_file:
            content_instruction_analysis = result_file.read()
            try:
                json_data = json.loads(content_instruction_analysis)
                if json_data["num_instructions"] != 0:
                    return True
            except Exception:
                return False
    return False

def getAnalysisResultByContext(config, analysis_name):
    """
    Returns all the results of the analysis by CONTEXT grouped by by sourcefile.
    If any result is present the method returns an empty array (no results)
    :param config: configuration of the flask app
    :param analysis_name: name of the analysis we want to check
    :return results: array of results
    """
    results = []
    file_path = os.path.join(config["RESULTS_DIR"], analysis_name + '.json')
    if os.path.exists(file_path):
        json_data = {}
        with open(file_path, "r") as result_file:
            analysis_content = result_file.read()
            json_data = json.loads(analysis_content)
        if "all_contexts" in json_data.keys():
            # group result of context analysis by source code (BAD CODE)
            should_replace_path, new_src_dir = isReplacementNeeded(config)
            for context in json_data["all_contexts"]:
                results_warnings = {}
                for warning in context["warnings"]:
                    filename = warning["warn_data"]["at_file"]
                    # remove junk
                    if should_replace_path and config["PATH_TO_BE_REPLACED"] in filename and \
                    new_src_dir is not None:
                        filename = filename.replace(config["PATH_TO_BE_REPLACED"], new_src_dir)
                    if results_warnings.has_key(filename):
                        results_warnings[filename].append(warning)
                    else:
                        results_warnings[filename] = [warning]
                results.append(results_warnings)
    return results

def getAnalysisResultByInstruction(config, analysis_name):
    """
    Returns all the results of the analysis by INSTRUCTION grouped by instruction.
    If any result is present the method returns an empty array (no results)
    :param config: configuration of the flask app
    :param analysis_name: name of the analysis we want to check
    :return results: array of results
    """
    results = {}
    file_path = os.path.join(config["RESULTS_DIR"], analysis_name + '.json.instr_warngs.json')
    if os.path.exists(file_path):
        json_data = {}
        with open(file_path, "r") as result_file:
            analysis_content = result_file.read()
            json_data = json.loads(analysis_content)
        if "all_instrs" in json_data.keys():
            # group result of context analysis by source code (BAD CODE)
            should_replace_path, new_src_dir = isReplacementNeeded(config)
            for instr in json_data["all_instrs"]:
                results_warnings = {}
                for warning in instr["warnings"]:
                    filename = warning["warn_data"]["at_file"]
                    # remove junk
                    if should_replace_path and config["PATH_TO_BE_REPLACED"] in filename and \
                    new_src_dir is not None:
                        filename = filename.replace(config["PATH_TO_BE_REPLACED"], new_src_dir)
                    if results_warnings.has_key(filename):
                        results_warnings[filename].append(warning)
                    else:
                        results_warnings[filename] = [warning]
                results[instr["at"]] = results_warnings
    return results

def isReplacementNeeded(config):
    """
    Check if we need to clean some junk from the source code files path
    :param config: configuration of the flask app
    :return (should_replace_path, new_src_dir)
    """
    should_replace_path = False
    if 'REPLACE_KERNEL_SRC' in config:
        should_replace_path = config['REPLACE_KERNEL_SRC']
    new_src_dir = None
    if 'SOURCECODE_DIR' in config:
        new_src_dir = config['SOURCECODE_DIR']
    return (should_replace_path, new_src_dir)
