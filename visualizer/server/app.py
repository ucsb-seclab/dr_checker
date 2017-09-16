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
        print app.config["RESULTS_DIR"]
        response["success"] = False
        response["msg"] = "Misconfigured results_dir"
        return jsonify(response)
    # return all the filename without the extension
    for filename in os.listdir(app.config["RESULTS_DIR"]):
        response["data"].append({"name" : os.path.splitext(filename)[0]})
    return jsonify(response)

if __name__ == "__main__":
    app.run()
