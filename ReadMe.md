# 1.安装必要工具

```shell
sudo apt-get install git
sudo apt install make
sudo apt-get install gcc
sudo apt-get install cmake
sudo apt-get install g++
sudo apt-get install yasm 
sudo apt-get install autoreconf
sudo apt-get install libtool
```

# 2.拉代码

```shell
#gitee
git clone https://gitee.com/relech/mediasync.git mediasyncsrc
#github
git clone https://github.com/yubaihe/mediasync.git mediasyncsrc
```

# 3.编译

```shell
cd mediasyncsrc
# 初始化环境变量
source ./initenv.sh DOCKER /home/relech/mediasync/
# 编译
make
# 将编译的临时文件删除掉
rm -rf $ROOTDIR/**
# 将打包文件移动到目的目录
cp package/install/** $ROOTDIR -R

```

# 4.运行

```shell
# 把启动脚本拷贝进目的目录
cp package/startup/docker.sh $ROOTDIR/start.sh
# 添加可执行权限并以root权限执行脚本
chmod +x $ROOTDIR/start.sh
sudo $ROOTDIR/start.sh &
```

# 5.支持samba访问

```shell
sudo vi /etc/samba/smb.conf 
# 增加
include = /home/relech/mediasync/samba/lib/smbmedia.conf

/home/relech/mediasync 这个需要修改为实际的地址

重启
sudo systemctl restart smbd
```

# 6.使用介绍

> [使用介绍](http://www.yubaihe.net/function/homepage.html)

# 7.发行

> Windows

[Windows](./Release/MediaSyncSetup.exe "点击下载")

> Android

[Android](./Release/MediaSync.apk "点击下载Android安装程序")

> MAC

[MAC](https://apps.apple.com/cn/app/%E7%BE%8E%E4%BA%BF%E6%A0%BC/id6444566528)

> IOS

<img src="./ios.png" align="left" />