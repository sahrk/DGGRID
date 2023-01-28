# Contributed Dockerfile for dggrid7

The Dockerfile allows to build dggrid inside a Docker container.

This can be useful in CI/CD, cloud or otherwise automated build and/or data processing scenarios, where one wouldn't want to install
a full C/C++ compiler build chain. 

## To build the container

`docker build . -t dggrid:7.7`

This builds dggrid and creates a docker container images named dggrid and tagged with version number 7.7, change as wish.

## run dggrid from within the container

There are two potential scenarios, run directly akin to a command-line tool, or use the container image as a base to build
other containerized applications that utilize dggrid.

To run dggrid like a command-line tool:

docker run -it -v $PWD:/data -w /data --rm dggrid:7.5 dggrid /data/aigenerate.meta

Explaination of the parameters:

- `-it` runs the app via an interactive terminal, os you see dggrid output for example
- `-v $PWD:/data` mounts your current working directory ($PWD) into the containers filesystem under /data in rw mode by default
- `-w /data` switches the containers working dir to /data, this way your current working directory and the containers executing folder are the same
- `--rm` removes the container after execution, this way you don't create old container instances lying around
- `dggrid:7.5` the name of the container image to run (of course the one that we just built)
- `dggrid` the command to execute in the container, our actual dggrid tool
- `/data/aigenerate.meta` the additional parameters for dggrid, i.e. our metafile, here consider, you could also just directly use `aigenerate.meta` as we switched the working directory to /data already.

To reproduce, cd into the ./examples/aigenerate folder of the dggrid source tree. This folder would serve as /data mount.

