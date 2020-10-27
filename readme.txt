安全导入客户端模块， 该程序作TCP连接的客户端安全导入服务器模块配套使用，外端自动将文件推送到安全导入服务器上，内端接收来自单导安全导入服务器推送的文件。支持数据加密及不加密的传输方式。
编译方法：
linux 下台下 执行 debug/make all

Win平台下，用VS2008打开，编译即可。

主要程序文件说明：
safeImport.cpp safeImport.cpp  主程序入口,外端文件自动推送到
SocketLayer.cpp SocketLayer.h 与安全导入服务器TCP通信类， 支持数据加密及不加密的传输方式。 采用openssl 安全套接字传输数据
filemark.cpp  filemark.h 	文件敏感标记类


safeImportClient/
├── Debug
│   ├── bdGapsafeImport.ini
│   ├── makefile
│   ├── objects.mk
│   ├── safeImport
│   │   ├── filemark.d
│   │   ├── filemark.o
│   │   ├── KAES.d
│   │   ├── KAES.o
│   │   ├── Release
│   │   │   └── subdir.mk
│   │   ├── safeImport.d
│   │   ├── safeImport.o
│   │   ├── SslSocketLayer.d
│   │   ├── SslSocketLayer.o
│   │   ├── stdafx.d
│   │   ├── stdafx.o
│   │   └── subdir.mk
│   ├── safeImportClient
│   ├── safeImport.exp
│   └── sources.mk
├── readme.txt
├── safeImport
│   ├── bd.ico
│   ├── Debug
│   │   ├── bdGapsafeImport.ini
│   │   ├── BuildLog.htm
│   │   ├── KAES.obj
│   │   ├── libeay32.dll
│   │   ├── libssl32.dll
│   │   └── ssleay32.dll
│   ├── filemark.cpp
│   ├── filemark.h
│   ├── KAES.cpp
│   ├── KAES.h
│   ├── lib
│   │   ├── 4758cca.lib
│   │   ├── aep.lib
│   │   ├── atalla.lib
│   │   ├── capi.lib
│   │   ├── chil.lib
│   │   ├── cswift.lib
│   │   ├── gmp.lib
│   │   ├── gost.lib
│   │   ├── libeay32.dll
│   │   ├── libeay32.lib
│   │   ├── libssl32.dll
│   │   ├── MinGW
│   │   │   ├── libeay32.def
│   │   │   └── ssleay32.def
│   │   ├── nuron.lib
│   │   ├── padlock.lib
│   │   ├── ssleay32.lib
│   │   ├── sureware.lib
│   │   ├── ubsec.lib
│   │   └── VC
│   │       ├── libeay32MDd.lib
│   │       ├── libeay32MD.lib
│   │       ├── libeay32MTd.lib
│   │       ├── libeay32MT.lib
│   │       ├── ssleay32MDd.lib
│   │       ├── ssleay32MD.lib
│   │       ├── ssleay32MTd.lib
│   │       └── ssleay32MT.lib
│   ├── ReadMe.txt
│   ├── Release
│   ├── resource.h
│   ├── safeImport.aps
│   ├── safeImport.cpp
│   ├── safeImport.pro
│   ├── safeImport.pro.user
│   ├── safeImport.pro.user.c5efccc
│   ├── safeImport.rc
│   ├── safeImport.vcproj
│   ├── safeImport.vcproj.PC-PC.Administrator.user
│   ├── safeImport.vcproj.YANGDX.Administrator.user
│   ├── safeImport.vcproj.YT2016072922.Administrator.user
│   ├── SslSocketLayer.cpp
│   ├── SslSocketLayer.h
│   ├── stdafx.cpp
│   ├── stdafx.h
│   └── targetver.h
├── safeImport.ncb
├── safeImport.sln
└── safeImport.suo
