#!/bin/sh
#自己开发的应用
PWD=$(pwd)/package
echo "All will packaged in :" $PWD

SambaFilter()
{
	echo "-----SambaFilter"
	mkdir -p $PWD/install/samba
	#bin
	mkdir -p $PWD/install/samba/bin
	cp $ROOTDIR/samba/bin/smbpasswd $PWD/install/samba/bin
	#lib
	mkdir -p $PWD/install/samba/lib
	cp $ROOTDIR/samba/lib/smb.conf $PWD/install/samba/lib/
	cp $ROOTDIR/samba/lib/smbmedia.conf $PWD/install/samba/lib/
	cp $ROOTDIR/samba/lib/smbusers $PWD/install/samba/lib/
#	cp $ROOTDIR/samba/lib/libattr.so** $PWD/install/samba/lib/
#	cp $ROOTDIR/samba/lib/libavahi-client.so** $PWD/install/samba/lib/
#	cp $ROOTDIR/samba/lib/libiconv.so** $PWD/install/samba/lib/
#	cp $ROOTDIR/samba/lib/libpcre.so.1** $PWD/install/samba/lib/
#	cp $ROOTDIR/samba/lib/libtalloc.so** $PWD/install/samba/lib/
#	cp $ROOTDIR/samba/lib/libtdb.so** $PWD/install/samba/lib/
#	cp $ROOTDIR/samba/lib/libtevent.so** $PWD/install/samba/lib/
#	cp $ROOTDIR/samba/lib/libwbclient.so** $PWD/install/samba/lib/
#	cp $ROOTDIR/samba/config/smb.conf $PWD/install/samba/lib/
	#log
	mkdir -p $PWD/install/samba/log
	#private
	cp -r $ROOTDIR/samba/private $PWD/install/samba/
	#sbin
	mkdir -p $PWD/install/samba/sbin
	cp -r $ROOTDIR/samba/sbin/nmbd $PWD/install/samba/sbin/
	cp -r $ROOTDIR/samba/sbin/smbd $PWD/install/samba/sbin/
	#cp -r $ROOTDIR/samba/share $PWD/install/samba/
	#var
	cp -r $ROOTDIR/samba/var $PWD/install/samba/
}	

LighttpdFilter()
{
	echo "-----LighttpdFilter"
	mkdir -p $PWD/install/lighttpd
	#config
	cp -R $ROOTDIR/lighttpd/config $PWD/install/lighttpd
	#fcgi
	mkdir -p $PWD/install/lighttpd/fcgi
	cp -R $ROOTDIR/lighttpd/fcgi/** $PWD/install/lighttpd/fcgi/
	#lib
	mkdir -p $PWD/install/lighttpd/lib
	cp -R $ROOTDIR/lighttpd/lib/*.so $PWD/install/lighttpd/lib/
	#log
	mkdir -p $PWD/install/lighttpd/log
	#sbin
	cp -R $ROOTDIR/lighttpd/sbin $PWD/install/lighttpd/
	#www
	cp -R $ROOTDIR/lighttpd/www $PWD/install/lighttpd/
}

LibFilter()
{
	echo "-----LibFilter"
	cp -R $ROOTDIR/libs/libexpat.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libpcre2-8.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libz.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libcommon.so  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libdbus-1.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libsqlite3.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libsqlite3.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libfcgi.so** $PWD/install/libs/
	cp -R $ROOTDIR/libs/libfcgi++.so.0** $PWD/install/libs/
	cp $ROOTDIR/libs/libntfs-3g.so** $PWD/install/libs/
	cp -R $ROOTDIR/libs/libcurl.so**  $PWD/install/libs/
}

BinFilter()
{
	echo "-----BinFilter"
	floders=`ls $ROOTDIR/bin`
	for floder in $floders
	do  
		echo "$floder"
		if [ "$floder" = "exiv2" ]; then
			echo "=========>exiv2 drop"
#			cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin/ 
#			cp -R $ROOTDIR/libs/libexiv2.so**   $PWD/install/libs/
		fi
		if [ "$floder" = "ffmpeg" ];then
			echo "=========>ffmpeg drop"
#			cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin/
#			cp $ROOTDIR/libs/libavcodec.so** $PWD/install/libs/
#			cp $ROOTDIR/libs/libavdevice.so** $PWD/install/libs/
#			cp $ROOTDIR/libs/libavfilter.so** $PWD/install/libs/
#			cp $ROOTDIR/libs/libavformat.so** $PWD/install/libs/
#			cp $ROOTDIR/libs/libavutil.so** $PWD/install/libs/
#			cp $ROOTDIR/libs/libswresample.so** $PWD/install/libs/
#			cp $ROOTDIR/libs/libswscale.so** $PWD/install/libs/
		fi
		if [ "$floder" = "file" ];then
			cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin/
		fi
		if [ "$floder" = "ntfs-3g" ];then
			cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin/
		fi
		if [ "$floder" = "dbus" ];then
			cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin/
		fi
		if [ "$floder" = "ntp" ];then
			cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin/
		fi
	done
}

rm -rf  $PWD/install/apps
rm -rf  $PWD/install/libs
rm -rf  $PWD/install/bin
rm -rf  $PWD/install/lighttpd
rm -rf  $PWD/install/samba

#所有相关的库
mkdir  -p $PWD/install/libs 
#把所有的应用拷贝进来
mkdir -p /home/relech/Linux/package/install/apps
cp -R $ROOTDIR/apps/DeviceMonitor $PWD/install/apps/DeviceMonitor
cp -R $ROOTDIR/apps/Server $PWD/install/apps/Server
#把所有的驱动拷贝进来
cp -R $ROOTDIR/driver $PWD/install/ 
#第三方可执行文件
mkdir -p $PWD/install/bin 
#bin拷贝
BinFilter
if [ -d "$ROOTDIR/lighttpd" ];then
	LighttpdFilter
fi
if [ -d "$ROOTDIR/samba" ];then
	SambaFilter
fi
LibFilter

echo $BUILDROOT_OUT
rm -rf $BUILDROOT_OUT/relech/home/relech/mediasync
mkdir $BUILDROOT_OUT/relech/home/relech/mediasync -p
cp $PWD/install/** $BUILDROOT_OUT/relech/home/relech/mediasync/ -R
du -h $BUILDROOT_OUT/relech/home/relech/mediasync/

#替换配置文件
## DeviceMonitor 配置为不可以导入
sed -i 's/"importenable":1,/"importenable":0,/g' $PWD/install/apps/DeviceMonitor/Config.json