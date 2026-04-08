#!/bin/sh
PWD=$(pwd)/package
echo "All will packaged in :" $PWD
mkdir  -p $PWD/install


SambaFilter()
{
	if [ ! -d "$ROOTDIR/samba/bin" ];then
		return
	fi
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
	#log
	mkdir -p $PWD/install/samba/log
	#private
	cp -r $ROOTDIR/samba/private $PWD/install/samba/
	#sbin
	mkdir -p $PWD/install/samba/sbin
	cp -r $ROOTDIR/samba/sbin/nmbd $PWD/install/samba/sbin/
	cp -r $ROOTDIR/samba/sbin/smbd $PWD/install/samba/sbin/
	#varlibpcre2
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
	cp -R $ROOTDIR/libs/libavutil.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libavformat.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libswscale.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libavfilter.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libavcodec.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libswresample.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libavdevice.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libx264.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libexiv2.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libiconv.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/preloadable_libiconv.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libpcre2-8.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libpcre2-posix.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libexpat.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libz.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libdbus-1.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libfcgi.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libfcgi++.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libcharset.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libcommon.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libsqlite3.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libmagic.so**  $PWD/install/libs/
	cp -R $ROOTDIR/libs/libcurl.so**  $PWD/install/libs/
}
BinFilter()
{
	echo "-----BinFilter"
	floders=`ls $ROOTDIR/bin`
	for floder in $floders
	do 
		#cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin 
		echo "$floder"
		if [ "$floder" = "exiv2" ]; then
			cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin/ 
		fi
		if [ "$floder" = "ffmpeg" ];then
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
		if [ "$floder" = "file" ];then
			cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin/
		fi
	done
}

rm -rf  $PWD/install
rm -rf  $PWD/install/apps
rm -rf  $PWD/install/libs
rm -rf  $PWD/install/bin
rm -rf  $PWD/install/lighttpd
rm -rf  $PWD/install/samba

# mkdir -p $PWD/install/apps
#所有相关的库
mkdir  -p $PWD/install/libs 
#把所有的应用拷贝进来
cp -R $ROOTDIR/apps $PWD/install/ 
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
