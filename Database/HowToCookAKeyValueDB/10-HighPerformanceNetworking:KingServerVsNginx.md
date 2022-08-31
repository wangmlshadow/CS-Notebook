# Implementing a Key-Value Store – Part 10: High-Performance Networking: KingServer vs. Nginx

This is Part 10 of the IKVS series, “Implementing a Key-Value Store”. You can also check the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) for other parts. In this series of articles, I describe the research and process through which I am implementing a key-value database, which I have named “KingDB”. The source code is available at [http://kingdb.org](http://kingdb.org/). Please note that you do not need to read the previous parts to be able to follow what is going on here. The previous parts were mostly exploratory, and starting with [Part 8](http://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/) is perfectly fine.

In this article, I explain the model and inner workings of KingServer, the network server for KingDB. In order to put things into perspective I also cover Nginx, the high-performance HTTP server well-known for its network stack, and how it differs from KingDB.

> ‎这是 IKVS 系列的第 10 部分“实现键值存储”。您还可以在‎[‎目录中‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎查看其他部分。在本系列文章中，我将介绍实现键值数据库的研究和过程，并将其命名为“KingDB”。源代码可在 ‎[‎http://kingdb.org‎](http://kingdb.org/)‎ 获得。请注意，您无需阅读前面的部分即可了解此处的情况。前面的部分大多是探索性的，从‎[‎第8部分‎](http://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/)‎开始是完全可以的。‎
>
> ‎在本文中，我将解释KingDb的网络服务器KingServer的模型和内部工作原理。为了更好的介绍KingServer，我还介绍了Nginx，这是一款以其网络堆栈而闻名的高性能HTTP服务器，以及它与KingDB的不同之处。‎

## TL;DR

If you remember just a few key elements from this article for designing high-performance network programs, they should be the following:

* Design your network protocol with care. When in doubt, copy existing time-tested protocols.
* Minimize the amount of CPU cycles wasted on blocking I/O.
* Keep the resource overhead for each connection as small as possible, so your design can scale sub-linearly with the number of connections.
* Look into the network stack of the Linux kernel and its different tuning parameters. This can lead to enormous and unexpected performance gains.

> 如果您只想了解本文中用于设计高性能网络程序的几个关键元素，它们如下所示：
>
> * ‎请谨慎设计您的网络协议。如有疑问，请复制经过时间考验的现有协议。‎
> * ‎最大限度地减少在阻塞 I/O 时浪费的 CPU 周期量。‎
> * ‎保持每个连接的资源开销尽可能小，以便您的设计可以随连接数进行亚线性扩展。‎
> * ‎查看 Linux 内核的网络堆栈及其不同的调优参数。这可以带来巨大且意想不到的性能提升。‎

## 1. Protocol and network clients

The protocol is an important part of any network program: it describes how the server talks to the clients, and poor decisions when designing a protocol can kill even the best architecture. For KingServer, I decided to implement the three basic operations of the Memcached protocol, `get`, `set`, and `delete`, in the exact way they were described in the Memcached repository [[6]](https://codecapsule.com/2016/07/21/implementing-a-key-value-store-part-10-high-performance-networking-kingserver-vs-nginx/#ref).

Using the Memcached protocol has some drawbacks. For example, the current version of KingDB has a multipart API to allow for large objects to be accessed while not requiring too much memory for the clients. However, KingServer is unable to offer that feature over the network as the Memcached protocol does not support multipart access. Indeed, Memcached was never designed to store very large blobs and therefore a multipart API was never needed.

But implementing the Memcached protocol also has advantages. The most important one is that the Memcached protocol has been around for a while and has stood the test of time. Therefore I save myself the labor of designing a new protocol which would inevitably require a lot of work and production testing before all friction points would be ironed out.

Another advantage of implementing the Memcached protocol is that the users of KingServer would be able to use all of the already existing Memcached client librairies in the programming languages they preferred. Thus essentially, I am leveraging the Memcached ecosystem and I save myself from having to implement and support client libraries in various programming languages.

My long-term plan was that if KingDB is to be succesful and used at a larger scale, then it would make sense to gather the most common use cases for using KingDB over the network through KingServer, and then and only then, create a protocol optimized for those use cases.

> 协议是任何网络程序的重要组成部分：它描述了服务器如何与客户端通信，在设计协议时做出的错误决策甚至会扼杀最好的架构。对于 KingServer，我决定实现 Memcached 协议的三个基本操作 get，set 和 delete，就像它们在 Memcached 存储库‎‎中描述的方式一样 [6]‎‎。
>
> ‎使用Memcached协议有一些缺点。例如，当前版本的 KingDB 具有多部分 API，允许访问大型对象，同时不需要客户端过多的内存。但是，KingServer无法通过网络提供该功能，因为Memcached协议不支持多部分访问。事实上，Memcached从未设计用于存储非常大的blob，因此从不需要多部分API。‎
>
> ‎但实现Memcached协议也有优势。最重要的是，Memcached协议已经存在了一段时间，并且经受住了时间的考验。因此，我省去了设计新协议的劳动，在消除所有冲突点之前，这不可避免地需要大量的工作和生产测试。
>
> 实现Memcached协议的另一个优点是，KingServer的用户将能够以他们喜欢的编程语言使用所有已经存在的Memcached客户端语言。因此，从本质上讲，我正在利用Memcached生态系统，并且我不必以各种编程语言实现和支持客户端库。‎
>
> ‎我的长期计划是，如果KingDB要成功并在更大的范围内使用，那么通过KingServer收集通过网络使用KingDB的最常见用例是有意义的，然后并且只有这样，才能创建一个针对这些用例进行优化的协议。‎

## 2. Blocking I/O vs non-blocking I/O

In this section I want to briefly compare blocking I/O and non-blocking I/O.

With a blocking I/O approach, calling a blocking syscall would make the caller thread idle and waste CPU cycles until the syscall returns. With a non-blocking I/O approach, the syscall would return right away after it is called, and the caller thread would be notified of incoming I/O operations through an event loop. For that reason, the non-blocking I/O approach is also called asynchronous or event-driven. Between the notification events, the thread is free to perform anything it wants, which means no CPU cycles are lost idling.

The original non-blocking syscalls were select() and poll() and were first introduced in the 1980’s. They become slow under load, due to their inefficient approach for managing many file descriptors. Modern non-blocking applications use epoll() and kqueue() through libraries such as libevent, libev, or libuv.

The rule of thumb is that non-blocking I/O is better, because it avoid wasting CPU cycles by design. However when using a single-threaded non-blocking approach with an event loop, you do have to be careful of what you do with the connection you’re handling at any given iteration of that loop. If you take too long to deal with the chunk of data that this connection needs handled, you run into the risk of making all the other connections assigned to this thread time out.

> ‎在本节中，我想简要比较阻塞 I/O 和非阻塞 I/O。‎
>
> ‎使用阻塞 I/O 方法时，调用阻塞系统调用将使调用方线程空闲并浪费 CPU 周期，直到系统调用返回。使用非阻塞 I/O 方法，系统调用将在调用后立即返回，并且调用方线程将通过事件循环收到传入 I/O 操作的通知。因此，非阻塞 I/O 方法也称为异步或事件驱动。在通知事件之间，线程可以自由地执行它想要的任何事情，这意味着没有CPU周期丢失空闲。‎
>
> 最初的非阻塞系统调用是select（）和poll（），并在1980年代首次引入。它们在大负载下变得缓慢，因为它们管理许多文件描述符的方法效率低下。现代非阻塞应用程序通过 libevent、libev 或 libuv 等库使用 epoll（） 和 kqueue（）。
>
> ‎经验法则是非阻塞 I/O 更好，因为它在设计上避免了浪费 CPU 周期。但是，当对事件循环使用单线程非阻塞方法时，您必须小心在该循环的任何给定迭代中对正在处理的连接执行的操作。如果处理此连接需要处理的数据块花费的时间太长，则可能会使分配给此线程的所有其他连接超时。‎

## 3. KingServer and the blocking thread-per-connection model

KingServer implements the classic blocking thread-per-connection approach, in which each new connection is handled by a dedicated thread using blocking network I/O. I envisioned using a non-blocking event loop with whatever flavor of epoll() or kqueue() through libev/libuv, which is often regarded as offering better performance as stated in the previous section, but in the end I decided otherwise.

The reason why I went for the thread-per-connection model is simply that KingDB is compressing data and calculating checksums, which can take some time, and I wasn’t entirely sure that this would not cause timeout issues as explained in the previous section. I knew that the thread-per-connection would work, with some known limitations, and as I was looking at getting a first version of KingServer out as quickly as possible, I settled for an inferior yet simpler design first.

For that first version of KingServer, I tried to keep things as simple as possible. KingServer uses a pool of threads created once and for all at start-up, which has the benefit of preventing the overhead of spawning a new thread with every incoming connection. A specialized thread, the receiver thread, listens for incoming connections and dispatches them to a queue.

The available worker threads are monitoring that queue for incoming connections using a C++11 condition_variable. When a connection is picked by a worker, that worker takes care of copying the data from the recv() buffer from kernel space into user space, and does the CPU-intensive tasks of compressing data and calculating checksums.

Having one thread per connection ensures that for example if a connection has to handle a large entry from a client and takes more time, other clients will not timeout as they can be handled by other cores. Figure 10.1 below represents the architecture of KingServer 0.9.0.

KingServer has the following specialized threads:

* **main thread** : keeps the KingServer program, the receiver thread and the worker thread pool running.
* **network receiver** : receives requests from all clients on the network and dispatches them to the worker threads through the queue.
* **worker threads** : each thread in the pool is a worker that picks the next incoming request from the queue, and handles that request by calling the methods of the KingDB object embedded by KingServer. For `put` requests, those threads will also compress data and compute checksums.

> ‎KingServer 实现了经典的按连接阻塞线程的方法，其中每个新连接都由使用阻塞网络 I/O 的专用线程处理。我设想通过libev/libuv使用一个非阻塞事件循环，其中包含任何风格的epoll（）或kqueue（），这通常被认为提供了上一节中所述的更好的性能，但最终我决定了相反的结果。‎
>
> ‎我选择每个连接线程模型的原因很简单，因为KingDB正在压缩数据并计算校验和，这可能需要一些时间，我不完全确定这不会导致超时问题，如上一节所述。我知道每个连接的线程可以工作，但有一些已知的限制，当我考虑尽快推出KingServer的第一个版本时，我首先选择了一个劣质但更简单的设计。
>
> ‎对于KingServer的第一个版本，我试图让事情尽可能简单。KingServer 使用在启动时一劳永逸地创建的线程池，其优点是可以防止每次传入连接生成新线程的开销。专用线程（接收器线程）侦听传入连接并将其分派到队列。‎
>
> ‎可用的工作线程正在使用 C++11 condition_variable监视该队列中的传入连接。当工作线程选取连接时，该工作线程负责将数据从 recv（） 缓冲区从内核空间复制到用户空间，并执行压缩数据和计算校验和的 CPU 密集型任务。
>
> ‎例如，如果一个连接必须处理来自客户端的大型条目并花费更多时间，则其他客户端不会超时，因为它们可以由其他内核处理。下面的图 10.1 显示了 KingServer 0.9.0 的体系结构。‎
>
> ‎KingServer具有以下专用线程：‎
>
> * **‎主线程‎**‎：保持 KingServer 程序、接收方线程和工作线程池运行。‎
> * **‎网络接收线程‎**‎：接收来自网络上所有客户端的请求，并通过队列将其分派给工作线程。‎
> * ‎工作线程‎‎：池中的每个线程都是一个工作线程，它从队列中选取下一个传入请求，并通过调用 KingServer 嵌入的 KingDB 对象的方法来处理该请求。对于 `put`请求，这些线程还将压缩数据并计算校验和。‎

![Architecture-of-KingServer](http://codecapsule.com/wp-content/uploads/2016/06/kingserver-architecture.svg)

Figure 10.1: Architecture of KingServer 0.9.0

When a process blocks on an I/O syscall, it loses the opportunity to use the CPU cycles of the core it runs on until the syscall returns. But those cycles are not completely lost, as the OS scheduler will detect the inactivity and will schedule another process to run on that core in the mean time.

Due to the design I chose for KingServer, a lot of CPU time in each thread is spent blocking on network I/O. In order to cope with that, KingServer will by default start with a pool of 150 worker threads, which can be adjusted using a parameter.

At the time I am writing this article, the CPU of a commodity server has an average of 12 or 24 cores. Having 150 worker threads means that there will be more threads than cores in the CPU. One upside of having so many threads is that many connections can be handled simultaneously, as active threads can be scheduled by the OS while other threads are idle and blocking on I/O. However, using so many threads also has a major drawback, which is that CPU cycles will be lost because of lock contention and context switching between threads, also known as thread thrashing.

My choice of the thread-per-connection model was motivated by my desire to release something as quickly as possible, and not by an intent to optimize for performance. The model I have used does the job and supports some level o concurrency, but is definitely not the best. Thus I think it would be only fair and relevant for this article if I covered a network model that allows for even higher networking performance, the Nginx network model.

> 当进程在 I/O 系统调用上阻塞时，它将失去使用其运行的核心的 CPU 周期的机会，直到系统调用返回。但这些周期并没有完全丢失，因为操作系统调度程序将检测不活动状态，并将同时安排另一个进程在该内核上运行。‎
>
> ‎由于我为KingServer选择的设计，每个线程中的大量CPU时间都花在了网络I / O上。为了解决这个问题，默认情况下，KingServer将从150个工作线程的池开始，这些线程可以使用参数进行调整。‎
>
> ‎在我写这篇文章的时候，商用服务器的CPU平均有12或24个内核。拥有 150 个工作线程意味着 CPU 中的线程数将多于内核数。拥有这么多线程的一个好处是可以同时处理许多连接，因为活动线程可以由操作系统调度，而其他线程处于空闲状态并在I / O上阻塞。但是，使用如此多的线程也有一个主要缺点，即 CPU 周期会因为锁争用和线程之间的上下文切换而丢失，也称为线程抖动。‎
>
> ‎我选择每个连接线程模型的动机是我希望尽快发布某些内容，而不是出于优化性能的意图。我使用的模型可以完成这项工作，并支持某种级别的o并发，但绝对不是最好的。因此，我认为只有我介绍一种允许更高网络性能的网络模型，即Nginx网络模型，它才与本文相关。‎

## 4. Nginx and its non-blocking model

Nginx is an open source high-performance HTTP server that does not use the classic blocking thread-per-connection. Instead, Nginx uses a non-blocking approach, which has proven to be remarkably efficient.

The first way by which Nginx is efficient is that it does not spawn a new process or thread for every incoming connection. On start-up, the Nginx master process forks a pool of worker processes, all of which are single-threaded. Because a single Nginx server can be configured to serve many websites at different address/port pairs, the master Nginx process creates one dedicated socket for each website it needs to serve. All the worker processes share those sockets, on which they listen and accept new requests: they inherit the sockets after the fork() from the master process. This means that all the requests to all the websites handled by a single Nginx server are distributed across all worker processes.

Balancing the requests across workers is left to the OS scheduling mechanism. By default, the `accept_mutex` Nginx parameter is set to `on`, which means that worker processes will accept new connections by turns, i.e. only one worker will accept new connections at a given time. If this parameter is set to `off`, all worker processes will be notified of new connections, which will make some workers waste CPU cycles if the number of connections is low [[1]](https://codecapsule.com/2016/07/21/implementing-a-key-value-store-part-10-high-performance-networking-kingserver-vs-nginx/#ref).

Since Nginx 1.9.1, the `reuseport` parameter makes it possible to use the `REUSE_PORT` socket option, which allows for multiple sockets to listen on the same address/port pair. This enables all the worker threads to have their own dedicated sockets instead of sharing sockets, which reduces lock contention around incoming requests and brings significant performance improvements [[5]](https://codecapsule.com/2016/07/21/implementing-a-key-value-store-part-10-high-performance-networking-kingserver-vs-nginx/#ref).

Each worker has its own run-loop which takes new connections and handles requests from connections already open for that process. The run-loop is non-blocking and event-driven. The master process creates, closes and binds sockets, and the worker processes accept, handle and process requests asynchronously. Figure 10.2 below was taken from the excellent article by Andrew Alexeev about the architecture of Nginx [[2]](https://codecapsule.com/2016/07/21/implementing-a-key-value-store-part-10-high-performance-networking-kingserver-vs-nginx/#ref), and illustrates how the master and worker processes interact.

Memory usage is kept under control because the number of processes and threads is limited by design and the overhead of each additional requests is small. Consequently, Nginx scales sub-linearly with the number of incoming requests. CPU usage is also optimal, as no cycles are wasted on process spawning or thread context switch and the I/O is non-blocking. Keeping a single worker per core allows for good CPU utilization while avoiding thread thrashing and lock contention.

The architecture and internals of Nginx are described in great details in articles from some of the authors themselves [[2, 3, 4]](https://codecapsule.com/2016/07/21/implementing-a-key-value-store-part-10-high-performance-networking-kingserver-vs-nginx/#ref)

> ‎Nginx是一个开源的高性能HTTP服务器，它不使用经典的阻塞线程每个连接。相反，Nginx使用非阻塞方法，这已被证明是非常有效的。‎
>
> ‎Nginx高效的第一种方法是它不会为每个传入连接生成新的进程或线程。在启动时，Nginx主进程分叉了一个工作进程池，所有这些进程都是单线程的。由于单个Nginx服务器可以配置为以不同的地址/端口对为许多网站提供服务，因此主Nginx进程为其需要服务的每个网站创建一个专用套接字。所有工作进程共享这些套接字，它们在其上侦听并接受新请求：它们从主进程继承 fork（） 之后的套接字。这意味着由单个Nginx服务器处理的所有网站的所有请求都分布在所有工作进程中。
>
> ‎在工作线程之间平衡请求由操作系统调度机制决定。默认情况下，Nginx 参数 `accept_mutex`设置为 `on` ，这意味着工作进程将轮流接受新连接，即在给定时间只有一个工作进程将接受新连接。如果将此参数设置为 `off` ，则所有工作进程都将收到新连接的通知，如果连接数较低 ‎‎[1]‎‎，这将使一些工作进程浪费 CPU 周期。‎
>
> ‎从Nginx 1.9.1开始，该参数 `reuseport`使得可以使用套接字选项，该选项 `REUSE_PORT`允许多个套接字侦听同一地址/端口对。这使得所有工作线程都有自己的专用套接字，而不是共享套接字，从而减少了围绕传入请求的锁争用，并带来了显着的性能改进‎‎[5]‎‎。‎
>
> ‎每个工作线程都有自己的运行循环，该循环采用新连接并处理来自已为该进程打开的连接的请求。运行循环是非阻塞和事件驱动的。主进程创建、关闭和绑定套接字，工作进程异步接受、处理和处理请求。下面的图 10.2 取自 Andrew Alexeev 关于 Nginx ‎[‎[2]‎](https://codecapsule.com/2016/07/21/implementing-a-key-value-store-part-10-high-performance-networking-kingserver-vs-nginx/#ref)‎ 架构的优秀文章，并说明了主进程和工作进程如何交互。‎
>
> ‎内存使用率受到控制，因为进程和线程的数量受到设计的限制，并且每个附加请求的开销很小。因此，Nginx会随着传入请求的数量进行亚线性扩展。CPU 使用率也是最佳的，因为不会在进程生成或线程上下文切换上浪费任何周期，并且 I/O 是非阻塞的。每个内核保留一个工作线程可实现良好的 CPU 利用率，同时避免线程抖动和锁争用。‎
>
> ‎Nginx的架构和内部结构在一些作者自己的文章中进行了非常详细的描述‎[‎[2，3，4]‎](https://codecapsule.com/2016/07/21/implementing-a-key-value-store-part-10-high-performance-networking-kingserver-vs-nginx/#ref)

![architecture-nginx](https://i0.wp.com/codecapsule.com/wp-content/uploads/2016/06/architecture-nginx.png)

Figure 10.2: Architecture of Nginx, taken from Andrew Alexeev’s article [[2]](https://codecapsule.com/2016/07/21/implementing-a-key-value-store-part-10-high-performance-networking-kingserver-vs-nginx/#ref)

## Conclusion

In this article, I briefly explained the difference between blocking and non-blocking I/O, and I presented two network models, the blocking thread-per-connection model used by KingServer, which I compared to the non-blocking event-driven model of Nginx.

The non-blocking model does have advantages over the blocking one, however one has to be careful with the non-blocking model that the each connection of the event loop are treated as fast as possible to prevent the other already opened connections for this event loop from timing out.

Finally, one thing to keep in mind when designing network programs, but which I have not covered here, is that the Linux network stack is full of parameters and nobs to play with. Spending some time profiling different levels of the network stack and tuning those parameters can lead to enormous and unexpected performance gains, and you should always consider it.

In the next article, I will conclude the series of articles about KingDB, by summarizing what I have achieved, the mistakes I have made, and what I have learned in the process.

> ‎在本文中，我简要解释了阻塞和非阻塞I / O之间的区别，并介绍了两种网络模型，即KingServer使用的阻塞线程每连接模型，我将其与Nginx的非阻塞事件驱动模型进行了比较。‎
>
> 与阻塞模型相比，非阻塞模型确实具有优势，但是必须小心非阻塞模型，即尽可能快地处理事件循环的每个连接，以防止此事件循环的其他已打开的连接超时。‎
>
> ‎最后，在设计网络程序时要记住的一件事，但我在这里没有介绍，那就是Linux网络堆栈充满了参数和nobs。花一些时间分析不同级别的网络堆栈并调整这些参数可能会导致巨大的和意想不到的性能提升，您应该始终考虑这一点。‎
>
> 在下一篇文章中，我将总结一下关于KingDB的系列文章，总结我所取得的成就，我所犯的错误，以及我在这个过程中学到的东西。‎

## References

[1] [http://nginx.org/en/docs/ngx_core_module.html#accept_mutex](http://nginx.org/en/docs/ngx_core_module.html#accept_mutex)
[2] [http://www.aosabook.org/en/nginx.html](http://www.aosabook.org/en/nginx.html)
[3] [https://www.nginx.com/blog/inside-nginx-how-we-designed-for-performance-scale/](https://www.nginx.com/blog/inside-nginx-how-we-designed-for-performance-scale/)
[4] [https://www.nginx.com/resources/library/infographic-inside-nginx/](https://www.nginx.com/resources/library/infographic-inside-nginx/)
[5] [https://www.nginx.com/blog/socket-sharding-nginx-release-1-9-1/](https://www.nginx.com/blog/socket-sharding-nginx-release-1-9-1/)
[6] [https://github.com/memcached/memcached/blob/master/doc/protocol.txt](https://github.com/memcached/memcached/blob/master/doc/protocol.txt)
