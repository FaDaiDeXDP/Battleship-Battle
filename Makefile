# 默认目标
all: server client

# 编译服务器程序
server: server.cpp
	g++ -o server server.cpp -pthread

# 编译客户端程序
client: client.cpp
	g++ -o client client.cpp -pthread

# 清理生成文件
clean:
	rm -f server client

# 伪目标，避免与文件名冲突
.PHONY: all clean
