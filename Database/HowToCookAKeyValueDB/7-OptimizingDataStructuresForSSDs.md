# Implementing a Key-Value Store – Part 7: Optimizing Data Structures for SSDs

This is Part 7 of the IKVS series, “Implementing a Key-Value Store”. You can also check the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) for other parts. In this series of articles, I describe the research and process through which I am implementing a key-value store database, which I have named “KingDB”.

In the previous articles, I have spent a fair amount of time reviewing existing key-value stores, interfaces, architectures, and I focused greatly on hash tables. In this article, I will talk about hardware considerations for storing data structures on solid-state drives (SSDs), and I will share a few notes regarding file I/O optimizations in Unix-based operating systems.

This article will cover:

1. Fast data structures on SSDs
2. File I/O optimizations
3. Done is better than perfect
4. References

> ‎这是 IKVS 系列的第 7 部分“实现键值存储”。您还可以在‎[‎目录中‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎查看其他部分。在本系列文章中，我将介绍实现键值存储数据库（我将其命名为“KingDB”）的研究和过程。‎
>
> ‎在前面的文章中，我花了相当多的时间回顾现有的键值存储、接口、体系结构，并且我非常关注哈希表。在本文中，我将讨论在固态硬盘 （SSD） 上存储数据结构的硬件注意事项，并将分享一些关于基于 Unix 的操作系统中文件 I/O 优化的注意事项。‎
>
> 本文将介绍：‎
>
> 1. 固态‎‎硬盘上的快速数据结构
> 2. 文件 I/O 优化‎
> 3. 先实现再优化
> 4. 引用‎

## 1. Fast data structures on SSDs

As I was researching resources on efficient ways to implement KingDB, I fell down a gigantic rabbit hole: solid-state drives. I have compiled the results of that research into another series of articles, *“Coding for SSDs”* [[1]](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref), and I have summarized the most important points in the last article of that series: *“What every programmer should know about solid-state drives”* [[3]](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref). I encourage you to read the whole series if you have time, and if not, then at least have a sneak peak at the summary. All that research and this series of articles on SSDs were necessary, as they gave me the confidence that my design decisions for KingDB were taking me in the right direction.

> ‎当我研究有关实现 KingDB 的有效方法的资源时，我发现了另一个指点研究的点：固态硬盘。我将这项研究的结果写成另一个系列文章‎*‎，“SSD相关编程”‎*[‎[1]‎](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref)‎，我总结了该系列最后一篇文章中最重要的观点：‎*‎“每个程序员都应该了解的关于固态硬盘的知识”‎*[‎[3]‎](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref)‎。我鼓励你有时间阅读整个系列，如果没有时间，那么至少要仔细阅读一下摘要。所有这些研究和这一系列关于 SSD 的文章都是有必要的，因为它们给了我信心，即我对 KingDB 的设计决策正在走向正确的方向。

### In-place updates are useless

SSDs differ dramatically from regular HDDs. The most important thing about SSDs is that, unlike HDDs, nothing is ever overwritten. Updates to an existing page are applied in an internal register, a RAM module in the SSD, and the updated version of the page is stored at a different location on the SSD. The original page is considered “stale” until the garbage collection process erases it and puts it back in the pool of free pages. As a direct consequence, any algorithm trying to be clever by applying updates in-place will bring no performance gain, and will needlessly increase code complexity.

> ‎SSD与常规HDD有很大不同。关于SSD最重要的事情是，与HDD不同，没有任何东西被覆盖写。对现有页面的更新将应用于内部寄存器、SSD 中的 RAM 模块，并且页面的更新版本存储在 SSD 上的不同位置。原始页面被视为“过时”，直到垃圾回收过程将其删除并将其放回可用页面池中。直接结果是，任何试图通过就地应用更新来变得高效的算法都不会带来性能提升，并且会不必要地增加代码复杂性。‎

### Metadata must be buffered or avoided

The smallest size of data that can be written to an SSD is a NAND-flash page — 4 KB, 8 KB, or 16 KB depending on the SSD model — which can have serious consequences for any metadata stored on the drive. Imagine that a hash table needs to persist on the drive the number of items it is currently holding, as a 64-bit integer. Even though that number is stored over 8 bytes, every time it is written to the drive an entire NAND-flash page will be written. Thus an 8-byte write turns into a 16 KB write, every single time, leading to dramatic write amplification. As a result, when stored on SSDs, data structures need to cache their metadata and write them to the drive as infrequently as possible. Ideally, the file format will be designed in a way that this metadata never needs to be updated in-place, or even better, that this metadata never needs to be stored at all [[2]](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref).

> SSD每次写入 的最小数据单元大小是 NAND 闪存页面（4 KB、8 KB 或 16 KB，具体取决于 SSD 型号），这可能会对存储在SSD的元数据产生严重影响。想象一下，哈希表需要以 64 位整数的形式在驱动器上保留它当前保存的项目数。即使该数字存储在8个字节以上，每次将其写入驱动器时，都会写入整个NAND闪存页面。因此，每次 8 字节写入都会变成 16 KB 写入，从而导致剧烈的写入放大。因此，当存储在 SSD 上时，数据结构需要缓存其元数据并尽可能少地将其写入驱动器。理想情况下，文件格式的设计方式是，此元数据永远不需要就地更新，甚至，此元数据根本不需要存储‎[‎[2]‎](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref)‎。

### Data is never really sequential

In SSDs, addresses that are sequential in the logical space do not correspond to addresses that are sequential in the physical space. This means that if you open a file and write sequentially to it, the data is not stored sequentially on the drive. Of course, if data is written sequentially within the alignment of a NAND-flash page, then it will be sequential on the SSD, but two pages that are sequential in the logical space have very little chances to be stored sequentially in the drive. Thus for chunks larger than the size of the NAND-flash page, rearranging data sequentially hoping to optimize for future reads will brings no improvements. Not only that, but the amount of data moved around eats up CPU time, and causes the NAND-flash memory modules in the SSD to wear off prematurely. Below the size of the NAND-flash page, storing data sequentially in the logical space still helps.

> ‎在 SSD 中，逻辑空间中顺序的地址与物理空间中顺序的地址不对应。这意味着，如果打开文件并按顺序写入该文件，则数据不会按顺序存储在驱动器上。当然，如果在NAND闪存页面的对齐方式中按顺序写入数据，那么它将在SSD上按顺序写入，但是在逻辑空间中按顺序存储的两个页面在驱动器中按顺序存储的机会很小。因此，对于大于NAND闪存页面大小的块，按顺序重新排列数据以希望针对将来的读取进行优化不会带来任何改进。不仅如此，移动的数据量会占用CPU时间，并导致SSD中的NAND闪存模块过早磨损。低于NAND闪存页面的大小，在逻辑空间中按顺序存储数据仍然有帮助。‎

### Random writes are fast if they are large enough

SSDs have several levels of internal parallelism. A “clustered block” is a set of NAND-flash memory modules that can be accessed concurrently, and has a size of 16 MB or 32 MB for most SSDs. If writes are performed in the size of at least one clustered block, they will use all the levels of internal parallelism, and reach the same performance as sequential writes [[2]](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref).

> ‎SSD 内部具有多个并行的级别。“群集块”是一组可以同时访问的 NAND 闪存模块，对于大多数 SSD，其大小为 16 MB 或 32 MB。如果写入数据至少有一个集群块的大小，它们将使用所有级别的并行写入，并达到与顺序写入相同的性能 ‎[‎[2]‎](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref)‎。‎

### Random reads are fast

The data written together in a buffer the size of a clustered block will use all the internal parallelism of an SSD. If the same data is later retrieved, the throughput will be very high because the SSD will be able to fire up multiple memory modules at the same time, using of all its internal parallelism. For most workloads, it is impossible to predict what data will be read, and therefore, the database cannot take advantage of this aspect of the SSD architecture. However, read performance in SSDs is very good on average, whether reads are sequential or random. Therefore the focus for data structures persisted on SSD should really be on optimizing for the writes rather than for the reads.

> ‎以群集块大小向SSD写入的数据将使用 SSD 的所有内部并行通道。如果以后检索相同的数据，吞吐量将非常高，因为SSD将能够同时启动多个内存模块，利用其所有内部并行性。对于大多数工作负载，无法预测将读取哪些数据，因此，数据库无法利用SSD架构的这一方面。但是，SSD 中的读取性能平均非常好，无论读取是顺序还是随机读取。因此，SSD上持久保存的数据结构的重点实际上应该放在优化写入而不是读取上。‎

### Log-structured storage

The goal that I set myself for KingDB is to minimize the number of writes performed to the SSD incurred by incoming updates. The best option to cope with the specificities of SSDs that I have just mentioned above is to use the “log-structured” paradigm. Originally developed for file systems [[4]](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref), more data structures have started to use that approach. One of the most famous is now the Log-Structured Merge-Tree (LSM tree), which is the data structure at the heart of the LevelDB key-value store [[5]](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref).

The idea behind log-structured storage is simple: treat the incoming updates as a log stream, and write them on the drive sequentially, exactly as they come. Let’s take an example to make things clearer. Assume that a database has an entry with key “foo”, and that a new update, a Put(), comes for the same key: the old value for that key needs to be replaced with the new value. What log-structured storage does is that it keeps the old value where it is, and store the new value in the free space available just after the last incoming update. Then the index is updated to make the key “foo” point to the new location. The space used by the old value will be later recovered during a compaction process. Again, note that the old value is not changed in-place, and this is good because SSDs cannot do in-place updates anyway.

Another interesting side effect of log-structured storage is that, because it allows for writes to be as large as one wants them to be, they can be in the order of magnitude of the clustered block (32 MB). Incoming random updates can be batched into a single large write operation, by buffering them into the RAM for example, effectively turning a workload of small random writes into a workload of large sequential writes.

> ‎我为 KingDB 设定的目标是最大限度地减少传入更新对 SSD 执行的写入次数。处理我上面提到的SSD的特殊性的最佳选择是使用“结构化日志”的方式。最初是为文件系统开发的‎[‎[4]‎](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref)‎，现在很多的数据结构已经开始使用这种方法。其中最著名的是Log-Structured Merge-Tree（LSM tree），它是LevelDB键值存储‎[‎[5]‎](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref)‎核心的数据结构。‎
>
> 结构化日志存储背后的想法很简单：将传入的更新视为日志流，并按顺序将它们写入驱动器，与它们完全相同。让我们举个例子来让事情更清楚。假设数据库有一个带有键“foo”的条目，并且一个新的更新，一个 Put（））来自同一个键：该键的旧值需要替换为新值。日志结构化存储的作用是将旧值保留在原处，并将新值存储在上次传入更新后的可用空间中。然后更新索引以使密钥“foo”指向新位置。旧值使用的空间稍后将在压缩过程中恢复。同样，请注意，旧值不会就地更改，这很好，因为 SSD 无论如何都无法执行就地更新。‎
>
> ‎日志结构存储的另一个有趣的副作用是，因为它允许写入大小与人们希望的那样大，所以它们可以处于集群块（32 MB）的数量级。传入的随机更新可以批处理为单个大型写入操作，例如，通过将它们缓冲到RAM中，有效地将小型随机写入的工作负载转换为大型顺序写入的工作负载。‎

## 2. File I/O optimizations

### Every syscall has a cost

The days when every system call was causing a full context switch are long gone. Nowadays, going from user land into kernel space only requires a “mode switch”, which is much faster as it does not require the CPU caches and TLB to be invalidated. Those mode switches still have a cost, roughly 50 to 100 ns depending on the CPU [[6]](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref).

In Chapter 13 of his excellent book “The Linux Programming Interface”, Michael Kerrisk benchmarked the time it takes to write 100 MB of data with different buffer sizes. In one of his tests, it took 72 seconds to write 100 MB with a buffer size of 1 byte, and only 0.11 second to write the same 100 MB with a buffer size of 4 KB. No actual file I/O was performed as the changes were not synced to secondary storage, thus the test was effectively measuring the time spent in the system calls. This is why minimizing the number of system calls and determining the correct sizes for the buffers is crucial to performance.

> ‎每次系统调用导致完整上下文切换的日子早已一去不复返了。如今，从用户空间到内核空间只需要一个“模式切换”，这要快得多，因为它不需要CPU缓存和TLB失效。这些模式开关仍然有成本，大约50到100 ns，具体取决于CPU ‎[‎[6]‎](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref)‎。‎
>
> ‎Michael Kerrisk 在他的杰作《Linux 编程接口》的第 13 章中，对不同缓冲区大小写入100 MB 数据所需的时间进行了基准测试。在他的一项测试中，写入 100 MB 且缓冲区大小为 1 字节需要 72 秒，写入缓冲区大小为 4 KB 的相同机器写入100 MB 仅需 0.11 秒。由于更改未同步到辅助存储，因此未执行实际的文件 I/O，因此测试有效地测量了在系统调用中花费的时间。这就是为什么最大限度地减少系统调用次数并确定缓冲区的正确大小对性能至关重要的原因。‎

### Memory maps

Memory maps are the best thing known to mankind after hash tables. A memory map is basically a mapping of an area of a file into an address in the memory. That address can then be accessed, for both reads and writes, using a pointer like any regular buffer of allocated memory, and the kernel takes care of actually reading and writing the data to the file on the drive. There are countless success stories of improvements achieved by replacing calls to read() and write() with memory maps, an interesting one being covered in the SQLite documentation [[7]](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref). Optimizing file I/O is hard, and most of the time, is better left to the kernel. As a consequence, I will try to use memory maps as much as possible for KingDB. I will indulge myself the option of using calls to read() and write() if I judge that it can speed up the development time. Vectored I/O, also known as scatter-gather I/O, is another tool worth mentioning as it can offer substantial performance improvements, and deserves a try whenever applicable [[8]](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref).

> ‎内存映射是在哈希表之后最需要了解的东西。内存映射基本上是将文件映射到内存中的地址。然后，可以使用指针（如任何分配内存的常规缓冲区）访问该地址进行读取和写入，内核负责实际读取数据并将其写入驱动器上的文件。通过将对read（）和write（）的调用替换为内存映射，有无数的成功案例实现了改进，SQLite文档‎[‎[7]‎](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref)‎中介绍了一个有趣的例子。优化文件 I/O 很难，大多数情况下，最好留给内核。因此，我将尝试在 KingDB 中尽可能多地使用内存映射。我会尝试使用调用read（）和write（）的选项，如果我发现它可以加快开发时间。矢量 I/O，也称为分散-聚集 I/O，是另一个值得一提的工具，因为它可以提供实质性的性能改进，并且值得在适用时尝试 ‎[‎[8]‎](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref)‎。‎

### Zero-copy

Most system calls copy data from a buffer in user land to a another buffer in kernel space, or conversely, causing a lot of unnecessary data duplication. Zero-copy is there precisely to minimize the amount of data that needs to be copied around, with the goal of copying absolutely zero byte between user space and kernel space. The sendfile() syscall, which sends a file to a socket, is a typical example of zero-copy transfer. A buffer is still required for sendfile(), as pages from the file need to be loaded into RAM before they are sent to the socket, but this buffer remains in kernel space, and there is no data copy between user and kernel space [[9, 10, 11]](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref). Memory maps are also performing zero-copy operations, thus it is good to stick to memory maps and not rush into using specific system calls such as sendfile() and splice().

Data copying is a very serious source of wasted time in a database system, as the total time spent needlessly duplicating data around can sum up to large latencies. It can happen anywhere, not only when using syscalls, and must be monitored closely.

> ‎大多数系统调用将数据从用户区复制到内核区，或者相反，导致大量不必要的数据重复。零拷贝正是为了最大限度地减少需要复制的数据量，目标是消除数据在用户空间和内核空间的复制。将文件发送到套接字的 sendfile（） 系统调用是零拷贝传输的典型示例。sendfile（） 仍然需要一个缓冲区，因为文件中的页面在发送到套接字之前需要加载到 RAM 中，但此缓冲区保留在内核空间中，并且用户和内核空间之间没有数据副本 ‎[‎[9， 10， 11]‎](https://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/#ref)‎。内存映射也执行零拷贝操作，因此最好坚持内存映射，而不是急于使用特定的系统调用，如 sendfile（） 和 splice（）。‎

## 3. Done is better than perfect

In the two sections above, I have covered the most important points regarding the storing of data structures on SSDs and file I/O optimizations. I want to consider those sections only as a general direction, from which I can steer away at any moment if I gather data to prove that other options are preferable. Also, it is fine to cut corners, because the goal for me is to get a working key-value store as soon as possible: done is better than perfect.

I have started implementing KingDB, the key-value store related to this series of article. The source code, still under development, is available at [http://kingdb.org](http://kingdb.org/). In the next article, I will review the architecture of KingDB and its components.

> ‎在上面的两节中，我介绍了有关在 SSD 上存储数据结构和文件 I/O 优化的要点。我只想把这些部分作为一个大方向来考虑，如果我收集数据来证明其他选择是可取的，我可以随时避开这个方向。此外，偷工减料也没关系，因为我的目标是尽快获得一个有效的键值存储：完成比完美更好。
>
> ‎我已经开始实现 KingDB，这是与本系列文章相关的键值存储。源代码仍在开发中，可在 ‎[‎http://kingdb.org‎](http://kingdb.org/)‎ 获得。在下一篇文章中，我将回顾KingDB及其组件的架构。‎

## 4. References

[1] [Coding for SSDs – Part 1: Introduction and Table of Contents](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)
[2] [Coding for SSDs – Part 5: Access Patterns and System Optimizations](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/)
[3] [Coding for SSDs – Part 6: A Summary – What every programmer should know about solid-state drives](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/)
[4] [Log-structured File System](http://en.wikipedia.org/wiki/Log-structured_file_system)
[5] [SSTable and Log Structured Storage: LevelDB](https://www.igvita.com/2012/02/06/sstable-and-log-structured-storage-leveldb/)
[6] [How long does it take to make a context switch](http://blog.tsunanet.net/2010/11/how-long-does-it-take-to-make-context.html)
[7] [SQLite Documentation: Memory-Mapped I/O](https://www.sqlite.org/mmap.html)
[8] [Fast Scatter-Gather I/O](http://www.gnu.org/software/libc/manual/html_node/Scatter_002dGather.html)
[9] [Zero-Copy Sockets](https://www.cs.duke.edu/ari/trapeze/freenix/node6.html)
[10] [Efficient data transfer through zero copy](http://www.ibm.com/developerworks/library/j-zerocopy/)
[11] [Zero Copy I: User-Mode Perspective](http://www.linuxjournal.com/article/6345)
