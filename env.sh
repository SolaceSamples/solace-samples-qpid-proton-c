#!/usr/bin/env sh
#global setup

# unsafe symlink reference
SCRIPT_HOME=`dirname ${BASH_SOURCE[0]}`/scripts

source $SCRIPT_HOME/helper.sh

# initialize syslink safe location varibles
ScriptInit ${BASH_SOURCE[0]}

# check for docker tool
check_docker

# set useful symlink safe locations variables
APP_HOME=$PRG_DIR/src

SCRIPT_HOME=$PRG_DIR/scripts

# functions 
usage(){
    echo "Usage: env.sh [options] <command> <args>"
    echo "<command>:"
    echo "    make <targets...>:                    pulls the docker container remotely and builds the samples in the container"
    echo "    clean:                                removes local docker container images"
    echo "    run <example_executable> <args...>:   executes sample in docker container from local host"
    echo "    activate:                             enters container command line"
    echo "    help:                                 prints this message"
    echo "[options]:"
    echo "    -t <tag>:         selects docker image tag <tag>"
    echo "    -u <username>:    Selects docker registry username. If omitted default is 'solace'"
}


docker_run(){
    if [ "$1" ]; then
        local EXTRA_OPTS=$1
        shift
    fi
    if [ "$1" ]; then
        local RUN_CMD=$1
        shift
    fi
    if [ "$1" ]; then
        local RUN_ARGS=$1
    fi

    local EXAMPLE_SOURCE_DIR=$APP_HOME
    local DOCKER_VOLUME_OPTS="-v ${EXAMPLE_SOURCE_DIR}:/sample/src"
    local DOCKER_RUN_OPTS="$DOCKER_VOLUME_OPTS $EXTRA_OPTS"
    if [ "$RUN_ARGS" ]; then
        echo "Running docker cmd:"
        echo "    docker run $DOCKER_RUN_OPTS $TAG_NAME $RUN_CMD \"$RUN_ARGS\""
        docker run $DOCKER_RUN_OPTS $TAG_NAME $RUN_CMD "$RUN_ARGS" || 
        echo "Docker container error with cmd : $RUN_CMD \"$RUN_ARGS\""
    else 
        echo "Running docker cmd:"
        echo "    docker run $DOCKER_RUN_OPTS $TAG_NAME $RUN_CMD "
        docker run $DOCKER_RUN_OPTS $TAG_NAME $RUN_CMD  ||
        echo "Docker container error with cmd : $RUN_CMD "
    fi

}

run_command(){
    # parse and run command

    local COMMAND=$1
    shift

    # locations inside container
    local CONTAINER_WORKDIR=/sample
    local CONTAINER_APP_HOME=$CONTAINER_WORKDIR/src

    if [ "$COMMAND" == "make" ]; then
        echo "docker pull and sample build here"
        tag "$IMAGE_NAME" $DOCKER_USERNAME
        local IMAGE_HASH=`docker image ls $TAG_NAME -q`
        if ! [ "$IMAGE_HASH" ]; then
            # pull remote image to local repository
            docker pull $TAG_NAME
        fi
        # build samples for selected Image
        echo "build samples"
        while [ "$1" ]; do
            local MAKE_TARGETS="$MAKE_TARGETS $1"
            shift
        done
        docker_run "--rm -a STDOUT -a STDIN -a STDERR" "/bin/sh -c" "make -f $CONTAINER_APP_HOME/makefile $MAKE_TARGETS"
    elif [ "$COMMAND" == "clean" ]; then
        tag "$IMAGE_NAME" $DOCKER_USERNAME
        local IMAGE_HASH=`docker image ls $TAG_NAME -q`
        # determine if the image is local
        if [ "$IMAGE_HASH" ]; then
            echo "Removing container executables"
            # use local image to remove sample executables with makefile
            docker_run "--rm -a STDOUT -a STDIN -a STDERR" "/bin/sh -c" "cd $CONTAINER_APP_HOME && make clean"
            # remove local image
            docker rmi -f $IMAGE_HASH && docker rm $DOCKER_CONTAINER_NAME -f -v
        fi
    elif [ "$COMMAND" == "run" ]; then
        # parse example execute reference
        local EXAMPLE=$1
        shift
        # build docker registry image reference
        tag "$IMAGE_NAME" $DOCKER_USERNAME

        # set location variable for container and local executable bin directories
        local EXAMPLE_BIN=$APP_HOME/bin
        local CONTAINER_BIN=$CONTAINER_APP_HOME/bin

        [ -e $EXAMPLE_BIN/$EXAMPLE ] || echo "Building $EXAMPLE"
        if ! [ -e $EXAMPLE_BIN/$EXAMPLE ]; then
            docker_run "--rm -a STDOUT -a STDIN -a STDERR" "/bin/sh -c" "cd $CONTAINER_APP_HOME && make clean build"
        fi
        if [ -e $EXAMPLE_BIN/$EXAMPLE ]; then
            local ARGS=$@
            docker_run "--rm -it -a STDOUT -a STDIN -a STDERR -P" "/bin/sh -c" "$CONTAINER_BIN/$EXAMPLE $ARGS"
        else
            echo "Sample application ($EXAMPLE) not found. Check $EXAMPLE_BIN for built executable."

        fi
    elif [ "$COMMAND" == "activate" ]; then
        tag "$IMAGE_NAME" $DOCKER_USERNAME
        if ! [ "`docker container ls -q -f name=$DOCKER_CONTAINER_NAME `" ]; then
            if ! [ "`docker container ls -aq -f name=$DOCKER_CONTAINER_NAME `" ]; then 
                docker container create -v $APP_HOME:$CONTAINER_APP_HOME --name $DOCKER_CONTAINER_NAME -it $TAG_NAME ||
                echo "Could not create docker container $DOCKER_CONTAINER_NAME for image $TAG_NAME"
            fi
            docker container start $DOCKER_CONTAINER_NAME > /dev/null || 
            echo "Could not start docker container $DOCKER_CONTAINER_NAME"
        fi
        docker container exec -it $DOCKER_CONTAINER_NAME /bin/bash 
        docker container stop $DOCKER_CONTAINER_NAME > /dev/null 
        #docker_run "-it --name $DOCKER_CONTAINER_NAME"
    elif [ "$COMMAND" == "help" ]; then
        usage
    elif [ "$COMMAND" == "" ]; then
        usage
    else
        echo "Unknown command '$COMMAND'"
        usage
    fi

}

# parse options

while : ; do
if [ "$1" == "-t" ]; then
    shift
    IMAGE_NAME=$1
    shift
elif [ "$1" == "-u" ]; then
    shift
    DOCKER_USERNAME=$1
    shift
else
    break
fi
done

# TODO make option?
DOCKER_CONTAINER_NAME=sample_container

# run command
run_command $@ 
