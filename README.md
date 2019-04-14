# microWebServer
C++ webServer

待改进：
       
       事件channel略重。

       可加一个inBuffer，不用每次读硬盘
       
       logging做成每线程一个，以防多线程同步降低效率
       
       线程池中线程负载均衡 (ready队列(加锁)或epoll（过重？）)
