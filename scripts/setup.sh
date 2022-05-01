#!/usr/bin/env bash

sudo apt remove --purge --auto-remove cmake -y
sudo apt update && \
sudo apt install -y software-properties-common lsb-release && \
sudo apt clean all
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
sudo apt update
sudo apt install kitware-archive-keyring
sudo rm /etc/apt/trusted.gpg.d/kitware.gpg
sudo apt update

sudo apt-get -y install \
    gcc \
    g++ \
    make \
    cmake \
    git-lfs \
    linux-tools-common \
    linux-tools-generic \
    linux-tools-`uname -r` \
    build-essential \
    libssl-dev \
    jq \
    gnupg-agent >> /dev/null


#* Clear pagecache, dentries, and inodes.
echo 3 | sudo tee /proc/sys/vm/drop_caches

#* Disable hyperthreads.
echo off | sudo tee /sys/devices/system/cpu/smt/control

#* Disable turbo boost.
if [[ -z $(which rdmsr) ]]; then
    sudo apt-get install msr-tools
fi

sudo modprobe msr

if [[ ! -z $1 && $1 != "enable" && $1 != "disable" ]]; then
    echo "Invalid argument: $1" >&2
    echo ""
    echo "Usage: $(basename $0) [disable|enable]"
    exit 1
fi

cores=$(cat /proc/cpuinfo | grep processor | awk '{print $3}')
for core in $cores; do
    if [[ $1 == "disable" ]]; then
        sudo wrmsr -p${core} 0x1a0 0x4000850089
    fi
    if [[ $1 == "enable" ]]; then
        sudo wrmsr -p${core} 0x1a0 0x850089
    fi
    state=$(sudo rdmsr -p${core} 0x1a0 -f 38:38)
    if [[ $state -eq 1 ]]; then
        echo "core ${core}: disabled"
    else
        echo "core ${core}: enabled"
    fi
done

# Build 
cd src/ext
git clone git@github.com:mitsuba-renderer/openexr.git
cd ../..
cmake . -DCMAKE_BUILD_TYPE=profile
make