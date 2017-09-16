#!flask/bin/python
"""
List of RESTful route
"""
import os
from flask import Flask, jsonify

app = Flask(__name__) # pylint: disable=invalid-name
app.config.from_object("config")

@app.route("/results", methods=["GET"])
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
        response["data"].append({"name" : os.path.splitext(filename)[0]})
    return jsonify(response)

@app.route("/result/<string:filename>")
def get_result(filename):
    """
    This route returns the details of the analysis relative to the filename specified
    along with the source code analyzed
    """
    response = {"success": True, "data" : {}}
    analysis_content_file_path = os.path.join(app.config["RESULTS_DIR"], filename + '.json')
    print analysis_content_file_path
    # returns error if filename does not exist
    if not os.path.exists(analysis_content_file_path):
        response["success"] = False
        response["msg"] = "The requested file does not exist"
        return jsonify(response)
    # returns the analysis results and the seurce code of the analyzed file
    response["data"]["summary"] = open(analysis_content_file_path, "r").readlines()
    return jsonify(response)
    



if __name__ == "__main__":
    app.run()
