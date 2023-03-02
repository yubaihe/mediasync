
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
function LocationFromGps()
{
    if(null == g_GpsPoint || g_iGpsIndex == g_GpsPoint.length - 1)
    {
        g_iGpsIndex = -1;
        setTimeout(function()
        {
            CheckGps();
        }, 100);
        return;
    }
    g_iGpsIndex++;
    console.log(g_GpsPoint);
    console.log(g_iGpsIndex);
    if(g_GpsPoint[g_iGpsIndex].weizhi.length == 0)
    {
        SetAddr(g_GpsPoint[g_iGpsIndex].weizhi, "0&0", "未知");
        LocationFromGps();
        return;
    }
    var point = g_GpsPoint[g_iGpsIndex].weizhi.split("&");
    if(parseInt(point[0]) == 0 || parseInt(point[1]) == 0)
    {
        SetAddr(g_GpsPoint[g_iGpsIndex].weizhi, "0&0", "未知");
        LocationFromGps();
        return;
    }
    var ggPoint = new BMap.Point(point[0],point[1]);
    var pointArr = [];
    pointArr.push(ggPoint);
    //开始获取GPS了
    var translateCallback = function (data)
    {
        if(data.status === 0) 
        {
            var myGeo = new BMap.Geocoder({extensions_town: true});   
            for(t = 0; t < data.points.length; ++t)
            {
                // 根据坐标得到地址描述    
                myGeo.getLocation(data.points[0], function(result){  
                if(null == result)
                {
                    SetAddr(g_GpsPoint[g_iGpsIndex].weizhi, g_GpsPoint[g_iGpsIndex].weizhi, "未知");
                    LocationFromGps();
                    return;
                }
                if (result.address.length != 0)
                {
                    var gps = "B-" + data.points[0].lng.toFixed(6) + "&" + data.points[0].lat.toFixed(6);
                    SetAddr(g_GpsPoint[g_iGpsIndex].weizhi, gps, result.address);  
                    LocationFromGps();
                    return;
                }      
                else
                {
                    SetAddr(g_GpsPoint[g_iGpsIndex].weizhi, g_GpsPoint[g_iGpsIndex].weizhi, "未知");
                    LocationFromGps();
                    return;
                }
                });
            }
        }
        else
        {
            SetAddr(g_GpsPoint[g_iGpsIndex].weizhi, g_GpsPoint[g_iGpsIndex].weizhi, "未知");
            LocationFromGps();
            return;
        }
    }
    var convertor = new BMap.Convertor();
    convertor.translate(pointArr, 1, 5, translateCallback);
}

function CheckGps()
{
    var str = "{\"action\":\"{0}\", \"limit\":\"20\"}".format("uncheckgps");
    ServerApi.request.uncheckgps(str,function(data,textStatus,jqXHR)
    {
        console.log(data);
        var jsonRoot = JSON.parse(data);
        var items = jsonRoot["items"];
        if(items.length == 0)
        {
            return;
        }
        g_GpsPoint = items;
        g_iGpsIndex = -1;
        LocationFromGps();
        
    },function(xhr,textStatus)
    {
    });
}

function LoadBaiduMapScript(callback) {
    //console.log("初始化百度地图脚本...");
    const AK = GetCookie("baiduak");
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