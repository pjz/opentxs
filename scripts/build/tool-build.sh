#!/bin/bash
GITREPO=`git rev-parse --show-toplevel`
if [ `pwd` != ${GITREPO} ]; then
   echo "must run from the top level of the git repo"
   exit 1
fi
DOCKERFILE=Dockerfiles/Dockerfile.tools
DFILEHASH=`git log --format=%h ${DOCKERFILE} | head -1`
NAME="opentxs_build:${DFILEHASH}"
if [ `docker images ${NAME} | wc -l` -lt 2 ] ; then
    mkdir -p .emptydir
    docker build -f ${DOCKERFILE} -t "${NAME}" .emptydir
fi
#docker run -t -v `pwd`:/src --user $(id -u):$(id -g) "${NAME}" /bin/sh -c "$@"
docker run -v `pwd`:/src "${NAME}" /bin/sh -c "$@"
OWNER=$(id -u):$(id -g)
docker run -v `pwd`:/src "${NAME}" /bin/sh -c "chown -R ${OWNER} ."
