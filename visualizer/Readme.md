# Visualizer for dr_checker
The visualizer is composed by two parts:
. **SERVER** : A flask application that implements a set of API which read and interpret the json result files of of dr_checker.
. **CLIENT** : A react application that uses the API provded by the server and displays those results in a fancy way.

I chose to separate these two components in two different project because I wanted to create a standalone API server without any client bound to it (for this reason I didn't integrate the react application directly in the flask application structure).
In this way it is possible to develop other types without touching the server. the react application must be interpreted just as an example.

## Installation

### Server
1. Install [python virtualenv](https://virtualenv.pypa.io/en/stable/) and [virtualenvwrapper](https://virtualenvwrapper.readthedocs.io/en/latest/)
2. Create a virtualenv for this project and name it whatever you want (i. e. dr_checker_venv)
    ```
    mkvirtualenv dr_checker_venv
    ```
3. Activate the created virtual environment
    ```
    workon dr_checker_venv
    ```
4. Install the dependencies
    ```
    pip install -r ./server/requirements.txt
    ```
5. Configure the settings in ./server/config.py (more information on each setting inside ./server/Readme.md)

### Client
1. Install [node js](https://nodejs.org/it/)
2. Install [npm](https://www.npmjs.com/)
3. Install [serve](https://www.npmjs.com/package/serve)
    ```
    npm install -g serve
    ```
4. Install the dependencies
    ```
    npm --prefix ./client install ./client
    ```
5. Build the application
    ```
    npm --prefix ./client run build ./client
    ```

## Run the visualizer

### Production
1. Set the DEBUG config (inside ./server/config.py) to False
2. Activate the virtualenv created during the installation
```
workon dr_checker_venv
```
3. Run the server
```
python ./server/app.py
```
4. Run the client
```
serve -s ./client/build
```

### Development
1. Set the DEBUG config (inside ./server/config.py) to True
2. Activate the virtualenv created during the installation
```
workon dr_checker_venv
```
3. Run the server
```
python ./server/app.py
```
4. Run the client
```
npm run start ./client
```
