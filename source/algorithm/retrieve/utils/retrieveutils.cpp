#include "retrieveutils.h"

string getPGConfInfo(const map<string, string> &argvMap) {
    string str = "";
    str = "dbname=" + argvMap.at("RETRIEVEPG_NAME") + " user=" + argvMap.at("RETRIEVEPG_USER") + " password=" + argvMap.at("RETRIEVEPG_PASSWD") + " host=" + argvMap.at("RETRIEVEPG_HOST") + " port=" + argvMap.at("RETRIEVEPG_PORT");
    return str;
}

void* retrieveInterface(void *args) {
    InputInterface* inputArgs= (InputInterface*)args;
    cout << inputArgs->imgurl << "; " << inputArgs->saveurl << endl;
    vector<vector<double>> imgFeatures;
    ASIFT_Ext_Features_Gdal(inputArgs->saveurl, inputArgs->imgurl, imgFeatures);
}
