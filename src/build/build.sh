#!/bin/bash

programname=$0
script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/.." && pwd)
build_folder=$build_root"/cmake/agent_linux"
build_debug=" "
unit_tests=" "
int_tests=" "
valgrind=" "
make_args=" "

print_usage() {
    echo "usage: $programname [--debug] [--unit-tests] [--int-tests] [--valgrind] [--make-args <make arguments>]"
    echo "  --debug           compile in debug mode"
    echo "  --unit-tests      compile unit tests"
    echo "  --int-tests       compile integration tests"
    echo "  --valgrind        compile unit tests with valgrind"
    echo "  --make-args       specify arguments for make"
    echo "example: $programname --debug --unit-tests --make-args -j4"
    exit 1
}

parse_args() {
  while [ "$1" != "" ] && [ "$1" != "--make-args" ]; do
    case $1 in
      --debug)        build_debug=" -DCMAKE_BUILD_TYPE=Debug "  ;;
      --unit-tests)   unit_tests=" -Drun_unittests:BOOL=ON "    ;;
      --int-tests)    int_tests=" -Drun_int_tests:BOOL=ON "    ;;
      --valgrind)     valgrind=" -Drun_valgrind:BOOL=ON "       ;;
      *)              print_usage                               ;;
    esac
    shift
  done
  if [ "$1" == "--make-args" ]; then
    shift
    make_args=$@
  fi
}

parse_args $*

echo $script_dir
echo $build_root

rm -rf $build_folder
mkdir -p $build_folder
pushd $build_folder

cmake $build_debug $unit_tests $int_tests $valgrind $build_root
cmake_result=$? 
if [[ $cmake_result -ne 0 ]]; then
  echo "cmake failed"
  exit $cmake_result
fi

date
make $make_args
make_result=$? 
date

if [[ $make_result -ne 0 ]]; then
  echo "make failed"
  exit $make_result
fi

popd
