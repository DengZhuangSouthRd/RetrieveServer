sed -i '/3.5/s/3.5/3.2/g' ../CMakeLists.txt

cd ../source/rpc/
cd fusion/
slice2cpp wisefuseRpc.ice
sed -i '/<wisefuseRpc.h>/s/<wisefuseRpc.h>/"wisefuseRpc.h"/g' wisefuseRpc.cpp

cd ../quality/
slice2cpp qualityjudgeRpc.ice
sed -i '/<qualityjudgeRpc.h>/s/<qualityjudgeRpc.h>/"qualityjudgeRpc.h"/g' qualityjudgeRpc.cpp

cd ../../../
cmake .
make