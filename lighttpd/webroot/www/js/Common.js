//获取url中的参数
function GetUrlParam(name) 
{
    var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)"); //构造一个含有目标参数的正则表达式对象
    var r = encodeURI(window.location.search).substr(1).match(reg);  //匹配目标参数
    if (r != null) return unescape(r[2]); return null; //返回参数值
}



function SetCookie(name, value, options = {}) {
    const defaultOptions = {
        days: 3650,
        path: '/',
        sameSite: 'Strict',
        secure: false,
        domain: undefined
    };
 
    const opts = { ...defaultOptions, ...options };
    const exp = new Date();
    exp.setDate(exp.getDate() + opts.days);
 
    let cookieString = `${encodeURIComponent(name)}=${encodeURIComponent(value)}`;
    cookieString += `; expires=${exp.toUTCString()}`;
    cookieString += `; path=${opts.path}`;
    
    if (opts.domain) cookieString += `; domain=${opts.domain}`;
    if (opts.sameSite) cookieString += `; SameSite=${opts.sameSite}`;
    if (opts.secure) cookieString += '; Secure';
 
    document.cookie = cookieString;
    console.log('Set Cookie:', document.cookie);
}
 
// 增强版Cookie获取
function GetCookie(name) {
    const cookies = document.cookie.split('; ');
    for (const cookie of cookies) {
        const [cookieName, cookieValue] = cookie.split('=');
        if (cookieName === encodeURIComponent(name)) {
            return decodeURIComponent(cookieValue);
        }
    }
    return null;
}

//删除cookies
function DelCookie(name)
{
    var exp = new Date();
    exp.setTime(exp.getTime() - 1);
    var cval=GetCookie(name);
    if(cval!=null)
        document.cookie= name + "="+cval+";expires="+exp.toGMTString();
}

String.prototype.format = function(args) {
    var result = this;
    if (arguments.length > 0) {
        if (arguments.length == 1 && typeof (args) == "object") {
            for (var key in args) {
                if(args[key]!=undefined){
                    var reg = new RegExp("({" + key + "})", "g");
                    result = result.replace(reg, args[key]);
                }
            }
        }
        else {
            for (var i = 0; i < arguments.length; i++) {
                if (arguments[i] != undefined) {
                    //var reg = new RegExp("({[" + i + "]})", "g");//这个在索引大于9时会有问题，谢谢何以笙箫的指出
                    var reg= new RegExp("({)" + i + "(})", "g");
                    result = result.replace(reg, arguments[i]);
                }
            }
        }
    }
    return result;
};

Date.prototype.format = function (format) {
    var args = {
        "M+": this.getMonth() + 1,
        "d+": this.getDate(),
        "H+": this.getHours(),
        "m+": this.getMinutes(),
        "s+": this.getSeconds(),
        "q+": Math.floor((this.getMonth() + 3) / 3),  //quarter
        "S": this.getMilliseconds()
    };
    if (/(y+)/.test(format))
        format = format.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
    for (var i in args) {
        var n = args[i];
        if (new RegExp("(" + i + ")").test(format))
            format = format.replace(RegExp.$1, RegExp.$1.length == 1 ? n : ("00" + n).substr(("" + n).length));
    }
    return format;
};

Date.prototype.toLocaleString = function() {
    // 补0   例如 2018/7/10 14:7:2  补完后为 2018/07/10 14:07:02
    function addZero(num) {
        if(num<10)
            return "0" + num;
        return num;
    }
    // 按自定义拼接格式返回
    return this.getFullYear() + "-" + addZero(this.getMonth() + 1) + "-" + addZero(this.getDate()) + " " +
        addZero(this.getHours()) + ":" + addZero(this.getMinutes()) + ":" + addZero(this.getSeconds());
};

function SizeToStringkb(iSize)
{
    if(iSize >= 1024*1024)
    {
        var num = iSize/(1024*1024*1.0);
        return num.toFixed(2) + "GB";
    }
    else if(iSize >= 1024)
    {
        var num = iSize/(1024*1.0);
        return num.toFixed(1) + "MB";
    }
    else 
    {
        var num = iSize;
        return parseInt(num) + "KB";
    }
    // else
    // {
    //     var num = iSize;
    //     return parseInt(num) + "B";
    // }
}
function SizeToString(iSize)
{
    if(iSize >= 1024*1024*1024)
    {
        var num = iSize/(1024*1024*1024*1.0);
        return num.toFixed(2) + "GB";
    }
    else if(iSize >= 1024*1024)
    {
        var num = iSize/(1024*1024*1.0);
        return num.toFixed(1) + "MB";
    }
    else   if(iSize >= 1024)
    {
        var num = iSize/(1024*1.0);
        return num.toFixed(1) + "KB";
    }
    else
    {
        var num = iSize;
        return parseInt(num) + "B";
    }
}
function GetPic(pic)
{
    var names = pic.split("\.");
    var end = names[names.length - 1];
    var start = pic.substr(0, pic.length - end.length - 1);
    var retPic = "media/.media/" + start + "_" + end;
    retPic += ".jpg";
    return retPic;
}
function GetBackupThumbPic(name, file)
{
    var names = file.split("\.");
    var end = names[names.length - 1];
    var start = file.substr(0, file.length - end.length - 1);
    var retPic = start + "_" + end;
    retPic += ".jpg";
    let url = "backup/.media/{0}/{1}".format(name, retPic);
    return url;
}
function SecToTime(iSec)
{
    var iDays = parseInt(iSec / 86400);//转换天数
    iSec = parseInt(iSec % 86400);//剩余秒数
    var iHours = parseInt(iSec / 3600);//转换小时数
    iSec = parseInt(iSec % 3600);//剩余秒数
    var iMinutes = parseInt(iSec / 60);//转换分钟
    iSec = parseInt(iSec % 60);//剩余秒数
    if(iDays > 0)
    {
        var strRet = LanguageText("{0}天{1}小时{2}分{3}秒").format(iDays, iHours, iMinutes, iSec);
        return strRet;
    }
    if(iHours > 0)
    {
        var strRet = LanguageText("{0}小时{1}分{2}秒").format(iHours, iMinutes, iSec);
        return strRet;
    }
    if(iMinutes > 0)
    {
        var strRet = LanguageText("{0}分{1}秒").format(iMinutes, iSec);
        return strRet;
    }
    var strRet = LanguageText("{0}秒").format(iSec);
    return strRet;
}
function GetSelectDeviceMap()
{
    var deviceid = GetCookie("deviceid");
    var selectDevice = GetCookie(deviceid + "_selectdevice");
    if(selectDevice == undefined || selectDevice.length == 0)
    {
        var map = new Map();
        return map;
    }
    var devicemap = new Map();
    var deviceArray = selectDevice.split('&');
    for(var i = 0; i < deviceArray.length; ++i)
    {
        devicemap.set(deviceArray[i], "1");
    }

    return devicemap;
}

function GetSelectDeviceString()
{
    var deviceid = GetCookie("deviceid");
    var selectDevice = GetCookie(deviceid + "_selectdevice");
    if(selectDevice == undefined)
    {
        selectDevice = "";
    }
    return selectDevice;
}

function CheckIp(value)
{
    var exp=/^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
    var reg = value.match(exp);
    if(reg==null)
    {
        return false;
    }
    return true;
}

function IsDeviceIos()
{
    var u = navigator.userAgent;
    var isiOS = !!u.match(/\(i[^;]+;( U;)? CPU.+Mac OS X/); 
    return isiOS;
}

function EnableScroll(item)
{
    
    // if(IsDeviceIos())
    // {
    //     return;
    // }
    //
    $(item).overlayScrollbars({ scrollbars:{autoHide:'scroll'}});
}

function DisableContextMenu()
{
    document.oncontextmenu=new Function("event.returnValue=false;");
//document.onselectstart=new Function("event.returnValue=false;");
}

function CanInput(value)
{ 
  // 规则对象(flag)
  var reg = new RegExp("[`/#:*<>?|\\\\\" ]");
  // 判断 even 是否包含特殊字符
  if(reg.test(value))
  {
    console.log('包含！');
    return false;
  }
  else
  {
  console.log('不包含！');
    return true;
  }
}

function FilterInput(value)
{ 
    var reg = new RegExp("[`\"/#:*<>?|\\\\ ]", "g");
    value = value.toString().replace(reg,""); 
    return $.trim(value);
}
function Transform(value)
{
    return value.replaceAll('"','\\"');
}

function GetUrl(str)
{
    var url = "media/";
    url += str;
    return url;
}
function GetExtraUrl(str)
{
    let array = str.split(".");
    let url = "media/.media_ex/";
    let tmpUrl = "";
    for(let i = 0; i < array.length - 1; ++i)
    {
        tmpUrl += array[i];
        tmpUrl += ".";
    }
    if(tmpUrl.length > 0)
    {
        tmpUrl = tmpUrl.slice(0, -1) + "_";
    }
    tmpUrl += array[array.length - 1];
    tmpUrl += ".mp4";
    url += tmpUrl;
    return url;
}
function GetThumbUrl(str)
{
    console.log("abcd:" + str);
    let array = str.split(".");
    var url = "media/.media/";
    url += array[0];
    url += "_";
    url += array[1];
    url += ".";
    url += "jpg";
    console.log(url);
    return url;
}
function Calculate1vh() 
{
    return window.innerHeight * 0.01;
}
window.onerror = function(message, source, lineno, colno, error) {
    console.log("Global error caught:");
    console.log(`Message: ${message}`);
    console.log(`Source: ${source}`);
    console.log(`Line: ${lineno}`);
    console.log(`Column: ${colno}`);
    console.log(`Error object:`, error);
    return false; // 返回false防止默认的错误处理机制（例如显示一个堆栈跟踪）
};
function RunInit()
{
    var initfunction = GetUrlParam("init");
    if(initfunction == null)
    {
        return;
    }
    let InitFun = $.base64.decode(initfunction);
    eval(InitFun);
}

function IsSupportGps()
{
    const AK = GetCookie("baiduak");
    console.log("AK:" + AK);
    if(null == AK || AK.length == 0)
    {
        return false;
    }
    return true;
}
function IsPc()
{
  return window.matchMedia('(pointer: fine), (hover: hover)').matches;
}