var g_clearcachetimer = null;
function IsClearCacheRunning()
{
    if(null == g_clearcachetimer)
    {
        return false;
    }
    return true;
}
function ClearCacheStart(DeviceApi)
    {
        if(null != g_clearcachetimer)
        {
            return;
        }
        SetToast(LanguageText("磁盘整理:") + "0%");
        var str = "{\"action\":\"clearcachestart\"}";
        ServerApi.request.clearcachestart(str,function(data,textStatus,jqXHR)
        {
            var jsonRoot = JSON.parse(data);
            if(jsonRoot.status != 1)
            {
                Toast(LanguageText("磁盘整理失败!"));
                return;
            }
            GetClearCacheStatus(DeviceApi);
        },
        function(xhr,textStatus)
        {
        });
    }

    function GetClearCacheStatus(DeviceApi)
    {
        g_clearcachetimer = setInterval(function(){
            var str = "{\"action\":\"clearcacheprocess\"}";
            ServerApi.request.clearcacheprocess(str, function(data,textStatus,jqXHR)
            {
                var jsonRoot = JSON.parse(data);
                console.log(data);
                SetToast(LanguageText("磁盘整理:") + jsonRoot.precent + "%");
                if(jsonRoot.status != 1)
                {
                    UnSetToast();
                    Toast(LanguageText("磁盘整理失败!"));
                    clearInterval(g_clearcachetimer);
                    g_clearcachetimer = null;
                    DeviceApi.device.ClearCacheError("response error");
                    return;
                }
                if(jsonRoot.pstatus == 1)
                {
                    UnSetToast();
                    Toast(LanguageText("磁盘整理成功!"));
                    clearInterval(g_clearcachetimer);
                    g_clearcachetimer = null;
                    DeviceApi.device.ClearCacheSuccess();
                }
                if(jsonRoot.pstatus == -1)
                {
                    UnSetToast();
                    Toast(LanguageText("磁盘整理失败!"));
                    clearInterval(g_clearcachetimer);
                    g_clearcachetimer = null;
                    DeviceApi.device.ClearCacheError("clear error");
                }
            },
            function(xhr,textStatus)
            {
                UnSetToast();
                Toast(LanguageText("磁盘整理失败!"));
                clearInterval(g_clearcachetimer);
                g_clearcachetimer = null;
                DeviceApi.device.ClearCacheError("request timeout");
            });
        }, 1*1000);
    }


    function GetClearCacheStatusWithCallBack(callBack)
    {
        var str = "{\"action\":\"clearcacheprocess\"}";
        ServerApi.request.clearcacheprocess(str, function(data,textStatus,jqXHR)
        {
            var jsonRoot = JSON.parse(data);
            console.log(data);
            if(jsonRoot.status != 1)
            {
                //Toast("磁盘整理失败");
                return;
            }
            if(jsonRoot.pstatus == 1)
            {
                //Toast("磁盘整理成功");
                return;
            }
            if(jsonRoot.pstatus == -1)
            {
                //Toast("磁盘整理失败");
                return;
            }
            callBack(jsonRoot.precent);
        },
        function(xhr,textStatus)
        {
            //Toast("磁盘整理失败");
        });
    }