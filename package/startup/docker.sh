#!/bin/bash
export ROOTDIR=/home/relech/mediasync/
export PATH=$PATH:$ROOTDIR/apps/Server
export PATH=$ROOTDIR/apps/MediaParse:$ROOTDIR/lighttpd/sbin:$ROOTDIR/bin/dbus:$ROOTDIR/bin/exiv2:$ROOTDIR/bin/ffmpeg:$ROOTDIR/bin/ntp::$ROOTDIR/bin/file:$PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ROOTDIR/libs
mkdir -p /run/user/$(id -u)
export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u)/bus

#config sambd
smbconfigfile="/etc/samba/smb.conf" 
mediaconfigfile="include = ${ROOTDIR}/samba/lib/smbmedia.conf"
if ! grep -q "^${mediaconfigfile}$" "$smbconfigfile"; then
    echo "$mediaconfigfile" >> "$smbconfigfile"
    echo "append"
else
    echo "not append"
fi
#start smbd
smbd &
nmbd &

$ROOTDIR/bin/dbus-daemon --config-file=${ROOTDIR}/bin/dbus-1/session.conf  --address=$DBUS_SESSION_BUS_ADDRESS &
mkdir $ROOTDIR/media/.media_tmp -p
$ROOTDIR/lighttpd/sbin/lighttpd -f $ROOTDIR/lighttpd/config/lighttpd.conf
$ROOTDIR/apps/MediaParse/MediaParse &
exec $ROOTDIR/apps/Server/Server