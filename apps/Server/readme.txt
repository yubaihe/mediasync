struct json_object* pJsonObj = json_object_new_object();
json_object_object_add(pJsonObj, "devname", json_object_new_string("111")); 
struct json_object* pJsonObj2 = json_object_new_object();
json_object_object_add(pJsonObj2, "devname", json_object_new_string("222")); 
struct json_object* pJsonObjAll = json_object_new_array();
json_object_array_add(pJsonObjAll, pJsonObj); 
json_object_array_add(pJsonObjAll, pJsonObj2); 
const char* pszJson = json_object_to_json_string(pJsonObjAll);
printf("%s\n", pszJson);


struct json_object* pParseItems = json_tokener_parse(pszJson);
int iLen = json_object_array_length(pParseItems);
struct json_object * pParseItem = json_object_array_get_idx(pParseItems, 0);
const char* pszSsid = json_object_get_string(json_object_object_get(pParseItem, "devname"));
printf("%s\n", pszSsid);
json_object_put(pJsonObjAll);
json_object_put(pParseItems);