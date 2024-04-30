#!/bin/sh

container='so_builder'
docker='sudo docker'

if ! $docker start $container; then
	exit $?
fi
$docker exec -it $container make -C /root/ clean
$docker exec -it $container make -C /root/ all
sudo chown "$USER:$USER" ./Image/x64BareBonesImage.qcow2
$docker stop $container &

# I have to create this files dynamically because .clangd doesn't accept relative paths
# so they need to be recreated for evry user or any time the project is moved.
# See: https://github.com/clangd/clangd/issues/1038
cat <<EOF > Kernel/.clangd
CompileFlags:
  Add:
  - --include-directory=$(pwd)/Kernel/include/
EOF

cat <<EOF > Userland/UserModule/.clangd
CompileFlags:
  Add:
  - --include-directory=$(pwd)/Userland/UserModule/include/
EOF
