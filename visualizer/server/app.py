#!flask/bin/python
"""
List of RESTful route
"""
import os
from datetime import timedelta
from functools import update_wrapper
from flask import Flask, jsonify, make_response, request, current_app, json


app = Flask(__name__) # pylint: disable=invalid-name
app.config.from_object("config")

def crossdomain(origin=None, methods=None, headers=None,
                max_age=21600, attach_to_all=True,
                automatic_options=True):
    if methods is not None:
        methods = ', '.join(sorted(x.upper() for x in methods))
    if headers is not None and not isinstance(headers, basestring):
        headers = ', '.join(x.upper() for x in headers)
    if not isinstance(origin, basestring):
        origin = ', '.join(origin)
    if isinstance(max_age, timedelta):
        max_age = max_age.total_seconds()

    def get_methods():
        if methods is not None:
            return methods

        options_resp = current_app.make_default_options_response()
        return options_resp.headers['allow']

    def decorator(f):
        def wrapped_function(*args, **kwargs):
            if automatic_options and request.method == 'OPTIONS':
                resp = current_app.make_default_options_response()
            else:
                resp = make_response(f(*args, **kwargs))
            if not attach_to_all and request.method != 'OPTIONS':
                return resp

            h = resp.headers

            h['Access-Control-Allow-Origin'] = origin
            h['Access-Control-Allow-Methods'] = get_methods()
            h['Access-Control-Max-Age'] = str(max_age)
            if headers is not None:
                h['Access-Control-Allow-Headers'] = headers
            return resp

        f.provide_automatic_options = False
        return update_wrapper(wrapped_function, f)
    return decorator


@app.route("/results", methods=["GET"])
@crossdomain(origin='*')
def get_results():
    """
    This route returns all the filenames present in the results
    folder of dr_checker as JSON
    """
    response = {"success" : True, "data" : []}
    # return error if path does not exist
    if not os.path.exists(app.config["RESULTS_DIR"]):
        response["success"] = False
        response["msg"] = "Misconfigured results_dir"
        return jsonify(response)
    # return all the filename without the extension
    for filename in os.listdir(app.config["RESULTS_DIR"]):
        if "instr_warngs" not in filename:
            response["data"].append({"name" : os.path.splitext(filename)[0]})
    return jsonify(response)


@app.route("/result/<string:filename>")
@crossdomain(origin='*')
def get_result(filename):
    """
    This route returns the details of the analysis relative to the filename specified
    along with the source code analyzed
    """
    response = {"success": True, "data" : {}}
    analysis_by_context_file_path = os.path.join(app.config["RESULTS_DIR"], filename + '.json')
    analysis_by_instruction_file_path = os.path.join(app.config["RESULTS_DIR"], filename + '.json.instr_warngs.json')
    # returns error if filename does not exist
    if not os.path.exists(analysis_by_context_file_path):
        response["success"] = False
        response["msg"] = "The requested file does not exist"
        return jsonify(response)
    # check if there is a json by instruction for the requested analysys
    is_analysis_by_instr_preent = True if os.path.exists(analysis_by_instruction_file_path) else False
    # returns the analysis results and the seurce code of the analyzed file
    content_context_analysis = ""
    with open(analysis_by_context_file_path, "r") as result_file:
        content_context_analysis = result_file.read()
    json_data = json.loads(content_context_analysis)
    # group result of context analysis by source code (BAD CODE)
    results = []
    for context in json_data["all_contexts"]:
        results_warnings = {}
        for warning in context["warnings"]:
            filename = warning["warn_data"]["at_file"]
            # remove junk
            if "/home/machiry/workdir/" in filename:
                filename = filename.replace("/home/machiry/workdir/", '')
            if results_warnings.has_key(filename):
                results_warnings[filename].append(warning)
            else:
                results_warnings[filename] = [warning]
        results.append(results_warnings)
    response["data"]["by_context"] = results
    results = {}
    if is_analysis_by_instr_preent:
        content_instruction_analysis = ""
        with open(analysis_by_instruction_file_path, "r") as result_file:
            content_instruction_analysis = result_file.read()
        json_data = json.loads(content_instruction_analysis)
        # group result of context analysis by source code (BAD CODE)
        for instr in json_data["all_instrs"]:
            results_warnings = {}
            for warning in instr["warnings"]:
                filename = warning["warn_data"]["at_file"]
                # remove junk
                if "/home/machiry/workdir/" in filename:
                    filename = filename.replace("/home/machiry/workdir/", '')
                if results_warnings.has_key(filename):
                    results_warnings[filename].append(warning)
                else:
                    results_warnings[filename] = [warning]
            results[instr["at"]] = results_warnings
    response["data"]["by_instruction"] = results

    return jsonify(response)


@app.route("/sourcecode/<string:path>")
@crossdomain(origin='*')
def get_sourcecode(path):
    """
    This route rreturns the sourcecode of the requested file if it exists
    """
    path = path.replace('*', '/')
    local_path_to_sourcecode = os.path.join(app.config["SOURCECODE_DIR"], path)
    resp = "No such file..."
    if(os.path.exists(local_path_to_sourcecode)):
        with open(local_path_to_sourcecode) as sourcecode_file:
            resp = sourcecode_file.read()
    return resp


if __name__ == "__main__":
    app.run()
