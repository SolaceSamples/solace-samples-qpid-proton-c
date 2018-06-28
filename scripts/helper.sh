#!/usr/bin/env sh

############ Helper functions #############

check_docker(){
    if ! ToolExists docker ; then
        echo "Missing command docker. Please install docker and make available."
        exit 1
    fi
}

ToolExists() {
    command -v $1 >/dev/null 2>&1
}

ScriptInit() {
    PRG="$1"
    SAVED="`pwd`"

    # Attempt to set APP_HOME
    # Resolve links: $0 may be a link
    # Need this for relative symlinks.
    local SOURCE=$PRG
#"${BASH_SOURCE[0]}"
    while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
        local DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
        local SOURCE="$(readlink "$SOURCE")"
        [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
    done
    PRG_DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

}

# tag is a helper function to construct the docker image tag
tag(){
    local IMAGE_NAME_TAG=$1
    shift
    if [ "$1" ]; then
        if [ "$1" == "local" ] || [ "$1" == "$USER" ]; then
            local USER_PREFIX=$USER/
        elif [ "$1" == "release" ]; then
            local USER_PREFIX=solace/
        else
            local USER_PREFIX=$1/
        fi
    else
        local USER_PREFIX=solace/
    fi
    shift

    if [ "$1" ]; then
        local TAG_VERSION=:${IMAGE_NAME_TAG}_$1
    elif [ $IMAGE_NAME_TAG ]; then
        local TAG_VERSION=:${IMAGE_NAME_TAG}
    else
        local TAG_VERSION=:latest
    fi

    TAG_NAME=${USER_PREFIX}docker-qpid-proton${TAG_VERSION}

}


############ END HELPER FUNCTIONS #########
