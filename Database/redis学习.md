# redis



## NoSQL

**特点**

1. 不支持 SQL 语法
2. key-value形式存储数据
3. 没有通用的语言
   **适用场景与SQL不同**



### redist安装

下载安装包：http://www.redis.cn/download.html
解压压缩包：tar -zxvf redis-6.0.6.tar.gz
编译安装需要gcc 并且gcc版本不能太低
gcc升级：yum -y install centos-release-scl && yum -y install devtoolset-9-gcc devtoolset-9-gcc-c++ devtoolset-9-binutils && scl enable devtoolset-9 bash



```bash
(base) [root@CentOS-wangml install]# mv redis-6.0.6/ /usr/local/
(base) [root@CentOS-wangml install]# cd /usr/local/
(base) [root@CentOS-wangml local]# ls
bin  etc  games  include  lib  lib64  libexec  qcloud  redis-6.0.6  sbin  share  src  yd.socket.server
(base) [root@CentOS-wangml local]# cd redis-6.0.6/
(base) [root@CentOS-wangml redis-6.0.6]# ls
00-RELEASENOTES  CONTRIBUTING  deps     Makefile   README.md   runtest          runtest-moduleapi  sentinel.conf  tests   utils
BUGS             COPYING       INSTALL  MANIFESTO  redis.conf  runtest-cluster  runtest-sentinel   src            TLS.md
(base) [root@CentOS-wangml redis-6.0.6]# make
......
    LINK redis-benchmark
    INSTALL redis-check-rdb
    INSTALL redis-check-aof

Hint: It's a good idea to run 'make test' ;)

make[1]: Leaving directory `/usr/local/redis-6.0.6/src'
```

**相关链接**

1. redis下载安装以及后续设置：https://my.oschina.net/u/3986411/blog/4715948?hmsr=kaifa_aladdin



#### 启动redis客户端以及退出

客户端启动程序/usr/local/redis-6.0.6/src/redis-cli 可以将该路径创建一个redis的软连接



```bash
(base) [root@CentOS-wangml study]# redis
127.0.0.1:6379> quit
(base) [root@CentOS-wangml study]# /usr/local/redis-6.0.6/src/redis-cli 
127.0.0.1:6379> quit
```



#### redis数据库

默认创建了16个数据库，编号0-15
选择数据库使用select Num命令



```bash
127.0.0.1:6379> select 9
OK
```



### 数据库操作



#### set

**设置key-value**



```redis
127.0.0.1:6379> set name Balabal
OK
127.0.0.1:6379> get name
"Balabal"
```

**设置多个key-value**



```redis
127.0.0.1:6379> mset name Bao age 12
OK
127.0.0.1:6379> get Bao
(nil)
127.0.0.1:6379> get name
"Bao"
127.0.0.1:6379> get age
"12"
```

**错误处理**



```redis
(error) MISCONF Redis is configured to save RDB snapshots, but it is currently not able to persist on disk. Commands that may modify the data set are disabled, because this instance is configured to report errors during writes if RDB snapshotting fails (stop-writes-on-bgsave-error option). Please check the Redis logs for details about the RDB error.  
127.0.0.1:6379> config set stop-writes-on-bgsave-error no
OK
```

**设置key-value以及存在时效**



```redis
127.0.0.1:6379> setex class 10 1
OK
127.0.0.1:6379> get class
"1"
127.0.0.1:6379> get class
"1"
... 10秒后
127.0.0.1:6379> get class
(nil)
```

**获取多个值**



```redis
127.0.0.1:6379> mget name age
Bob123哈哈哈
12
```



#### append

追加值，返回追加后长度



```redis
127.0.0.1:6379> set name Bob
OK
127.0.0.1:6379> get name
"Bob"
127.0.0.1:6379> append name 123
(integer) 6
127.0.0.1:6379> get name
"Bob123"
```

**中文乱码问题**



```redis
127.0.0.1:6379> append name 哈哈哈
(integer) 15
127.0.0.1:6379> get name
"Bob123\xe5\x93\x88\xe5\x93\x88\xe5\x93\x88"
(base) [root@CentOS-wangml study]# redis --raw
127.0.0.1:6379> get name
Bob123哈哈哈
```



#### key操作

删除的key 就无法再获取对应值



```redis
127.0.0.1:6379> keys *
age
name
127.0.0.1:6379> mset a1 1 a2 2 a3 3
OK
127.0.0.1:6379> keys *
a2
a1
name
age
a3
127.0.0.1:6379> keys a*
a2
a1
age
a3
127.0.0.1:6379> exists class
0
127.0.0.1:6379> exists a1
1
127.0.0.1:6379> exists a*
0
127.0.0.1:6379> del a1
1
127.0.0.1:6379> keys *
a2
name
age
a3
127.0.0.1:6379> del a1 a2 a3 a4
2     成功删除的key数量
```



#### expire

设置key的存在时间



```redis
127.0.0.1:6379> keys *
name
age
127.0.0.1:6379> expire age 10
1
127.0.0.1:6379> get age
12
127.0.0.1:6379> get age

127.0.0.1:6379> keys *
name
```



#### ttl

查看key还可以存活多久，返回正数表示还可以存活的秒数，-2表示key不存在，-1表示一直存活



```redis
127.0.0.1:6379> mset a1 1 a2 2 a3 3
OK
127.0.0.1:6379> expire a1 100
1
127.0.0.1:6379> ttl a1
94
127.0.0.1:6379> ttl a1
88
127.0.0.1:6379> expire a1 3
1
127.0.0.1:6379> ttl a1
2
127.0.0.1:6379> ttl a1
-2
127.0.0.1:6379> ttl a1
-2
127.0.0.1:6379> ttl a2
-1
```



#### hash值

**设置hash值**



```redis
127.0.0.1:6379> hset b1 name Bobob
1
127.0.0.1:6379> get name
Bob123哈哈哈
127.0.0.1:6379> get b1
WRONGTYPE Operation against a key holding the wrong kind of value

127.0.0.1:6379> type name
string
127.0.0.1:6379> type b1
hash
127.0.0.1:6379> keys *
a2
b1
a3
name
127.0.0.1:6379> hset b1 age 29
1
127.0.0.1:6379> hset b1 class 101
1
127.0.0.1:6379> hget b1 name
Bobob
127.0.0.1:6379> hget b1 class
101
```

**获取hash的keys-values**



```redis
127.0.0.1:6379> hkeys b1
1) "name"
2) "age"
3) "class"
127.0.0.1:6379> hvals b1
1) "Bobob"
2) "29"
3) "101
127.0.0.1:6379> hmget b1 name age class
1) "Bobob"
2) "29"
3) "101"
```

**删除hash中的的行**



```redis
127.0.0.1:6379> hdel b1 name
(integer) 1
127.0.0.1:6379> hkeys b1
1) "age"
2) "class"
```



### list类型

**list的插入和读取**



```redis
127.0.0.1:6379> lpush c1 a1 a2 a3 a1 a4
(integer) 5
127.0.0.1:6379> keys *
1) "c1"
127.0.0.1:6379> lrange c1 0 5
1) "a4"
2) "a1"
3) "a3"
4) "a2"
5) "a1"
127.0.0.1:6379> rpush c2 a1 a2 a3 a4
(integer) 4
127.0.0.1:6379> lrange c2 0 4    # 从左到右 第0个到第3个元素
1) "a1"
2) "a2"
3) "a3"
4) "a4"
127.0.0.1:6379> type c1
list
127.0.0.1:6379> lrange c2 0 -1
1) "a1"
2) "a2"
3) "a3"
4) "a4"
```

**删除list元素**



```redis
127.0.0.1:6379> lpush c3 a1 a1 a2 a3 a1 a2 a3
(integer) 7
127.0.0.1:6379> lrange c3 0 -1
1) "a3"
2) "a2"
3) "a1"
4) "a3"
5) "a2"
6) "a1"
7) "a1"
127.0.0.1:6379> lrem c3 1 a1  # 从左到右 删除遇到的 1 个 a1
(integer) 1
127.0.0.1:6379> lrange c3 0 -1
1) "a3"
2) "a2"
3) "a3"
4) "a2"
5) "a1"
6) "a1"
127.0.0.1:6379> lrem c3 -1 a2  # 从右到左 删除遇到的 1 个 a2
(integer) 1
127.0.0.1:6379> lrange c3 0 -1
1) "a3"
2) "a2"
3) "a3"
4) "a1"
5) "a1"
```

**插入元素**



```redis
127.0.0.1:6379> lrange c3 0 -1
1) "a3"
2) "a2"
3) "a3"
4) "a1"
5) "a1"
127.0.0.1:6379> linsert c3 before a2 a4
(integer) 6
127.0.0.1:6379> lrange c3 0 -1
1) "a3"
2) "a4"
3) "a2"
4) "a3"
5) "a1"
6) "a1"
127.0.0.1:6379> linsert c3 after a4 a5
(integer) 7
127.0.0.1:6379> lrange c3 0 -1
1) "a3"
2) "a4"
3) "a5"
4) "a2"
5) "a3"
6) "a1"
7) "a1"
```



#### set

**set**集合，集合中无重复数据



##### 无序

**sadd**向集合添加数据 返回添加的数据个数 重复的数据只添一次



```redis
127.0.0.1:6379> sadd bb1 a1 a2 a3 a1 a2 a4
(integer) 4
```

**smembers**查看集合中数据



```redis
127.0.0.1:6379> smembers bb1
1) "a4"
2) "a2"
3) "a1"
4) "a3"
```

***srem*删除集合元素



```redis
127.0.0.1:6379> srem bb1 a1
(integer) 1
127.0.0.1:6379> smembers bb1
1) "a4"
2) "a2"
3) "a3"
```



##### 有序



```redis
127.0.0.1:6379> zadd cc1 2 a1 4 a2 1 a3   # 有序 添加权重和值
(integer) 3
127.0.0.1:6379> zrange cc1 0 -1   # 按权重从小到大排序返回
1) "a3"
2) "a1"
3) "a2"
127.0.0.1:6379> zrangebyscore cc1 2 4  # 返回指定权重区间内的值[2, 4]
1) "a1"
2) "a2"
127.0.0.1:6379> zadd cc1 16 b1 22 b2 10 b3 6 b4
(integer) 4
127.0.0.1:6379> zrange cc1 0 -1
1) "a3"
2) "a1"
3) "a2"
4) "b4"
5) "b3"
6) "b1"
7) "b2"
127.0.0.1:6379> zrangebyscore cc1 10 20
1) "b3"
2) "b1"
127.0.0.1:6379> zremrangebyscore cc1 10 20  # 按权重删除 
(integer) 2
127.0.0.1:6379> zrange cc1 0 -1
1) "a3"
2) "a1"
3) "a2"
4) "b4"
5) "b2"
```



## Go 操作redis

**Go中文文档**https://www.bookstack.cn/read/topgoer/d130e7abc85f1c4c.md
**goredis文档**https://pkg.go.dev/github.com/gomodule/redigo/redis



### 所需包的安装



```go
package main

// go与redis交互 相关包安装 go get github.com/gomodule/redigo/redis
// 测试上述包是否安装成功

import (
	"github.com/gomodule/redigo/redis"
)

func main() {
	conn, _ := redis.Dial("tcp", ":6379")
	defer conn.Close()
	conn.Do("set", "c1", "hello")
}

/*
上述代码执行成功后
(base) [root@CentOS-wangml redis]# redis
127.0.0.1:6379> get c1
"hello"
127.0.0.1:6379> 
*/
```



### 操作

**连接数据库**



```go
Dial(network, address string) (conn, err)
// conn, _ := redis.Dial("tcp", ":6379")
```

**执行数据库操作命令**



```go
Send(commandName string, args ...interface{}) err// 执行命令
Flush()// 刷新数据库
Receive()// 接收返回值
```



```go
package main

import (
	"github.com/gomodule/redigo/redis"
	"fmt"
)

func main() {
	conn, _ := redis.Dial("tcp", ":6379")
	defer conn.Close()
	conn.Send("get", "c1")// 将此条放在上面
	conn.Send("set", "aaa", "cccc")
	//conn.Send("get", "c1")
	conn.Flush()
	rel, err := conn.Receive()// 返回值仅接收第一条命令的执行结果
	/*
	127.0.0.1:6379> get aaa
	"cccc"
	*/
	if err != nil {
		fmt.Println("Receive error:", err)
	}
	//fmt.Println("c1:", rel)// c1: OK 接收到conn.Send("set", "aaa", "cccc")
	fmt.Println("c1:", rel)// c1: [104 101 108 108 111] 返回的时 c1:hello hello的每个字母的字节码
}
```



```go
package main

import (
	"github.com/gomodule/redigo/redis"
	"fmt"
)

func main() {
	conn, _ := redis.Dial("tcp", ":6379")
	defer conn.Close()
	
	//rel, err := conn.Do("get", "c1")// 执行命令 刷新缓冲区 返回结果 将Send Flush Receive结合一步完成
	// 将字节码转为string
	rel, err := redis.String(conn.Do("get", "c1"))
	if err != nil {
		fmt.Println("Receive error:", err)
	}
	
	//fmt.Println("c1:", rel)// c1: [104 101 108 108 111]
	fmt.Println("c1:", rel)// c1: hello

	// 127.0.0.1:6379> mset name Bob age 11 class 101
	// OK
	//rel2, err := redis.Strings(conn.Do("mget", "name", "age", "class"))
	rel2, err := redis.Values(conn.Do("mget", "name", "age", "class"))
	var name string
	var age int
	var class string
	redis.Scan(rel2, &name, &age, &class)
	if err != nil {
		fmt.Println("Receive error:", err)
	}
	
	//fmt.Println(rel2)// [Bob 11 101]
	fmt.Println("name:", name, "age:", age, "class:", class)// name: Bob age: 11 class: 101
}
```