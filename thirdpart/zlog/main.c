#include <stdio.h>
#include <zlog.h>
#include <unistd.h>
#include <stdlib.h>
#include<unistd.h>
int main()
{
	int iRet = access("/dev/sda1", R_OK);
	printf("%d\n", iRet);
	//char* pSdMount = getenv("SD_MOUNT");
	// if(NULL == pSdMount)
	// {
	// 	return 1;
	// }
	// int iMount = atoi(pSdMount);
	// if (iMount == 1)
	// {
	// 	system("mount -o remount rw /media/mmcblk0p1/");
	// 	system("mkdir -p /media/mmcblk0p1/log/;rm -rf /media/mmcblk0p1/log/**");
	// 	rc = dzlog_init(INI_CONF_1, ".");
	// 	LOG_DBG("read INI_CONF_1");
	// }
	// else
	// {
	// 	system("mkdir -p /media/mmcblk0p1/log/;rm -rf /media/mmcblk0p1/log/**");
	// 	dzlog_init(INI_CONF_2, ".");
	// 	LOG_DBG("read INI_CONF_2");
	// }
	
	// while (1)
	// {
	// 	char* pszTmpSdMount = getenv("SD_MOUNT");
	// 	if(NULL == pszTmpSdMount)
	// 	{
	// 		return 1;
	// 	}
	// 	int iTmpMount = atoi(pszTmpSdMount);
	// 	if(iTmpMount != iMount)
	// 	{
	// 		return 1;
	// 	}
	// }
	


    // int rc;
	// rc = dzlog_init("zlog.conf", "my_class"); //指定配置文件路径及类型名 初始化zlog
	// if (rc) 
	// {
	// 	printf("init failed\n");
	// 	return -1;
	// }
    // do 
    // {
    //     dzlog_info("hello, zlog info"); //打印普通信息
	//     dzlog_error("hello, zlog error");//打印错误信息
	//     dzlog_warn("hello, zlog warning"); //打印报警信息
	//     dzlog_debug("hello, zlog debug"); //打印调试信息
	// 	usleep(1000);
    // }while (1);
	// zlog_fini();  //释放zlog
    return 1;
}