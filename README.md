# coordinate-lib

本例程基于ros 的catkin 包使用。

在新的硬件平台上 先 生成 libcoorsys.so

cd makeso/src

gcc coordinate_sysddd.c -fPIC -shared -o libcoorsys.so

然后将libcoorsys.so 拷贝到 gps_ws/src/gps_node/src 下 

重新catkin_make 

ros下使用GPS节点获取GPS信息
概述
通过usb和ublox的转串口获取gprmc信息，符合NMEA8013协议。
用到了serial的相关内容。
cd ~
mkdir  -p gps_ws/src
cd ~/gps_ws/src
catkin_create_pkg gps_node roscpp serial
cd ~/gps_ws
catkin_make

在和include同目录的节点Cmakelist下添加
include_directories(
  include/gps_node/
  ${catkin_INCLUDE_DIRS}
)
link_directories(
lib
  ${catkin_LIB_DIRS}
)
link_libraries(coorsys)
# 并将名称保存到 DIR_SRCS 变量

add_executable(gps_node  
src/serial_port.cpp  
src/cJSON.c #库里有调用所以要写
)

target_link_libraries(gps_node
  ${catkin_LIBRARIES} coorsys
)
在树莓派中测试发现，调用libcoorsys.so时，调用了cjson.c
所以要写入 src/cJSON.c


