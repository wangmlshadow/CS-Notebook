# Coding for SSDs – Part 6: A Summary – What every programmer should know about solid-state drives

This is Part 6 over 6 of “Coding for SSDs”. For other parts and sections, you can refer to the [Table to Contents](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/). This is a series of articles that I wrote to share what I learned while documenting myself on SSDs, and on how to make code perform well on SSDs.

In this part, I am summarizing the content from all the other parts in the form of concise self-contained paragraphs. Each paragraph is referencing one or more sections of the other parts, which allow to get more detailed information regarding each topic.

> ‎这是“SSD 编码”的第 6 部分，而不是第 6 部分。对于其他部分，可以参考‎[‎目录‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)‎。这是我写的一系列文章，旨在分享我在SSD上记录自己时学到的东西，以及如何使代码在SSD上表现良好。‎
>
> ‎在这一部分中，我以简明扼要的独立段落的形式总结了所有其他部分的内容。每个段落都引用了其他部分的一个或多个部分，从而可以获取有关每个主题的更多详细信息。‎

## Basics

### 1. Memory cell types

A solid-state drives (SSD) is a flash-memory based data storage device. Bits are stored into cells, which exist in three types: 1 bit per cell (single level cell, SLC), 2 bits per cell (multiple level cell, MLC), 3 bits per cell (triple-level cell, TLC).

> ‎固态硬盘 （SSD） 是一种基于闪存的数据存储设备。位存储在单元中，这些单元存在三种类型：每个单元1位（单级单元，SLC），每个单元2位（多级单元，MLC），每个单元3位（三级单元，TLC）。‎

See also: [Section 1.1](http://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/)

### 2. Limited lifespan

Each cell has a maximum number of P/E cycles (Program/Erase), after which the cell is considered defective. This means that NAND-flash memory wears off and has a limited lifespan.

> ‎每个单元都有最大 P/E 周期数（编程/擦除），之后该单元将被视为损坏。这意味着NAND闪存会磨损并且寿命有限。‎

See also: [Section 1.1](http://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/)

### 3. Benchmarking is hard

Testers are humans, therefore not all benchmarks are exempt of errors. Be careful when reading the benchmarks from manufacturers or third parties, and use multiple sources before trusting any numbers. Whenever possible, run your own in-house benchmarking using the specific workload of your system, along with the specific SSD model that you want to use. Finally, make sure you look at the performance metrics that matter most for the system at hand.

> ‎测试人员是人类，因此并非所有基准测试都可以免于错误。在阅读制造商或第三方的基准时要小心，并在信任任何数字之前使用多个来源。只要有可能，就使用系统的特定工作负载以及要使用的特定SSD型号运行自己的内部基准测试。最后，请务必查看对手头系统最重要的性能指标。‎

See also: [Sections 2.2 and 2.3](http://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/)

## Pages and blocks

### 4. NAND-flash pages and blocks

Cells are grouped into a grid, called a block, and blocks are grouped into planes. The smallest unit through which a block can be read or written is a page. Pages cannot be erased individually, only whole blocks can be erased. The size of a NAND-flash page size can vary, and most drive have pages of size 2 KB, 4 KB, 8 KB or 16 KB. Most SSDs have blocks of 128 or 256 pages, which means that the size of a block can vary between 256 KB and 4 MB. For example, the Samsung SSD 840 EVO has blocks of size 2048 KB, and each block contains 256 pages of 8 KB each.

> ‎单元格被分组到一个网格中，称为块，块被分组到平面中。可以读取或写入块的最小单位是页面。页面不能单独擦除，只能擦除整个块。NAND 闪存页面大小的大小可能会有所不同，并且大多数驱动器的页面大小为 2 KB、4 KB、8 KB 或 16 KB。大多数 SSD 的块为 128 或 256 页，这意味着块的大小可以在 256 KB 到 4 MB 之间变化。例如，三星SSD 840 EVO具有大小为2048 KB的块，每个块包含256页，每个页面8 KB。‎

See also: [Section 3.2](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 5. Reads are aligned on page size

It is not possible to read less than one page at once. One can of course only request just one byte from the operating system, but a full page will be retrieved in the SSD, forcing a lot more data to be read than necessary.

> ‎一次读少于一页是不可能的。当然，人们能从操作系统请求一个字节，但是在SSD中将检索到一整页，从而强制读取比必要更多的数据。‎

See also: [Section 3.2](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 6. Writes are aligned on page size

When writing to an SSD, writes happen by increments of the page size. So even if a write operation affects only one byte, a whole page will be written anyway. Writing more data than necessary is known as write amplification. Writing to a page is also called “to program” a page.

> ‎写入 SSD 时，写入按页面大小的增量进行。因此，即使写入操作仅影响一个字节，也会写入整个页面。写入超出必要数量的数据称为写入放大。写入页面也称为“编程”页面。‎

See also: [Section 3.2](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 7. Pages cannot be overwritten

A NAND-flash page can be written to only if it is in the “free” state. When data is changed, the content of the page is copied into an internal register, the data is updated, and the new version is stored in a “free” page, an operation called “read-modify-write”. The data is not updated in-place, as the “free” page is a different page than the page that originally contained the data. Once the data is persisted to the drive, the original page is marked as being “stale”, and will remain as such until it is erased.

> ‎只有当NAND闪存页面处于“空闲”状态时，才能写入该页面。当数据更改时，页面的内容被复制到内部寄存器中，数据被更新，新版本存储在“空闲”页面中，这种操作称为“读-修改-写”。数据不会就地更新，因为“免费”页面与最初包含数据的页面不同。一旦数据保存到驱动器，原始页面将被标记为“过时”，并将保持原样，直到它被擦除。‎

See also: [Section 3.2](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 8. Erases are aligned on block size

Pages cannot be overwritten, and once they become stale, the only way to make them free again is to erase them. However, it is not possible to erase individual pages, and it is only possible to erase whole blocks at once.

> ‎页面不能被覆盖，一旦它们变得陈旧，使它们再次自由的唯一方法就是擦除它们。但是，无法擦除单个页面，并且只能一次擦除整个块。‎

See also: [Section 3.2](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

## SSD controller and internals

### 9. Flash Translation Layer

The Flash Translation Layer (FTL) is a component of the SSD controller which maps Logical Block Addresses (LBA) from the host to Physical Block Addresses (PBA) on the drive. Most recent drives implement an approach called “hybrid log-block mapping” or one of its derivatives, which works in a way that is similar to log-structured file systems. This allows random writes to be handled like sequential writes.

> ‎闪存转换层 （FTL） 是 SSD 控制器的一个组件，它将主机上的逻辑块地址 （LBA） 映射到驱动器上的物理块地址 （PBA）。最新的驱动器实现了一种称为“混合日志块映射”或其衍生方法之一的方法，其工作方式类似于日志结构化文件系统。这允许像顺序写入一样处理随机写入。‎

See also: [Section 4.2](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 10. Internal parallelism

Internally, several levels of parallelism allow to write to several blocks at once into different NAND-flash chips, to what is called a “clustered block”.

> ‎在内部，多个级别的并行性允许将多个块同时写入不同的NAND闪存芯片，即所谓的“集群块”。‎

See also: [Section 6](http://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/)

### 11. Wear leveling

Because NAND-flash cells are wearing off, one of the main goals of the FTL is to distribute the work among cells as evenly as possible so that blocks will reach their P/E cycle limit and wear off at the same time.

> ‎由于NAND闪存单元正在磨损，FTL的主要目标之一是尽可能均匀地在单元之间分配工作，以便块将达到其P / E周期极限并同时磨损。‎

See also: [Section 3.4](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 12. Garbage collection

The garbage collection process in the SSD controller ensures that “stale” pages are erased and restored into a “free” state so that the incoming write commands can be processed.

> ‎SSD 控制器中的垃圾回收过程可确保擦除“陈旧”页面并将其还原为“空闲”状态，以便可以处理传入的写入命令。‎

See also: [Section 4.4](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 13. Background operations can affect foreground operations

Background operations such as garbage collection can impact negatively on foreground operations from the host, especially in the case of a sustained workload of small random writes.

> ‎后台操作（如垃圾回收）可能会对主机的前台操作产生负面影响，尤其是在持续工作负荷为小随机写入的情况下。‎

See also: [Section 4.4](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

## Access patterns

### 14. Never write less than a page

Avoid writing chunks of data that are below the size of a NAND-flash page to minimize write amplification and prevent read-modify-write operations. The largest size for a page at the moment is 16 KB, therefore it is the value that should be used by default. This size depends on the SSD models and you may need to increase it in the future as SSDs improve.

> ‎避免写入低于 NAND 闪存页大小的数据块，以最大程度地减少写入放大并防止读取-修改-写入操作。目前页面的最大大小为 16 KB，因此默认情况下应使用该值。此大小取决于 SSD 型号，随着 SSD 的改进，将来可能需要增加它。‎

See also: [Sections 3.2 and 3.3](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 15. Align writes

Align writes on the page size, and write chunks of data that are multiple of the page size.

> ‎根据页面大小对齐写入，并写入页面大小的倍数数据块。‎

See also: [Sections 3.2 and 3.3](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 16. Buffer small writes

To maximize throughput, whenever possible keep small writes into a buffer in RAM and when the buffer is full, perform a single large write to batch all the small writes.

> ‎为了最大限度地提高吞吐量，请尽可能将小写入操作保留在 RAM 中的缓冲区中，当缓冲区已满时，执行一次大写入操作以对所有小写入进行批处理。‎

See also: [Sections 3.2 and 3.3](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 17. To improve the read performance, write related data together

Read performance is a consequence of the write pattern. When a large chunk of data is written at once, it is spread across separate NAND-flash chips. Thus you should write related data in the same page, block, or clustered block, so it can later be read faster with a single I/O request, by taking advantage of the internal parallelism.

> ‎读取性能是写入模式的结果。当一次写入大量数据时，它会分布在单独的NAND闪存芯片上。因此，您应该在同一页、块或集群块中写入相关数据，以便以后通过利用内部并行性，通过单个 I/O 请求更快地读取这些数据。‎

See also: [Section 7.3](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/)

### 18. Separate read and write requests

A workload made of a mix of small interleaved reads and writes will prevent the internal caching and readahead mechanism to work properly, and will cause the throughput to drop. It is best to avoid simultaneous reads and writes, and perform them one after the other in large chunks, preferably of the size of the clustered block. For example, if 1000 files have to be updated, you could iterate over the files, doing a read and write on a file and then moving to the next file, but that would be slow. It would be better to reads all 1000 files at once and then write back to those 1000 files at once.

> ‎由小型交错读取和写入混合组成的工作负荷将阻止内部缓存和预读机制正常工作，并将导致吞吐量下降。最好避免同时读取和写入，并以大块的形式一个接一个地执行它们，最好是簇状块的大小。例如，如果必须更新 1000 个文件，则可以循环访问这些文件，对文件执行读取和写入操作，然后移动到下一个文件，但速度会很慢。最好一次读取所有1000个文件，然后一次写回这1000个文件。‎

See also: [Section 7.4](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/)

### 19. Invalidate obsolete data in batch

When some data is no longer needed or need to be deleted, it is better to wait and invalidate it in a large batches in a single operation. This will allow the garbage collector process to handle larger areas at once and will help minimizing internal fragmentation.

> ‎当不再需要或需要删除某些数据时，最好等待并在单个操作中大批量使其失效。这将允许垃圾回收器进程一次处理更大的区域，并将有助于最大限度地减少内部碎片。‎

See also: [Section 4.4](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 20. Random writes are not always slower than sequential writes

If the writes are small (i.e. below the size of the clustered block), then random writes are slower than sequential writes.
If writes are both multiple of and aligned to the size of a clustered block, the random writes will use all the available levels of internal parallelism, and will perform just as well as sequential writes. For most drives, the clustered block has a size of 16 MB or 32 MB, therefore it is safe to use 32 MB.

> ‎如果写入很小（即小于簇状块的大小），则随机写入比顺序写入慢。‎
> ‎如果写入操作既是簇状块的倍数又与之大小对齐，则随机写入将使用所有可用的内部并行度级别，并且性能与顺序写入一样好。对于大多数驱动器，群集块的大小为 16 MB 或 32 MB，因此使用 32 MB 是安全的。‎

See also: [Section 7.2](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/)

### 21. A large single-threaded read is better than many small concurrent reads

Concurrent random reads cannot fully make use of the readahead mechanism. In addition, multiple Logical Block Addresses may end up on the same chip, not taking advantage or of the internal parallelism. A large read operation will access sequential addresses and will therefore be able to use the readahead buffer if present, and use the internal parallelism. Consequently if the use case allows it, it is better to issue a large read request.

> ‎并发随机读取不能完全利用预读机制。此外，多个逻辑块地址可能最终位于同一芯片上，不会利用内部并行性。大型读取操作将访问顺序地址，因此将能够使用预读缓冲区（如果存在）并使用内部并行度。因此，如果用例允许，最好发出一个大的读取请求。‎

See also: [Section 7.3](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/)

### 22. A large single-threaded write is better than many small concurrent writes

A large single-threaded write request offers the same throughput as many small concurrent writes, however in terms of latency, a large single write has a better response time than concurrent writes. Therefore, whenever possible, it is best to perform single-threaded large writes.

> ‎大型单线程写入请求提供与许多小型并发写入相同的吞吐量，但在延迟方面，大型单次写入的响应时间优于并发写入。因此，只要有可能，最好执行单线程大型写入。‎

See also: [Section 7.2](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/)

### 23. When the writes are small and cannot be grouped or buffered, multi-threading is beneficial

Many concurrent small write requests will offer a better throughput than a single small write request. So if the I/O is small and cannot be batched, it is better to use multiple threads.

> ‎许多并发小型写入请求将提供比单个小型写入请求更好的吞吐量。因此，如果 I/O 很小且无法批处理，则最好使用多个线程。‎

See also: [Section 7.2](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/)

### 24. Split cold and hot data

Hot data is data that changes frequently, and cold data is data that changes infrequently. If some hot data is stored in the same page as some cold data, the cold data will be copied along every time the hot data is updated in a read-modify-write operation, and will be moved along during garbage collection for wear leveling. Splitting cold and hot data as much as possible into separate pages will make the job of the garbage collector easier.

> ‎热数据是频繁更改的数据，冷数据是不经常更改的数据。如果某些热数据与某些冷数据存储在同一页面中，则每次在读-修改-写操作中更新热数据时，冷数据都会被复制，并在垃圾回收期间移动以进行磨损均衡。将冷数据和热数据尽可能多地拆分到单独的页面中将使垃圾回收器的工作更容易。‎

See also: [Section 4.4](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

### 25. Buffer hot data

Extremely hot data and other high-change metadata should be buffered as much as possible and written to the drive as infrequently as possible.

> ‎应尽可能多地缓冲极热的数据和其他高变化的元数据，并尽可能少地写入驱动器。‎

See also: [Section 4.4](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

## System optimizations

### 26. PCI Express and SAS are faster than SATA

The two main host interfaces offered by manufacturers are SATA 3.0 (550 MB/s) and PCI Express 3.0 (1 GB/s per lane, using multiple lanes). Serial Attached SCSI (SAS) is also available for enterprise SSDs. In their latest versions, PCI Express and SAS are faster than SATA, but they are also more expensive.

> ‎制造商提供的两个主要主机接口是 SATA 3.0 （550 MB/s） 和 PCI Express 3.0（每通道 1 GB/s，使用多个通道）。串行连接 SCSI （SAS） 也可用于企业 SSD。在最新版本中，PCI Express和SAS比SATA更快，但它们也更昂贵。‎

See also: [Section 2.1](http://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/)

### 27. Over-provisioning is useful for wear leveling and performance

A drive can be over-provisioned simply by formatting it to a logical partition capacity smaller than the maximum physical capacity. The remaining space, invisible to the user, will still be visible and used by the SSD controller. Over-provisioning helps the wear leveling mechanisms to cope with the inherent limited lifespan of NAND-flash cells. For workloads in which writes are not so heavy, 10% to 15% of over-provisioning is enough. For workloads of sustained random writes, keeping up to 25% of over-provisioning will improve performance. The over-provisioning will act as a buffer of NAND-flash blocks, helping the garbage collection process to absorb peaks of writes.

> ‎只需将驱动器格式化为小于最大物理容量的逻辑分区容量，即可对其进行过度置备。对用户不可见的剩余空间仍将可见并由 SSD 控制器使用。过度配置有助于磨损均衡机制应对NAND闪存单元固有的有限寿命。对于写入量不大的工作负载，10% 到 15% 的过度预配就足够了。对于持续随机写入的工作负载，保留高达 25% 的过度预配将提高性能。过度配置将充当 NAND 闪存块的缓冲区，帮助垃圾回收过程吸收写入峰值。‎

See also: [Section 5.2](http://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/)

### 28. Enable the TRIM command

Make sure your kernel and filesystem support the TRIM command. The TRIM command notifies the SSD controller when a block is deleted. The garbage collection process can then erase blocks in background during idle times, preparing the drive to face large writes workloads.

> ‎确保您的内核和文件系统支持 TRIM 命令。TRIM 命令在删除块时通知 SSD 控制器。然后，垃圾回收过程可以在空闲时间在后台擦除块，使驱动器准备好面对大型写入工作负载。‎

See also: [Section 5.1](http://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/)

### 29. Align the partition

To ensure that logical writes are truly aligned to the physical memory, you must align the partition to the NAND-flash page size of the drive.

> ‎要确保逻辑写入与物理内存真正对齐，必须将分区与驱动器的 NAND 闪存页面大小对齐。‎

See also: [Section 8.1](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/)

## Conclusion

This summary concludes the “Coding for SSDs” article series. I hope that I was able to convey in an understandable manner what I have learned during my personal research over solid-state drives.

If after reading this series of articles you want to go more in-depth about SSDs, a good first step would be to read some of the publications and articles linked in the reference sections of Part 2 to 5.

Another great resource is the FAST conference (the USENIX Conference on File and Storage Technologies). A lot of excellent research is being presented there every year. I highly recommend their website, a good starting point being the videos and publications for [FAST 2013](https://www.usenix.org/conference/fast13).

> ‎本摘要总结了“SSD 编码”系列文章。我希望我能够以一种可以理解的方式传达我在个人研究固态硬盘时所学到的知识。‎
>
> ‎如果在阅读本系列文章后，您想更深入地了解SSD，那么第一步就是阅读第2部分至第5部分参考部分中链接的一些出版物和文章。‎
>
> ‎另一个很好的资源是FAST会议（USENIX文件和存储技术会议）。每年都有很多优秀的研究在那里展出。我强烈推荐他们的网站，一个很好的起点是‎[‎FAST 2013‎](https://www.usenix.org/conference/fast13)‎的视频和出版物。‎
