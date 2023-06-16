# SHTTPD

一个简单的 SHTTPD 服务器。

原本的项目名: SHTTPD Config Analyzer

《网络程序设计》结课作业

## 编译方式

```bash
make
```

生成可执行文件 emu-shttpd。直接运行 ./emu-shttpd 即可开启一个 demo http 服务器。

服务器的根目录是 ./demo。

## 配置

支持下面的参数:

```text
Usage: ./emu-shttpd [OPTION]...
To start a simple http server (with custom options).

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

Repository page: <https://github.com/Qing-LKY/simple-http-server.git>
Read README.md for more infomation.
```

配置文件参考 emu-shttpd.conf

实际实现中，CGIRoot 其实没有使用到。

## 支持功能

最低限度的 HTTP 请求解析和响应: 只解析和响应了第一行、Content-Length 和 Content 的内容。

POST 请求仅能对可执行文件发起，表单的内容会在去除 `&` 后作为参数调用该可执行文件，并将其标准输出作为页面显示。

具体可以尝试 index.html 中的表单。
