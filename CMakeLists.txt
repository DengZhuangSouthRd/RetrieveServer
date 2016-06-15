cmake_minimum_required(VERSION 3.5)
project(RetrieveServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("/usr/local/include")
include_directories("source/algorithm/third_party/")
include_directories("source/algorithm/third_party/")
include_directories("source/algorithm/third_party/")
LINK_DIRECTORIES("/usr/local/lib")

set(SOURCE_FILES
        source/server.h
        source/server.cpp
        source/utils/log.cpp
        source/utils/log.h
        source/utils/utils.cpp
        source/utils/utils.h
        source/utils/pgdb.h
        source/utils/pgdb.cpp
        source/utils/jsoncpp.cpp
        source/utils/json/json.h
        source/utils/json/json-forwards.h
        source/threadpool/mutex.cpp
        source/threadpool/mutex.h
        source/threadpool/task.cpp
        source/threadpool/task.h
        source/threadpool/threadpool.cpp
        source/threadpool/threadpool.h
        source/rpc/retrieve/ImageRetrieveRpc.h
        source/rpc/retrieve/ImageRetrieveRpc.cpp
        source/algorithm/thumb/imagethumb.cpp
        source/algorithm/thumb/imagethumb.h
        source/algorithm/retrieve/utils/retrieveutils.h
        source/algorithm/retrieve/utils/retrieveutils.cpp
        source/algorithm/retrieve/detect/basicoperator.h
        source/algorithm/retrieve/detect/readcsv.h
        source/algorithm/retrieve/detect/sr.h
		source/algorithm/retrieve/detect/regbygeoinf.h
        source/algorithm/retrieve/ASIFT/compute_asift_keypoints.cpp
        source/algorithm/retrieve/ASIFT/compute_asift_keypoints.h
        source/algorithm/retrieve/ASIFT/compute_asift_matches.cpp
        source/algorithm/retrieve/ASIFT/compute_asift_matches.h
        source/algorithm/retrieve/ASIFT/demo_lib_sift.cpp
        source/algorithm/retrieve/ASIFT/demo_lib_sift.h
        source/algorithm/retrieve/ASIFT/domain.cpp
        source/algorithm/retrieve/ASIFT/domain.h
        source/algorithm/retrieve/ASIFT/extract_feature.cpp
        source/algorithm/retrieve/ASIFT/extract_feature.h
        source/algorithm/retrieve/ASIFT/filter.cpp
        source/algorithm/retrieve/ASIFT/filter.h
        source/algorithm/retrieve/ASIFT/flimage.cpp
        source/algorithm/retrieve/ASIFT/flimage.h
        source/algorithm/retrieve/ASIFT/fproj.cpp
        source/algorithm/retrieve/ASIFT/fproj.h
        source/algorithm/retrieve/ASIFT/frot.cpp
        source/algorithm/retrieve/ASIFT/frot.h
        source/algorithm/retrieve/ASIFT/gdalio.h
        source/algorithm/retrieve/ASIFT/library.cpp
        source/algorithm/retrieve/ASIFT/library.h
        source/algorithm/retrieve/ASIFT/numerics1.cpp
        source/algorithm/retrieve/ASIFT/numerics1.h
        source/algorithm/retrieve/ASIFT/orsa.cpp
        source/algorithm/retrieve/ASIFT/orsa.h
        source/algorithm/retrieve/ASIFT/splines.cpp
        source/algorithm/retrieve/ASIFT/splines.h
        source/retrieveserver.cpp
        source/retrieveserver.h)

add_executable(RetrieveServer ${SOURCE_FILES})
target_link_libraries(RetrieveServer pqxx pq Ice IceUtil gdal proj pthread)
