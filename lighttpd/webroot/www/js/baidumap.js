
var g_GpsPoint = null;
var g_iGpsIndex = -1;
function SetAddr(weizhi, bdweizhi, location)
{
    var str = "{\"action\":\"{0}\",\"weizhi\":\"{1}\",\"bdweizhi\":\"{2}\",\"location\":\"{3}\"}".format("updategpslocation", weizhi, bdweizhi, location);
    console.log(str);
    ServerApi.request.updategpslocation(str,function(data,textStatus,jqXHR)
    {
    },function(xhr,textStatus)
    {
    });
}

function LoadBaiduMapScript(callback) {
    //console.log("初始化百度地图脚本...");
    const AK = GetCookie("baiduak");
    console.log("AK:" + AK);
    if(null == AK || AK.length == 0)
    {
        callback();
        return;
    }
    //alert(AK);
    const BMap_URL = "https://api.map.baidu.com/api?v=3.0&ak="+ AK +"&s=1&callback=onBMapCallback";
    return new Promise(function(resolve, reject) {
        // 如果已加载直接返回
        if(typeof BMap !== "undefined") {
            callback();
            return true;
        }
        // 百度地图异步加载回调处理
        window.onBMapCallback = function () {
            console.log("百度地图脚本初始化成功...");
            callback();
        };
        // 插入script脚本

        let scriptNode = document.createElement("script");
        scriptNode.setAttribute("type", "text/javascript");
        scriptNode.setAttribute("src", BMap_URL);
        document.body.appendChild(scriptNode);
    });
}

function LoadBaiduMapWebGlScript(callback) {
    //console.log("初始化百度地图脚本...");
    const AK = GetCookie("baiduak");
    console.log("AK:" + AK);
    if(null == AK || AK.length == 0)
    {
        callback();
        return;
    }
    const BMap_URL = "https://api.map.baidu.com/api?type=webgl&v=3.0&ak="+ AK +"&s=1&callback=onBMapCallback";
    return new Promise(function(resolve, reject) {
        // 如果已加载直接返回
        if(typeof BMap !== "undefined") {
            callback();
            return true;
        }
        // 百度地图异步加载回调处理
        window.onBMapCallback = function () {
            console.log("百度地图脚本初始化成功...");
            callback();
        };
        // 插入script脚本
        let scriptNode = document.createElement("script");
        scriptNode.setAttribute("type", "text/javascript");
        scriptNode.setAttribute("src", BMap_URL);
        document.body.appendChild(scriptNode);
    });
}

function GetAddr(itemdiv, weizhi, location, callBack) //weizhi= longitude&latitude weizhi = "118.756828&32.058644";
{ 
    console.log("GetAddr weizhi:" + weizhi);
   // weizhi = "118.756828&32.058644";
    if(location.length > 0)
    {
        callBack(itemdiv, location, weizhi);
        return;
    }
    let gps = RemoveGpsPrefix(weizhi);
    let array = gps.split("&");
    let longitude = parseInt(array[0]);
    let latitude = parseInt(array[1]);
    if(longitude == 0 || latitude == 0)
    {
        callBack(itemdiv, "未知", "B-0&0");
        return;
    }
    GetGps(weizhi, function(gps, bdgps, location){
        callBack(itemdiv, location, bdgps);
    });
}
function GetGps(weizhi, callback)
{
    let json = {};
    json["action"] = "gpslocation";
    json["gps"] = weizhi;
    json["parse"] = 0;
    
    ServerApi.request.gpslocation(JSON.stringify(json),function(data,textStatus,jqXHR)
    {
        var jsonRoot = JSON.parse(data);
        if(jsonRoot["status"] == 1)
        {
            if(jsonRoot["gps2"].length != 0 || jsonRoot["location"].length != 0)
            {
                callback(jsonRoot["gps"], jsonRoot["gps2"], jsonRoot["location"]);
                return;
            }
            GetGpsFromBaiDuServer(weizhi, callback);
        }
        
    },function(xhr,textStatus)
    {
        GetGpsFromBaiDuServer(weizhi, callback);
    });
}
function GetGpsFromBaiDuServer(weizhi, callback)
{
    console.log("GetGpsFromBaiDuServer:" + weizhi);
    const AK = GetCookie("baiduak");
    console.log(typeof AK);
    if(AK == null || AK.length == 0)
    {
        callback(weizhi, weizhi, weizhi);
        return;
    }
    let gps = RemoveGpsPrefix(weizhi);
    if(gps.length == weizhi.length)
    {
        console.log("===>" + weizhi);
        try {
            let gpsArray = gps.split("&");
            var ggPoint = new BMap.Point(gpsArray[0], gpsArray[1]);
            var translateCallback = function (data) {
                if (data.status === 0) 
                {
                    var myGeo = new BMap.Geocoder({ extensions_town: true });
                    // 根据坐标得到地址描述
                    myGeo.getLocation(data.points[0], function (result)
                    {
                        console.log(result);
                        if (result.address != 0) 
                        {
                            var bdgps = "B-" + data.points[0].lng.toFixed(6) + "&" + data.points[0].lat.toFixed(6);
                            callback(weizhi, bdgps, result.address);
                            SetAddr(weizhi, bdgps, result.address);
                        }
                        else 
                        {
                            callback(weizhi, "", "未知");
                        }
                    });
                }
                else {
                    callback(weizhi, "", "未知");
                }
            }
    
            var convertor = new BMap.Convertor();
            var pointArr = [];
            pointArr.push(ggPoint);
            convertor.translate(pointArr, 1, 5, translateCallback);
        }
        catch (err) 
        {
            callback(weizhi, "", "未知");
        }
    }
    else
    {
        let bdGps = GpsToBdPoint(gps);
        console.log(bdGps);
        var myGeo = new BMap.Geocoder({ extensions_town: true });
        // 根据坐标得到地址描述
        myGeo.getLocation(bdGps, function (result)
        {
            //console.log(result);
            if (result.address != 0) 
            {
                var bdgps = "B-" + bdGps.lng.toFixed(6) + "&" + bdGps.lat.toFixed(6);
                callback("", bdgps, result.address);
                SetAddr("", bdgps, result.address);
            }
            else 
            {
                callback("", "", "未知");
            }
        });
    }
}
function GpsToBdPoint(gps)
{
    gps = RemoveGpsPrefix(gps);
    array = gps.split("&");
    
    var bdpoint = new BMap.Point(parseFloat(array[0]).toFixed(6), parseFloat(array[1]).toFixed(6));
    return bdpoint;
}
function RemoveGpsPrefix(gps)
{
    if (gps.length >= 2 && gps[0] === 'B' && gps[1] === '-')
    {
        return gps.substring(2);
    }
    return gps;
}