# 1.安装必要工具

```shell
sudo apt-get install git  -y
sudo apt install make  -y
sudo apt-get install gcc  -y
sudo apt-get install cmake  -y
sudo apt-get install g++  -y
sudo apt-get install yasm  -y
sudo apt-get install pkgconf  -y
sudo apt-get install libtool  -y
sudo apt-get install bzip2  -y
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
# 安装samba
sudo apt install samba
# 创建samba用户s
sudo smbpasswd -a relech
# 修改samba配置文件
sudo vi /etc/samba/smb.conf 
# 增加 (检查一下 start.sh启动后默认会添加的，没有的话 手动添加上)
# /home/relech/mediasync 这个需要修改为实际的地址
include = /home/relech/mediasync/samba/lib/smbmedia.conf

# 重启
sudo systemctl restart smbd

# windows 建立samba连接
net use \\xxx.xxx.xxx.xxx\* /delete
net use Z: "\\xxx.xxx.xxx.xxx\media" /user:relech password 
```

# 6.使用介绍

> [使用介绍](http://www.yubaihe.net/function/homepage.html)

# 7.发行

> Windows

[Windows](http://www.yubaihe.net/pkg/MediaSync.exehttp://www.yubaihe.net/pkg/MediaSync.exe "点击下载Window程序")

> Android

[Android](http://www.yubaihe.net/pkg/MediaSync.apk "点击下载Android安装程序")

> MAC

[MAC](https://apps.apple.com/cn/app/%E7%BE%8E%E4%BA%BF%E6%A0%BC/id6444566528 "点击下载MAC程序")

> IOS

<img src="./ios.png" alt="扫描二维码下载IOS程序" />

# 8.百度网盘地址
```shell
#Android和Windows可以从网盘下载
通过网盘分享的文件：美亿格
链接: https://pan.baidu.com/s/1JwS0SoolUXsxjUecnATuSA?pwd=bhdz 提取码: bhdz
```
