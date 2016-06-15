sed -i '/3.5/s/3.5/3.2/g' ../CMakeLists.txt

cd ../source/rpc/retrieve/
slice2cpp ImageRetrieveRpc.ice
sed -i '/<ImageRetrieveRpc.h>/s/<ImageRetrieveRpc.h>/"ImageRetrieveRpc.h"/g' ImageRetrieveRpc.cpp

cd ../../../
cmake .
make