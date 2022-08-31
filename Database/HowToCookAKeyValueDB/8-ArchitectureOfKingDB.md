# Implementing a Key-Value Store – Part 8: Architecture of KingDB


This is Part 8 of the IKVS series, “Implementing a Key-Value Store”. You can also check the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) for other parts. In this series of articles, I describe the research and process through which I am implementing a key-value database, which I have named “KingDB”. The source code is available at [http://kingdb.org](http://kingdb.org/). Please note that you do not need to read the previous parts to be able to follow. The previous parts were mostly exploratory, and starting with Part 8 is perfectly fine.

In the previous articles, I have laid out the research and discussion around what needs to be considered when implementing a new key-value store. In this article, I will present the architecture of KingDB, the key-value store of this article series that I have finally finished implementing.

> ‎这是 IKVS 系列的第 8 部分“实现键值存储”。您还可以在‎[‎目录中‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎查看其他部分。在本系列文章中，我将介绍实现键值数据库的研究和过程，并将其命名为“KingDB”。源代码可在 ‎[‎http://kingdb.org‎](http://kingdb.org/)‎ 获得。请注意，您无需阅读前面的部分即可阅读这部分的内容。前面的部分大多是探索性的，从第8部分开始阅读是完全可以的。
>
> ‎在之前的文章中，我围绕实现新的键值存储时需要考虑的事项进行了研究和讨论。在本文中，我将介绍 KingDB 的体系结构，这是我最终完成实现的本系列文章的键值存储。

## 1. Introduction

KingDB is a persistent key-value store. All data is written to disk to minimize data loss in case of system failure. Each entry that KingDB stores is a pair of the type <key, value>, where key and value are byte arrays. Keys can have a size up to (2^31 – 1) bytes, and values a size of up to 2^64 bytes.

KingDB can be used as an embedded database in another program, either by compiling the KingDB source code with the program, or by compiling KingDB as a library and linking to it. KingDB can also be used as a network database through KingServer, that embeds KingDB and exposes a network interface which network clients can access through the Memcached protocol [[1]](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref). Therefore, KingDB can be used through a network simply by using any of the already available Memcached libraries in any programming language. Note that currently, only a subset of the Memcached protocol is supported: the get, set, and delete commands.

Data is persisted to disk using log-structured storage, in which all writes and deletes are written to disk sequentially like in a log file. It is similar to Write-Ahead Logging (WAL), except that unlike with WAL where the logs have to be merged into the database at some point, log-structure storage only needs to compact old unused entries. The data format used by KingDB is the Hashed String Table, or HSTable. Each HSTable is just a flat file that contains entries, and at the end of that file, an Offset Array that contains the hashed keys and location of the entries in the file for rapid indexing.

I have used LevelDB as a model for KingDB, which is why if you are familiar with LevelDB, you will find similarities in the way that the code is organized in KingDB. LevelDB is a little marvel, and drawing inspiration from it has been for me a great way to learn and improve. If you haven’t had a chance to go through the source code of LevelDB, I recommend that you do so, it’s among the best source code I have ever read! The architecture and source code of LevelDB are just very clear and explicit. In addition, the authors of LevelDB have implemented in a concise manner all the components required for a medium-sized C++ project: error management, logging, unit-testing framework, benchmarking, etc., which is great if you want to avoid external dependencies and learn how to implement a minimal self-sustaining system. However the resemblance between KingDB and LevelDB stops there: all the internal algorithms and data structures in KingDB are new, and I implemented them from scratch.

> ‎KingDB 是一个持久的键值存储。所有数据都写入磁盘，以在系统发生故障时最大限度地减少数据丢失。KingDB 存储的每个条目都是一个键值对类型<键、值>，其中键和值是字节数组。键的大小最大为 （2^31 – 1） 字节，值的大小最大为 2^64 字节。
>
> ‎KingDB 可以用作另一个程序中的嵌入式数据库，方法是使用该程序编译 KingDB 源代码，或者将 KingDB 编译为库并链接到它。KingDB也可以通过KingServer用作网络数据库，它嵌入了KingDB并公开了网络客户端可以通过Memcached协议‎[‎[1]‎](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref)‎访问的网络接口。因此，KingDB可以通过网络使用，只需使用任何编程语言中任何可用的Memcached库即可。请注意，目前仅支持 Memcached 协议的一个子集：获取、设置和删除命令。
>
> 使用日志结构存储将数据保存到磁盘，其中所有写入和删除操作都按顺序写入磁盘，就像在日志文件中一样。它类似于预写日志记录 （WAL），不同之处在于，与必须在某个时候将日志合并到数据库中的 WAL 不同，日志结构存储只需要压缩旧的未使用的条目。KingDB 使用的数据格式是 Hashed String Table 或 HSTable。每个 HSTable 只是一个包含条目的平面文件，在该文件的末尾是一个偏移数组，其中包含用于快速索引的哈希键和条目的位置。
>
> ‎我使用LevelDB作为KingDB的模型，这就是为什么如果你熟悉LevelDB，你会发现在KingDB中代码的组织方式有相似之处。LevelDB是一个小小的奇迹，从中汲取灵感对我来说是学习和改进的好方法。如果您还没有机会浏览LevelDB的源代码，我建议您这样做，它是我读过的最好的源代码之一！LevelDB的架构和源代码非常清晰和明确。此外，LevelDB的作者以简洁的方式实现了中型C++项目所需的所有组件：错误管理，日志记录，单元测试框架，基准测试等，如果您想避免外部依赖并学习如何实现最小的自我维持系统，这非常有用。然而，KingDB和LevelDB之间的相似之处就此为止：KingDB中的所有内部算法和数据结构都是新的，我从头开始实现它们。‎

## 2. Database, Snapshots and Iterators

KingDB’s default interface is the Database class, which offers three basic operations: Get(), Put(), and Delete().

KingDB also offers read-only iterators that can iterate over all the entries stored in a database. Because KingDB uses a hash table to store the locations of entries, the iterators will not return entries in order of keys, but unordered, in the sequence with which they have been written to disk.

Another way to access a KingDB database is to do it through a read-only snapshot. The Snapshot class allows all the concurrent random operations that the regular Database class does, and it guarantees a consistent view of the data.

> KingDB 的默认接口是 Database 类，它提供了三个基本操作：Get（）、Put（） 和 Delete（）。‎
>
> ‎KingDB 还提供只读迭代器，可以迭代存储在数据库中的所有条目。由于 KingDB 使用哈希表来存储条目的位置，因此迭代器不会按照写入磁盘的顺序按键顺序返回条目，而是无序返回条目。‎
>
> 访问 KingDB 数据库的另一种方法是通过只读快照进行访问。Snapshot 类允许常规 Database 类执行的所有并发随机操作，并保证数据的一致视图。‎

## 3. Architectural overview

Figure 8.1 below is an architectural overview of KingDB. All layers are represented, from the client application at the top, down to the storage engine and file system at the bottom. Step-by-step explanations of the read and write processes are also represented on the diagram using numbered circles. They are enough to get a good understanding as to how the data is flowing, and how the main components and threads are interacting inside KingDB. In the remaining sections of this article, I give more details regarding the architectural choices and the role of all the components in this architecture.

> ‎下面的图 8.1 是 KingDB 的架构概述。表示所有层，从顶部的客户端应用程序到底部的存储引擎和文件系统。读写过程的分步说明也使用编号圆圈在图中表示。它们足以很好地理解数据是如何流动的，以及主要组件和线程如何在 KingDB 内部交互。在本文的其余部分中，我将提供有关体系结构选择以及此体系结构中所有组件的作用的更多详细信息。‎

![Architecture-of-KingDB-web](http://codecapsule.com/wp-content/uploads/2015/05/Architecture-of-KingDB-vecto.svg)

Figure 8.1: Architecture of KingDB v0.9.0

## 4. Index

The role of the index is to quickly find the location of an entry in the secondary storage using only the key of that entry. For this, KingDB uses a pair of 64-bit integers for each entry: the hashed key and the location:

* The **hashed key** of an entry is a 64-bit integer computed by passing the key of that entry through a selected hash function. Because hashing collisions exist, entries with different keys can have the same hashed key.
* The **location** of an entry is a unique 64-bit integer: the first 32 bits are pointing to the HSTable, i.e. path of the file on disk, and the last 32 bits are pointing to the offset in the HSTable file where the entry can be found.

KingDB uses a std::multimap as index, and for each hashed key associates all the locations where entries having that hashed key can be found. When the location of a new entry is stored, the hashed key for that entry is used as the key for the std::multimap, and the location is inserted in the bucket for that hashed key. KingDB adds locations as they are incoming, thus if an entry has multiple versions, i.e. a single entry was written multiple times, each time with the same key but with a different value, then the latest version will be added last to the std::multimap. When the location of an entry is retrieved, its hashed key is computed and used to find the list of locations where entries sharing that hashed key can be found. The locations are then considered in reverse order, which guarantees that the latest version of the retrieved entry will always be found first, and the older versions are simply ignored [[2]](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref). This is also one of the reasons why the writes are fast: in case an entry has multiple versions, all versions are stored in the index and on disk. Same goes with the deletes: nothing is really deleted, KingDB only registers the deletion orders and hides the deleted entries from the user. The compaction process later takes care, in the background, of removing old versions and deleted entries from the file storage and from the index.

Accesses to the index are done in the form of multiple readers, single writer, using a single lock to ensure that the index is not modified while it is being read. Because all updates to the index are happening through the Storage Engine, they are grouped and applied by small batches within short time intervals, which minimizes the delay during which the index needs to be locked and any chance of blocking the reads while the index is updated.

Using a std::multimap has been one of the most important design decisions that I took early on for KingDB. I considered implementing my own lock-free version of Robin Hood hashing and use it for the index of KingDB, but it really felt like premature optimization. So I decided that for the first version of KingDB, I would use a container from the standard library and see how far it could take me. I am happy that I used std::multimap because it saved time and enabled me to focus on the on-disk data format, however it did introduce some drawbacks:

1. Data is hashed twice: the first time when the *hashed key* is computed, and the second time when the std::multimap computes another hash from the hashed key, for its own internal representation. Hashing data is a CPU-costly operation.
2. The entire index needs to be stored in memory at all time. This is fine if the database is small, but this can be several GBs of data if the database grows.
3. This makes snapshots costly. Indeed, snapshots require their own index to guarantee that they can offer a read-only consistent view of a database. So each time a snapshot is created, the entire index has to be copied and stored in an extra memory space. For large databases, this means several GBs of data duplicated in the memory.

It is clear that the only way to improve the design and the overall database performance at this stage is to implement a custom data structure that will serve as the index for KingDB. Ideally, this index will be stored on disk so it won’t take too much memory while it could still be loaded using read-only memory maps, and another good feature for this index would be to be in multiple parts, so that snapshots wouldn’t need to duplicate the entire index and could only lock the parts of the index that they need.

> ‎索引的作用是仅使用某个条目的键在辅助存储中快速查找该条目的位置。为此，KingDB 为每个条目使用一对 64 位整数：散列键和位置：‎
>
> - 条目‎**‎的哈希键‎**‎是一个 64 位整数，通过所选的哈希函数传递该条目的键来计算。由于存在哈希冲突，因此具有不同键的条目可以具有相同的哈希键。‎
> - 条目‎**‎的位置‎**‎是唯一的 64 位整数：前 32 位指向 HSTable，即磁盘上文件的路径，最后 32 位指向 HSTable 文件中的偏移量，可以在其中找到该条目。
>
> ‎KingDB 使用 std：：multimap 作为索引，对于每个散列键，可以关联所有可以找到具有该散列键的条目的位置。存储新条目的位置时，该条目的哈希键将用作 std：：multimap 的键，并且该位置将插入到该哈希键的存储桶中。KingDB 在传入位置时添加位置，因此，如果一个条目有多个版本，即多次写入单个条目，每次都使用相同的键但具有不同的值，则最新版本将最后添加到 std：：multimap 中。检索条目的位置时，将计算其哈希键，并用于查找可在其中找到共享该哈希键的条目的位置列表。然后按相反的顺序考虑位置，这保证了始终首先找到检索到的条目的最新版本，而旧版本则被忽略 ‎[‎[2]‎](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref)‎。这也是写入速度快的原因之一：如果一个条目有多个版本，则所有版本都存储在索引和磁盘上。删除也是如此：没有真正删除任何内容，KingDB仅注册删除订单并对用户隐藏已删除的条目。稍后，压缩过程会在后台处理从文件存储和索引中删除旧版本和已删除的条目。‎
>
> ‎对索引的访问以多个读取器、单个写入器的形式完成，使用单个锁来确保在读取索引时不被修改。由于索引的所有更新都是通过存储引擎进行的，因此它们在较短的时间间隔内按小批量分组和应用，从而最大限度地减少了需要锁定索引的延迟，以及在更新索引时阻止读取的任何机会。‎
>
> ‎使用std：：multimap是我早期为KingDB做出的最重要的设计决策之一。我考虑过实现我自己的无锁版本的罗宾汉哈希，并将其用于KingDB的索引，但这真的感觉像是过早的优化。所以我决定，对于KingDB的第一个版本，我将使用标准库中的容器，看看它能带我走多远。我很高兴我使用了std：：multimap，因为它节省了时间，使我能够专注于磁盘上的数据格式，但是它确实引入了一些缺点：‎
>
> 1. 数据被散列两次：第一次是在计算‎*‎散列键‎*‎时，第二次是当 std：：multimap 从散列键计算另一个散列时，用于其自己的内部表示。散列数据是一项 CPU 成本高昂的操作。
> 2. ‎整个索引需要始终存储在内存中。如果数据库很小，这很好，但如果数据库增长，这可能是几 GB 的数据。
> 3. 这使得快照成本高昂。实际上，快照需要自己的索引来保证它们可以提供数据库的只读一致视图。因此，每次创建快照时，都必须复制整个索引并将其存储在额外的内存空间中。对于大型数据库，这意味着内存中复制了几 GB 的数据。
>
> 很明显，现阶段改进设计和整体数据库性能的唯一方法是实现一个自定义数据结构，该结构将作为KingDB的索引。理想情况下，此索引将存储在磁盘上，因此在仍然可以使用只读内存映射加载它时不会占用太多内存，并且此索引的另一个好功能是位于多个部分中，因此快照不需要复制整个索引，而只能锁定它们需要的索引部分。‎

## 5. The ByteArray class: RAII and zero-copy buffering through memory maps

All reads to entries are made through the ByteArray class, which provides a pointer to an array and a size. The ByteArray class encapsulates how the entry is really accessed, and in the current version of KingDB, it is done through read-only memory maps. Resources such as file descriptors and memory map pointers are held by the ByteArray class, and released by its destructor when they are no longer needed. Thus ByteArray is doing RAII, Resource Acquisition Is Initialization. Other sub-classes of ByteArray also allow for entries to be stored in memory, or for memory locations that are shared among multiple ByteArray instances, through a shared_ptr.

The reason why I have chosen to use memory maps to serve reads is because they allow for zero-copy buffering. Indeed, with the memory map, when data is read from disk to be sent on the network, it is first read from disk into a buffer in kernel space, and then it stays in kernel space when it is used by send(). This prevents a useless copy of the data into a buffer of the user space. Copying buffers around, all the more so between user and kernel space, is time consuming, thus this trick allows KingDB to save time and serve data faster. Another interesting point is that the data retrieved is likely to be larger than the maximum payload that can transit on a network, thus any type of caching while reading data from disk is going to increase the throughput. Memory maps are an easy way to get such caching, by reading only what is needed from the buffer and letting the kernel handle the caching.

> ‎对条目的所有读取都是通过 ByteArray 类进行的，该类提供指向数组和大小的指针。ByteArray 类封装了如何真正访问条目，在当前版本的 KingDB 中，它是通过只读内存映射完成的。文件描述符和内存映射指针等资源由 ByteArray 类持有，并在不再需要时由其析构函数释放。因此，ByteArray正在做RAII，资源获取是初始化。ByteArray 的其他子类还允许通过shared_ptr将条目存储在内存中，或用于在多个 ByteArray 实例之间共享的内存位置。‎
>
> 我选择使用内存映射来提供读取的原因是因为它们允许零复制缓冲。实际上，使用内存映射，当从磁盘读取数据以发送到网络上时，首先将其从磁盘读取到内核空间中的缓冲区中，然后在 send（） 使用时将其保留在内核空间中。这可以防止将数据的无用副本复制到用户空间的缓冲区中。复制缓冲区，尤其是在用户和内核空间之间，非常耗时，因此此技巧使 KingDB 能够节省时间并更快地提供数据。另一个有趣的点是，检索到的数据可能大于可以在网络上传输的最大有效负载，因此从磁盘读取数据时任何类型的缓存都会增加吞吐量。内存映射是获取此类缓存的一种简单方法，它只需从缓冲区中读取所需的内容，并让内核处理缓存。

## 6. Multipart API

During the development of KingDB, I wanted to split the development of the storage engine and of the network server. My plan was to first work on the core storage engine and get it to an acceptable stage with enough of the bugs washed out, and then work on the network part. My expectation was that if a bug occurred, I would know more easily if it came from the storage engine or the network. But this has bitten me pretty hard afterwards: I had design KingDB to accept entries of any size, but the Storage Engine was unable to handle very large entries. The worker threads in the server were trying to allocate arrays as big as the entries they were receiving to buffer their content, and then were passing them to the write buffer. I performed a few tests with all clients sending very large files, and of course, crashed the server with a nice Out Of Memory error. But the problem is not only that this design can eat up too much memory, it is also that if the entries are even of medium size, copying their data around can just take too long and end up making the network clients timeout. It is at that moment that I introduced the notion of “part” in the Write Buffer and the Storage Engine, and that I realized that because I wanted KingDB to accept entries of any size through a network, I would have to implement a Multipart API. In retrospect that seems obvious, but while I was programming, it was not.

When the database is embedded, there are no timeout constraints: in case the process embedding the database is trying to store a very large entry, it will just block on I/O and resume control when the entry has been persisted to disk. But when using the database through a network, the client cannot wait for a large entry to be persisted, as this will cause a timeout and make the entire transfer fail. The solution to this problem is to offer a Multipart API, and to cut large entries into smaller piece, which I call “parts”. The database knows what entries are currently being received and which clients are sending them. Entries are written part by part, as they come. From there it is even possible to play with the size of those parts and see what value brings in the best performance.

If there are few concurrent clients, then larger part sizes will allow for larger buffers to be used by recv(), and then sent to the Storage Engine, which means that they will incur fewer calls the write() syscall. But as the number of concurrent threads increases, the memory footprint of the program could explode. With smaller part sizes, more write() syscalls are incurred, but the memory footprint is reduced.

> ‎在 KingDB 开发期间，我想拆分存储引擎和网络服务器的开发。我的计划是首先在核心存储引擎上工作，让它达到一个可接受的阶段，去除但部分的bug，然后在网络部分工作。我的期望是，如果发生错误，我会更容易知道它是否来自存储引擎或网络。但这事后让我非常痛苦：我设计了KingDB来接受任何大小的条目，但存储引擎无法处理非常大的条目。服务器中的工作线程尝试分配与它们接收的条目一样大的数组来缓冲其内容，然后将它们传递到写入缓冲区。我对所有发送非常大的文件的客户端执行了一些测试，当然，服务器崩溃了，出现了一个内存不足错误。但问题不仅在于这种设计会占用太多的内存，还在于如果条目的大小过大，复制它们的数据可能会花费太长时间，最终使网络客户端超时。正是在那一刻，我在写入缓冲区和存储引擎中引入了“部分”的概念，并且我意识到，因为我希望KingDB通过网络接受任何大小的条目，所以我必须实现一个多部分API。回想起来，这似乎是显而易见的，但是当我在编程时，事实并非如此。‎
>
> ‎当嵌入数据库时，没有超时约束：如果嵌入数据库的进程试图存储一个非常大的条目，它将在 I/O 上阻塞，并在条目保存到磁盘时恢复控制。但是，当通过网络使用数据库时，客户端不能等待一个大的条目被持久化，因为这将导致超时并使整个传输失败。这个问题的解决方案是提供一个多部分API，并将大的条目切成更小的部分，我称之为“部分”。数据库知道当前正在接收哪些条目以及哪些客户端正在发送这些条目。条目是逐部分编写的，因为它们来了。从那里甚至可以考虑这些部件的尺寸，看看什么价值带来了最佳性能。‎
>
> ‎如果并发客户端很少，则较大的部件大小将允许 recv（） 使用较大的缓冲区，然后将其发送到存储引擎，这意味着它们将产生较少的写入（） 系统调用。但随着并发线程数量的增加，程序的内存占用量可能会爆炸式增长。对于较小的部件大小，会产生更多的写入（）系统调用，但内存占用量会减少。‎

## 7. Write buffer and the Order class

The writes are cached in memory in the write buffer, and when there is enough of them, they are written to disk in batch. The write buffer is here to turn any workload of small random writes into a workload of large sequential writes. Large writes also have caveats and can cause write stalls as they trash the caches of the CPU and disks, but that is another story [[3, 4]](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref).

The write buffer needs to be locked before it can be flushed. Thus if KingDB had only one write buffer, the flushing would block incoming operations. For that reason, the write buffer contains not just one, but two buffers: at any given time, one buffer has the role of receiver for incoming requests, and another buffer has the role of source for the flush operation. When the buffer used for incoming requests is ready to be flushed, its role is swapped with the buffer that was previously used to flush. The buffer with the latest requests can therefore be locked and flushed without blocking the flow of incoming requests, which are persisted to the other buffer.

The two std::vector in the Write Buffer are storing instances of the Order class. Each order contains the key of the entry it belongs to, a part of data for the value of that entry, and the offset of that part in the value array. Keys and parts are instances of ByteArray, which allows to share allocated memory buffers when they are needed all along the persisting pipeline, and seamlessly release them once they have been persisted by the Storage Engine.

> 写入操作缓存在写入缓冲区的内存中，当有足够的写入操作时，它们将批量写入磁盘。写入缓冲区用于将任何小型随机写入的工作负载转换为大型顺序写入的工作负载。大型写入也有警告，并且可能导致写入停滞，因为它们会丢弃CPU和磁盘的缓存，但这是另一回事‎[‎[3，4]‎](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref)‎。‎
>
> ‎需要先锁定写缓冲区，然后才能刷新它。因此，如果 KingDB 只有一个写入缓冲区，则刷新将阻止传入的操作。因此，写入缓冲区不仅包含一个缓冲区，而且包含两个缓冲区：在任何给定时间，一个缓冲区具有传入请求的接收者角色，另一个缓冲区具有刷新操作的源角色。当用于传入请求的缓冲区准备好进行刷新时，其角色将与以前用于刷新的缓冲区交换。因此，可以锁定和刷新具有最新请求的缓冲区，而不会阻止传入请求的流，这些请求将持久保存到另一个缓冲区。‎
>
> ‎写入缓冲区中的两个 std：：vector 正在存储 Order 类的实例。每个订单都包含它所属条目的键、该条目值的一部分数据以及值数组中该部分的偏移量。键和部分是 ByteArray 的实例，它允许在持久化管道中需要分配的内存缓冲区时共享它们，并在存储引擎持久化后无缝释放它们。‎

## 8. Threads and synchronization

Let’s imagine that all the code were running in a single thread. At some point, a client thread would try to do a small Put() but would block due the write buffer persisting its data to disk and waiting on I/O for example, when actually those two operations could have been running in parallel. The obvious solution is to assign one dedicated thread to every independent operation that is likely to wait on I/O or that is expected to be time consuming. That way, even when one of those threads waits on I/O, the others can still make progress. There will still be moments when a thread is waiting for another one to finish, but overall, downtimes are reduced.

Because those threads are a breakdown of a larger process, they need to be synchronized, and for this I am simply using the std::mutex from the standard C++11. When I started the design of KingDB and thought about how to coordinate those threads, it was thinking of using lock-free algorithms. But because KingDB is a persisted key-value store, the bottlenecks are going to be in the file I/O, therefore even though there are clear benefits to lock-free algorithms when using hundreds of concurrent threads, using lock-free solutions did feel like over-engineering. Thus I have decided to do the first implementation simply using the standard std::mutex offered by C++11, and see how far I could get with that.

The threads used by KingDB are the following (they are also represented in Figure 8.1):

In the Write Buffer:

* **Buffer Manager** : monitors the states of the in-memory write buffers, and send one of them to the Storage Engine when its size in bytes reaches a certain threshold.

In the Storage Engine:

* **Entry Writer** : waits on EventManager::flush_buffer and processes a vector of Orders incoming from the Write Buffer
* **Index Updater** : waits on EventManager::update_index and updates the index when needed
* **Compactor** : at regular interval, it checks various metrics and determines if compaction needs to happen on the database. If so, it calls Compaction() and proceeds to compact data on disk to reclaim disk space.
* **System Statistics Poller** : at regular interval, it polls various system calls to get system metrics (free disk space, etc.)

Some of the threads need to communicate data and this is when issues of coupling can arise in the code. One of the solution I am using to avoid strong dependencies is a minimal message library which I have built. I am saying library but it’s really just 70 lines of code in a single class. It is built on top of std::condition_variable and std::mutex from the C++11 standard library, and can be found in the source code at [thread/event_manager.h](https://github.com/goossaert/kingdb/blob/master/thread/event_manager.h).

For each waypoint where two threads need a connection to pass data among them, a dedicated instance of the Event class is used. Another class, EventManager, embeds all the instances of Event that are needed for a single instance of a KingDB database:

* **flush_buffer** : Used to pass a vector of Orders from the Write Buffer to the Storage Engine. (Flush Start event in Figure 8.1)
* **update_index** : Used to pass a list of locations and hashed keys that were persisted to disk by the Storage Engine to the Index. (Index Update event in Figure 8.1)
* **clear_buffer** : Used to indicate to the Write Buffer that the last batch of Orders was saved in the Index. (Flush End event in Figure 8.1)
* **compaction_status** : Used by the user-triggered compaction to send a confirmation when it has finished. (This event was not represented in Figure 8.1)

Finally, note that no synchronization is needed to access the data persisted by KingDB in secondary storage. Indeed, the internal algorithms and log-structure storage in KingDB guarantee that no locations in any file can be at the same time used for reads and writes. Therefore, the data in HSTable files can be accessed without having to use locks.

> ‎让我们想象一下，所有代码都在单个线程中运行。在某些时候，客户端线程会尝试执行一个 Put（），但由于写入缓冲区将其数据持久保存到磁盘并等待 I/O，例如，当这两个操作实际上可以并行运行时，因此会阻塞。显而易见的解决方案是为可能等待 I/O 或预计耗时的每个独立操作分配一个专用线程。这样，即使其中一个线程等待 I/O，其他线程仍然可以取得进展。仍然会有一些时刻，一个线程正在等待另一个线程完成，但总的来说，停机时间会减少。‎
>
> ‎因为这些线程是较大进程的分解，所以它们需要同步，为此，我只是使用标准C++11中的std：：mutex。当我开始设计 KingDB 并考虑如何协调这些线程时，它正在考虑使用无锁算法。但是，由于KingDB是一个持久的键值存储，瓶颈将存在于文件I / O中，因此，尽管在使用数百个并发线程时，无锁算法有明显的好处，但使用无锁解决方案确实感觉像是过度工程。因此，我决定简单地使用C++11提供的标准std：：mutex进行第一个实现，看看我能走多远。
>
> ‎KingDB 使用的线程如下（图 8.1 中也表示了这些线程）：‎
>
> ‎在写入缓冲区中：‎
>
> - **缓冲区管理器‎**‎：监视内存中写入缓冲区的状态，并在其中一个缓冲区的大小（以字节为单位）达到特定阈值时将其发送到存储引擎。‎
>
> ‎在存储引擎中：‎
>
> * **‎条目编写器‎**‎：等待 EventManager：：flush_buffer 并处理从写入缓冲区传入的订单向量‎
> * **索引更新程序‎**‎：等待 EventManager：：update_index并在需要时更新索引‎
> * **压缩器‎**‎：定期检查各种指标，并确定是否需要在数据库上进行压缩。如果是这样，它将调用 Compaction（） 并继续压缩磁盘上的数据以回收磁盘空间。‎
> * **‎系统统计轮询器‎**‎：定期轮询各种系统调用以获取系统指标（可用磁盘空间等）‎
>
> 某些线程需要传递数据，这是代码中可能出现耦合问题的时候。我用来避免强依赖关系的解决方案之一是我构建的最小消息库。我说的是库，但它实际上只是一个类中的70行代码。它构建在C++11标准库中的std：：condition_variable和std：：mutex之上，可以在‎[‎thread/event_manager.h‎](https://github.com/goossaert/kingdb/blob/master/thread/event_manager.h)‎的源代码中找到。
>
> ‎对于两个线程需要连接才能在它们之间传递数据的每个航点，将使用 Event 类的专用实例。另一个类，EventManager，嵌入了KingDB数据库的单个实例所需的所有事件实例：‎
>
> * **‎flush_buffer‎**‎：用于将订单向量从写入缓冲区传递到存储引擎。（图 8.1 中的刷新启动事件）‎
> * **‎update_index‎**‎：用于将存储引擎保存到磁盘的位置和哈希键的列表传递给索引。（图 8.1 中的索引更新事件）‎
> * **‎clear_buffer‎**‎：用于向写入缓冲区指示最后一批订单已保存在索引中。（图 8.1 中的“刷新结束”事件）‎
> * **‎compaction_status‎**‎：由用户触发的压缩用于在完成时发送确认。（图 8.1 中未表示此事件）‎
>
> ‎最后，请注意，访问 KingDB 在辅助存储中保存的数据不需要同步。事实上，KingDB中的内部算法和日志结构存储保证了任何文件中的任何位置都不能同时用于读取和写入。因此，无需使用锁即可访问 HSTable 文件中的数据。‎

## 9. Error management

I had covered various error management techniques in a previous article of the IKVS series [[5]](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref). I have chosen to use a simple Status class as the error management scheme for KingDB, as it is done by LevelDB, which is itself derived from Google’s C++ style guide and their entire code base [[6]](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref). All methods return this class and can pass to their caller an error message pointing to the exact root cause of a failure. It’s basically integer error codes on steroids.

The return values of all system calls are tested and handled appropriately. On the other hand, errors during memory allocation are not handled, and this is done on purpose. Indeed, if the program gets out of memory, there is little it can do at this stage in order to recover, so the policy is just to let it die. I have put KingDB through Valgrind for memory leaks, heap corruption, data races, etc., and all the bugs that could be found have already been fixed.

Because KingDB is implemented in C++, an obvious question is why not use exceptions? The reason why I rejected the use of exceptions is because I wanted KingDB to be optimized for readability, and exceptions work against that goal as they introduce an additional layer of complexity. In his presentation [[6]](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref), Titus Winters arguments in favor of a Status class and against exceptions, which makes sense in the case of KingDB:

* For the readers of the source code, the Status class makes it easy to see when something can go wrong and how you’re handling it, or to see that you are explicitly not handling it
* Exceptions are hard to maintain: imagine adding or changing additional exception types for error cases you hadn’t thought about before
* Code locality matters, and exceptions make everything they can to make that not the case — technically, exceptions are free as long as they are not thrown; when they’re thrown, they incur a 10-20x slowdown with the Zero-Cost model [[7]](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref).

> ‎我在 IKVS 系列 ‎[‎[5]‎](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref)‎ 的上一篇文章中介绍了各种错误管理技术。我选择使用一个简单的Status类作为KingDB的错误管理方案，因为它是由LevelDB完成的，它本身来自Google的C++风格指南及其整个代码库‎[‎[6]‎](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref)‎。所有方法都返回此类，并且可以向其调用方传递指向失败的确切根本原因的错误消息。它基本上是的整数错误代码。
>
> ‎将对所有系统调用的返回值进行测试和适当处理。另一方面，不处理内存分配期间的错误，这是故意的。实际上，如果程序内存不足，那么在这个阶段它几乎没有可能恢复，因此策略只能让它死亡。我已经通过Valgrind放置了KingDB，用于内存泄漏，堆损坏，数据竞赛等，并且所有可以找到的错误都已修复。
>
> ‎因为KingDB是在C++中实现的，一个明显的问题是为什么不使用异常呢？我拒绝使用异常的原因是因为我希望 KingDB 针对可读性进行优化，而异常与该目标背道而驰，因为它们引入了额外的复杂性层。在他的演讲‎[‎[6]‎](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref)‎中，Titus Winters支持Status class和反对例外，这在KingDB的情况下是有道理的：‎
>
> * ‎对于源代码的读者来说，Status 类可以很容易地看到某些事情何时可能出错以及你如何处理它，或者看到你明确没有处理它。‎
> * ‎异常很难维护：想象一下，为以前没有考虑过的错误案例添加或更改其他异常类型‎
> * ‎代码局部性很重要，异常使得它们所能做的一切事情都不是这样 ——从技术上讲，只要不抛出异常，异常就是免费的;当它们被抛出时，它们会导致零成本模型的减速10-20倍‎[‎[7]‎](https://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/#ref)‎。‎

## 10. Parametrization and Logging

Parametrization follows the design of LevelDB and is done through a set of classes, and parameters are set by setting attributes of these classes. Objects of these parameter classes are created and passed to the database Open() method, along with the Put() and Get() methods. Four independent classes control the parameters:

* **DatabaseOptions** is controlling all the parameters impacting the database, such as internal buffer sizes, HSTable sizes, timeout durations, etc.
* **ReadOptions** is controlling the parameters that can be tweaked when reading an entry, such as whether or not the checksum needs to be verified.
* **WriteOptions** is controlling the parameters that can be tweaked when writing an entry, such as syncing the data to secondary storage for example.
* **ServerOptions** is controlling the parameters related to the network server, such as the recv() buffer size, the listening port, etc.

The advantage of having the parameters in their own respective classes is first that the overall design is more modular, and second that parameters objects can be reused across different databases and method calls.

Logging is done through a set of methods that have the same prototype as printf(), each for a different level of alerting. That way, it is easy to control the granularity of details that one wants to display by simply changing the value of the loglevel parameter. When developing and debugging, I often use the “trace” level, which is the lowest level possible and displays all messages. When running the database in production, the “error” level, or any higher level is recommended, as it will only display messages related to errors and critical events.

> 参数化遵循 LevelDB 的设计，通过一组类完成，参数通过设置这些类的属性来设置。这些参数类的对象与 Put（） 和 Get（） 方法一起创建并传递到数据库 Open（） 方法。四个独立的类控制参数：
>
> * **‎DatabaseOptions ‎控制影响数据库的所有参数，例如内部缓冲区大小、HSTable 大小、超时持续时间等。‎**
> * ‎**ReadOptions‎****‎ 控制在读取条目时可以调整的参数，例如是否需要验证校验和。‎**
> * **WriteOptions‎****‎ 控制在写入条目时可以调整的参数，例如将数据同步到辅助存储。‎**
> * **ServerOptions‎****‎控制与网络服务器相关的参数，例如recv（）缓冲区大小，侦听端口等。‎**
>
> ‎将参数放在各自的类中的好处是，首先整体设计更加模块化，其次，参数对象可以在不同的数据库和方法调用中重用。
>
> ‎日志记录是通过一组与 printf（） 具有相同原型的方法完成的，每种方法都用于不同级别的警报。这样，只需更改 loglevel 参数的值，即可轻松控制要显示的详细信息粒度。在开发和调试时，我经常使用“trace”级别，这是可能的最低级别，并显示所有消息。在生产环境中运行数据库时，建议使用“错误”级别或任何更高级别，因为它仅显示与错误和关键事件相关的消息。‎

## 11. Compression, checksum, and hashing

For compression, only the LZ4 algorithm is currently available: it is very fast and has proven to provide high compression ratios. As for the checksum, only CRC32 is currently available. And regarding the hashing functions used to compute the hashed keys in the index, the user can choose between the 64-bit Murmurhash3, and the 64-bit xxHash.

For all those algorithms, I have reused existing implementations with compatible software licenses, and I have imported their source code into KingDB. Thus when compiling KingDB, everything is ready, and there are no dependencies.

> ‎对于压缩，目前只有LZ4算法可用：它非常快，并且已被证明可以提供高压缩比。至于校验和，目前只有CRC32可用。关于用于计算索引中哈希键的哈希函数，用户可以在64位Murmurhash3和64位xxHash之间进行选择。‎
>
> ‎对于所有这些算法，我已经重用了具有兼容软件许可证的现有实现，并且我已将其源代码导入KingDB。因此，在编译 KingDB 时，一切都已准备就绪，并且没有依赖项。

## Coming next

In the next article, I will review the data format of the Hashed String Table (HSTable), along with memory management in KingDB.

> ‎在下一篇文章中，我将回顾哈希字符串表（HSTable）的数据格式，以及KingDB中的内存管理。‎

## References

[1] [Memcached Protocol](https://github.com/memcached/memcached/blob/master/doc/protocol.txt)
[2] [C++ ISO specifications, N3092, Section 23.2.4/4](http://open-std.org/jtc1/sc22/wg21/docs/papers/2010/n3092.pdf)
[3] [Syscall overhead](http://stackoverflow.com/a/12759003)
[4] [Why buffered writes are sometimes stalled](http://yoshinorimatsunobu.blogspot.nl/2014/03/why-buffered-writes-are-sometimes.html)
[5] [IKVS Part 3, Section 3.6](http://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/)
[6] [CppCon 2014: Titus Winters “The Philosophy of Google’s C++ Code”](https://www.youtube.com/watch?v=NOCElcMcFik)
[7] [TR18015 – Technical Report on C++ Performance](http://www.open-std.org/jtc1/sc22/wg21/docs/TR18015.pdf)
