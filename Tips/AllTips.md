# 问题整理
## C++
### 基础
#### 什么类不能被继承
- 将构造函数、析构函数定义为private
```c++
class MakeFinal {
       friend class FinalClass;
private:
       int a;
       MakeFinal() {}
       ~MakeFinal() {}
};
 
 
//FinalClass是一个不能被继承的类，同时，可以在
//堆和栈上产生该类的实例
class FinalClass: virtual public MakeFinal {
public:
       void test() {cout << a << endl;}
       FinalClass() {}
       ~FinalClass() {}
};
 
 
//这里尝试继承FinalClass的行为会引发编译错误:
//FinalClass不能被继承
class Try: public FinalClass {
public:
      Try() {}
      ~Try() {}
};
 
 
int main() {
       FinalClass *f1;
       FinalClass obj;
}
```
- 是用final关键字，在类名称后面使用关键字final，继承该类会导致编译错误。
```c++
class Super final
{
  //......
};
```
### 智能指针
#### new和make_share
> https://www.jianshu.com/p/03eea8262c11
## Git
#### Git&&Game
https://learngitbranching.js.org/?locale=zh_CN
#### Git发生冲突的情形以及解决方式
- 在分支合并时，多个分支修改了**同一个文件**
- git pull --rebase
## 网络
#### 网络的字节序
> https://blog.csdn.net/JMW1407/article/details/108637540
- Little endian：将低序字节存储在起始地址
- Big endian：将高序字节存储在起始地址
- ![image](https://user-images.githubusercontent.com/29187126/162908980-76462c2e-4f6e-499b-8168-a4c25cf2ae2a.png)
- 判断方式：
```c++
/* 确定你的电脑是大端字节序还是小端字节序 */
#include <stdio.h>

int check1()
{
	int i = 1; //1在内存中的表示： 0x00000001
	char *pi = (char *)&i; //将int型的地址强制转换为char型
	return *pi == 0; //如果读取到的第一个字节为1，则为小端法，为0，则为大端法
}

int main()
{
	if (check1() == 1)
		printf("big\n");
	else
		printf("little\n");

	return 0;
}

第二种方法，我们用联合结构解决，其本质差异不大
/* 确定你的电脑是大端字节序还是小端字节序 */
#include <stdio.h>

int check2()
{
	union test {
		char ch;
		int i;
	}test0;
	test0.i = 1;
	return test0.ch == 0;
}
int main()
{
	if (check1() == 1)
		printf("big\n");
	else
		printf("little\n");

	return 0;
}
```
#### 四次挥手时最后的ACK
> https://blog.csdn.net/u014520797/article/details/118371896
- 当第四步的A发送的确认报文，B收到时，A会等待2MSL的时间后，连接彻底关闭。(因为B收到了，所以2MSL时间内B不会重发第三步的释放报文)
- 当第四步的A发送的确认报文，B没有收到时，B会继续发送第三步的释放报文，A收到后会继续发送第四步的确认报文（此时会重新启动2MSL计时器，重新等待2MSL时间），若在接下来的2MSL的时间内未收到B发送的第三步的释放报文，则意味着B已经收到了A的ack确认报文，连接彻底关闭
- ![image](https://user-images.githubusercontent.com/29187126/162910216-1af5758a-6602-4acc-bc39-8b5aece02557.png)
#### 为什么是四次挥手而不是三次
> https://www.zhihu.com/question/63264012
- 服务端通常需要等待完成数据的发送和处理，所以服务端的 ACK 和 FIN 一般都会分开发送，从而比三次握手导致多了一次
#### 为什么等待2MSL
> https://www.zhihu.com/question/67013338
- TIME_WAIT至少需要持续2MSL时长，这2个MSL中的第一个MSL是为了等自己发出去的最后一个ACK从网络中消失，而第二MSL是为了等在对端收到ACK之前的一刹那可能重传的FIN报文从网络中消失。
#### 如何设计一个可靠的UDP
> https://www.infoq.cn/article/how-to-make-udp-reliable
#### TCP沾包、半包问题
> https://zhuanlan.zhihu.com/p/126279630
#### Keep-Alive
> https://bbs.huaweicloud.com/blogs/285330
## 数据库
### MySQL
#### 添加索引
> https://blog.csdn.net/nangeali/article/details/73384780
#### 怎么设计一个索引
> https://segmentfault.com/a/1190000038921156
### Redis
#### 缓存雪崩、穿透、击穿
> https://segmentfault.com/a/1190000022029639
