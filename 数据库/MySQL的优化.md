# MySQL的优化

参考：[这些程序员最应该知道的MySQL知识！可惜之前没人告诉我…执行计划/索引/数据结构/设计原则/优化/锁/树/hash表/MVCC/优化..._哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1Xq4y1K7ck?p=1)

[TOC]



## 索引

**索引是帮助MySQL高效获取数据的<u>排好序的数据结构</u>**

> 索引数据结构：
>
> 1. 二叉树
> 2. 红黑树
> 3. Hash表
> 4. B树

**索引和实际数据都是存储在磁盘的，只不过在进行数据读取的时候，优先把索引加载到内存中**（引擎不是memory）

> **局部性原理：**
>
> 1. 时间局部性，之前访问过的数据有可能被再次访问
> 2. 空间局部性，数据和程序都有聚集成群的倾向
>
> **磁盘预读：**内存和磁盘在交互时，有一个最小的逻辑单位（页），一般为4k或8k，有操作系统决定，在进行数据读取时，一般会读取页的整数倍，innodb存储引擎在进行数据加载时读取的是16kb的数据

#### 索引实现的数据结构----B+树

[一文详解 B-树，B+树，B*树 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/98021010)

#### 索引采用Hash表时的问题

1. 需要比较好的hash算法，如果算法不好的话，会导致hash碰撞，hash冲突，导致数据散列不均匀
2. 但需要进行范围查找时，需要挨个遍历，效率比较低

> memory的存储引擎支持的就是hash索引，同时注意innodb存储引擎支持自适应hash

#### 采用B+树而不是B树的原因

> 使用B+树时，可以将所有的数据存在叶子节点，非叶子节点只放索引，而B树不可以，此处原因参考B+树和B的的结构差异
>
> 因此相对于B树，B+树在相同大小的索引块中可以存放更多的索引
>
> 此外，B+树对范围查找的支持要优于B树，B+树叶子节点之间有指针相连，B树没有

**一般三到四层的B+树已经足够支撑千万级别的数据存储**

#### 使用int还是varchar作为索引

此时应该考虑varchar(n)的大小比int大还是小，索引key的选取要尽可能的少占用空间

[(11条消息) MYSQL数据库索引设计的原则_IT技术分享社区-CSDN博客](https://blog.csdn.net/xishining/article/details/86653622)

#### 前缀索引

[MySQL前缀索引和索引选择性 - balfish - 博客园 (cnblogs.com)](https://www.cnblogs.com/balfish/p/9003794.html)

#### 索引优化



#### 索引失效



#### 回表

> 例如：表student有id、name、age等字段，id是主键，name是普通索引
>
> 执行：select * from student where name = 'AAA';
>
> 执行过程：先根据name索引的B+树匹配到对应叶子节点，查询到对应行记录的id值，再根据id去id的B+树种检索整行记录，这个过程称之为**回表**

#### 索引覆盖

> 例如：表student有id、name、age等字段，id是主键，name是普通索引
>
> 执行：select id, name from student where name = 'AAA';
>
> 执行过程：先根据name索引的B+树匹配到对应叶子节点，查询到对应行记录的id值，索引的叶子节点中包含了查询的所有列，此时不需要回表，这个过程叫做索引覆盖，using index的提示信息，推荐使用，**在某些场景中可以考虑将要查询的所有列都变成组合索引，此时会使用索引覆盖，加快查询效率**

#### 联合索引

> 创建索引的时候可以选择多个列来共同组成索引，此时叫做组合索引或者联合索引，要遵循**最左匹配原则**
>
> 例如：表student有id、name、age等字段，id是主键，name、age是组合索引
>
> 1. select * from table where name = 'AAA' and age = 12;
> 2. select * from table where name = 'AAA';
> 3. select * from table where age = 12;
> 4. select * from table where age = 12 and name = 'AAA';
>
> 上述查询语句，1和2会走组合索引，3不会，4会（优化器会将age = 12 and name = 'AAA'变为name = 'AAA' and age = 12来满足最左匹配）

#### **最左匹配**

> 例如：表student有id、name、age等字段，id是主键，name、age是组合索引
>
> 使用name和age匹配数据时，最左匹配先比较name，当name相同时才比较age，因此可以看到，如果仅仅查询age，在没有name的情况下，查询需要对整张表进行扫描，因为单独的age是无序的。

#### 索引下推

> select * from student where name = 'AAA' and age = 12;
>
> 没有索引下推之前，先根据name从存储引擎拉取数据到server层，然后在server层中对age进行数据过滤
>
> 有了索引下推之后，根据name和age两个条件来做数据筛选，将筛选之后的结果返回给server层
>
> **索引下推，将根据索引筛选的工作由server层下推到存储引擎**

#### 聚簇索引

**数据和索引存储在一起叫做聚簇索引，没有存储在一起叫做非聚簇索引**

#### InnoDB索引实现（聚簇，不准确）

> innodb存储引擎在进行数据插入的时候，数据必须要跟某一个索引存储在一起，这个索引列可以是主键，如果没有主键，选择唯一键，如果没有唯一键，选择6字节的rowid来进行存储
>
> 此时数据必定是跟一个索引绑定在一起的，绑定数据的索引叫做聚簇索引
>
> 其他索引的叶子节点中存储的数据不再是整行的记录，而是聚簇索引的id值
>
> 例如：表student有id、name、age等字段，id是主键，name是普通索引，此时：id是聚簇索引，name对应的索引的B+树上的叶子节点存储的就是id值

**innodb中既有聚簇索引也有非聚簇索引**

<u>MyISAM索引文件和数据文件是分离的（非聚簇），里面只有非聚簇索引</u>

1. 表数据文件本身是按B+Tree组织的一个索引文件

   > B+树相对于B树，在非叶子节点存储的只是索引，没有数据，这样同样大小的空间可存储的索引数量更多，使得整个树的高度降低，查找索引更快
   >
   > B树相邻叶子节点之间没有使用指针连接，对范围查找的支持没有B+树好

2. 聚集索引，叶节点包含了完整的数据记录

3. 为什么innoDB表必须建主键，并推荐使用整型自增主键

   > 当不建立主键时，MySQL会自动建立一个隐藏列作为主键，并由此主键作为聚集索引，自己建主键节省空间
   >
   > 整型作为索引，在查找索引时，真心的比较更快，空间更少
   >
   > 使用自增

4. 为什么非主键索引结构叶子节点存储的是主键值

### 执行计划

**一条sql语句的执行过程**

 ```mysql
 mysql> explain select * from t1;
 +----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+-------+
 | id | select_type | table | partitions | type | possible_keys | key  | key_len | ref  | rows | filtered | Extra |
 +----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+-------+
 |  1 | SIMPLE      | t1    | NULL       | ALL  | NULL          | NULL | NULL    | NULL |    6 |   100.00 | NULL  |
 +----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+-------+
 1 row in set, 1 warning (0.01 sec)
 
 mysql> explain select * from t1 where id = 3;
 +----+-------------+-------+------------+-------+---------------+---------+---------+-------+------+----------+-------+
 | id | select_type | table | partitions | type  | possible_keys | key     | key_len | ref   | rows | filtered | Extra |
 +----+-------------+-------+------------+-------+---------------+---------+---------+-------+------+----------+-------+
 |  1 | SIMPLE      | t1    | NULL       | const | PRIMARY       | PRIMARY | 4       | const |    1 |   100.00 | NULL  |
 +----+-------------+-------+------------+-------+---------------+---------+---------+-------+------+----------+-------+
 1 row in set, 1 warning (0.00 sec)
 ```

#### 优化问题

 一般的优化并不是出现了问题才优化，在进行数据库建模和数据库设计的时候会预先考虑一些优化问题，比如表字段的类型、长度等，包括创建合适的索引等方式，但这些方式都属于提前的预防，并不一定能解决所有的问题，所以当生产环境出现sql问题时，需要从数据库的性能监控、索引的创建和维护、sql语句的调整、参数的设置、架构的调整等多个方面进行综合考虑，性能监控可以使用show profiles，performance_schema来进行监控，等等

## 存储引擎

#### innodb的四大特性

[innodb引擎的4大特性 - h_s - 博客园 (cnblogs.com)](https://www.cnblogs.com/zhs0/p/10528520.html)

> 1. 插入缓冲
> 2. 二次写
> 3. 自适应哈希
> 4. 预读

#### 隔离级别

[浅析MySQL InnoDB的隔离级别 - 码畜的一生 - 博客园 (cnblogs.com)](https://www.cnblogs.com/lingqin/p/10279415.html)

> 在**SQL标准**中定义了4种隔离级别，分别是：
>
> - **Read uncommitted**: 未提交读，事务中的修改，即使没有提交，对其他事务也是可见的。存在脏读
> - **Read committed**: 提交读，大多数数据库系统的默认隔离级别(MySQL不是), 一个事务从开始到提交之前，所做的修改对其他事务不可见。解决脏读，存在幻读和不可重复读
> - **repeatable read**: 可重复读，该级别保证在同一事务中多次读取同样记录的结果是一致的。解决脏读和不可重复读，理论上存在幻读，但是在InnoDB引擎中解决了幻读
> - **Serializable**：可串行化，强制事务串行执行。

#### 联合索引

> 创建索引的时候可以选择多个列来共同组成索引，此时叫做组合索引或者联合索引，要遵循**最左匹配原则**
>
> 例如：表student有id、name、age等字段，id是主键，nam、age是组合索引
>
> 1. select * from table where name = 'AAA' and age = 12;
> 2. select * from table where name = 'AAA';
> 3. select * from table where age = 12;
> 4. select * from table where age = 12 and name = 'AAA';
>
> 上述查询语句，1和2会走组合索引，3不会，4会（优化器会将age = 12 and name = 'AAA'变为name = 'AAA' and age = 12来满足最左匹配）

### MVCC多版本并发控制

**当前读：**读取的是数据的最新版本，总是读取到最新的数据

> **当前读的数据**
>
> 1. select .... lock in share mode
> 2. select .... for update
> 3. update
> 4. delete
> 5. insert

**快照读：**读取的是历史版本的记录

> 实例：
>
> 事务A：select  -> ............................. ->select
>
> 事务B：select -> update -> commit -> ...
>
> **事务A第二次select能否获取事务B修改后的最新数据？**
>
> > 隔离级别：
> >
> > 1. 读未提交
> > 2. 读已提交RC
> > 3. 可重复读RR（默认隔离级）
> > 4. 串行化
>
> 1. RC：可以读取到最新的结果记录
> 2. RR：不可以读取到最新的结果记录
>
> **可见性算法导致上述的是否可读最新数据**

#### MVCC的三个主要组件

**第一部分：隐藏字段**

> 每一行记录上都包含几个用户不可见字段
>
> 1. DB_TRX_ID：创建或者修改该记录的最近一次的事务id
> 2. DB_ROW_ID：隐藏主键
> 3. DB_ROLL_PTR：回滚指针，与undolog配合使用

![image-20210901150336791](C:\Users\Wangml\AppData\Roaming\Typora\typora-user-images\image-20210901150336791.png)

**第二部分：undolog回滚日志**

> 存储事务历史信息
>
> **但不同事务对同一条记录做修改的时候，会导致该记录的undolog形成一个链表，链表的尾部是最早的历史记录，链表的首部是最新的历史记录**
>
> 如出现事务3对同一条记录做修改，那么事务3修改后，最新记录的DB_TRX_ID变为3，DB_ROLL_PTR指向新的历史记录（形成undolog的链表）
>
> **当出现事务4，那么事务4读取到的数据是哪个版本？**
>
> 有对应规则，按对应规则判断读取，见第三部分

![image-20210901150632108](C:\Users\Wangml\AppData\Roaming\Typora\typora-user-images\image-20210901150632108.png)

**第三部分：readview读视图**

> readview：事务在进行快照读的时候产生的读视图
>
> readview中包括：
>
> 1. trx_list：系统活跃的事务id
> 2. up_limit_id：列表中事务最小的id
> 3. low_limit_id：系统尚未分配的下一个事务id

![image-20210901153705828](C:\Users\Wangml\AppData\Roaming\Typora\typora-user-images\image-20210901153705828.png)

> 事务2能否读到刚修改的记录值？
>
> 分析：
>
> 1. trx_list：包含 1、2、3 （4此时不在trx_list，提交后不属于活跃事务）
> 2. up_limit_id：1
> 3. low_limit_id：5
> 4. DB_TRX_ID：4
>
> **可见性算法判断规则：**
>
> 1. 首先比较DB_TRX_ID < up_limt_id，如果小于，则当前事务能看到DB_TRX_ID所在的记录，如果大于，进入下一个判断
> 2. 接下来判断DB_TRX_ID >= low_limit_id，如果大于等于，则代表DB_TRX_ID所在的记录在Read View生成后才出现，那么对当前事务肯定不可见，如果小于，则进入下一步判断
> 3. 判断DB_TRX_ID是否在活跃事务中，如果在，则代表Read View生成时刻，这个事务还是活跃状态，还没有commit，修改的数据当前事务也还是看不到，如果不在，则说明这个事务在Read View生成之前就已经开始commit，那么修改结果是能够看见的。
>
> **经过当前可见性算法的判断之后，可以得到结论，能看到修改的记录**
>
> 1. RC：可以读取到最新的结果记录
>
> 2. RR：不可以读取到最新的结果记录
>
>    > 不同隔离级别导致readview生成的时机是不同的
>    >
>    > RC：每次在进行快照读的时候都会生成新的readview
>    >
>    > RR：只有在第一次进行快照读的时候才会生成readview，之后读操作都会用第一次生成的readview
>    >
>    > ​	**RR下，如果使用update等需要当前读的操作，会出现换读问题**

### ACID

#### 原子性

**原子性通过undolog实现**



#### 一致性



#### 隔离性

**隔离性通过MVCC实现**



#### 持久性

**通过redolog实现**

**二阶段提交问题**

> WAL，write ahead log预写日志
>
> **先写日志再写数据**
>
> 写日志是比较快的，而查数据是比较慢的（顺序读写与随机读写问题）
>
> 因为随机读写的效率要低于随机读写，为了保证数据的一致性，可以先将数据通过顺序读写的方式写道日志文件中，然后再将数据写入到对应的磁盘文件，这个过程顺序的效率要远远高于随机的效率，换句话说，如果实际的数据没有写入磁盘，只要日志文件保存成功，那么数据就不会丢失，可以根据日志来进行数据的恢复。

![image-20210901163402004](C:\Users\Wangml\AppData\Roaming\Typora\typora-user-images\image-20210901163402004.png)

#### 查看MySQL锁情况

> set global innodb_status_output_locks=1;
>
> show engine innodb status\G;

```mysql
mysql> show engine innodb status\G;
*************************** 1. row ***************************
  Type: InnoDB
  Name: 
Status: 
=====================================
2021-09-02 09:28:36 0x7f0e9c0da700 INNODB MONITOR OUTPUT
=====================================
Per second averages calculated from the last 44 seconds
-----------------
BACKGROUND THREAD
-----------------
srv_master_thread loops: 14 srv_active, 0 srv_shutdown, 235821 srv_idle
srv_master_thread log flush and writes: 235835
----------
SEMAPHORES
----------
OS WAIT ARRAY INFO: reservation count 38
OS WAIT ARRAY INFO: signal count 35
RW-shared spins 0, rounds 76, OS waits 38
RW-excl spins 0, rounds 0, OS waits 0
RW-sx spins 0, rounds 0, OS waits 0
Spin rounds per wait: 76.00 RW-shared, 0.00 RW-excl, 0.00 RW-sx
------------
TRANSACTIONS# 注意此处
------------
Trx id counter 3401
Purge done for trx's n:o < 3400 undo n:o < 0 state: running but idle
History list length 0
LIST OF TRANSACTIONS FOR EACH SESSION:
---TRANSACTION 3400, ACTIVE 65 sec
2 lock struct(s), heap size 1136, 1 row lock(s)
MySQL thread id 9, OS thread handle 139700724410112, query id 106 localhost root starting
show engine innodb status
Trx read view will not see trx with id >= 3400, sees < 3400
TABLE LOCK table `study`.`student` trx id 3400 lock mode IX
RECORD LOCKS space id 62 page no 3 n bits 80 index PRIMARY of table `study`.`student` trx id 3400 lock_mode X locks r
Record lock, heap no 8 PHYSICAL RECORD: n_fields 6; compact format; info bits 0
 0: len 4; hex 80000008; asc     ;;
 1: len 6; hex 0000000008e1; asc       ;;
 2: len 7; hex a60000011a011c; asc        ;;
 3: len 5; hex 64616c696e; asc dalin;;
 4: len 4; hex 80000012; asc     ;;
 5: len 4; hex 78697975; asc xiyu;;

--------
FILE I/O
--------
I/O thread 0 state: waiting for completed aio requests (insert buffer thread)
I/O thread 1 state: waiting for completed aio requests (log thread)
I/O thread 2 state: waiting for completed aio requests (read thread)
I/O thread 3 state: waiting for completed aio requests (read thread)
I/O thread 4 state: waiting for completed aio requests (read thread)
I/O thread 5 state: waiting for completed aio requests (read thread)
I/O thread 6 state: waiting for completed aio requests (write thread)
I/O thread 7 state: waiting for completed aio requests (write thread)
I/O thread 8 state: waiting for completed aio requests (write thread)
I/O thread 9 state: waiting for completed aio requests (write thread)
Pending normal aio reads: [0, 0, 0, 0] , aio writes: [0, 0, 0, 0] ,
 ibuf aio reads:, log i/o's:, sync i/o's:
Pending flushes (fsync) log: 0; buffer pool: 0
438 OS file reads, 390 OS file writes, 184 OS fsyncs
0.00 reads/s, 0 avg bytes/read, 0.00 writes/s, 0.00 fsyncs/s
-------------------------------------
INSERT BUFFER AND ADAPTIVE HASH INDEX
-------------------------------------
Ibuf: size 1, free list len 0, seg size 2, 0 merges
merged operations:
 insert 0, delete mark 0, delete 0
discarded operations:
 insert 0, delete mark 0, delete 0
Hash table size 34679, node heap has 0 buffer(s)
Hash table size 34679, node heap has 0 buffer(s)
Hash table size 34679, node heap has 0 buffer(s)
Hash table size 34679, node heap has 0 buffer(s)
Hash table size 34679, node heap has 0 buffer(s)
Hash table size 34679, node heap has 0 buffer(s)
Hash table size 34679, node heap has 0 buffer(s)
Hash table size 34679, node heap has 0 buffer(s)
0.00 hash searches/s, 0.02 non-hash searches/s
---
LOG
---
Log sequence number 5171984
Log flushed up to   5171984
Pages flushed up to 5171984
Last checkpoint at  5171975
0 pending log flushes, 0 pending chkp writes
120 log i/o's done, 0.00 log i/o's/second
----------------------
BUFFER POOL AND MEMORY
----------------------
Total large memory allocated 137428992
Dictionary memory allocated 131171
Buffer pool size   8192
Free buffers       7748
Database pages     444
Old database pages 0
Modified db pages  0
Pending reads      0
Pending writes: LRU 0, flush list 0, single page 0
Pages made young 0, not young 0
0.00 youngs/s, 0.00 non-youngs/s
Pages read 402, created 42, written 241
0.00 reads/s, 0.00 creates/s, 0.00 writes/s
Buffer pool hit rate 1000 / 1000, young-making rate 0 / 1000 not 0 / 1000
Pages read ahead 0.00/s, evicted without access 0.00/s, Random read ahead 0.00/s
LRU len: 444, unzip_LRU len: 0
I/O sum[0]:cur[0], unzip sum[0]:cur[0]
--------------
ROW OPERATIONS
--------------
0 queries inside InnoDB, 0 queries in queue
1 read views open inside InnoDB
Process ID=19776, Main thread ID=139700631226112, state: sleeping
Number of rows inserted 12, updated 2, deleted 0, read 214
0.00 inserts/s, 0.00 updates/s, 0.00 deletes/s, 0.02 reads/s
----------------------------
END OF INNODB MONITOR OUTPUT
============================

1 row in set (0.01 sec)

ERROR: 
No query specified
```



#### 日志

1. MySQL：binlog
2. innodb，插件引擎：
   1. undolog
   2. redolog

> 因为两种日志属于不同的组件，所以为了保证数据的一致性，要保证binlog和redolog的一致性，所以有了二阶段提交的概念
>
> ![image-20210901163755408](C:\Users\Wangml\AppData\Roaming\Typora\typora-user-images\image-20210901163755408.png)
>
> 

## ShardingSphere

[ShardingSphere (apache.org)](https://shardingsphere.apache.org/index_zh.html)

## HyperLogLog

[HyperLogLog在redis有什么使用原理？ - 知乎 (zhihu.com)](https://www.zhihu.com/question/40671988/answer/1227323251)