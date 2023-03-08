# SHTTPD Config Analyzer

A linux config analyzer for SHTTPD web server, an example of config file parse and command line args parse.

It can analyse the config file (use `-f` to specified one or default in `/etc/emu-shttpd.conf`) and command-line options, and print what it gains.

Implement for homework of <网络程序设计>.

## Compile

```bash
make
```

## Usage

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

## TODO

- add docs about implement
- add check for non-closed quotes