Using DR.CHECKER from docker
===================
Thanks to [Sebastiano](https://github.com/Phat3), we created a docokerized environment for DR.CHECKER. 
We have three images main, server and client:
* main: This is the main analysis image, that has all the LLVM setup and can the actual DR.CHECKER analysis.
* server: This is the visualization server, that processes the JSONs and serves them.
* client: This is the visualization client, that serves the webpage for viewing DR.CHECKER results.

By default,
* The folder: `<repo_path>/docker/dockershare` will be mounted at `/dockershare` in main and server image.
* This git repo will be present in the folder `/dr_checker/gitrepo` of all the images.
* server image has its port `5000` mapped to port `5000` of the host machine.
* client image has its port `5000` mapped to port `8080` of the host machine.

Please follow the below instructions on how to use it.

## Setting up folders
First, copy the kernel sources to `<repo_path>/docker/dockershare` folder.
## Getting images
* Pull the images
   ```
   cd <repo_path>/docker/
   docker-compose pull
   ```
All the building of kernel and the LLVM bitcode files have to be done on the main docker image.
## Running the analysis
First, run and log into the main container:
```
cd <repo_path>/docker/
docker-compose run main /bin/bash
```
From inside the main container follow all the instructions from: [Building](https://github.com/ucsb-seclab/dr_checker#2-building) to [Running Analysis](https://github.com/ucsb-seclab/dr_checker#32-running-drchecker-analysis)
**NOTE: ALL THE FILES (like toolchains) NEEDED TO BUILD THE KERNEL SHOULD BE IN THE FOLDER: `<repo_path>/docker/dockershare`**
## Viewing results
For this we need to run both the server and client images:
### Running server
Run and log into the server container:
```
cd <repo_path>/docker/
docker-compose run server /bin/bash
```
Follow the Steps 1 and 2 of [Running Visualizer](https://github.com/ucsb-seclab/dr_checker/tree/speedy/visualizer#running-the-visualizer)
### Running client
Run and log into the client container:
```
cd <repo_path>/docker/
docker-compose run client /bin/bash
## inside the new shell
cd /dr_checker/gitrepo/visualizer
serve -s ./client/build
```

### Accessing the web-interface
You can view all the results from: `http:://localhost:8080`
## Example
