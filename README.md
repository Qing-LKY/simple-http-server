# SHTTPD

一个简单的 SHTTPD 服务器。

源项目名: SHTTPD Config Analyzer

## SHTTPD Config Analyzer

### 题目背景

服务器配置文件及命令行处理。要求：

如果命令行中携带了参数，能够正确提取相关参数。  
如果命令行中没有参数，访问配置文件，从配置文件中读取参数。  
如果如果命令行中没有参数，配置文件中没有参数，使用默认的参数（程序中设定）。  
参数优先级：命令行>配置文件>默认参数。

### 编译与使用

```bash
make
```

会在项目目录下生成可执行文件 emu-shttpd。其使用帮助如下:

```
Usage: ./emu-shttpd [OPTION]...  
Emulate to boot SHTTPD server (with custom options).  
Use for testing implementation of configuring.  

Configure:  
  -c DIRECTORY, --CGIRoot=DIRECTORY  
  -d FILE, --DefaultFile=FILE  
  -f FILE, --ConfigFile=FILE  
  -o DIRECTORY, --DocumentRoot=DIRECTORY  
  -l NUM, --ListenPort=NUM  
  -m NUM, --MaxClient=NUM  
  -t NUM, --TimeOut=NUM  

Show this message and exit:
  -h, --Help
```

配置文件的默认路径是 /etc/emu-shttpd.conf（定义在 main.c 中）。可以自行通过 -f 指定。示例默认配置文件为 example.conf。

下面的指令可以自动安装 emu-shttpd 到 /usr/local/bin 中，并创建 /etc 下的默认配置文件 emu-shttpd.conf。

```bash
# need `make' before
sudo make install
```

下面的指令可以清除编译的文件和卸载程序：

```bash
make clean
sudo make uninstall
```

### 细节

程序中将**未定义**的字符串参数设置为**空串**("")，将未定义的整数参数设置为 -1，因此诸如 `-c ""` 或 `--TimeOut=-1` 的设置会被**直接忽略**。

如果使用空串设定整数参数，例如 `-t ""`，会被识别为 0，也就是等价于 `-t 0`。

strtol 的 base 设为 0，可以自动识别标准八进制（0??）和 16 进制数（0x??）。

字符串参数的最大长度被设置为 125。

命令行的解析使用 getopt_long() 实现。配置文件的解析为个人实现。

配置文件支持的左值有：CGIRoot DefaultFile DocumentRoot ListenPort MaxClient TimeOut。与命令行相比少了 ConfigFile。（哪有自己指定自己的）

我并没有检查引号的闭合性，只是单纯的在第一个非空白字符和最后一个非空白字符为引号时将其跳过。因此，写法 `CGIRoot = "aba" ac` 会被识别为 `aba" ac`。

取右值的原理是排除等号后和结束或 `#` 前的空白字符。因此在数字后面塞怪东西是错误的。例如 `TimeOut = 0x12 ab%*asf`。

### 已知问题

程序在分析文件行时设置的缓冲区大小为 1024。当一行长度超过 1023 字节时这一行会被分割，这会导致行数的定位出现问题。例如在后面塞一大堆空白字符。不过行数定位这个功能只是为了指出配置文件哪一行有问题，对主要功能影响不大。

~~linux 的文件和目录名是可以出现等号的。这会导致程序因为找到两个等号而判断出错。~~（已解决，左值中不可能有等号，取第一个等号作为基准，删去重复等号的判断即可）

### 具体实现

#### 项目结构

| 文件名 | 功能 |
| --- | --- |
| main.c | 程序入口 |
| ds.c, ds.h | 定义了配置信息的数据结构，封装了配置信息的合并与打印 |
| io.c, io.h | 封装了部分文件操作和字符串、数字提取 |
| opt_cmd.c, opt_cmd.h | 封装了命令行解析 |
| opt_file.c, opt_file.h | 封装了配置文件解析 |

#### 重要接口

| 接口 | 模块 | 功能 |
| --- | --- | --- |
| line_convert | fileopt | 从一行中提取等式的左值和右值，右值不允许为空，出错返回 1，空行时 left 与 right 指针都返回 0 |
| line_parse | fileopt | 根据左值和右值更新配置信息 |
| fileopt_parse | fileopt | 解析配置文件 path，并把参数保存到 cfg |
| cmdopt_parse | cmdopt | 解析命令行参数，并保存到 cfg |
| copy_string | io | 复制字符串，会判断长度是否超过 125，超限返回 1 |
| copy_number | io | 通过 strtol 提取数字，出错返回 2 |
| open_conf | io | 打开文件，会输出错误信息 |
| update_para | ds | 使用 upd 的参数覆盖 cfg 的，用于配置信息的合并 |

详情查看代码。
