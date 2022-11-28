
    //sudo apt-get install nfs-common
    //sudo apt-get install cifs-utils 
    //sudo apt-get install virtualbox-guest-utils
    //如果是fat格式的U盘，挂载命令：mount -t vfat /dev/sdb1 /mnt/udisk
//如果是ntfs格式的U盘，挂载命令：mount -t ntfs-3g /dev/sdb1 /mnt/udisk
    #include <stdio.h>  
    #include <stdlib.h>  
    #include <string.h>  
    #include <fcntl.h>  
     #include <sys/types.h>
       #include <unistd.h>
    #include <sys/socket.h>  
    #include <linux/netlink.h>  
    #define UEVENT_BUFFER_SIZE 65535  
    int main()
{

     struct sockaddr_nl client;  
    struct timeval tv;  
    int CppLive, rcvlen, ret;  
    fd_set fds;  
    int buffersize = 1024; 
 char *pszFindStr = NULL;
 char aUsbNode[15] = {"/dev/"};
 char command[100];

/*创建AF_NETLINK套接字*/
    CppLive = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);  
    /*许该套接字复用其他端口*/
    setsockopt(CppLive, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));  
 
 /*绑定套接字*/
 memset(&client, 0, sizeof(client));  
    client.nl_family = AF_NETLINK;  
    client.nl_pid = getpid();  
    client.nl_groups = 1; /* receive broadcast message*/  
    bind(CppLive, (struct sockaddr*)&client, sizeof(client));
    while (1) {  
        char buf[UEVENT_BUFFER_SIZE] = { 0 };  
        FD_ZERO(&fds);  
        FD_SET(CppLive, &fds);  
        tv.tv_sec = 0;  
        tv.tv_usec = 100 * 1000;  
  /*监听socket*/
        ret = select(CppLive + 1, &fds, NULL, NULL, &tv);  
        if(ret < 0)  
            continue;  
        if(!(ret > 0 && FD_ISSET(CppLive, &fds)))  
            continue;  
        /* receive data */  
        rcvlen = recv(CppLive, &buf, sizeof(buf), 0);  
        if (rcvlen > 0) {  
            printf("%s\n", buf);  
            /*提取USB节点*/
   if( (strstr(buf, "add@/device") != NULL) && (strstr(buf, "block/sd") != NULL))
   {
    pszFindStr = strstr(buf, "block/sd");
    sscanf(pszFindStr+6,"%s",aUsbNode+5);
    printf("udisk_path = %s \r\n",aUsbNode);
    
    /*节点挂载*/
    memset(command,0,sizeof(command));
    sprintf(command,"mount -t vfat %s %s > /dev/null",aUsbNode,"/mnt/usb");
    if( 0 == system(command))
    {
     printf("Usb mount success\r\n");
     /*操作USB*/
     FILE *fp = 0;
     fp = fopen("/mnt/usb/helloword.txt","a+");
     if(fp != 0)
     {
      fwrite("hellowold",1,sizeof("hellowold"),fp);
      fclose(fp);
      system("ls /mnt/usb/hell*");
     }
    }
    else
     printf("mount err\r\n");
   }
        }  
    }  
    close(CppLive);  
    return 0;  
}