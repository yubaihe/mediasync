#include "libdbus.h"
#include "stdio.h"
#include "CriticalSectionManager.h"
void __attribute__((constructor)) CommonInit() 
{
    //printf("CommonInit\n");
    dbus_threads_init_default();
}
void __attribute__((destructor)) CommonUnInit() 
{
    //printf("CommonUnInit\n");
    dbus_shutdown();
    CCriticalSectionManager::Release();
}

