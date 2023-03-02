# 1.安装必要工具

```shell
sudo apt-get install git
sudo apt install make
sudo apt-get install gcc
sudo apt-get install cmake
sudo apt-get install g++
sudo apt-get install yasm 
```

# 2.编译

```
source ./initenv.sh LINUX /home/relech/syncmedia/
make
```

# 3.修改配置项

![](https://github.com/yubaihe/mediasync/blob/main/config.png)

# 4.运行

```shell
#设置环境变量
#需要以root权限运行
export ROOTDIR=/home/relech/syncmedia/
#启动
$ROOTDIR/start.sh
#停止
$ROOTDIR/stop.sh
```




