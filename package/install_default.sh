#!/bin/sh
PWD=$(pwd)/package
echo "All will packaged in :" $PWD
mkdir  -p $PWD/install


LighttpdFilter()
{
	echo "-----LighttpdFilter"
	mkdir -p $PWD/install/lighttpd
	#config
	cp -R $ROOTDIR/lighttpd/config $PWD/install/lighttpd
	#fcgi
	mkdir -p $PWD/install/lighttpd/fcgi
	cp -R $ROOTDIR/lighttpd/fcgi/server.fcgi $PWD/install/lighttpd/fcgi/
	cp -R $ROOTDIR/lighttpd/fcgi/upload.fcgi $PWD/install/lighttpd/fcgi/
	cp -R $ROOTDIR/lighttpd/fcgi/Config.json $PWD/install/lighttpd/fcgi/
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
    cp -R $ROOTDIR/libs/libexiv2.so**  $PWD/install/libs/
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
			cp -R $ROOTDIR/bin/$floder/exiv2json  $PWD/install/bin/ 
		fi
		if [ "$floder" = "ffmpeg" ];then
			cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin/
		fi
		if [ "$floder" = "file" ];then
			cp -R $ROOTDIR/bin/$floder/**  $PWD/install/bin/
		fi
		if [ "$floder" = "imagemagic" ];then
			cp -R $ROOTDIR/bin/$floder/magick  $PWD/install/bin/
			cp -R $ROOTDIR/bin/$floder/convert  $PWD/install/bin/
		fi
		if [ "$floder" = "ntfs-3g" ];then
			cp -R $ROOTDIR/bin/$floder/ntfs-3g  $PWD/install/bin/
		fi
	done
}

rm -rf  $PWD/install
rm -rf  $PWD/install/apps
rm -rf  $PWD/install/libs
rm -rf  $PWD/install/bin
rm -rf  $PWD/install/lighttpd

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
LibFilter

rm -rf $ROOTDIR
mkdir $ROOTDIR -p
cp package/install/** $ROOTDIR -R
rm $ROOTDIR/apps/Server/Config_default.json

echo '#!/bin/bash' > $ROOTDIR/start.sh
echo 'if [ -z "$ROOTDIR" ]' >> $ROOTDIR/start.sh
echo 'then' >> $ROOTDIR/start.sh
echo '	export ROOTDIR=$(dirname $(realpath $0))' >> $ROOTDIR/start.sh
echo 'fi' >> $ROOTDIR/start.sh
echo 'export PATH=$PATH:$ROOTDIR/apps/Server:$ROOTDIR/apps/MetaParse:$ROOTDIR/apps/DeviceMonitor:$ROOTDIR/lighttpd/sbin:$ROOTDIR/bin' >> $ROOTDIR/start.sh
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ROOTDIR/libs' >> $ROOTDIR/start.sh
echo '$ROOTDIR/lighttpd/sbin/lighttpd -f $ROOTDIR/lighttpd/config/lighttpd.conf' >> $ROOTDIR/start.sh
echo '$ROOTDIR/apps/Server/Server &' >> $ROOTDIR/start.sh
chmod +x $ROOTDIR/start.sh

echo '#!/bin/bash' > $ROOTDIR/stop.sh
echo 'killall lighttpd' >> $ROOTDIR/stop.sh
echo 'killall Server' >> $ROOTDIR/stop.sh
chmod +x $ROOTDIR/stop.sh