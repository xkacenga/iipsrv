#!/usr/bin/python

import sys, os, json

FLESH_FILE_NAME = "flesh.tif"
PARAMETERS_FILE_NAME = "parameters.json"

# from https://stackoverflow.com/questions/800197/how-to-get-all-of-the-immediate-subdirectories-in-python
def get_immediate_subdirectories(a_dir):
    return [
        a_dir + "/" + name
        for name in os.listdir(a_dir)
        if os.path.isdir(os.path.join(a_dir, name))
    ]


def getParameters(expPath):
    """Gets parameters for the specified experiment file path"""
    paramPath = ""
    if expPath.endswith(".tif"):
        paramPath = expPath[:-4] + ".json"
    try:
        with open(paramPath, "r") as paramFile:
            return json.load(paramFile)
    except:
        return None


def create_scan_json(scanDir, rootPath):
    """Creates json object for a specific scan directory.
    The object contains path to flesh and experiments, and parameters.
    """
    experimentPaths = os.listdir(scanDir)

    experimentPaths = list(
        map(lambda path: scanDir + "/" + path, experimentPaths)
    )  # map experimentPaths from relative to absolute paths

    flashPath = list(filter(lambda path: FLESH_FILE_NAME in path, experimentPaths))[0]
    experimentPaths.remove(flashPath)

    experiments = []
    for e in experimentPaths:
        if ".json" not in e:
            parameters = getParameters(e)
            experiments.append({"data": e, "parameters": parameters})

    scanName = os.path.basename(scanDir)
    return {scanName: {"flesh": flashPath, "experiments": experiments}}


def main(argv):
    if len(sys.argv) != 2:
        print(
            "The number of arguments = "
            + str(len(sys.argv) - 1)
            + " is wrong. Script only accepts 1 argument (path to the root directory)."
        )
        return

    rootPath = os.path.abspath(argv[1])
    if not os.path.isdir(rootPath):
        print("The provided file is not a directory.")
        return

    scanDirs = get_immediate_subdirectories(
        rootPath
    )  # will be used as a key in resulting json file

    jsonData = []
    for scanDir in scanDirs:
        jsonData.append(create_scan_json(scanDir, rootPath))
    jsonObject = json.dumps(jsonData, indent=4)
    with open(rootPath + "/index.json", "w") as outfile:
        outfile.write(jsonObject)


if __name__ == "__main__":
    main(sys.argv)
