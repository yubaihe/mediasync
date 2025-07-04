var mLanguageMap = new Map();
var mLanguageIndex = -1;
function AssembleLanguage(jsonRoot, languageIndex)
{
    mLanguageIndex = languageIndex;
    for(var i = 0; i < jsonRoot.length; ++i)
    {
        var itemList = [];
        var jsonItem = jsonRoot[i];
        for (var j = 1; j < jsonItem.length; ++j)
        {
            itemList.push(jsonItem[j]);
        }
        if(itemList.length > 0)
        {
            mLanguageMap.set(jsonItem[0], itemList);
        }
    }
}
function GetLanguageID()
{
    return localStorage.getItem("languageid");
}
function SwitchLanguage(languageID)
{
    localStorage.setItem("languageid", languageID);
    if(mLanguageIndex < 0)
    {
        var languagecontent = $.parseJSON(localStorage.getItem("languagecontent"));
        AssembleLanguage(languagecontent, languageID);
    }
    else
    {
        mLanguageIndex = languageID;
    }
}
function LanguageText(strText)
{
    if(mLanguageIndex < 0)
    {
        var languagecontent = $.parseJSON(localStorage.getItem("languagecontent"));
        console.log(languagecontent);
        if(null == languagecontent)
        {
            return strText;
        }
        var languageid = localStorage.getItem("languageid");
        AssembleLanguage(languagecontent, languageid);
    }
    if(mLanguageMap.length == 0)
    {
        return strText;
    }
     var itemList = mLanguageMap.get(strText);
     if (typeof(itemList) == "undefined")
     {
        console.log(strText + " not find 11");
        return strText;
     }
     return itemList[mLanguageIndex];
}
function SetLanguageInfo(languagecontent, languageid)
{
    console.log("==============================");
    console.log("relech 12" + $.base64.decode(languagecontent));
    localStorage.setItem("languagecontent", $.base64.decode(languagecontent));
    localStorage.setItem("languageid", languageid);
}
// function InitLanguage()
// {
//     alert("InitLanguage");
//     $.ajax({
//         url: "../js/language.js",
//         type: "GET",
//         dataType: "text",
//         success: function (data)
//         {
//             console.log(data);
//             var jsonRoot = $.parseJSON(data);
//             AssembleLanguage(jsonRoot);
//         }
//     });
// }