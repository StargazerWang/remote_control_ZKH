# Telink Linux开发环境搭建

## 安装并配置Linux环境

### 安装WSL

1. 启用或关闭Windows功能  
    首先在搜索栏中搜索并打开“启用或关闭Windows功能”，勾选“适用于Linux的Windows子系统”项。只有开启这项设置才能正常安装WSL。

2. 安装 WSL  
    在微软应用商店搜索 Linux，可以看到一系列 Linux 发行版，根据自己需要选择适合自己的发行版，这里我选用 Ubuntu 20 LTS，下载完成后启动，等待安装完成，输入账户和密码，我们便得到了一个 Linux 环境了

3. WSL 文件位置  
    如果想在 Linux 查看其他分区，WSL 将其它盘符挂载在 /mnt 下

### 安装Telink GCC编译器

1. 复制tc32_gcc_v2.0.tar.bz2到HOME目录  
    如"tc32_gcc_v2.0.tar.bz2"所在目录为"D:\Telink\tc32_gcc_v2.0.tar.bz2",执行命令:  
    ```
    cp /mnt/D/Telink/tc32_gcc_v2.0.tar.bz2 ~/  
    ```

2. 解压并安装到/opt目录  
    ```
    sudo tar -xvjf　tc32_gcc_v2.0.tar.bz2　-C /opt/
    ```
3. 添加路径到PATH,为了避免每次重启都都要重设路径,可以将此行加入.bashrc  
    ```
    export PATH=$PATH:/opt/tc32/bin
    ```

4. 测试是否搭建成功  
    ```
    tc32-elf-gcc -v
    ```

    如果搭建成功将打印如下信息:
    ```
    Using built-in specs.
    COLLECT_GCC=tc32-elf-gcc
    COLLECT_LTO_WRAPPER=/opt/tc32/lib/gcc/tc32-elf/4.5.1.tc32-elf-1.5/lto-wrapper
    Target: tc32-elf
    Configured with: ../../gcc-4.5.1/configure --program-prefix=tc32-elf- --target=tc32-elf --prefix=/opt/tc32 --enable-languages=c --libexecdir=/opt/tc32/lib --with-gnu-as --with-gnu-ld --without-headers --disable-decimal-float --disable-nls --disable-mathvec --with-pkgversion='Telink TC32 version 2.0 build' --without-docdir --without-fp --without-tls --disable-shared --disable-threads --disable-libffi --disable-libquadmath --disable-libstdcxx-pch --disable-libmudflap --disable-libgomp --disable-libssp -v --without-docdir --enable-soft-float --with-newlib --with-gcc --with-gnu- --with-gmp=/opt/tc32/addontools --with-mpc=/opt/tc32/addontools --with-mpfr=/opt/tc32/addontools
    Thread model: single
    gcc version 4.5.1.tc32-elf-1.5 (Telink TC32 version 2.0 build) 
    ```
### 安装Telink BDT(windows)或者Tcbd(Linux)下载工具

#### windows+wsl
1. 在windows中，使用安装包安装Telink BDT Tool，记录安装路径，如:"C:/Telink_sdk/BDT/config/Cmd_download_tool.exe"
2. 在wsl中，新建环境变量BDT_TOOL,设置为上述路径   
    ```
    export BDT_TOOL=/mnt/C/Telink_sdk/BDT/config/Cmd_download_tool.exe
    ```

#### Ubuntu
1. Download the SWS 3-wire burning tool from the link below

   https://meta.box.lenovo.com/v/link/view/af115a5f96ab4601b204b6eb994f352c

   with password: telink123

   The current version of the tool is in folder "Google Ref RCU For ATV/Linux_tcdb"

2. Extact the downloaded tgz file into a a directory and cd into the directory.

3. Run ./install_tcdb.sh to install the tool. Or

4. Follow the installation instructions in the document "Telink Linux tcdb user guide.pdf" to install the tool.

5. 新建环境变量BDT_TOOL,可以将此行加入.bashrc   
    ```
    export BDT_TOOL=/usr/local/bin/tcdb
    ```

### 安装make
    sudo apt-get install make

### 下载SDK

1. 使用Git,仓库地址:  
    git clone http://gitlab.syseneasy.com//seneasy/TLSR825X_RCU_SDK.git  





