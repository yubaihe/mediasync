function ClearCacheStart()
    {
        SetToast(LanguageText("缓存清理:") + "0%");
        var str = "{\"action\":\"clearcachestart\"}";
        ServerApi.request.clearcachestart(str,function(data,textStatus,jqXHR)
        {
            var jsonRoot = JSON.parse(data);
            if(jsonRoot.status != 1)
            {
                Toast(LanguageText("缓存清理失败!"));
                return;
            }
            GetClearCacheStatus();
        },
        function(xhr,textStatus)
        {
        });
    }

    function GetClearCacheStatus()
    {
        var timer = setInterval(function(){
            var str = "{\"action\":\"clearcacheprocess\"}";
            ServerApi.request.clearcacheprocess(str, function(data,textStatus,jqXHR)
            {
                var jsonRoot = JSON.parse(data);
                console.log(data);
                SetToast(LanguageText("缓存清理:") + jsonRoot.precent + "%");
                if(jsonRoot.status != 1)
                {
                    UnSetToast();
                    Toast(LanguageText("缓存清理失败!"));
                    return;
                }
                if(jsonRoot.pstatus == 1)
                {
                    UnSetToast();
                    Toast(LanguageText("缓存清理成功!"));
                    clearInterval(timer);
                }
                if(jsonRoot.pstatus == -1)
                {
                    UnSetToast();
                    Toast(LanguageText("缓存清理失败!"));
                    clearInterval(timer);
                }
            },
            function(xhr,textStatus)
            {
                UnSetToast();
                Toast(LanguageText("缓存清理失败!"));
                clearInterval(timer);
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
                //Toast("缓存清理失败");
                return;
            }
            if(jsonRoot.pstatus == 1)
            {
                //Toast("缓存清理成功");
                return;
            }
            if(jsonRoot.pstatus == -1)
            {
                //Toast("缓存清理失败");
                return;
            }
            callBack(jsonRoot.precent);
        },
        function(xhr,textStatus)
        {
            //Toast("缓存清理失败");
        });
    }