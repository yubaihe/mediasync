# ntfs

```shell
#https://blog.csdn.net/wkd_007/article/details/117068747
tar zxf ntfs-3g_ntfsprogs-2017.3.23.tgz  
cd ntfs-3g_ntfsprogs-2017.3.23/
./configure  --prefix=$PWD/result --exec-prefix=$PWD/result --host=arm-poky-linux-gnueabi --disable-shared --enable-static
make
```

