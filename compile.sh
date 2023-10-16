#!/bin/sh

container='arqui_builder'
docker='sudo docker'

$docker start $container
if [[ $? -ne 0 ]]; then
  exit $?
fi
$docker exec -it $container make clean -C/root/Toolchain/
$docker exec -it $container make clean -C/root/
$docker exec -it $container make -C/root/Toolchain/
$docker exec -it $container  make -C/root/
$docker stop $container
sudo chown $USER:$USER ./Image/x64BareBonesImage.qcow2

cat <<EOF > .clangd
CompileFlags:
  Add:
  - --include-directory=$(pwd)/Kernel/include/
EOF
