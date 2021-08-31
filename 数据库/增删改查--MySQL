### MySQL相关操作

<u>参考之前自己写的博客[数据库 - 随笔分类 - 荒唐了年少 - 博客园 (cnblogs.com)](https://www.cnblogs.com/lnlin/category/1006265.html)</u>

#### 登录数据库

```mysql
(base) [root@CentOS-wangml ~]# mysql -uroot -p
Enter password: 
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 86
Server version: 5.7.34 MySQL Community Server (GPL)

Copyright (c) 2000, 2021, Oracle and/or its affiliates.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.
```

#### 查看MySQL中现存数据库

```mysql
mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| data               |
| mysql              |
| performance_schema |
| study              |
| sys                |
| wordpress          |
+--------------------+
7 rows in set (0.01 sec)
```

#### 创建数据库

```mysql
mysql> create database temp;
Query OK, 1 row affected (0.01 sec)
```

#### 删除数据库

```MYSQL
mysql> drop database temp;
Query OK, 0 rows affected (0.04 sec)
```

#### 查看已经创建好的数据库信息

```mysql
mysql> show create database study;
+----------+------------------------------------------------------------------+
| Database | Create Database                                                  |
+----------+------------------------------------------------------------------+
| study    | CREATE DATABASE `study` /*!40100 DEFAULT CHARACTER SET latin1 */ |
+----------+------------------------------------------------------------------+
1 row in set (0.01 sec)
```

#### 选择使用某个数据库

```mysql
mysql> use study;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed
```

#### 查看数据库中的表

```mysql
mysql> show tables;
+-----------------+
| Tables_in_study |
+-----------------+
| stu             |
| stu1            |
| stu2            |
| t1              |
| t2              |
| t3              |
| t4              |
+-----------------+
7 rows in set (0.00 sec)
```

#### 创建表

```mysql
mysql> create table student ( name varchar(20), age int(10));
Query OK, 0 rows affected (0.13 sec)

create table 表名
(
  字段名1 数据类型[完整约束条件],
  字段名2 数据类型[完整约束条件],

  字段名n 数据类型[完整约束条件]
);
```

#### 删除表

```mysql
mysql> drop table student;
Query OK, 0 rows affected (0.04 sec)
```

#### 查看数据表信息

```mysql
mysql> show create table stu;
+-------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Table | Create Table                                                                                                                                                                                                                                                               |
+-------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| stu   | CREATE TABLE `stu` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'primary_key',
  `name` varchar(20) NOT NULL,
  `addr` varchar(50) DEFAULT 'noone knows',
  `score` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=latin1 |
+-------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

#### 查看数据表字段信息

```mysql
mysql> describe stu;
+-------+-------------+------+-----+-------------+----------------+
| Field | Type        | Null | Key | Default     | Extra          |
+-------+-------------+------+-----+-------------+----------------+
| id    | int(11)     | NO   | PRI | NULL        | auto_increment |
| name  | varchar(20) | NO   |     | NULL        |                |
| addr  | varchar(50) | YES  |     | noone knows |                |
| score | int(11)     | YES  |     | NULL        |                |
+-------+-------------+------+-----+-------------+----------------+
4 rows in set (0.01 sec)

mysql> desc stu;
+-------+-------------+------+-----+-------------+----------------+
| Field | Type        | Null | Key | Default     | Extra          |
+-------+-------------+------+-----+-------------+----------------+
| id    | int(11)     | NO   | PRI | NULL        | auto_increment |
| name  | varchar(20) | NO   |     | NULL        |                |
| addr  | varchar(50) | YES  |     | noone knows |                |
| score | int(11)     | YES  |     | NULL        |                |
+-------+-------------+------+-----+-------------+----------------+
4 rows in set (0.00 sec)
```

#### 修改表名

```mysql
mysql> alter table stu rename student;
Query OK, 0 rows affected (0.04 sec)
```

#### 修改字段的字段名和字段数据类型

```mysql
mysql> alter table student change age score float;
Query OK, 4 rows affected (0.11 sec)
Records: 4  Duplicates: 0  Warnings: 0

mysql> desc student;
+-------+-------------+------+-----+-------------+----------------+
| Field | Type        | Null | Key | Default     | Extra          |
+-------+-------------+------+-----+-------------+----------------+
| id    | int(11)     | NO   | PRI | NULL        | auto_increment |
| name  | varchar(20) | NO   |     | NULL        |                |
| addr  | varchar(50) | YES  |     | noone knows |                |
| score | float       | YES  |     | NULL        |                |
+-------+-------------+------+-----+-------------+----------------+
4 rows in set (0.00 sec)

mysql> alter table student change column score age int(10);
Query OK, 4 rows affected (0.14 sec)
Records: 4  Duplicates: 0  Warnings: 0

mysql> desc student;
+-------+-------------+------+-----+-------------+----------------+
| Field | Type        | Null | Key | Default     | Extra          |
+-------+-------------+------+-----+-------------+----------------+
| id    | int(11)     | NO   | PRI | NULL        | auto_increment |
| name  | varchar(20) | NO   |     | NULL        |                |
| addr  | varchar(50) | YES  |     | noone knows |                |
| age   | int(10)     | YES  |     | NULL        |                |
+-------+-------------+------+-----+-------------+----------------+
4 rows in set (0.00 sec)
```

#### 只修改字段数据类型

```mysql
mysql> alter table student modify name varchar(10);
Query OK, 4 rows affected (0.08 sec)
Records: 4  Duplicates: 0  Warnings: 0

mysql> desc student;
+-------+-------------+------+-----+-------------+----------------+
| Field | Type        | Null | Key | Default     | Extra          |
+-------+-------------+------+-----+-------------+----------------+
| id    | int(11)     | NO   | PRI | NULL        | auto_increment |
| name  | varchar(10) | YES  |     | NULL        |                |
| addr  | varchar(50) | YES  |     | noone knows |                |
| age   | int(10)     | YES  |     | NULL        |                |
+-------+-------------+------+-----+-------------+----------------+
4 rows in set (0.00 sec)
```

#### 添加字段

```mysql
mysql> alter table student add class varchar(10);
Query OK, 0 rows affected (0.08 sec)
Records: 0  Duplicates: 0  Warnings: 0

mysql> desc student;
+-------+-------------+------+-----+-------------+----------------+
| Field | Type        | Null | Key | Default     | Extra          |
+-------+-------------+------+-----+-------------+----------------+
| id    | int(11)     | NO   | PRI | NULL        | auto_increment |
| name  | varchar(10) | YES  |     | NULL        |                |
| addr  | varchar(50) | YES  |     | noone knows |                |
| age   | int(10)     | YES  |     | NULL        |                |
| class | varchar(10) | YES  |     | NULL        |                |
+-------+-------------+------+-----+-------------+----------------+
5 rows in set (0.00 sec)
```

#### 删除字段

```mysql
mysql> alter table student drop class;
Query OK, 0 rows affected (0.13 sec)
Records: 0  Duplicates: 0  Warnings: 0
```

#### 修改字段的排列位置

```mysql
mysql> alter table student modify name varchar(10) first;# 将name字段排在第一位
Query OK, 0 rows affected (0.09 sec)
Records: 0  Duplicates: 0  Warnings: 0

mysql> desc student;
+-------+-------------+------+-----+-------------+----------------+
| Field | Type        | Null | Key | Default     | Extra          |
+-------+-------------+------+-----+-------------+----------------+
| name  | varchar(10) | YES  |     | NULL        |                |
| id    | int(11)     | NO   | PRI | NULL        | auto_increment |
| addr  | varchar(50) | YES  |     | noone knows |                |
| age   | int(10)     | YES  |     | NULL        |                |
+-------+-------------+------+-----+-------------+----------------+
4 rows in set (0.00 sec)

mysql> alter table student modify age int(10) after id;# 将age字段放在id后面
Query OK, 0 rows affected (0.14 sec)
Records: 0  Duplicates: 0  Warnings: 0

mysql> desc student;\
+-------+-------------+------+-----+-------------+----------------+
| Field | Type        | Null | Key | Default     | Extra          |
+-------+-------------+------+-----+-------------+----------------+
| name  | varchar(10) | YES  |     | NULL        |                |
| id    | int(11)     | NO   | PRI | NULL        | auto_increment |
| age   | int(10)     | YES  |     | NULL        |                |
| addr  | varchar(50) | YES  |     | noone knows |                |
+-------+-------------+------+-----+-------------+----------------+
4 rows in set (0.00 sec)

```

#### 向表中插入数据

```mysql
mysql> insert into student (name, age, addr) values ('xiaoming', 13, 'xinjiang');# 插入一条
Query OK, 1 row affected (0.00 sec)

mysql> insert into student (name, age, addr) values ('xiaohuang', 15, 'tianjin'), ('dalin', 18, 'xiyu');# 插入多条
Query OK, 2 rows affected (0.00 sec)
Records: 2  Duplicates: 0  Warnings: 0

mysql> insert into student values ('liai', 9, 17, 'hubei');# 省略字段名 默认按照字段排列顺序插入
Query OK, 1 row affected (0.00 sec)

mysql> insert into student set name = 'Chaera', age = 23, addr = 'Laoda';# insert的另一种写法
Query OK, 1 row affected (0.01 sec)

mysql> select * from student;
+-----------+----+------+-------------+
| name      | id | age  | addr        |
+-----------+----+------+-------------+
| tom       |  1 |   89 | shanghai    |
| bers      |  2 | NULL | Beij        |
| haha      |  3 | NULL | Janpan      |
| lalala    |  4 |   66 | noone knows |
| xiaoming  |  6 |   13 | xinjiang    |
| xiaohuang |  7 |   15 | tianjin     |
| dalin     |  8 |   18 | xiyu        |
| liai      |  9 |   17 | hubei       |
| Chaera    | 10 |   23 | Laoda       |
+-----------+----+------+-------------+
9 rows in set (0.00 sec)
```

#### 修改数据

```mysql
mysql> update student set addr = 'Nanjin' where id = 10;# 修改表中所有数据时 不需要设置where条件
Query OK, 1 row affected (0.01 sec)
Rows matched: 1  Changed: 1  Warnings: 0

mysql> select * from student;
+-----------+----+------+-------------+
| name      | id | age  | addr        |
+-----------+----+------+-------------+
| tom       |  1 |   89 | shanghai    |
| bers      |  2 | NULL | Beij        |
| haha      |  3 | NULL | Janpan      |
| lalala    |  4 |   66 | noone knows |
| xiaoming  |  6 |   13 | xinjiang    |
| xiaohuang |  7 |   15 | tianjin     |
| dalin     |  8 |   18 | xiyu        |
| liai      |  9 |   17 | hubei       |
| Chaera    | 10 |   23 | Nanjin      |
+-----------+----+------+-------------+
9 rows in set (0.00 sec)
```

#### 删除数据

```mysql
mysql> delete from student where id = 4;# 删除全部数据时 不需要加上where条件
Query OK, 1 row affected (0.00 sec)

mysql> select * from student;
+-----------+----+------+----------+
| name      | id | age  | addr     |
+-----------+----+------+----------+
| tom       |  1 |   89 | shanghai |
| bers      |  2 | NULL | Beij     |
| haha      |  3 | NULL | Janpan   |
| xiaoming  |  6 |   13 | xinjiang |
| xiaohuang |  7 |   15 | tianjin  |
| dalin     |  8 |   18 | xiyu     |
| liai      |  9 |   17 | hubei    |
| Chaera    | 10 |   23 | Nanjin   |
+-----------+----+------+----------+
8 rows in set (0.00 sec)
```

**还可以使用 truncate 表名 来删除全部数据**

> ```
> truncate 表名 删除全部数据 与 delete from 表名 删除全部数据的异同：
> 它们都可以删除全部数据，但对于自动增加字段的值，truncate 表名 删除全部数据之后再向表中添加数据时，自动增加字段的默认值时从1开始，而对于 delete from 表名 删除全部数据，自动增加字段的默认值是从未删除时该字段的值加1开始。
> ```

#### select语句的基本语法

```mysql
select 查询内容
from 表名
where 表达式
group by 表达式
having 表达式
order by 字段名
limit 记录数
```

#### **使用通配符\*表示所有内容**

```mysql
mysql> select * from student;
+-----------+----+------+----------+
| name      | id | age  | addr     |
+-----------+----+------+----------+
| tom       |  1 |   89 | shanghai |
| bers      |  2 | NULL | Beij     |
| haha      |  3 | NULL | Janpan   |
| xiaoming  |  6 |   13 | xinjiang |
| xiaohuang |  7 |   15 | tianjin  |
| dalin     |  8 |   18 | xiyu     |
| liai      |  9 |   17 | hubei    |
| Chaera    | 10 |   23 | Nanjin   |
+-----------+----+------+----------+
8 rows in set (0.00 sec)
```

#### 查询指定字段

```mysql
mysql> select id, name from student;
+----+-----------+
| id | name      |
+----+-----------+
|  1 | tom       |
|  2 | bers      |
|  3 | haha      |
|  6 | xiaoming  |
|  7 | xiaohuang |
|  8 | dalin     |
|  9 | liai      |
| 10 | Chaera    |
+----+-----------+
8 rows in set (0.00 sec)
```

#### where按条件查询

> **select 查询内容 from 表名 where 表达式；**
>
> 在MySQL语句中，**条件表达式**是指**select语句的查询条件**，在where子句中可以使用**关系运算符**连接操作数作为查询条件对数据进行选择。
> **关系运算符：**
>
> 1. =  等于
> 2. <> 不等于
> 3. != 不等于
> 4. <  小于
> 5. \>  大于
> 6. <= 小于等于
> 7. \>= 大于等于

```mysql
mysql> select id, name, age from student where id > 5;
+----+-----------+------+
| id | name      | age  |
+----+-----------+------+
|  6 | xiaoming  |   13 |
|  7 | xiaohuang |   15 |
|  8 | dalin     |   18 |
|  9 | liai      |   17 |
| 10 | Chaera    |   23 |
+----+-----------+------+
5 rows in set (0.00 sec)
```

#### in关键字

**指定集合内**

```mysql
mysql> select id, name, age from student where id in (1, 3, 7, 9);
+----+-----------+------+
| id | name      | age  |
+----+-----------+------+
|  1 | tom       |   89 |
|  3 | haha      | NULL |
|  7 | xiaohuang |   15 |
|  9 | liai      |   17 |
+----+-----------+------+
4 rows in set (0.00 sec)
```

#### between关键字

**指定范围内内**

```mysql
mysql> select id, name, age from student where id between 6 and 9;
+----+-----------+------+
| id | name      | age  |
+----+-----------+------+
|  6 | xiaoming  |   13 |
|  7 | xiaohuang |   15 |
|  8 | dalin     |   18 |
|  9 | liai      |   17 |
+----+-----------+------+
4 rows in set (0.00 sec)
```

#### 查询某些为空或非空的记录

```mysql
mysql> select * from student where age is NULL;
+------+----+------+--------+
| name | id | age  | addr   |
+------+----+------+--------+
| bers |  2 | NULL | Beij   |
| haha |  3 | NULL | Janpan |
+------+----+------+--------+
2 rows in set (0.00 sec)

mysql> select * from student where age is not NULL;
+-----------+----+------+----------+
| name      | id | age  | addr     |
+-----------+----+------+----------+
| tom       |  1 |   89 | shanghai |
| xiaoming  |  6 |   13 | xinjiang |
| xiaohuang |  7 |   15 | tianjin  |
| dalin     |  8 |   18 | xiyu     |
| liai      |  9 |   17 | hubei    |
| Chaera    | 10 |   23 | Nanjin   |
+-----------+----+------+----------+
6 rows in set (0.00 sec)
```

#### 过滤重复记录

**在使用distinct指定多个字段时，只有被指定的这些字段的值都相同，才会被认为是重复的**

```mysql
mysql> select * from newbaby;
+----+----------+
| id | name     |
+----+----------+
|  1 | xiaoli   |
|  2 | xiaoming |
|  3 | xiaoming |
|  4 | xiaoli   |
+----+----------+
4 rows in set (0.00 sec)

mysql> select distinct name from newbaby;
+----------+
| name     |
+----------+
| xiaoli   |
| xiaoming |
+----------+
2 rows in set (0.01 sec)

mysql> select distinct id, name from newbaby;
+----+----------+
| id | name     |
+----+----------+
|  1 | xiaoli   |
|  2 | xiaoming |
|  3 | xiaoming |
|  4 | xiaoli   |
+----+----------+
4 rows in set (0.00 sec)
```

#### 模糊查询

**select 查询内容 from 表名 where 内容 (not) like ‘匹配的字符串’**

> **百分号通配符 %：**表示匹配任意长度的任意字符串
>
> **下划线通配符 _** :表示匹配任意单个字符，如果需要匹配多个字符，则需要使用多个 _ 
>
> 如果需要查询带有 **% 或 _** 的数据，由于 % 和 _ 是通配符，则需要使用 **\ 进行转义**\% 表示 %，\_ 表示 _

```mysql
mysql> select id, name, age from student where name like 'x%';
+----+-----------+------+
| id | name      | age  |
+----+-----------+------+
|  6 | xiaoming  |   13 |
|  7 | xiaohuang |   15 |
+----+-----------+------+
2 rows in set (0.00 sec)

mysql> select id, name, age from student where name like ('to_');
+----+------+------+
| id | name | age  |
+----+------+------+
|  1 | tom  |   89 |
+----+------+------+
1 row in set (0.00 sec)
```

#### 多个限定条件

> and(&&)、or(||) 表示 与、或

```mysql
mysql> select id, name, age from student where name like 'x%' and age > 14;
+----+-----------+------+
| id | name      | age  |
+----+-----------+------+
|  7 | xiaohuang |   15 |
+----+-----------+------+
1 row in set (0.01 sec)

mysql> select id, name, age from student where name like ('to_') or name = 'liai';
+----+------+------+
| id | name | age  |
+----+------+------+
|  1 | tom  |   89 |
|  9 | liai |   17 |
+----+------+------+
2 rows in set (0.00 sec)
```

#### 聚合函数

> 1. **count()函数：**统计记录条数 select count(记录) from 表名
> 2. **sum()函数：**计算表中某个字段值的总和，select sum(字段名) from 表名
> 3.  **avg()函数：**计算表中某个字段的平均值 select avg(字段名) from 表名
> 4. **max()函数：**返回表中某个字段中的最大值
> 5. **min()函数：**返回表中某个字段中的最小值

```mysql
mysql> select count(name) from student;
+-------------+
| count(name) |
+-------------+
|           8 |
+-------------+
1 row in set (0.00 sec)

mysql> select sum(age) from student;
+----------+
| sum(age) |
+----------+
|      175 |
+----------+
1 row in set (0.00 sec)

mysql> select avg(age) from student;
+----------+
| avg(age) |
+----------+
|  29.1667 |
+----------+
1 row in set (0.00 sec)

mysql> select max(age) from student;
+----------+
| max(age) |
+----------+
|       89 |
+----------+
1 row in set (0.00 sec)

mysql> select min(age) from student;
+----------+
| min(age) |
+----------+
|       13 |
+----------+
1 row in set (0.00 sec)
```

#### 分组查询

```mysql
  mysql> select * from test;
  +------+------+------+-------+
  | id   | name | age  | class |
  +------+------+------+-------+
  |    1 | A    |    4 |     1 |
  |    2 | B    |    7 |     1 |
  |    3 | C    |    5 |     1 |
  |    4 | D    |   12 |     2 |
  |    5 | E    |    0 |     2 |
  |    6 | F    |    8 |     3 |
  +------+------+------+-------+
  6 rows in set (0.00 sec)
  
  mysql> select max(age) from test group by class;
  +----------+
  | max(age) |
  +----------+
  |        7 |
  |       12 |
  |        8 |
  +----------+
  3 rows in set (0.03 sec)
```

**此处可能会遇到Expression #1 of SELECT list is not in GROUP BY clause and contains nonaggre问题**

#### 对查询结果进行排序

> **select 查询内容 from 表名 order by 排序条件 asc/desc，**asc表示升序 desc表示降序

```mysql
mysql> select name, age, addr from student order by age asc;
+-----------+------+----------+
| name      | age  | addr     |
+-----------+------+----------+
| xiaoming  |   13 | xinjiang |
| xiaohuang |   15 | tianjin  |
| bers      |   16 | Beij     |
| haha      |   16 | Janpan   |
| liai      |   17 | hubei    |
| dalin     |   18 | xiyu     |
| Chaera    |   23 | Nanjin   |
| tom       |   89 | shanghai |
+-----------+------+----------+
8 rows in set (0.00 sec)

mysql> select name, age, addr from student order by age desc;
+-----------+------+----------+
| name      | age  | addr     |
+-----------+------+----------+
| tom       |   89 | shanghai |
| Chaera    |   23 | Nanjin   |
| dalin     |   18 | xiyu     |
| liai      |   17 | hubei    |
| bers      |   16 | Beij     |
| haha      |   16 | Janpan   |
| xiaohuang |   15 | tianjin  |
| xiaoming  |   13 | xinjiang |
+-----------+------+----------+
8 rows in set (0.00 sec)
```

#### 限制查询

> **select 查询内容 from 表名 limit 偏移量m 记录数n**，表示从第m+1个记录开始查询出n条记录

```mysql
mysql> select name, age, addr from student order by age asc limit 2, 4;
+-------+------+--------+
| name  | age  | addr   |
+-------+------+--------+
| bers  |   16 | Beij   |
| haha  |   16 | Janpan |
| liai  |   17 | hubei  |
| dalin |   18 | xiyu   |
+-------+------+--------+
4 rows in set (0.00 sec)
```

#### where和having

[(10条消息) 正确理解MySQL中的where和having的区别_Benjamin的博客-CSDN博客_where和having](https://blog.csdn.net/yexudengzhidao/article/details/54924471)

> 1. where和having都可以使用的场景
>    select goods_price,goods_name from sw_goods where goods_price > 100
>
>    select goods_price,goods_name from sw_goods having goods_price > 100
>
>    解释：上面的having可以用的前提是我已经筛选出了goods_price字段，在这种情况下和where的效果是等效的，但是如果我没有select goods_price 就会报错！！因为having是从前筛选的字段再筛选，而where是从数据表中的字段直接进行的筛选的。
>
> 2. 只可以用where，不可以用having的情况
>    select goods_name,goods_number from sw_goods where goods_price > 100
>
>    select goods_name,goods_number from sw_goods having goods_price > 100 //报错！！！因为前面并没有筛选出goods_price 字段
>
> 3. 只可以用having，不可以用where情况
>    查询每种goods_category_id商品的价格平均值，获取平均价格大于1000元的商品信息
>
>    select goods_category_id , avg(goods_price) as ag from sw_goods group by goods_category having ag > 1000
>
>    select goods_category_id , avg(goods_price) as ag from sw_goods where ag>1000 group by goods_category //报错！！因为from sw_goods 这张数据表里面没有ag这个字段
>
>    注意:where 后面要跟的是数据表里的字段，如果我把ag换成avg(goods_price)也是错误的！因为表里没有该字段。而having只是根据前面查询出来的是什么就可以后面接什么。

#### 交叉查询

> 结果：返归第一个表中复合查询条件的数据行数乘以第二个表中复合查询条件的行数 
>
> 语法格式：select 查询内容 from 表1 cross join 表2

```mysql
mysql> select * from t1;
+----+------+
| id | name |
+----+------+
|  1 | AAA  |
|  2 | BBB  |
|  3 | CCC  |
+----+------+
3 rows in set (0.00 sec)

mysql> select * from t2;
+------+------+
| name | age  |
+------+------+
| AAA  |   12 |
| BBB  |   13 |
| CCC  |   15 |
+------+------+
3 rows in set (0.00 sec)

mysql> select * from t1 cross join t2;
+----+------+------+------+
| id | name | name | age  |
+----+------+------+------+
|  1 | AAA  | AAA  |   12 |
|  2 | BBB  | AAA  |   12 |
|  3 | CCC  | AAA  |   12 |
|  1 | AAA  | BBB  |   13 |
|  2 | BBB  | BBB  |   13 |
|  3 | CCC  | BBB  |   13 |
|  1 | AAA  | CCC  |   15 |
|  2 | BBB  | CCC  |   15 |
|  3 | CCC  | CCC  |   15 |
+----+------+------+------+
9 rows in set (0.00 sec)
```

#### 内连接

> 内连接：又称为简单连接或自然连接
> 在内连接查询中，只有满足条件的记录才会出现在查询结果中
> 语法格式：select 查询内容 from 表1 join 表2 on 表1.关系字段 = 表2.关系字段

```mysql
mysql> select * from t1;
+----+------+
| id | name |
+----+------+
|  1 | AAA  |
|  2 | BBB  |
|  3 | CCC  |
|  4 | GGG  |
|  5 | HHH  |
|  6 | XXX  |
+----+------+
6 rows in set (0.00 sec)

mysql> select * from t2;
+------+------+
| name | age  |
+------+------+
| AAA  |   12 |
| BBB  |   13 |
| CCC  |   15 |
| DDD  |   12 |
| EEE  |   13 |
| FFF  |   15 |
+------+------+
6 rows in set (0.00 sec)

mysql> select * from t1 join t2 where t1.name = t2.name;
+----+------+------+------+
| id | name | name | age  |
+----+------+------+------+
|  1 | AAA  | AAA  |   12 |
|  2 | BBB  | BBB  |   13 |
|  3 | CCC  | CCC  |   15 |
+----+------+------+------+
3 rows in set (0.00 sec)

```

#### 外连接

> 结果：不仅包含符合查询条件和连接条件的数据，而且还包含左表，右表，或两个表中的所有数据。
> 语法格式：select 查询内容 from 表1 left|right join 表2 on 表1.关系字段 = 表2.关系字段 where 条件；
> 左连接：返回结果为左表中的所有记录和右表中符合条件的记录
> 右连接：返回结果为右表中的所有记录和左表中符合条件的记录

**左连接**

> 左连接：如果左表中的某条记录在右表中不存在，则为空
> mysql> select con.name, com.name from con left join com on con.id = com.id;

```mysql
mysql> select t1.name, t2.name from t1 left join t2 on t1.name = t2.name;
+------+------+
| name | name |
+------+------+
| AAA  | AAA  |
| BBB  | BBB  |
| CCC  | CCC  |
| GGG  | NULL |
| HHH  | NULL |
| XXX  | NULL |
+------+------+
6 rows in set (0.00 sec)
```

**右连接**

> 右连接：如果右表中的某条记录在左表中不存在，则为空
> mysql> select con.name, com.name from con right join com on con.id = com.id;

```mysql
mysql> select t1.name, t2.name from t1 right join t2 on t1.name = t2.name;
+------+------+
| name | name |
+------+------+
| AAA  | AAA  |
| BBB  | BBB  |
| CCC  | CCC  |
| NULL | DDD  |
| NULL | EEE  |
| NULL | FFF  |
+------+------+
6 rows in set (0.00 sec)
```

#### 子查询

> **一个查询语句嵌套在另一个查询语句内部**

**where 型子查询:内层sql的返回值在where后作为条件表达式的一部分**

```mysql
mysql> select * from t2 where name = (select name from t1 where id = 2);
+------+------+
| name | age  |
+------+------+
| BBB  |   13 |
+------+------+
1 row in set (0.00 sec)
```

**from 型子查询:内层sql查询结果,作为一张表,供外层的sql语句再次查询**

```mysql
mysql> select * from (select * from t1 where id > 3) as temp where name = 'HHH';
+----+------+
| id | name |
+----+------+
|  5 | HHH  |
+----+------+
1 row in set (0.00 sec)
```

**带in关键字的子查询**

> 将内层查询语句返回值与外层查询语句进行比较操作
> 语法结构：select 查询内容 from表1 where did [not] in (select did from 表2 where 限制条件)

```mysql
mysql> select * from t1 where name in (select name from t2);
+----+------+
| id | name |
+----+------+
|  1 | AAA  |
|  2 | BBB  |
|  3 | CCC  |
+----+------+
3 rows in set (0.00 sec)
```

**带exists关键字的子查询**

> exists关健字后可以是任意一个子查询，这个子查询的作用相当于测试，它不产生任何数据，只返回true或者false
>
> 只有当返回值为true时，外层查询才会执行。
>
> 语法结构：select 查询内容 from表1 where exists (select did from 表2 where 限制条件) 

```mysql
mysql> select * from t1 where exists (select * from t2 where age < 15);
+----+------+
| id | name |
+----+------+
|  1 | AAA  |
|  2 | BBB  |
|  3 | CCC  |
|  4 | GGG  |
|  5 | HHH  |
|  6 | XXX  |
+----+------+
6 rows in set (0.00 sec)

mysql> select * from t1 where exists (select * from t2 where age < 10);
Empty set (0.00 sec)
```

**带any的关键字子查询**

> any关键字表示满足其中的任意一个条件，它允许创建一个表达式对子查询的返回值列表进行比较，只要满足内层子查询的任意一个比较条件，就返回一个结果作为外层查询条件。
> 语法结构：select 查询内容 from表1 where 字段1 关系运算符 any(select 字段1 from 表2)

```mysql
mysql> select * from t1 where name = any(select name from t2);
+----+------+
| id | name |
+----+------+
|  1 | AAA  |
|  2 | BBB  |
|  3 | CCC  |
+----+------+
3 rows in set (0.00 sec)
```

**带all关键字的子查询**

> all关键字与any关键字类似，但all关键字表示需要满足所有子查询返回结果
> 语法结构：select 查询内容 from表1 where 字段1 关系运算符 all(select 字段1 from 表2)

```mysql
mysql> select * from student;
+-----------+----+------+----------+
| name      | id | age  | addr     |
+-----------+----+------+----------+
| tom       |  1 |   89 | shanghai |
| bers      |  2 |   16 | Beij     |
| haha      |  3 |   16 | Janpan   |
| xiaoming  |  6 |   13 | xinjiang |
| xiaohuang |  7 |   15 | tianjin  |
| dalin     |  8 |   18 | xiyu     |
| liai      |  9 |   17 | hubei    |
| Chaera    | 10 |   23 | Nanjin   |
+-----------+----+------+----------+
8 rows in set (0.00 sec)

mysql> select * from student where age > all(select age from t2);
+--------+----+------+----------+
| name   | id | age  | addr     |
+--------+----+------+----------+
| tom    |  1 |   89 | shanghai |
| bers   |  2 |   16 | Beij     |
| haha   |  3 |   16 | Janpan   |
| dalin  |  8 |   18 | xiyu     |
| liai   |  9 |   17 | hubei    |
| Chaera | 10 |   23 | Nanjin   |
+--------+----+------+----------+
6 rows in set (0.00 sec)
```

#### union查询

> 显示所有子查询结果的并集，一般不在union查询的子查询中使用order by

```mysql
mysql> select * from t1 union select * from t2;
+------+------+
| id   | name |
+------+------+
| 1    | AAA  |
| 2    | BBB  |
| 3    | CCC  |
| 4    | GGG  |
| 5    | HHH  |
| 6    | XXX  |
| AAA  | 12   |
| BBB  | 13   |
| CCC  | 15   |
| DDD  | 12   |
| EEE  | 13   |
| FFF  | 15   |
+------+------+
12 rows in set (0.00 sec)
```
