#!/bin/bash

# obtain number of CPUs
# - set it manually, if you prefer, or if auto detection fails
export BUILD_NPROC=$([ $(uname) = 'Darwin' ] && sysctl -n hw.ncpu || nproc)
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi
echo "detected $BUILD_NPROC cpus"

# primary prefix: 
# note: if you prefer a different prefix, change it here
export PRIMARY_PREFIX=$EIC_SHELL_PREFIX

# cmake packages
export IRT_ROOT=$PRIMARY_PREFIX # overrides container version with local version
export EICD_ROOT=$PRIMARY_PREFIX # overrides container version with local version

### tempory juggler vars
export JUGGLER_INSTALL_PREFIX=$PRIMARY_PREFIX
export LD_LIBRARY_PATH=$JUGGLER_INSTALL_PREFIX/lib:$LD_LIBRARY_PATH
export PYTHONPATH=${JUGGLER_INSTALL_PREFIX}/python:${PYTHONPATH} # make sure gaudirun.py prioritizes local juggler installation
export JUGGLER_DETECTOR="epic"
export BEAMLINE_CONFIG="ip6"
export JUGGLER_SIM_FILE=$(pwd)/out/sim.root
export JUGGLER_REC_FILE=test.root
export JUGGLER_N_EVENTS=100
export JUGGLER_RNG_SEED=1
export JUGGLER_N_THREADS=$BUILD_NPROC

# environment from reconstruction_benchmarks
if [ -f "reconstruction_benchmarks/.local/bin/env.sh" ]; then
  pushd reconstruction_benchmarks
  source .local/bin/env.sh
  popd
fi

# fix env vars which would have been overwritten by `reconstruction_benchmarks/.local/bin/env.sh`:
export LOCAL_DATA_PATH=$(pwd)
export DETECTOR=epic
export DETECTOR_PATH=$LOCAL_DATA_PATH/$DETECTOR
#export BEAMLINE_CONFIG_VERSION=master
#export DETECTOR_VERSION=main

if [ -f "reconstruction_benchmarks/.local/bin/env.sh" ]; then
  printf "\n\n--------------------------------\n"
  print_env.sh
  echo "--------------------------------"
fi

# use local rbenv ruby shims, if installed
export RBENV_ROOT=$(pwd)/.rbenv
if [ -d "$RBENV_ROOT" ]; then
  export PATH=$RBENV_ROOT/bin:$PATH
  eval "$(.rbenv/bin/rbenv init - bash)"
  export PYTHON=$(which python) # for pycall gem
fi

### additional comfort settings, some dependent on host machine; 
### feel free to add your own here
export PATH=$(pwd)/bin:$PATH  # add ./bin to $PATH
export PATH=.:$PATH  # add ./ to $PATH
shopt -s autocd      # enable autocd
if [ -d "${HOME}/bin" ]; then export PATH=$PATH:${HOME}/bin; fi   # add ~/bin to $PATH
