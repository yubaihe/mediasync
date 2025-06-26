//var ServerApi = new API("http://" + window.location.host );
//////////////////////////////低版本浏览器不支持Map////////////////////////////////////////
if (typeof Map == "undefined")
{
    function Map() {

        this.allkeys = new Array();
        this.data = new Object();

        this.set = function(key, value) 
        {
            if (this.data[key] == null) 
            {
                if (this.allkeys.indexOf(key) == -1) 
                {
                    this.allkeys.push(key);
                }
            }
            this.data[key] = value;
        }

        this.get = function(key) 
        {
            return this.data[key];
        }
        this.has = function(key)
        {
            return this.data[key] == undefined?false:true;
        }
        this.delete = function(key)
        {
            for(let i = 0; i < this.allkeys.length; ++i)
            {
                if(this.allkeys[i] == key)
                {
                    this.allkeys.splice(i, 1);
                    delete this.data[key];
                    break;
                }
            }
            // console.log("======>" + Object.keys(this.data).length);
        }
        this.keys = function()
        {
            return this.allkeys;
        }
    }
}

///////////////////////////////这里是为IOS准备的//////////////////////////////////////////
    function JsWebMethod()
    {
        var m = new Map();
        var timeid = null;
        JsWebMethod.prototype.GetVar = function (key)
        {
            return m.get(key);
        }
        JsWebMethod.prototype.SetVar = function (key, value)
        {
            m.set(key, value);
            if(timeid == null)
            {
                timeid = setInterval(()=>
                {
                    var time = new Date().getTime();
                    let keys = Array.from(m.keys());
                    //console.log("key len:" + keys.length);
                    for(let i = 0; i < keys.length; ++i)
                    {
                        if(time - keys[i] > 1000*60*2)//大于2分钟删除掉
                        {
                            this.RemoveVar(keys[i]);
                        }
                    }
                }, 1000*3);
            }
        }
        JsWebMethod.prototype.RemoveVar = function (key)
        {
            m.delete(key);
        }
        this.JsMethod = function(param1, param2)
        {
             if(window.chrome != undefined && window.chrome.webview != undefined)
            {
                //edge
                window.chrome.webview.postMessage(param2);
            }
            else if(window.webkit != undefined)
            {
                //Ios
                window.webkit.messageHandlers.JsCallback.postMessage(param2);
            }
            else
            {
                JsCallback.jsMethod(param2);
            }
            
        }
        this.CallBack = function(param1, param2)
        {
            var key = Number(param1);
            var ajaxData = g_JsCallback.GetVar(key);
            g_JsCallback.RemoveVar(key);
            ajaxData.success(param2, 200, "");
        }
        this.ExecuteJS = function(param1, param2)
        {
            //param1 这次请求的标识
            //param2 这次请求的数据
            // param2 = "TestTome();"; 
            // param2 = "function greet() { return 'Hello, World!'; } greet();"; 
            var strIdentify = param1+"";
            var ExecuteJsCallBack = "/executejscallback";
            console.log(param2);
            // let func = new Function(`return ${param2}`);  
            // let result = func();  
               // let func = new Function(param2);  
               // let result = func();  
               let result = eval(param2);
               if(typeof result == "undefined")
               {
                    result = "";
               }

            var data = strIdentify.length + (" " + strIdentify) + ExecuteJsCallBack.length + " " + ExecuteJsCallBack + result;
            g_JsCallback.JsMethod(ExecuteJsCallBack, data);
        }
    }
    var g_JsCallback = new JsWebMethod;
///////////////////////////////jquery 使用ajax部分高本的jquery在IE7下显示有错误 这里自己实现ajax//////////////////////////////////////////
function ajax(){
    var ajaxData = {
        type:arguments[0].type || "GET",
        url:arguments[0].url || "",
        async:arguments[0].async == false?false:true,
        data:arguments[0].data || "",
        dataType:arguments[0].dataType || "text",
        contentType:arguments[0].contentType || "application/x-www-form-urlencoded",
        beforeSend:arguments[0].beforeSend || function(){},
        success:arguments[0].success || function(){},
        error:arguments[0].error || function(){},
        timeout:arguments[0].timeout||5000
    }
   
    if(ajaxData.url.indexOf("fcgi") == -1)
    {
        if(typeof(JsCallback) == "undefined" && typeof(window.webkit) == "undefined" && typeof(window.chrome.webview) == "undefined")
        {
            console.log("not suport this action is for device:" + ajaxData.url);
            return;
        }

        setTimeout(function()
        {
            var time = new Date().getTime();
            var strTime = time+"";
            var data = (time+"").length + (" " + time) + ajaxData.url.length + " " + ajaxData.url + ajaxData.data;
            g_JsCallback.SetVar(time, ajaxData);
            g_JsCallback.JsMethod(ajaxData.url, data);

            // var time = new Date().getTime();
            // var data = time + "&" + ajaxData.url + "&" + ajaxData.data;
            // g_JsCallback.SetVar(time, ajaxData);
            // g_JsCallback.JsMethod(ajaxData.url, data);
            //ajaxData.success(ret, 200, "");
        },1);

        return;
    }
    ajaxData.beforeSend()
    var xhr = createxmlHttpRequest();
    var timeout = false;
    var timer = setTimeout(function(){
        timeout = true;
        xhr.abort();//请求中止
    },ajaxData.timeout);

    if(true == ajaxData.async)
    {
        // xhr.responseType=ajaxData.dataType;
    }
    
    try
    {
        xhr.open(ajaxData.type, ajaxData.url, ajaxData.async);
        if(ajaxData.data instanceof FormData)
    	{

    	}

        else
        {
        	xhr.setRequestHeader("Content-Type",ajaxData.contentType);
        }
        //xhr.send(convertData(ajaxData.data));
        xhr.send(ajaxData.data);
    }
    catch(err)
    {
        if(!timeout)
        {
            clearTimeout(timer);//取消等待的超时
            timeout = true;
        }
    }
    if(false == ajaxData.async)
    {

        if(xhr.status == 200){
            ajaxData.success(xhr.responseText, xhr.status, xhr);
        }else{
            if(!timeout)
            {
                clearTimeout(timer);//取消等待的超时
                timeout = true;
            }
            ajaxData.error(xhr, xhr.status);
        }
    }
    else {
        xhr.onreadystatechange = function() {
            if (xhr.readyState == 4) {
                if(xhr.status == 200){
                    ajaxData.success(xhr.responseText, xhr.status, xhr);
                }else{
                    if(!timeout)
                    {
                        clearTimeout(timer);//取消等待的超时
                        timeout = true;
                    }
                    ajaxData.error(xhr, xhr.status);
                }
            }
        }
    }
}

function createxmlHttpRequest() {
    if (window.ActiveXObject) {
        return new ActiveXObject("Microsoft.XMLHTTP");
    } else if (window.XMLHttpRequest) {
        return new XMLHttpRequest();
    }
}

// function convertData(data){
//     if( typeof data === 'object' ){
//         var convertResult = "" ;
//         for(var c in data){
//             convertResult+= c + "=" + data[c] + "&";
//         }
//         convertResult=convertResult.substring(0,convertResult.length-1)
//         return convertResult;
//     }else{
//         return data;
//     }
// }
//////////////////////////////////////////////////////////ajax实现结束////////////////////////////////////////////////////////////////////
function API(baseurl)
{
    var api = new Object;
    api.urlprefix = baseurl;
    api.m = new Map();
    api.GetVar = function (key) {
        return m[key];
    }
    api.SetVar = function (key, value) {
        m[key] = value;
    }
    api.RemoveVar = function (key) {
        delete m[key];
    }
    api.submit = function(method, url, param, callback, errorCallback)
    {
        var action = api.urlprefix+""+url;
        if(method == "get")
        {
            //get提交
            if(param.length > 0)
            {
                action = urlprefix+""+url+ "?"+param;
                param="";
            }
        }

        ajax({
            url:action,
            type:method, //GET
            async:true,    //或false,是否异步
            data:param,
            timeout:5000,    //超时时间
            ContentType: "application/octet-stream",
            dataType:'text',
            success:function(data,textStatus,jqXHR){
                if(null != callback)
                {
                    callback(data,textStatus,jqXHR);
                }
            },
            error:function(xhr,textStatus){
                if(null != errorCallback)
                {
                    errorCallback(xhr,textStatus);
                }
            }
        });
    }

    api.Post = function(action)
    {
        // console.log("==1>" + action);
        return function ()
        {
            var args = Array.prototype.slice.call(arguments);
            var callback = null, errorCallback = null;
            if(args.length && (args[args.length - 1] instanceof Function)) {
                callback = args.pop();
            }
            if(args.length && (args[args.length - 1] instanceof Function)) {
                errorCallback = callback;
                callback = args.pop();
            }
            var param = "";
            var urlsep = action.split("?");
            if(urlsep.length > 1)
            {
                param = urlsep[1];
            }
            else if (args.length >= 1)
            {
                param = args[0];
            }
            // console.log("==>" + param);
            var m = param instanceof FormData?true:param.match(/\{(\d+)\}/);
            if(m)
            {
                console.log(param);
                // if(urlsep.length > 1)
                // {
                //     //格式化的字符串在url里面
                //     for(var i = 0; i < args.length;++i)
                //     {
                //         param = param.replace("{"+i+"}", args[i]);
                //     }
                // }
                // else
                // {
                //     //格式化的字符串在参数里面
                //     for(var i = 1; i < args.length;++i)
                //     {
                //         alert(param);
                //         param = param.replace("{"+(i - 1)+"}", args[i]);
                //     }
                // }
            }
            else
            {
                // console.log("~~~~" + param);
                // console.log("没有找到:==>" + param);
            }
            
            api.submit("post", urlsep[0], param, callback, errorCallback);
        }
    }
    /*
    var oFormData = new FormData(); 
    oFormData.append('file',oMyFile.files[0]);
    oFormData.append('file',oMyFile.files[0], "a.txt");
    ServerApi.file.upload(oFormData, function(loaded,total,ev)
    {
        var iScale = ev.loaded / ev.total;
        oDiv2.style.width = 300 * iScale + 'px';//显示上传进度
        oDiv3.innerHTML = iScale * 100 + '%'; //显示上传百分百
    },function(data,textStatus,jqXHR){
        alert('data' + textStatus);
    },function(xhr,textStatus){
        alert("err");
    });
    */
    api.UploadFile = function(action)
    {
        return function(){
            var args = Array.prototype.slice.call(arguments);
        var param = "";
        if (args.length >= 1)
        {
            param = args[0];
        }
        var m = param instanceof FormData?true:false;

        var successCallback = null, errorCallback = null, processCallback = null;
        if(args.length && (args[args.length - 1] instanceof Function)) {
            errorCallback = args.pop();
        }
        if(args.length && (args[args.length - 1] instanceof Function)) {
            successCallback = args.pop();
        }
        if(args.length && (args[args.length - 1] instanceof Function)) {
            processCallback = args.pop();
        }
        if(m)
        {
            var xhr = createxmlHttpRequest();
            xhr.onload = function()
            {
                successCallback(xhr.responseText, xhr.status, xhr);
            }
            xhr.error = function()
            {
                errorCallback(xhr.responseText, xhr.status, xhr);
            }
            var oUpload = xhr.upload;
            // 上传过程中是连续被触发的 监控上传进度
            oUpload.onprogress = function(ev)
            {
                //已经发送 ev.loaded
                //总发送量 ev.total
                processCallback(ev.loaded, ev.total, ev);
            }
            xhr.open("POST", api.urlprefix + action, true);
            //在使用post发送的时候必须要带一些请求头信息
            xhr.setRequestHeader('X-Request-With', 'XMLHttpRequest');
            xhr.send(param);
        }
        };
    }

    api.Get = function( action)
    {
        return function ()
        {
            var args = Array.prototype.slice.call(arguments);
            var callback = null, errorCallback = null;
            if(args.length && (args[args.length - 1] instanceof Function)) {
                callback = args.pop();
            }
            if(args.length && (args[args.length - 1] instanceof Function)) {
                errorCallback = callback;
                callback = args.pop();
            }
            var param = "";
            var urlsep = action.split("?");
            if(urlsep.length > 1)
            {
                param = urlsep[1];
            }
            else if (args.length >= 1)
            {
                param = args[0];
            }

            var m = param instanceof FormData?false:param.match(/\{(\d+)\}/);
            if(m)
            {
                if(urlsep.length > 1)
                {
                    //格式化的字符串在url里面
                    for(var i = 0; i < args.length;++i)
                    {
                        param = param.replace("{"+i+"}", args[i]);
                    }
                }
                else
                {
                    //格式化的字符串在参数里面
                    for(var i = 1; i < args.length;++i)
                    {
                        param = param.replace("{"+(i - 1)+"}", args[i]);
                    }
                }
            }

            API.submit("get", urlsep[0], param, callback, errorCallback);
        }
    }

    api.DoPost = function()
    {
    	 var args = Array.prototype.slice.call(arguments);
    	 var callback = null, errorCallback = null;
         if(args.length && (args[args.length - 1] instanceof Function)) {
             callback = args.pop();
         }
         if(args.length && (args[args.length - 1] instanceof Function)) {
             errorCallback = callback;
             callback = args.pop();
         }
         var param = "";
         var urlsep = args[0].split("?");
         if(urlsep.length > 1)
         {
             param = urlsep[1];
         }
         else if (args.length >= 2)
         {
             param = args[1];
         }

         var m = param instanceof FormData?false:param.match(/\{(\d+)\}/);
         if(m)
         {
             if(urlsep.length > 1)
             {
                 //格式化的字符串在url里面
                 for(var i = 1; i < args.length;++i)
                 {
                     param = param.replace("{"+(i - 1)+"}", args[i]);
                 }
             }
             else
             {
                 //格式化的字符串在参数里面
                 for(var i = 2; i < args.length;++i)
                 {
                     param = param.replace("{"+(i - 2)+"}", args[i]);
                 }
             }
         }

         API.submit("post", urlsep[0], param, callback, errorCallback);
    }
    api.device={
        toast:api.Post("/toast"), //toast
        loginsuccess:api.Post("/loginsuccess"),
        mediasync:api.Post("/mediasync"),
        mediasetting:api.Post("/mediasetting"),
        medialist:api.Post("/medialist"),
        mediafavorite:api.Post("/mediafavorite"),
        settingupdate:api.Post("/settingupdate"),
        resetuser:api.Post("/resetuser"),
        netsetting:api.Post("/netsetting"),
        languageset:api.Post("/languageset"),
        howtouse:api.Post("/howtouse"),
        aboutdevice:api.Post("/aboutdevice"),
        inputchange:api.Post("/inputchange"), //输入框改变
        itemdetail:api.Post("/itemdetail"), //显示媒体详细信息
        yearinfo:api.Post("/yearinfo"),
        recentupload:api.Post("/recentupload"),
        gpsaddress:api.Post("/gpsaddress"),
        favorite:api.Post("/favorite"),
        cover:api.Post("/cover"),
        confirm:api.Post("/confirm"),
        locationgroup:api.Post("/locationgroup"),
        updatetime:api.Post("/updatetime"),
        timeupdate:api.Post("/timeupdate"),
        gpsupdate:api.Post("/gpsupdate"),
        canback:api.Post("/canback"),
        monthsel:api.Post("/monthsel"),
        daysinfo:api.Post("/daysinfo"),
        daysel:api.Post("/daysel"),
        groupinfo:api.Post("/groupinfo"),
        groupitemselect:api.Post("/groupitemselect"),
        groupchange:api.Post("/groupchange"),
        mediaitemgroupchange:api.Post("/mediaitemgroupchange"),
        updatefenzu:api.Post("/updatefenzu"),
        removeitems:api.Post("/removeitems"),
        enterselectmode:api.Post("/enterselectmode"),
        dataempty:api.Post("/dataempty"),
        setitemsgroup:api.Post("/setitemsgroup"),
        mediaitemsgroupadd:api.Post("/mediaitemsgroupadd"),
        languageset:api.Post("/languageset"),
        languagesetsure:api.Post("/languagesetsure"),
        ClearCacheSuccess:api.Post("/clearcachesuccess"),
        ClearCacheError:api.Post("/clearcacheerror"),
        callback:api.Post("/callback"),
        diskchoose:api.Post("/diskchoose"),
        selectdisk:api.Post("/selectdisk"),
        showignorepage:api.Post("/showignorepage"),
        loadfinish:api.Post("/loadfinish"),
        diskuploadfinish:api.Post("/diskuploadfinish"),
        diskuploadstop:api.Post("/diskuploadstop"),
        diskdeleteitem:api.Post("/diskdeleteitem"),
        diskswitchitem:api.Post("/diskswitchitem"),
        disksetignoreack:api.Post("/disksetignoreack"),
        diskaddignore:api.Post("/diskaddignore"),
        diskremoveignore:api.Post("/diskremoveignore"),
        closewindow:api.Post("/closewindow"),
        deleteitem:api.Post("/deleteitem"),
        selectitemchange:api.Post("/selectitemchange"),
        selectbackupfold:api.Post("/selectbackupfold"),
        changebackupfold:api.Post("/changebackupfold"),
        backupfoldchange:api.Post("/backupfoldchange"),
        backup:api.Post("/backup"),
        backupfoldselect:api.Post("/backupfoldselect"),
        backupremoveitems:api.Post("/backupremoveitems"),
        backupdeleteitem:api.Post("/backupdeleteitem"),
        backupswitchitem:api.Post("/backupswitchitem"),
        mediapinchange:api.Post("/mediapinchange"),
        backupfilesync:api.Post("/backupfilesync"),
    };

    api.request={
        deviceinfo:api.Post("/server.fcgi"),
        diskinfo:api.Post("/server.fcgi"),
        login:api.Post("/server.fcgi"),
        pwdtip:api.Post("/server.fcgi"),
        recentupload:api.Post("/server.fcgi"),
        favorite:api.Post("/server.fcgi"),
        yearitemlist:api.Post("/server.fcgi"),
        yearlist:api.Post("/server.fcgi"),
        monthlist:api.Post("/server.fcgi"),
        recentinfo:api.Post("/server.fcgi"),
        yearinfo:api.Post("/server.fcgi"),
        devnames:api.Post("/server.fcgi"),
        updategpslocation:api.Post("/server.fcgi"),
        yearlocationgroup:api.Post("/server.fcgi"),
        yearlocationgrouptongji:api.Post("/server.fcgi"),
        aboutdevice:api.Post("/server.fcgi"),
        updatetime:api.Post("/server.fcgi"),
        daylist:api.Post("/server.fcgi"),
        yearmonthdaycover:api.Post("/server.fcgi"),
        clearcachestart:api.Post("/server.fcgi"),
        clearcacheprocess:api.Post("/server.fcgi"),
        groupinfo:api.Post("/server.fcgi"),
        groupadd:api.Post("/server.fcgi"),
        groupupdate:api.Post("/server.fcgi"),
        groupdel:api.Post("/server.fcgi"),  
        groupdetail:api.Post("/server.fcgi"),
        mediaitemgroupsetting:api.Post("/server.fcgi"),
        groupitemlist:api.Post("/server.fcgi"),
        delmediaitem:api.Post("/server.fcgi"),
        groupnamesfromitemid:api.Post("/server.fcgi"),
        mediaitemsgroupadd:api.Post("/server.fcgi"),
        listdisk:api.Post("/server.fcgi"),
        deventer:api.Post("/mediaparse.fcgi"),
        devsyncprecent:api.Post("/mediaparse.fcgi"),
        diskmediaitemlist:api.Post("/mediaparse.fcgi"),
        diskaddignore:api.Post("/mediaparse.fcgi"),
        diskmediaitemignorelist:api.Post("/mediaparse.fcgi"),
        diskremoveignore:api.Post("/mediaparse.fcgi"),
        diskuploaditemlist:api.Post("/mediaparse.fcgi"),
        diskitemuploadprecent:api.Post("/mediaparse.fcgi"),
        diskitemupload:api.Post("/mediaparse.fcgi"),
        diskitemdetail:api.Post("/mediaparse.fcgi"),
        diskdeleteitem:api.Post("/mediaparse.fcgi"),
        disksetignore:api.Post("/mediaparse.fcgi"),
        backupfolds:api.Post("/mediaparse.fcgi"),
        backupfolddel:api.Post("/mediaparse.fcgi"),
        backupfoldadd:api.Post("/mediaparse.fcgi"),
        backupfoldmodify:api.Post("/mediaparse.fcgi"),
        backupuploaditem:api.Post("/mediaparse.fcgi"),
        backupuploaditemprecent:api.Post("/mediaparse.fcgi"),
        backupinfo:api.Post("/mediaparse.fcgi"),
        backupuploaditemlist:api.Post("/mediaparse.fcgi"),
        backupuploaditemdel:api.Post("/mediaparse.fcgi"),
        backupfolditemdetail:api.Post("/mediaparse.fcgi"),
        synctodevice:api.Post("/mediaparse.fcgi"),
        synctodeviceprecent:api.Post("/mediaparse.fcgi"),
        itemdetail:api.Post("/server.fcgi"),
        mediafavorite:api.Post("/server.fcgi"),
        setcover:api.Post("/server.fcgi"),
        delmediaitem:api.Post("/server.fcgi"),
        gpslocation:api.Post("/server.fcgi"),
        setbaiduak:api.Post("/server.fcgi"),
        setmediapinned:api.Post("/server.fcgi"),
        getwifistatus:api.Post("/network.fcgi"),
        getbrtype:api.Post("/network.fcgi"),
        setbrtype:api.Post("/network.fcgi"),
        getwifiitems:api.Post("/network.fcgi"),
        getstorewifiitems:api.Post("/network.fcgi"),
        gethotpot:api.Post("/network.fcgi"),
        sethotpot:api.Post("/network.fcgi"),
        connectwifiitem:api.Post("/network.fcgi"),
        unconnectwifiitem:api.Post("/network.fcgi"),
        delstorewifiitem:api.Post("/network.fcgi"),
        getenv:api.Post("/network.fcgi"),
        
    };
    api.file={
        upload:api.UploadFile("/upload.fcgi")
    };
    return api;
}
