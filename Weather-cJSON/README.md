看了b站up主的悉心讲解，的确，对没有项目的小白来说，获取项目的最好途径就是研究开源项目，所以在此对cJSON这种数据交换格式进行了一定研究并实现通信应用。


基于CJSON的天气客户端的实现
1、对cJSON编解码器进行一定研究，掌握其内存管理及数据构建、解析等原理（主要是对链表的一些增删改查操作，但很受用）；
2、将cJSON应用在心知天气API中，通过调用socket编程的API接口获取天气信息，实现服务端与客户端之间的通信。
其中，需要用到http的get请求方法，由心知天气的请求地址逐步解析获取到天气信息。
    该程序在windows系统下使用gcc编译器编译，编译命令为：
gcc weather_client.c cJSON.c utf8togbk.c -o weather_client.exe -lwsock32


