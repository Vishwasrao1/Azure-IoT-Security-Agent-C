#!/usr/bin/env bash

script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/.." && pwd)
build_folder=$build_root"/cmake/agent_linux"

pushd $build_folder/tests

failed_tests=0

for f in *; do

    if [ -d ${f} ]; then
        if [ ${f} != "CMakeFiles" ]; then
            # Will not run if no directories are available
            ./${f}/${f}_exe
            failed_tests=$(expr $failed_tests + $?)
        fi
    fi
done

if [[ "$failed_tests" -eq 0 ]]; then
    echo "All test have passed"
else 
    echo  "$failed_tests test failed"
fi

popd

exit $failed_tests