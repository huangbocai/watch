#!/bin/bash

#更新库
sudo cp -af ./lib/* /usr/lib/

#可执行文件
chmod a+x ./bin/*
sudo cp ./bin/watchDiamond /usr/bin/

#辅助文件
mkdir -p /home/u/cnc/configs/ppmc/o_nc
cp -fp ./share/o_nc/* /home/u/cnc/configs/ppmc/o_nc/
cp -fp ./bin/M10* /home/u/cnc/configs/ppmc/
cp -f ./configs/watchDiamond.hal /home/u/cnc/configs/ppmc/
sync

#定位方案
mkdir -p /home/u/cnc/镶钻存档
cp -pt ./share/镶钻存档 /home/u/cnc/镶钻存档
sync

sudo cp -f ./configs/dhhvux-load.rules /etc/udev/rules.d/
sync

echo "更新完成"
sleep 2

exit 0



