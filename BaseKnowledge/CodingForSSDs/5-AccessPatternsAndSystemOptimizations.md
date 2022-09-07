# Coding for SSDs – Part 5: Access Patterns and System Optimizations

This is Part 5 over 6 of “Coding for SSDs”, covering Sections 7 and 8. For other parts and sections, you can refer to the [Table to Contents](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/). This is a series of articles that I wrote to share what I learned while documenting myself on SSDs, and on how to make code perform well on SSDs. If you’re in a rush, you can also go directly to [Part 6](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/), which is summarizing the content from all the other parts.

Now that I have covered most of the inner workings of solid-state drives in the previous sections, I can provide data that will help build an understanding of which access patterns should be used and why they are indeed better than others. In this part, I explain how writes should be done, how reads should be done, and why concurrent read and write operations are interfering. I also cover a few optimizations at the level of the filesystem which can improve performance.

> ‎这是“SSD 编码”的第 5 部分，第 6 部分，涵盖第 7 节和第 8 节。对于其他部分和节，可以参考‎[‎目录‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)‎。这是我写的一系列文章，旨在分享我在SSD上记录自己时学到的东西，以及如何使代码在SSD上表现良好。如果您赶时间，也可以直接转到‎[‎第6部分‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/)‎，该部分总结了所有其他部分的内容。‎
>
> ‎现在，我已经在前面的部分中介绍了固态硬盘的大部分内部工作原理，我可以提供数据，这些数据将有助于理解应该使用哪些访问模式，以及为什么它们确实比其他模式更好。在这一部分中，我将解释应该如何进行写入，应该如何进行读取，以及为什么并发读取和写入操作会受到干扰。我还介绍了文件系统级别的一些优化，这些优化可以提高性能。‎

 ![ssd-presentation-05](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/02/ssd-presentation-05.jpg?resize=720%2C579)

## 7. Access patterns

### 7.1 Defining sequential and random I/O operations

In the following sub-sections, I will be referring to accesses as being “sequential” or “random”. An I/O operation is said to be *sequential* if its starting logical block address (LBA) directly follows the last LBA of the previous I/O operation. If this is not the case, then the operation is said to be  *random* . It is important to note that due to the dynamic mapping performed by the FTL, contiguous addresses in the logical space may refer to addresses that are not contiguous in the physical space.

> ‎在下面的小节中，我将把访问称为“顺序”或“随机”。如果 I/O 操作的起始逻辑块地址 （LBA） 紧跟在上一个 I/O 操作的最后一个 LBA 之后，则称其为‎*‎顺序‎*‎操作。如果不是这种情况，则该操作称为‎*‎随机‎*‎操作。请务必注意，由于 FTL 执行的动态映射，逻辑空间中的连续地址可能是指物理空间中不连续的地址。‎

### 7.2 Writes

Benchmarks and manufacturer data sheets show that random writes are slower than sequential writes, though it is not always true as it depends upon the exact type of random write workload. If the size of the writes is small, and by small I mean less than the size of a clustered block (i.e. < 32 MB), then yes, random writes are slower than sequential writes. However, if the random writes are both multiple of and aligned to the size of a clustered block, then they perform just as well as sequential writes.

The explanation is as follows. As seen in Section 6, the internal parallelism in SSDs allows for clustered blocks to be written at once using a combination of parallelism and interleaving. Therefore, whether they are sequential or random, the writes will be striped over multiple channels and chips in the same way internally, and performing a write that has the size of the clustered block guarantees that all of the internal parallelism will be used. Write operations on clustered blocks will be covered in Section 7.3 below. Now on the performance side, as shown in Figures 8 and 9 reproduced from [[2]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref) and [[8]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref) respectively, random writes reach a throughput as high as the one of sequential writes when the size of the benchmark write buffer is equal or greater to the size of the clustered block, which is 16 or 32 MB for most SSDs.

> ‎基准测试和制造商数据手册显示，随机写入比顺序写入慢，尽管这并不总是正确的，因为它取决于随机写入工作负载的确切类型。如果写入的大小很小，并且小的意思是小于簇状块的大小（即<32 MB），那么是的，随机写入比顺序写入慢。但是，如果随机写入既是簇状块的倍数又与簇状块的大小对齐，则它们的性能与顺序写入一样好。‎
>
> ‎解释如下。如第6节所示，SSD中的内部并行性允许使用并行性和交错的组合一次写入集群块。因此，无论它们是顺序的还是随机的，写入都将以内部相同的方式在多个通道和芯片上进行条带化，并且执行具有簇状块大小的写入可以保证将使用所有内部并行性。对集群块的写入操作将在下面的第 7.3 节中介绍。现在，在性能方面，如图 8 和图 9 分别从 ‎[‎[2]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎ 和 ‎[‎[8]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎ 中复制，当基准写入缓冲区的大小等于或大于群集块的大小（对于大多数 SSD 为 16 或 32 MB）时，随机写入的吞吐量与顺序写入的吞吐量一样高。‎

![writes-random-01](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/01/writes-random-01.jpg?resize=720%2C621)

Figure 8: Comparison of the effects of a sequential write workload versus a random write workload over four SSDs — Reproduced from Kim et al., 2012 [[2]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)

![writes-random-02](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/01/writes-random-02.jpg?resize=720%2C476)

Figure 9: Comparison of the effects of a sequential write workload versus a random write workload over three SSDs — Reproduced from Min et al., 2012 [[8]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)

However, if the writes are small — by small I mean smaller than a NAND-flash page, i.e. < 16 KB — then the controller has more work to do in order to maintain the metadata necessary for the block mapping. Indeed, some SSDs are using tree-like data structures to represent the mapping between logical block addresses and physical block addresses [[1]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref), and a lot of small random writes will translate into a lot of updates to the mapping in RAM. As this mapping is being persisted from RAM to flash memory [[1, 5]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref), all those updates in the RAM will cause a lot of writes on the flash memory. A sequential workload incurs less updates to the metadata, and therefore less writes to the flash memory.

> ‎但是，如果写入很小 （我说的写法是指比 NAND 闪存页小，即 < 16 KB），则控制器需要做更多的工作来维护块映射所需的元数据。事实上，一些 SSD 正在使用树状数据结构来表示逻辑块地址和物理块地址 ‎[‎[1]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎ 之间的映射，并且许多小的随机写入将转换为对 RAM 中映射的大量更新。由于此映射从 RAM 持续到闪存 ‎[‎[1， 5]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎，因此 RAM 中的所有这些更新都将导致闪存上出现大量写入。顺序工作负载对元数据的更新较少，因此对闪存的写入较少。‎

#### Random writes are not always slower than sequential writes

If the writes are small (i.e. below the size of the clustered block), then random writes are slower than sequential writes.

If writes are both multiple of and aligned to the size of a clustered block, the random writes will use all the available levels of internal parallelism, and will perform just as well as sequential writes.

Another reason is that if the random writes are small, they will cause a higher number of **copy-erase-write** operations on the blocks. On the other hand, sequential writes of at least the size of a block allow for the faster **switch merge** optimization to be used. Moreover, small random writes are known to invalidate data randomly. Instead of having a few blocks fully invalidated, many blocks will have only one page invalidated, which causes stale pages to be spread out in physical space instead of being localized. This phenomenon is known as  **internal fragmentation** , and causes the **cleaning efficiency** to drop, by requiring a larger number of erase operations to be run by the garbage collection process to create free pages.

Finally regarding concurrency, it has been shown that writing a large buffer with one thread is just as fast as writing many smaller buffers with many concurrent threads. Indeed, a large write guarantees that all of the internal parallelism of the SSD is used. Therefore, trying to perform multiple writes in parallel will not improve the throughput [[1, 5]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref). However, many parallel writes will cause the latency to increase compared to a single thread access [[3, 26, 27]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref).

> ‎如果写入很小（即小于簇状块的大小），则随机写入比顺序写入慢。‎
>
> ‎如果写入操作既是集群块的倍数又与之大小对齐，则随机写入将使用所有可用的内部并行度级别，并且性能与顺序写入一样好。‎
>
> ‎另一个原因是，如果随机写入很小，它们将导致块上的‎**‎复制 - 擦除 - 写入‎**‎操作次数更多。另一方面，至少为块大小的顺序写入允许使用更快的‎**‎交换机合并‎**‎优化。此外，已知小型随机写入会使数据随机失效。许多块将只有一个页面无效，而不是让几个块完全失效，这会导致过时的页面分散在物理空间中，而不是被本地化。这种现象被称为‎**‎内部碎片‎**‎，通过要求垃圾回收进程运行大量擦除操作来创建可用页面，从而导致‎**‎清理效率‎**‎下降。‎
>
> ‎最后，关于并发性，已经证明，使用一个线程编写大型缓冲区与使用许多并发线程写入许多较小的缓冲区一样快。实际上，大写入可以保证使用SSD的所有内部并行性。因此，尝试并行执行多次写入不会提高吞吐量 ‎[‎[1， 5]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。但是，与单线程访问相比，许多并行写入将导致延迟增加 ‎[‎[3， 26， 27]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。‎

#### A single large write is better than many small concurrent writes

A single large write request offers the same throughput as many small concurrent writes, however in terms of latency, a large single write has a better response time than concurrent writes. Therefore, whenever possible, it is best to perform large writes.

> ‎单个大型写入请求提供与许多小型并发写入相同的吞吐量，但在延迟方面，大型单写入的响应时间优于并发写入。因此，只要有可能，最好执行大型写入。‎

#### When the writes are small and cannot be grouped or buffered, multi-threading is beneficial

Many concurrent small write requests will offer a better throughput than a single small write request. So if the I/O is small and cannot be batched, it is better to use multiple threads.

> ‎许多并发小型写入请求将提供比单个小型写入请求更好的吞吐量。因此，如果 I/O 很小且无法批处理，则最好使用多个线程。‎

### 7.3 Reads

Reads are faster than writes. As for sequential reads versus random reads, it all depends. The FTL is mapping logical block dynamically to physical blocks, and stripes writes across channels. This approach is sometimes referred to as “ *write-order-based* ” mapping [[3]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref). If the data is read completely randomly in a way that does not match the way it was originally written to, then there is no guarantee that consecutive reads are spread across different channels. It is even possible that consecutive random reads are accessing different blocks from a single channel, thus not taking advantage of the internal parallelism. Acunu has written a blog article on this which shows, at least for the drive they tested, that read performance is directly linked to how closely the read access patterns matches how the data was originally written [[47]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref).

> ‎读取速度比写入速度快。至于顺序读取与随机读取，这完全取决于。FTL 将逻辑块动态映射到物理块，并跨通道进行条带写入。这种方法有时被称为“‎*‎基于写入顺序”的‎*‎映射‎[‎[3]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。如果数据的读取方式与最初写入的方式不匹配，则无法保证连续读取分布在不同的通道上。甚至有可能连续随机读取从单个通道访问不同的块，因此没有利用内部并行性。Acunu为此写了一篇博客文章，表明，至少对于他们测试的驱动器，读取性能与读取访问模式与数据最初写入方式的紧密程度直接相关‎[‎[47]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。‎

#### To improve the read performance, write related data together

Read performance is a consequence of the write pattern. When a large chunk of data is written at once, it is spread across separate NAND-flash chips. Thus you should write related data in the same page, block, or clustered block, so it can later be read faster with a single I/O request, by taking advantage of the internal parallelism.

Figure 10 below shows an SSD that has two channels and four chips with one plane per chip. Note that this is technically invalid as SSDs always have two or more planes per chip, but for the sake of keeping the schematic compact and simple, I decided to show only one plane per chip. Capital letters represent data which has the size of a NAND-flash block. The operation represented at the top of Figure 10 is writing four blocks sequentially, `[A B C D]`, which in this example is the size of the clustered block. The write operation is being striped over the four planes using parallelism and interleaving, making it faster. Even though the four blocks are sequential in the logical block address space, they are stored in four distinct planes internally.

With write-order-based FTLs, all blocks in a plane are equally-likely to be chosen for incoming writes, therefore a clustered block will not necessarily have to be formed of blocks that have the same PBN in their respective planes. For example in Figure 10, the first clustered block is formed of blocks from four different planes, the PBNs in their respective planes being 1, 23, 11, and 51.

Two read operations are represented at the bottom of Figure 10, `[A B E F]` and `[A B G H]`. For `[A B E F]`, A and E belong to the same plane, and B and F belong to another plane, thus `[A B E F]` is forced to read from only two planes over one channel. In the case of `[A B G H]`, A, B, G, and H are stored on four different planes, therefore `[A B G H]` can read from four planes over two channels at the same time. Reading from more planes and more channels enables to take advantage of more of the internal parallelism, therefore granting better read performance.

> ‎读取性能是写入模式的结果。当一次写入大量数据时，它会分布在单独的NAND闪存芯片上。因此，您应该在同一页、块或集群块中写入相关数据，以便以后通过利用内部并行性，通过单个 I/O 请求更快地读取这些数据。‎
>
> ‎下面的图 10 显示了一个 SSD，它具有两个通道和四个芯片，每个芯片一个平面。请注意，这在技术上是无效的，因为SSD每个芯片总是有两个或更多平面，但是为了保持原理图的紧凑性和简单性，我决定每个芯片只显示一个平面。大写字母表示具有 NAND 闪存块大小的数据。图 10 顶部表示的操作是按顺序写入四个块，在本例中为聚簇块的大小。写入操作使用并行性和交错在四个平面上进行条带化，使其更快。即使这四个块在逻辑块地址空间中是连续的，它们也存储在内部的四个不同的平面中。‎
>
> ‎使用基于写入顺序的FTL，平面中的所有块都同样有可能被选择用于传入写入，因此集群块不一定必须由在各自平面中具有相同PBN的块组成。例如，在图 10 中，第一个聚簇块由来自四个不同平面的块组成，它们各自平面中的 PBN 为 1、23、11 和 51。‎
>
> ‎图 10 底部显示了两个读取操作，以及 。因为，A和E属于同一平面，而B和F属于另一个平面，因此被迫在一个通道上只从两个平面读取。在 的情况下，A、B、G 和 H 存储在四个不同的平面上，因此可以同时从两个通道上的四个平面读取。从更多平面和更多通道读取数据可以利用更多的内部并行度，从而提供更好的读取性能。‎

![ssd-exploiting-parallelism](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/02/ssd-exploiting-parallelism.jpg?resize=720%2C1158)

Figure 10: Exploiting the internal parallelism of SSDs

A direct consequence of the internal parallelism is that trying to read data concurrently using multiple threads will not necessarily result in increasing performance. Indeed, if the locations accessed by the threads do not have knowledge of the internal mapping and do not take advantage of it, they could end up accessing the same channel. It has also been shown that concurrent read threads can impair the readahead (prefetching buffer) capabilities of SSDs [[3]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref).

> ‎内部并行性的直接后果是，尝试使用多个线程并发读取数据并不一定会导致性能提高。实际上，如果线程访问的位置不了解内部映射并且没有利用它，则它们最终可能会访问相同的通道。还表明，并发读取线程会损害 SSD 的预读（预取缓冲区）功能 ‎[‎[3]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。‎

#### A single large read is better than many small concurrent reads

Concurrent random reads cannot fully make use of the readahead mechanism. In addition, multiple Logical Block Addresses may end up on the same chip, not taking advantage or of the internal parallelism. Moreover, a large read operation will access sequential addresses and will therefore be able to use the readahead buffer if present. Consequently, it is preferable to issue large read requests.

SSD manufacturers generally do not communicate the sizes of the page, block and clustered block. It is however possible to reverse engineer most of the basic characteristics of an SSD with great confidence by running simple workloads [[2, 3]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref). This information can then be used to optimize the size of the buffer with which reads and writes are made, and also to align partitions to the underlying SSD characteristics when formatting the drive, as it is covered in Section 8.4.

> ‎并发随机读取不能完全利用预读机制。此外，多个逻辑块地址可能最终位于同一芯片上，不会利用内部并行性。此外，大型读取操作将访问顺序地址，因此将能够使用预读缓冲区（如果存在）。因此，最好发出大型读取请求。‎
>
> ‎SSD制造商通常不会传达页面，块和群集块的大小。但是，通过运行简单的工作负载‎[‎[2，3]，‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎可以非常自信地对SSD的大多数基本特征进行逆向工程。然后，此信息可用于优化进行读取和写入的缓冲区的大小，以及在格式化驱动器时将分区与底层 SSD 特征对齐，如第 8.4 节所述。‎

### 7.4 Concurrent reads and writes

Interleaving small reads and writes causes performance to decrease [[1, 3]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref). The main reason for is that reads and writes are competing for the same internal resources, and that mixing them prevent some mechanism such as the **readahead** to be exploited fully.

> ‎交错小的读取和写入会导致性能下降 ‎[‎[1， 3]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。主要原因是读取和写入正在争用相同的内部资源，并且混合它们会阻止某些机制（如‎**‎预读）‎**‎被充分利用。‎

#### Separate read and write requests

A workload made of a mix of small interleaved reads and writes will prevent the internal caching and readahead mechanism to work properly, and will cause the throughput to drop. It is best to avoid simultaneous reads and writes, and perform them one after the other in large chunks, preferably of the size of the clustered block. For example, if 1000 files have to be updated, you could iterate over the files, doing a read and write on a file and then moving to the next file, but that would be slow. It would be better to reads all 1000 files at once and then write back to those 1000 files at once.

> ‎由小型交错读取和写入混合组成的工作负荷将阻止内部缓存和预读机制正常工作，并将导致吞吐量下降。最好避免同时读取和写入，并以大块的形式一个接一个地执行它们，最好是簇状块的大小。例如，如果必须更新 1000 个文件，则可以循环访问这些文件，对文件执行读取和写入操作，然后移动到下一个文件，但速度会很慢。最好一次读取所有1000个文件，然后一次写回这1000个文件。‎

## 8. System optimizations

### 8.1 Partition alignment

As explained in Section 3.1, writes are aligned on page size. A write request that is the size of a page and which is also aligned on the page size will be written to an NAND-flash physical page directly. A write request that is the size of a page but which is not aligned will require writing to two NAND-flash physical pages, and incur two read-modify-write operations [[53]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref). Therefore, it is critical to ensure that the partition used to write to an SSD is aligned with the size of the physical NAND-flash page of the drive used. Various guides and tutorials show how to align a partition to the parameters of an SSD when formatting [[54, 55]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref). A quick search on Google will generally reveal the sizes of the NAND-flash page, NAND-flash block and clustered block for a specific SSD model. And in case this information is not available, it is still possible to use some reverse engineering to uncover those parameters [[2, 3]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref).

It has been shown that performance improves significantly with partition alignment [[43]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref). It has also been shown in a test on one drive that by-passing the filesystem and writing directly to the drive improved performance, although the improvement was very tiny [[44]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref).

> ‎如第 3.1 节所述，写入操作根据页面大小对齐。页面大小且也与页面大小对齐的写入请求将直接写入 NAND 闪存物理页面。与页面大小相同但未对齐的写入请求将需要写入两个 NAND 闪存物理页，并产生两个读-修改-写入操作 ‎[‎[53]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。因此，确保用于写入 SSD 的分区与所用驱动器的物理 NAND 闪存页的大小一致至关重要。各种指南和教程展示了在格式化时如何将分区与SSD的参数对齐‎[‎[54，55]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。在Google上快速搜索通常会显示特定SSD型号的NAND闪存页面，NAND闪存块和集群块的大小。如果这些信息不可用，仍然可以使用一些逆向工程来发现这些参数‎[‎[2，3]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。‎
>
> ‎已经证明，分区对齐后性能显著提高 ‎[‎[43]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。在一个驱动器上的测试中也表明，绕过文件系统并直接写入驱动器可以提高性能，尽管改进非常小‎[‎[44]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。‎

#### Align the partition

To ensure that logical writes are truly aligned to the physical memory, you must align the partition to the NAND-flash page size of the drive.

> ‎要确保逻辑写入与物理内存真正对齐，必须将分区与驱动器的 NAND 闪存页面大小对齐。‎

### 8.2 Filesystem parameters

Not all filesystems support the TRIM command [[16]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref) as explained in Section 5.1. On Linux 2.6.33 and above, ext4 and XFS support TRIM, which still needs to be enabled using the `discard` parameter. From there, a few other tweaks are also to disable the updating of the metadata if it is not needed for anything, by removing the `relatime` parameter if present and adding `noatime,nodiratime` [[40, 55, 56, 57]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref).

> ‎并非所有文件系统都支持 TRIM 命令 ‎[‎[16]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎，如第 5.1 节所述。在 Linux 2.6.33 及更高版本上，ext4 和 XFS 支持 TRIM，这仍然需要使用参数来启用。从那里，其他一些调整也是通过删除参数（如果存在）并添加‎[‎[40，55，56，57]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎来禁用元数据的更新（如果不需要）。‎

#### Enable the TRIM command

Make sure your kernel and filesystem support the TRIM command. The TRIM command notifies the SSD controller when a block is deleted. The garbage collection process can then erase blocks in background during idle times, preparing the drive to face large writes workloads.

> ‎确保您的内核和文件系统支持 TRIM 命令。TRIM 命令在删除块时通知 SSD 控制器。然后，垃圾回收过程可以在空闲时间在后台擦除块，使驱动器准备好面对大型写入工作负载。‎

### 8.3 Operating system I/O scheduler

The default I/O scheduler on Linux is the CFQ scheduler (Completely Fair Queuing). CFQ was designed to minimize the seek latenties in spinning hard disk drives by grouping I/O requests that are physically close together. Such I/O request re-ordering is not necessary for SSDs as they have no mechanical parts. Various guides and discussions advocate that changing the I/O schedular from CFQ to NOOP or Deadline will reduce latencies on SSDs [[56, 58]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref). However since the version 3.1 of Linux, CFQ offers some optimizations for solid-state drives [[59]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref). Benchmarks are also are showing that the performance of the schedulers depends on the workload applied to an SSD (i.e. the application), and on the drive itself [[40, 60, 61, 62]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref).

My personal take on the matter is that unless the workload is very specific and that application-specific benchmarks are here to show the advantage of a scheduler over another, it’s a safe bet to stick to CFQ.

> ‎Linux 上的默认 I/O 调度程序是 CFQ 调度程序（完全公平排队）。CFQ 旨在通过将物理上靠近在一起的 I/O 请求分组来最大程度地减少旋转硬盘驱动器中的寻道潜移。对于 SSD，不需要此类 I/O 请求重新订购，因为它们没有机械部件。各种指南和讨论都主张将 I/O 调度从 CFQ 更改为 NOOP 或 Deadline 将减少 SSD 上的延迟 ‎[‎[56， 58]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。但是，从Linux的3.1版本开始，CFQ为固态硬盘提供了一些优化‎[‎[59]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。基准测试还表明，调度程序的性能取决于应用于SSD（即应用程序）的工作负载以及驱动器本身‎[‎[40，60，61，62]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎。‎
>
> ‎我个人对这个问题的看法是，除非工作负载非常具体，并且特定于应用程序的基准测试在这里显示调度程序相对于另一个调度程序的优势，否则坚持使用CFQ是一个安全的选择。‎

### 8.4 Swap

Due to the high numbers of I/O requests incurred by swapping pages to the drive, a swap partition on an SSD can increase the rate at which the drive wears off, and significantly reduce its lifespan. In the Linux kernel, the `vm.swappiness` parameter controls how often pages should be swapped to the drive. It can have a value between 0 and 100, 0 meaning that the kernel should avoid swapping as much as possible, and 100 meaning that the kernel should swap as much as possible. On Ubuntu for example, the default swappiness is 60. When using SSDs, reducing the swappiness to the lowest possible value, which is 0, will avoid incurring unnecessary writes to the drive and increase its lifespan [[56, 63]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref). Some guides recommend the value 1, which in practice is essentially the same as 0 [[57, 58]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref).

Other options are to use a RAM disk for swap, or to avoid swap altogether.

> ‎由于将页面交换到驱动器会产生大量 I/O 请求，因此 SSD 上的交换分区可以提高驱动器磨损的速率，并显著缩短其使用寿命。在 Linux 内核中，该参数 `vm.swappiness`控制将页面交换到驱动器的频率。它可以有一个介于 0 和 100 之间的值，0 表示内核应尽可能避免交换，100 表示内核应尽可能多地交换。例如，在 Ubuntu 上，默认交换率为 60。使用 SSD 时，将交换率降低到尽可能低的值（即 0）将避免对驱动器进行不必要的写入操作，并延长其使用寿命 ‎‎[56， 63]‎‎。一些指南建议使用值 1，这在实践中与 0 ‎‎[57， 58] 基本‎‎相同。‎
>
> ‎其他选项是使用 RAM 磁盘进行交换，或完全避免交换。‎

### 8.5 Temporary files

All temporary files and all log files that do not need to be persisted are wasting P/E cycles on SSDs. Such files can be stored into RAM using the tmpfs filesystem [[56, 57, 58]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref).

> ‎所有不需要保留的临时文件和所有日志文件都在 SSD 上浪费 P/E 周期。这些文件可以使用 tmpfs 文件系统 ‎[‎[56， 57， 58]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/#ref)‎ 存储到 RAM 中。‎

## What’s next

Part 6, which is summarizing the content from all the other parts, is available [here](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/). You can also go to the [Table of Content](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/) for this series of articles.

To receive a notification email every time a new article is posted on Code Capsule, you can subscribe to the newsletter by filling up the form at the top right corner of the blog.
As usual, comments are open at the bottom of this post, and I am always happy to welcome questions, corrections and contributions

> ‎第 6 部分总结了所有其他部分的内容，可‎[‎在此处‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/)‎找到。您还可以转到本系列文章的‎[‎目录‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)‎。‎
>
> ‎要在每次在 Code Capsule 上发布新文章时收到通知电子邮件，您可以通过填写博客右上角的表单来订阅新闻稿。‎‎像往常一样，评论在这篇文章的底部是开放的，我总是很乐意欢迎问题，更正和贡献！‎

## References

[1] [Understanding Intrinsic Characteristics and System Implications of Flash Memory based Solid State Drives, Chen et al., 2009](http://www.cse.ohio-state.edu/hpcs/WWW/HTML/publications/papers/TR-09-2.pdf)
[2] [Parameter-Aware I/O Management for Solid State Disks (SSDs), Kim et al., 2012](http://csl.skku.edu/papers/CS-TR-2010-329.pdf)
[3] [Essential roles of exploiting internal parallelism of flash memory based solid state drives in high-speed data processing, Chen et al, 2011](http://bit.csc.lsu.edu/~fchen/paper/papers/hpca11.pdf)
[4] [Exploring and Exploiting the Multilevel Parallelism Inside SSDs for Improved Performance and Endurance, Hu et al., 2013](http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=6165265)
[5] [Design Tradeoffs for SSD Performance, Agrawal et al., 2008](http://research.microsoft.com/pubs/63596/usenix-08-ssd.pdf)
[6] [Design Patterns for Tunable and Efficient SSD-based Indexes, Anand et al., 2012](http://instartlogic.com/resources/research_papers/aanand-tunable_efficient_ssd_indexes.pdf)
[7] [BPLRU: A Buffer Management Scheme for Improving Random Writes in Flash Storage, Kim et al., 2008](https://www.usenix.org/legacy/events/fast08/tech/full_papers/kim/kim.pdf)
[8] [SFS: Random Write Considered Harmful in Solid State Drives, Min et al., 2012](https://www.usenix.org/legacy/event/fast12/tech/full_papers/Min.pdf)
[9] [A Survey of Flash Translation Layer, Chung et al., 2009](http://idke.ruc.edu.cn/people/dazhou/Papers/AsurveyFlash-JSA.pdf)
[10] [A Reconfigurable FTL (Flash Translation Layer) Architecture for NAND Flash-Based Applications, Park et al., 2008](http://idke.ruc.edu.cn/people/dazhou/Papers/a38-park.pdf)
[11] [Reliably Erasing Data From Flash-Based Solid State Drives, Wei et al., 2011](https://www.usenix.org/legacy/event/fast11/tech/full_papers/Wei.pdf)
[12] [http://en.wikipedia.org/wiki/Solid-state_drive](http://en.wikipedia.org/wiki/Solid-state_drive)
[13] [http://en.wikipedia.org/wiki/Write_amplification](http://en.wikipedia.org/wiki/Write_amplification)
[14] [http://en.wikipedia.org/wiki/Flash_memory](http://en.wikipedia.org/wiki/Flash_memory)
[15] [http://en.wikipedia.org/wiki/Serial_ATA](http://en.wikipedia.org/wiki/Serial_ATA)
[16] [http://en.wikipedia.org/wiki/Trim_(computing)](http://en.wikipedia.org/wiki/Trim_(computing))
[17] [http://en.wikipedia.org/wiki/IOPS](http://en.wikipedia.org/wiki/IOPS)
[18] [http://en.wikipedia.org/wiki/Hard_disk_drive](http://en.wikipedia.org/wiki/Hard_disk_drive)
[19] [http://en.wikipedia.org/wiki/Hard_disk_drive_performance_characteristics](http://en.wikipedia.org/wiki/Hard_disk_drive_performance_characteristics)
[20] [http://centon.com/flash-products/chiptype](http://centon.com/flash-products/chiptype)
[21] [http://www.thessdreview.com/our-reviews/samsung-64gb-mlc-ssd/](http://www.thessdreview.com/our-reviews/samsung-64gb-mlc-ssd/)
[22] [http://www.anandtech.com/show/7594/samsung-ssd-840-evo-msata-120gb-250gb-500gb-1tb-review](http://www.anandtech.com/show/7594/samsung-ssd-840-evo-msata-120gb-250gb-500gb-1tb-review)
[23] [http://www.anandtech.com/show/6337/samsung-ssd-840-250gb-review/2](http://www.anandtech.com/show/6337/samsung-ssd-840-250gb-review/2)
[24] [http://www.storagereview.com/ssd_vs_hdd](http://www.storagereview.com/ssd_vs_hdd)
[25] [http://www.storagereview.com/wd_black_4tb_desktop_hard_drive_review_wd4003fzex](http://www.storagereview.com/wd_black_4tb_desktop_hard_drive_review_wd4003fzex)
[26] [http://www.storagereview.com/samsung_ssd_840_pro_review](http://www.storagereview.com/samsung_ssd_840_pro_review)
[27] [http://www.storagereview.com/micron_p420m_enterprise_pcie_ssd_review](http://www.storagereview.com/micron_p420m_enterprise_pcie_ssd_review)
[28] [http://www.storagereview.com/intel_x25-m_ssd_review](http://www.storagereview.com/intel_x25-m_ssd_review)
[29] [http://www.storagereview.com/seagate_momentus_xt_750gb_review](http://www.storagereview.com/seagate_momentus_xt_750gb_review)
[30] [http://www.storagereview.com/corsair_vengeance_ddr3_ram_disk_review](http://www.storagereview.com/corsair_vengeance_ddr3_ram_disk_review)
[31] [http://arstechnica.com/information-technology/2012/06/inside-the-ssd-revolution-how-solid-state-disks-really-work/](http://arstechnica.com/information-technology/2012/06/inside-the-ssd-revolution-how-solid-state-disks-really-work/)
[32] [http://www.anandtech.com/show/2738](http://www.anandtech.com/show/2738)
[33] [http://www.anandtech.com/show/2829](http://www.anandtech.com/show/2829)
[34] [http://www.anandtech.com/show/6489](http://www.anandtech.com/show/6489)
[35] [http://lwn.net/Articles/353411/](http://lwn.net/Articles/353411/)
[36] [http://us.hardware.info/reviews/4178/10/hardwareinfo-tests-lifespan-of-samsung-ssd-840-250gb-tlc-ssd-updated-with-final-conclusion-final-update-20-6-2013](http://us.hardware.info/reviews/4178/10/hardwareinfo-tests-lifespan-of-samsung-ssd-840-250gb-tlc-ssd-updated-with-final-conclusion-final-update-20-6-2013)
[37] [http://www.anandtech.com/show/6489/playing-with-op](http://www.anandtech.com/show/6489/playing-with-op)
[38] [http://www.ssdperformanceblog.com/2011/06/intel-320-ssd-random-write-performance/](http://www.ssdperformanceblog.com/2011/06/intel-320-ssd-random-write-performance/)
[39] [http://en.wikipedia.org/wiki/Native_Command_Queuing](http://en.wikipedia.org/wiki/Native_Command_Queuing)
[40] [http://superuser.com/questions/228657/which-linux-filesystem-works-best-with-ssd/](http://superuser.com/questions/228657/which-linux-filesystem-works-best-with-ssd/)
[41] [http://blog.superuser.com/2011/05/10/maximizing-the-lifetime-of-your-ssd/](http://blog.superuser.com/2011/05/10/maximizing-the-lifetime-of-your-ssd/)
[42] [http://serverfault.com/questions/356534/ssd-erase-block-size-lvm-pv-on-raw-device-alignment](http://serverfault.com/questions/356534/ssd-erase-block-size-lvm-pv-on-raw-device-alignment)
[43] [http://rethinkdb.com/blog/page-alignment-on-ssds/](http://rethinkdb.com/blog/page-alignment-on-ssds/)
[44] [http://rethinkdb.com/blog/more-on-alignment-ext2-and-partitioning-on-ssds/](http://rethinkdb.com/blog/more-on-alignment-ext2-and-partitioning-on-ssds/)
[45] [http://rickardnobel.se/storage-performance-iops-latency-throughput/](http://rickardnobel.se/storage-performance-iops-latency-throughput/)
[46] [http://www.brentozar.com/archive/2013/09/iops-are-a-scam/](http://www.brentozar.com/archive/2013/09/iops-are-a-scam/)
[47] [http://www.acunu.com/2/post/2011/08/why-theory-fails-for-ssds.html](http://www.acunu.com/2/post/2011/08/why-theory-fails-for-ssds.html)
[48] [http://security.stackexchange.com/questions/12503/can-wiped-ssd-data-be-recovered](http://security.stackexchange.com/questions/12503/can-wiped-ssd-data-be-recovered)
[49] [http://security.stackexchange.com/questions/5662/is-it-enough-to-only-wipe-a-flash-drive-once](http://security.stackexchange.com/questions/5662/is-it-enough-to-only-wipe-a-flash-drive-once)
[50] [http://searchsolidstatestorage.techtarget.com/feature/The-truth-about-SSD-performance-benchmarks](http://searchsolidstatestorage.techtarget.com/feature/The-truth-about-SSD-performance-benchmarks)
[51] [http://www.theregister.co.uk/2012/12/03/macronix_thermal_annealing_extends_life_of_flash_memory/](http://www.theregister.co.uk/2012/12/03/macronix_thermal_annealing_extends_life_of_flash_memory/)
[52] [http://www.eecs.berkeley.edu/~rcs/research/interactive_latency.html](http://www.eecs.berkeley.edu/~rcs/research/interactive_latency.html)
[53] [http://blog.nuclex-games.com/2009/12/aligning-an-ssd-on-linux/](http://blog.nuclex-games.com/2009/12/aligning-an-ssd-on-linux/)
[54] [http://www.linux-mag.com/id/8397/](http://www.linux-mag.com/id/8397/)
[55] [http://tytso.livejournal.com/2009/02/20/](http://tytso.livejournal.com/2009/02/20/)
[56] [https://wiki.debian.org/SSDOptimization](https://wiki.debian.org/SSDOptimization)
[57] [http://wiki.gentoo.org/wiki/SSD](http://wiki.gentoo.org/wiki/SSD)
[58] [https://wiki.archlinux.org/index.php/Solid_State_Drives](https://wiki.archlinux.org/index.php/Solid_State_Drives)
[59] [https://www.kernel.org/doc/Documentation/block/cfq-iosched.txt](https://www.kernel.org/doc/Documentation/block/cfq-iosched.txt)
[60] [http://www.danielscottlawrence.com/blog/should_i_change_my_disk_scheduler_to_use_NOOP.html](http://www.danielscottlawrence.com/blog/should_i_change_my_disk_scheduler_to_use_NOOP.html)
[61] [http://www.phoronix.com/scan.php?page=article&amp;item=linux_iosched_2012](http://www.phoronix.com/scan.php?page=article&item=linux_iosched_2012)
[62] [http://www.velobit.com/storage-performance-blog/bid/126135/Effects-Of-Linux-IO-Scheduler-On-SSD-Performance](http://www.velobit.com/storage-performance-blog/bid/126135/Effects-Of-Linux-IO-Scheduler-On-SSD-Performance)
[63] [http://www.axpad.com/blog/301](http://www.axpad.com/blog/301)
[64] [http://en.wikipedia.org/wiki/List_of_solid-state_drive_manufacturers](http://en.wikipedia.org/wiki/List_of_solid-state_drive_manufacturers)
[65] [http://en.wikipedia.org/wiki/List_of_flash_memory_controller_manufacturers](http://en.wikipedia.org/wiki/List_of_flash_memory_controller_manufacturers)
[66] [http://blog.zorinaq.com/?e=29](http://blog.zorinaq.com/?e=29)
[67] [http://www.gamersnexus.net/guides/956-how-ssds-are-made](http://www.gamersnexus.net/guides/956-how-ssds-are-made)
[68] [http://www.gamersnexus.net/guides/1148-how-ram-and-ssds-are-made-smt-lines](http://www.gamersnexus.net/guides/1148-how-ram-and-ssds-are-made-smt-lines)
[69] [http://www.tweaktown.com/articles/4655/kingston_factory_tour_making_of_an_ssd_from_start_to_finish/index.html](http://www.tweaktown.com/articles/4655/kingston_factory_tour_making_of_an_ssd_from_start_to_finish/index.html)
[70] [http://www.youtube.com/watch?v=DvA9koAMXR8](http://www.youtube.com/watch?v=DvA9koAMXR8)
[71] [http://www.youtube.com/watch?v=3s7KG6QwUeQ](http://www.youtube.com/watch?v=3s7KG6QwUeQ)
[72] [Understanding the Robustness of SSDs under Power Fault, Zheng et al., 2013](https://www.usenix.org/conference/fast13/technical-sessions/presentation/zheng) — [[discussion on HN]](https://news.ycombinator.com/item?id=7047118)
[73] [http://lkcl.net/reports/ssd_analysis.html](http://lkcl.net/reports/ssd_analysis.html) — [[discussion on HN]](https://news.ycombinator.com/item?id=6973179)
