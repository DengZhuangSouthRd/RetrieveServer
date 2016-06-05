#include "retrieveutils.h"



void* retrieveInterface(void *args) {
    InputInterface* inputArgs= (InputInterface*)args;
    cout << inputArgs->imgurl << "; " << inputArgs->saveurl << endl;
    vector<vector<float>> imgFeatures;
    AsiftFeature(inputArgs->saveurl, inputArgs->imgurl, imgFeatures);
    return NULL;
}
