# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
# 设置头文件
include_directories(../../../engine/inc/libCoreCommon)
include_directories(../../../engine/inc)
include_directories(../../../3rd/tinyxml2/include)

# 设置输出目录
SET(LIBRARY_OUTPUT_PATH ${BIN_OUTPUT_PATH})

# 生成libCoreCommon.a
add_library(CoreCommon ${DIR_SRCS})