//var ServerApi = new API("http://" + window.location.host );
//////////////////////////////低版本浏览器不支持Map////////////////////////////////////////
if (typeof Map == "undefined")
{
    function Map() {

        this.keys = new Array();
        this.data = new Object();

        this.set = function(key, value) {
            if (this.data[key] == null) {
                if (this.keys.indexOf(key) == -1) {
                    this.keys.push(key);
                }
            }
            this.data[key] = value;
        }

        this.get = function(key) {
            return this.data[key];
        }
        this.has = function(key)
        {
            return this.data[key] == undefined?false:true;
        }
    }
}
///////////////////////////////这里是为IOS准备的//////////////////////////////////////////
    function JsWebMethod()
    {
        var m = new Map();
        JsWebMethod.prototype.GetVar = function (key)
        {
            return m[key];
        }
        JsWebMethod.prototype.SetVar = function (key, value)
        {
            m[key] = value;
        }
        JsWebMethod.prototype.RemoveVar = function (key)
        {
            delete m[key];
        }
        this.JsMethod = function(param1, param2)
        {
            if(window.webkit == undefined)
            {
                JsCallback.jsMethod(param2);
            }
            else
            {
                window.webkit.messageHandlers.JsCallback.postMessage(param2);
            }
            
        }
        this.CallBack = function(param1, param2)
        {
            var ajaxData = g_JsCallback.GetVar(param1);
            g_JsCallback.RemoveVar(param1);
            ajaxData.success(param2, 200, "");
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
        if(typeof(JsCallback) == "undefined" && typeof(window.webkit) == "undefined")
        {
            console.log("not suport this action is for device:" + ajaxData.url);
            return;
        }

        setTimeout(function(){
            var time = new Date().getTime();
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
        languagesetsure:api.Post("/languagesetsure")
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
        uncheckgps:api.Post("/server.fcgi"),
        updategpslocation:api.Post("/server.fcgi"),
        yearlocationgroup:api.Post("/server.fcgi"),
        yearlocationgrouptongji:api.Post("/server.fcgi"),
        aboutdevice:api.Post("/server.fcgi"),
        updatetime:api.Post("/server.fcgi"),
        getwifistatus:api.Post("/server.fcgi"),
        getbrtype:api.Post("/server.fcgi"),
        setbrtype:api.Post("/server.fcgi"),
        getwifiitems:api.Post("/server.fcgi"),
        getstorewifiitems:api.Post("/server.fcgi"),
        gethotpot:api.Post("/server.fcgi"),
        sethotpot:api.Post("/server.fcgi"),
        connectwifiitem:api.Post("/server.fcgi"),
        unconnectwifiitem:api.Post("/server.fcgi"),
        delstorewifiitem:api.Post("/server.fcgi"),
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
        mediaitemsgroupadd:api.Post("/server.fcgi")
    };
    api.file={
        upload:api.UploadFile("/upload.fcgi")
    };
    return api;
}
