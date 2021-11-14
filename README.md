# gpos
写个操作系统

build up a new Operating System from scratch.
 1. 显示安装的内存
   
    show installed memory

![图片](https://user-images.githubusercontent.com/39359146/134795177-f30e7ff5-f600-42c5-819e-9aa3645698de.png)

2. 开始运行第一个进程0

   start running first process 0
  
![图片](https://user-images.githubusercontent.com/39359146/138578370-109e8a61-a714-4c2b-bab8-c356c8bc7cfd.png)

3. 系统调用fork()创建进程１，并加入进程调度;)

   use fork() system call to create second process 1, which will be scheduled by process scheduler.
   
![图片](https://user-images.githubusercontent.com/39359146/139679170-9ca64eeb-1c60-414a-b50f-635493fcab6e.png)

4. 内存管理

   4.1 添加内存管理

   Add Memory management
   
   4.2 Copy On Write 开始工作
   
   Copy On Write works now.
