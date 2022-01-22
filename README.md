# gpos
写个操作系统

build up a new Operating System from scratch.
 1. 显示安装的内存
   
    show installed memory

![图片](https://user-images.githubusercontent.com/39359146/134795177-f30e7ff5-f600-42c5-819e-9aa3645698de.png)

2. 开始运行第一个进程0

   start running first process 0
  
![图片](https://user-images.githubusercontent.com/39359146/138578370-109e8a61-a714-4c2b-bab8-c356c8bc7cfd.png)

3. 系统调用

   3.1 系统调用fork()创建进程１，并加入进程调度;)

   Use fork() system call to create second process 1, which will be scheduled by process scheduler.
   
![图片](https://user-images.githubusercontent.com/39359146/139679170-9ca64eeb-1c60-414a-b50f-635493fcab6e.png)

     3.2 setup() 系统调用
   
     Add setup() system call.
     
     3.3 open() 系统调用
   
     Add open() system call.
4. 内存管理

   4.1 添加内存管理

   Add Memory management
   
   4.2 Copy On Write 开始工作
   
   Copy On Write works now.
   
5. 文件系统
   
   File system
   
   4.1 文件系统缓存

   File system buffer.
   
   4.2 挂载硬盘根文件系统
   
   mount root file system from hard disk.
   
6. 驱动程序

   Drivers

   6.1 硬盘驱动
   
       Hard disk driver
       
   6.2 键盘驱动

       Keyboard driver
   ![keyboard driver](https://user-images.githubusercontent.com/39359146/150623941-4f5ba464-d7fd-43ce-86e3-7747e25c40ff.PNG)

   
   
