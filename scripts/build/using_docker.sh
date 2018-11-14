#!/bin/bash -e
GITREPO=`git rev-parse --show-toplevel`
if [ `pwd` != ${GITREPO} ]; then
   echo "must run from the top level of the git repo"
   exit 1
fi
git submodule update --init --recursive
if ! ls opentxs-proto*.deb >/dev/null ; then
    ./scripts/build/tool-build.sh "mkdir -p deps/opentxs-proto/build && cd deps/opentxs-proto/build && cmake -DBUILD_VERBOSE=OFF -DPYTHON3:BOOL=ON .. && make"
    ./scripts/build/tool-build.sh "cd deps/opentxs-proto/build && checkinstall -D --fstrans=yes --install=no --pkgname opentxs-proto --nodoc -y make install"
    cp deps/opentxs-proto/build/*.deb .
else
    echo "found `ls opentxs-proto*.deb`"
fi
if ! ls libopentx*.deb >/dev/null ; then
    ./scripts/build/tool-build.sh "dpkg --install deps/opentxs-proto/build/opentxs-proto*.deb && mkdir -p build && cd build && cmake .. && make test"
    ./scripts/build/tool-build.sh "dpkg --install deps/opentxs-proto/build/opentxs-proto*.deb && cd build && checkinstall -D --fstrans=yes --install=no --pkgname libopentxs --nodoc -y make install"
    cp build/*.deb .
else
    echo "found `ls libopentx*.deb`"
fi

