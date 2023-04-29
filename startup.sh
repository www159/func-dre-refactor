#/bin/bash
mode=release
cmakeFlag=-DCMAKE_BUILD_TYPE=Release
if [ $1 == "debug" ]; then
export G_MESSAGES_DEBUG=all
cmakeFlag=-DCMAKE_BUILD_TYPE=Debug
mode=debug
fi
mkdir build 2> /dev/null
cd build
rm -rf *
cmake $cmakeFlag ..
make
echo dre start in $mode mode!
./dre