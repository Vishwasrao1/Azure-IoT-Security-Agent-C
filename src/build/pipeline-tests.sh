#!/usr/bin/env bash

script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/.." && pwd)
build_folder=$build_root"/cmake/agent_linux"

echo 'script dir: ' $script_dir
echo 'build root: '$build_root
echo 'build folder: '$build_folder

pushd $build_folder

ctest --no-compress-output --output-on-failure -j$(nproc) -T test
test_result=$?

popd

if [ -f $build_folder/Testing/TAG ] ; then
   xsltproc $script_dir/ctest2junit.xsl $build_folder/Testing/`head -n 1 < $build_folder/Testing/TAG`/Test.xml > $build_folder/CTestResults.xml
fi

if [[ $test_result -ne 0 ]]; then
  echo "tests failed"
  exit $test_result
fi
