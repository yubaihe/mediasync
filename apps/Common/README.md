# Dbus

> Dbus需要在相同的session下才能通信
>
> 命令行下 `id -u` 可以查询当前sessionid， 比如1000是当前用户， 0是root用户 

```shell
relech@Ubuntu:~/Linux$ id -u
1000
```

```shell
#设置dbus环境（0:代表root用户）
mkdir -p /run/user/$(id -u)
export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u)/bus
#启动dbus-daemon如果要通信需要保证后续启动进程的sessionid保持一致
/home/relech/mediasync/bin/dbus/dbus-daemon --session --address=$DBUS_SESSION_BUS_ADDRESS &


#----------------------说明-----------
#可以通过不同的DBUS_SESSION_BUS_ADDRESS启动多个dbus-daemon，如果发现通信不上可以查看DBUS_SESSION_BUS_ADDRESS
echo $DBUS_SESSION_BUS_ADDRESS
#如果不同或者为空将上面的 $(id -u) 替换成dbus-daemon的--address指定的值，比如dbus-daemon是这样的：
dbus-daemon --session --address=unix:path=/run/user/1000/bus
 #那么export指令应该是这样的
	export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus
```

# 查询Dbus服务列表的指令

```shell
relech@Ubuntu:~/Linux$ /home/relech/mediasync/bin/dbus/dbus-send --session --dest=org.freedesktop.DBus --print-reply /org/freedesktop/DBus org.freedesktop.DBus.ListNames
method return time=1741085320.985152 sender=org.freedesktop.DBus -> destination=:1.0 serial=3 reply_serial=2
   array [
      string "org.freedesktop.DBus"
      string ":1.0"
   ]
```

