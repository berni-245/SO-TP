#!/bin/sh

container='arqui_builder'
docker='sudo docker'

$docker start $container;
$docker exec -it $container make clean -C/root/Toolchain/;
$docker exec -it $container make clean -C/root/;
$docker exec -it $container make -C/root/Toolchain/;
$docker exec -it $container  make -C/root/;
$docker stop $container;
