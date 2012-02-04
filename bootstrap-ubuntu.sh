#!/bin/bash
if [ "$1" != "go" ]; then
        echo "This script downloads, compiles and installs the QuaZIP lib which are required for PTB to compile."
        echo "Call with 'go' to proceed at your own risk."
        exit 1
fi

sudo apt-get install zlib1g-dev libbz2-dev

rm -rf bootstrap
mkdir bootstrap
cd bootstrap

# download and install QuaZIP
wget -O quazip.zip "http://downloads.sourceforge.net/project/quazip/quazip/0.4.4/quazip-0.4.4.zip?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fquazip%2F&ts=1328389079&use_mirror=heanet"
unzip quazip.zip
cd quazip-0.4.4
sed -i 's/SHARED/STATIC/g' quazip/CMakeLists.txt
cmake .
make
sudo make install

# clean up
cd ..
rm -rf bootstrap
