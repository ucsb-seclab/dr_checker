#!flask/bin/python
"""
List of RESTful route
"""
import os
from datetime import timedelta
from functools import update_wrapper
from flask import Flask, jsonify, make_response, request, current_app
import utils


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
        filename_fp = os.path.join(app.config["RESULTS_DIR"], filename)
        if os.path.isdir(filename_fp) or not filename_fp.endswith('json'):
   	       continue
        filename_without_extension = filename.split('.')[0]
        if utils.isContextAnalysisPresent(app.config, filename):
            response["data"].append(filename_without_extension)
            continue
        if utils.isInstructionAnalysisPresent(app.config, filename) and \
           filename_without_extension not in response["data"]:
            response["data"].append(filename_without_extension)
    return jsonify(response)


@app.route("/result/<string:filename>")
@crossdomain(origin='*')
def get_result(filename):
    """
    This route returns the details of the analysis relative to the filename specified
    along with the source code analyzed
    """
    response = {
        "success": True,
        "data" : {
            "by_context" : utils.getAnalysisResultByContext(app.config, filename),
            "by_instruction" : utils.getAnalysisResultByInstruction(app.config, filename)
        }
    }
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
    if os.path.exists(local_path_to_sourcecode):
        with open(local_path_to_sourcecode) as sourcecode_file:
            resp = sourcecode_file.read()
    return resp


if __name__ == "__main__":
    if app.config["RUN_REMOTE"]:
        app.run(host='0.0.0.0')
    else:
        app.run()
