#!/bin/bash

echo 'build started'

programname=$0
script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/.." && pwd)
build_folder=$build_root"/cmake/agent_linux"

echo 'script dir: ' $script_dir
echo 'build root: '$build_root
echo 'build folder: '$build_folder

rm -rf $build_folder
mkdir -p $build_folder
pushd $build_folder

cmake -Drun_unittests:BOOL=ON -Drun_int_tests:BOOL=ON -Drun_valgrind:BOOL=ON -DCMAKE_BUILD_TYPE=Release -Dno_logging=ON $build_root
cmake_result=$? 
if [[ $cmake_result -ne 0 ]]; then
  echo "cmake failed"
  exit $cmake_result
fi

date
make -j$(nproc)
make_result=$? 
date
if [[ $make_result -ne 0 ]]; then
  echo "make failed"
  exit $make_result
fi

popd

installed_packages_output=$build_root"/cmake/installed_packages.txt"
dpkg -l cmake build-essential uuid-dev libssl-dev libaudit-dev libauparse-dev moreutils iptables-dev valgrind > $installed_packages_output

echo 'build finished'
