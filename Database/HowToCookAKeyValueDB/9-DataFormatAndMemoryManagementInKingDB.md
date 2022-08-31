# Implementing a Key-Value Store – Part 9: Data Format and Memory Management in KingDB

This is Part 9 of the IKVS series, “Implementing a Key-Value Store”. You can also check the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) for other parts. In this series of articles, I describe the research and process through which I am implementing a key-value database, which I have named “KingDB”. The source code is available at [http://kingdb.org](http://kingdb.org/). Please note that you do not need to read the previous parts to be able to follow. The previous parts were mostly exploratory, and starting with [Part 8](http://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/) is perfectly fine.

In this article, I explain how the storage engine of KingDB works, including details about the data format. I also cover how memory management is done through the use of a compaction process.

> ‎这是 IKVS 系列的第 9 部分“实现键值存储”。您还可以在‎[‎目录中‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎查看其他部分。在本系列文章中，我将介绍实现键值数据库的研究和过程，并将其命名为“KingDB”。源代码可在 ‎[‎http://kingdb.org‎](http://kingdb.org/)‎ 获得。请注意，您无需阅读前面的部分即可阅读本部分。前面的部分大多是探索性的，从‎[‎第8部分‎](http://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/)‎开始是完全可以的。‎
>
> ‎在本文中，我将解释KingDB的存储引擎的工作原理，包括有关数据格式的详细信息。我还介绍了如何使用压缩过程来完成内存管理。

## 1. Single-file vs. multiple-file storage engines

When I started to work on KingDB, one of the important design decision that I had to make was to choose between a single-file database or a multiple-file database. The first difference between the two solutions is the number of inodes and system calls needed. With a single-file database, only a single inode and a single call to open() are needed. With a multiple-file database, obviously more inodes are needed, every read might require that an open() system call be done, and the kernel will need to maintain more file descriptors. Because system calls are expensive, my first design iteration was in the direction of the single-file design.

I took a pile of white paper sheets, and I started to draw random sequences of write and delete operations, sketching what the database would be, and what a compaction process would look like for that single file. This is when I became aware of a few interesting cases. For example, imagine that a very large entry is written right in the middle of that single-file database, and that later on, a delete operation was emitted for that same entry: the disk space used by that entry will have to be reclaimed somehow. One option for that is to shift up the rest of the file: that operation is going to be very costly if the database file is very big, thus it is not feasible. Another option is to mark the space previously used by the deleted entry as “free” space, so that incoming writes could use it. With this option, the storage engine would need to create blocks of memory, allocate those blocks for entries, and keep track of which of these blocks are used and which are free. So essentially, the storage engine would have to implement a memory allocator or a file system. Ouch! Having to implement a memory management system was going to add a lot of complexity to the code, and this is exactly what I wanted to avoid in my design. At this point, I came to another realization: why implement a file system when the operating system is giving you one? And so I started thinking about how I could solve this storage problem using a multiple-file approach.

With multiple files, the compaction process is going to be greatly simplified: just combine uncompacted files, the ones that have deleted or overlapping entries, and store them into new compacted files. Then simply delete the uncompacted files, and the file system will reclaim the free disk space. And when some extra space is needed, the program just has to create a new file, and the file system takes care of everything: allocation, fragmentation, free space management, etc. Keeping in mind that KingDB needs to handle small entries just as well as large entries, I still had to find what the data format for this multiple-file database would be like. After a few iterations, I reached a solution that allows for all entry sizes to be handled properly, while still keeping the overall design simple. One important detail: most file systems do poorly with many small files, so the multiple-file design would need to be made such that the size of those files could be parametrized so they can be made large enough to be made efficient for a file system.

> 当我开始在 KingDB 上工作时，我必须做出的重要设计决策之一就是在单文件数据库或多文件数据库之间进行选择。这两种解决方案之间的第一个区别是所需的inode和系统调用的数量。对于单文件数据库，只需要一个 inode 和对 open（） 的一次调用。对于多文件数据库，显然需要更多的inode，每次读取都可能需要执行open（）系统调用，并且内核需要维护更多的文件描述符。由于系统调用成本高昂，我的第一次设计迭代是朝着单文件设计的方向进行的。
>
> ‎我拿了一堆白纸，开始绘制随机的写入和删除操作序列，绘制数据库的草图，以及单个文件的压缩过程。这时，我意识到了一些有趣的案例。例如，假设一个非常大的条目直接写入该单文件数据库的中间，并且稍后对同一条目发出了删除操作：必须以某种方式回收该条目使用的磁盘空间。一种选择是向上移动文件的其余部分：如果数据库文件非常大，则该操作将非常昂贵，因此不可行。另一种选择是将已删除条目以前使用的空间标记为“可用”空间，以便传入的写入可以使用它。使用此选项，存储引擎需要创建内存块，为条目分配这些块，并跟踪哪些块被使用，哪些是空闲的。因此，从本质上讲，存储引擎必须实现内存分配器或文件系统。哎哟！必须实现内存管理系统会给代码增加很多复杂性，而这正是我在设计中想要避免的。在这一点上，我得出了另一个认识：当操作系统给你一个文件系统时，为什么要实现一个文件系统？因此，我开始考虑如何使用多文件方法解决此存储问题。‎
>
> 使用多个文件，压缩过程将大大简化：只需合并未压缩的文件，已删除或重叠条目的文件，并将其存储到新的压缩文件中即可。然后只需删除未压缩的文件，文件系统将回收可用磁盘空间。当需要一些额外的空间时，程序只需要创建一个新文件，文件系统就会处理所有事情：分配，碎片，可用空间管理等。请记住，KingDB 需要处理小条目和大条目，我仍然必须找到这个多文件数据库的数据格式。经过几次迭代，我达成了一个解决方案，该解决方案允许正确处理所有条目大小，同时仍然保持整体设计简单。一个重要的细节：大多数文件系统在许多小文件上表现不佳，因此需要进行多文件设计，以便可以参数化这些文件的大小，以便它们可以变得足够大，以便对文件系统有效。

## 2. Data Format: Hashed String Tables (HSTables)

The data format used by KingDB is the Hashed String Table, or HSTable. Don’t google that term, you won’t find anything about it: I developed this data format and made up its name as I designed KingDB. It’s just a data format that stores key-value type binary data.

Each HSTable file starts with a header at position 0, and the first 8192 bytes are reserved for the header. The entries start at position 8192, and are stored contiguously. Each entry starts with an Entry Header that contains some metadata, and is followed by the sequence of bytes for the key and the value of the entry. At the end of each HSTable, an Offset Array stores a compact representation of the entries stored in the HSTable: for each entry in the HSTable, the Offset Array has one row which is the hashed key of that entry, and the offset where the entry can be found in the file. The Offset Array can be used to quickly build a hash table in memory, mapping hashed keys to locations in HSTables.

Here is what a typical HSTable looks like:

> ‎KingDB 使用的数据格式是 Hashed String Table 或 HSTable。不要谷歌这个术语，你不会找到任何关于它的信息：我开发了这种数据格式，并在我设计KingDB时编造了它的名字。它只是一种存储键值类型二进制数据的数据格式。‎
>
> ‎每个 HSTable 文件都以位于位置 0 的标头开头，前 8192 个字节保留给标头。这些条目从位置 8192 开始，并连续存储。每个条目都以包含一些元数据的条目标头开头，后跟键的字节序列和条目的值。在每个 HSTable 的末尾，偏移数组存储存储在 HSTable 中的条目的紧凑表示形式：对于 HSTable 中的每个条目，Offset Array 都有一行，该行是该条目的哈希键，以及可在文件中找到该条目的偏移量。偏移数组可用于在内存中快速构建哈希表，将哈希键映射到 HSTables 中的位置。‎
>
> ‎以下是典型的HSTable的样子：‎

```
[HSTableHeader]
[zero padding until byte 8192]
[EntryHeader for entry 0] 
   [byte sequence for the key of entry 0]
   [byte sequence for the value of entry 0]
   [zero padding for the value of entry 0 if compression enabled]
   ...
[EntryHeader for entry N-1] 
[byte sequence for the key of entry N-1]
[byte sequence for the value of entry N-1]
[zero padding for the value of entry N-1 if compression enabled]
[Row 0 of the OffsetArray: hashed key of entry 0 and offset to entry 0]
...
[Row N-1 of the OffsetArray: hashed key of entry N-1 and offset to entry N-1]
[HSTableFooter] 
```

And below is the content of the various parts of an HSTable presented above:

> ‎以下是上面介绍的HSTable各个部分的内容：‎

<pre class="wp-block-preformatted"><b>/* HSTableHeader */</b>

uint32_t checksum;                     // 32-bit checksum for the HSTable header
uint32_t version_major;                // Data format version (major)
uint32_t version_minor;                // Data format version (minor)
uint32_t version_revision;             // Data format version (revision)
uint32_t version_build;                // Data format version (build)
uint32_t version_data_format_major;    // Data format version (major)
uint32_t version_data_format_minor;    // Data format version (minor)
uint32_t filetype;                     // whether the file is regular or large,
                                       // and whether or not it was already compacted
uint64_t timestamp;                    // Timestamp of the HSTable, i.e. the order in which this
                                       // HSTable must be considered to guarantee the order
                                       // of operations in the database
</pre>

<pre class="wp-block-preformatted"><b>/* EntryHeader */</b>

uint32_t checksum_header;        // Checksum for the EntryHeader
uint32_t checksum_content;       // Checksum for the sequence of bytes across entry key and value data
uint32_t flags;                  // Flags, such as: kTypeRemove, kHasPadding, and kEntryFull
uint64_t size_key;               // Size of the key (in bytes)
uint64_t size_value;             // Size of the value when uncompressed (in bytes)
uint64_t size_value_compressed;  // Size of the value when compressed,
                                 // or 0 if compression is disabled (in bytes)
uint64_t size_padding;           // Size of the padding at the end of the entry, if any (in bytes)
uint64_t hash;                   // Hashed key
</pre>

<pre class="wp-block-preformatted"><b>/* OffsetArray Row */</b>

uint64_t hashed_key;      // hashed key of the entry
uint32_t offset_entry;    // offset where the entry can be found in the HSTable 
</pre>

<pre class="wp-block-preformatted"><b>/* HSTableFooter */</b>
uint32_t filetype;        // Same as filetype in HSTableHeader
uint32_t flags;           // Flags, such as: kHasPaddingInValues, and kHasPaddingInValues
uint64_t offset_offarray; // Offset of the OffsetArray in the HSTable
uint64_t num_entries;     // Number of entries
uint64_t magic_number;    // Magic number
uint32_t checksum;        // Checksum for the rows in the Offset Array and the HSTableFooter
</pre>

For the headers and footers above, all the data is serialized in little-endian for cross-platform compatibility. Some of the fields, such as the sizes in the EntryHeader, are stored using Variable-Length Integers, “varints”, which allow for a compact representation of integers and save a significant amount of disk space [[1, 2]](https://codecapsule.com/2015/08/03/implementing-a-key-value-store-part-9-data-format-and-memory-management-in-kingdb/#ref).

Also, note that the checksums are computed for each entry independently. The upside is that since each entry has its own checksum, and a wrong checksum means that only that entry will be deleted. The downside is that when entries are small, this ends up using more disk space.

> ‎对于上面的headers和footers，所有数据都以小端序列化，以实现跨平台兼容性。某些字段（如 EntryHeader 中的大小）使用可变长度整数“varints”进行存储，它允许整数的紧凑表示并节省大量磁盘空间 ‎[‎[1， 2]‎](https://codecapsule.com/2015/08/03/implementing-a-key-value-store-part-9-data-format-and-memory-management-in-kingdb/#ref)‎。‎
>
> 另请注意，校验和是为每个条目独立计算的。好处是，由于每个条目都有自己的校验和，而错误的校验和意味着只有该条目会被删除。缺点是，当条目很小时，这最终会占用更多的磁盘空间。

## 3. KingDB’s Storage Engine

I designed the storage engine with the network in mind. Chunks of data received from the network by a call to recv() are saved into a buffer first, and then persisted to disk. If the size of this buffer is too large, then writing the data to secondary storage can take more time than the acceptable network inactivity delay, which will make the connection timeout. Therefore, the size of the recv() buffer must kept small, and a good practical size for that is 8KB.

KingDB stores data in HSTable files, as presented in Section 2. Those HSTable files have a maximum size of 256MB by default — this is a parameter that can be changed when creating a database. Assuming that the recv() buffer of the server is 8KB, there are three types of entries:

* Small entries: smaller than the recv() buffer, thus size <= 8KB
* Medium entries: larger than the recv() buffer, but smaller than the size of the HSTable files: 8KB < size <= 256MB
* Large entries: larger than the size of a HSTable file: size > 256MB

When small entries are incoming, they are copied into a buffer before being persisted to disk, into the next new created HSTable. This means that workloads of small random writes are turned into a workload of large sequential writes.

When medium entries arrive, their first part is copied to the buffer, and then some space is reserved on disk so that the subsequent parts can be written contiguously. Each subsequent part requires a call to pwrite(). Another option would have been to allow for parts to be stored in different files, but then with a slow client, the database could end up with a situation where the N parts of an entry are stored at random locations in N different files, which makes compaction obviously more complex. Keeping all parts for a given entry contiguous simplifies the compaction process.

Finally, when a large entry is incoming, it is given its own dedicated file, and every part is persisted to disk with its own call to pwrite().

Of course, the use of multiple pwrite() syscalls is not optimal, but it guarantees that workloads of concurrent small, medium, and large entries, can be handled. In addition, benchmarks over the write syscall shows easily that past a certain buffer size, the cost of the syscall is amortized, thus by picking the size of the recv() buffer to be large enough, 8KB, the cost of the additional calls to pwrite() are amortized [[3]](https://codecapsule.com/2015/08/03/implementing-a-key-value-store-part-9-data-format-and-memory-management-in-kingdb/#ref).

Figure 9.1 below illustrates what the storage engine looks like when it is receiving buffers of entry parts to write to disk. This is a purely random sequence of entries that I have made up so I could show how the storage engine works. Entries are represented with a unique lowercase letters and a distinct color. In this simplified representation, the buffer is of size 6 and the HSTable have a maximum size of 8. All entries have integer number sizes, and all the HSTables are filled exactly up to the maximum HSTable size. This maximum HSTable size only applies to small and medium entries, and can of course be extended in the case of large entries as explained above. In the real implementation of KingDB, HSTables can have sizes a bit lower or a bit larger than the maximum HSTable size, depending on the sizes of the incoming entries they have to hold.

> 我在设计存储引擎时考虑了网络。通过调用 recv（） 从网络接收的数据块首先保存到缓冲区中，然后保存到磁盘。如果此缓冲区的大小太大，则将数据写入辅助存储所需的时间可能比可接受的网络不活动延迟所花费的时间长，这将导致连接超时。因此，recv（） 缓冲区的大小必须保持较小，并且其实际大小为 8KB。
>
> ‎KingDB 将数据存储在 HSTable 文件中，如第 2 节所述。默认情况下，这些HSTable文件的最大大小为256MB - 这是一个可以在创建数据库时更改的参数。假设服务器的 recv（） 缓冲区为 8KB，则有三种类型的条目：‎
>
> * **小条目：‎**‎小于 recv（） 缓冲区，因此大小 <= 8KB‎
> * **‎中等条目：‎**‎大于 recv（） 缓冲区，但小于 HSTable 文件的大小：8KB <大小 <= 256MB‎
> * **‎大条目：‎**‎大于HSTable文件的大小：大小>256MB‎
>
> 当传入小条目时，它们被复制到缓冲区中，然后保存到磁盘，放入下一个新创建的HSTable中。这意味着小型随机写入的工作负载将转换为大型顺序写入的工作负载。‎
>
> ‎当中等条目到达时，它们的第一部分被复制到缓冲区，然后在磁盘上保留一些空间，以便后续部分可以连续写入。每个后续部分都需要调用 pwrite（）。另一种选择是允许将部分存储在不同的文件中，但是对于慢速客户端，数据库最终可能会出现条目的N个部分存储在N个不同文件中的随机位置的情况，这使得压缩显然更加复杂。使给定条目的所有部件保持连续可简化压缩过程。‎
>
> ‎最后，当一个大条目传入时，它被赋予自己的专用文件，并且每个部分都通过自己对pwrite（）的调用持久保存到磁盘。‎
>
> 当然，使用多个 pwrite（） 系统调用并不是最佳选择，但它保证了可以处理并发小型、中型和大型条目的工作负载。此外，对写入系统调用的基准测试很容易表明，超过一定的缓冲区大小后，系统调用的成本是摊销的，因此通过选择 recv（） 缓冲区的大小足够大（8KB），对 pwrite（） 的额外调用的成本是摊销的 ‎[‎[3]‎](https://codecapsule.com/2015/08/03/implementing-a-key-value-store-part-9-data-format-and-memory-management-in-kingdb/#ref)‎。‎
>
> 下面的图 9.1 说明了存储引擎接收要写入磁盘的入口部分的缓冲区时的外观。这是我编造的一个纯粹随机的条目序列，以便我可以展示存储引擎的工作原理。条目用唯一的小写字母和独特的颜色表示。在此简化表示中，缓冲区的大小为 6，HSTable 的最大大小为 8。所有条目都具有整数大小，并且所有 HSTable 的填充都精确到最大 HSTable 大小。此最大 HSTable 大小仅适用于中小型条目，当然，在大条目的情况下也可以扩展，如上所述。在 KingDB 的实际实现中，HSTables 的大小可以略小于或略大于最大 HSTable 大小，具体取决于它们必须保存的传入条目的大小。‎

![storage-engine-kingdb-0.9.0](https://i0.wp.com/codecapsule.com/wp-content/uploads/2015/08/storage-engine-kingdb-0.9.0.png?resize=780%2C1681)

Figure 9.1: Storage Engine of KingDB v0.9.0

## 4. Database start-up, recovery and index building

When a database is opened, its parameters are read from the option file, so that the appropriate hashing, compression, and checksum functions can be created. Then the HSTables are ordered by timestamp first and file id second, and handled one after the other.

For each HSTable, the HSTableFooter is read:

* If anything is wrong with the footer, i.e. invalid magic number or invalid checksum, the HSTable enters recovery mode. In recovery mode, all the entries are read from the header one by one, and their checksum are computed to verify their integrity: invalid entries are discarded.
* If the footer is valid, the position of the Offset Array is read from offset_offarray, and all the items of that Offset Array are loaded into the in-memory hash table of KingDB. Building the index requires only a single bulk sequential read in each HSTable. With this data format and the Offset Array in each HSTable, building the entire index is fast, and thus the start-up time is very small: an average-size database can be loaded and used after just a few seconds.

> 打开数据库时，将从选项文件中读取其参数，以便可以创建适当的哈希、压缩和校验和函数。然后，HSTables 首先按时间戳排序，其次按文件 ID 排序，并一个接一个地处理。
>
> ‎对于每个 HSTable，HSTableFooter 将读取：
>
> * 如果页脚有任何错误，即无效的幻数或无效的校验和，HSTable将进入恢复模式。在恢复模式下，从标头逐个读取所有条目，并计算其校验和以验证其完整性：丢弃无效条目。‎
> * ‎如果页脚有效，则从offset_offarray读取偏移数组的位置，并将该偏移数组的所有项目加载到 KingDB 的内存中哈希表中。构建索引只需要在每个 HSTable 中进行一次批量顺序读取。使用此数据格式和每个HSTable中的偏移数组，可以快速构建整个索引，因此启动时间非常短：可以在几秒钟后加载和使用平均大小的数据库。‎

## 5. Compaction

KingDB uses log-structured storage to persist entries to disk. With that solution, if multiple versions of an entry — i.e. entries with same key — are saved in the database, all these versions will be stored on disk, but only the last one has to be kept. Therefore a compaction process must be applied to the entries so that the space occupied by outdated entries can be reclaimed. It is frequently reported that storage systems undergo slowdowns when the compaction process kicks in. This is nonetheless the design that I have chosen to use for KingDB, because of its simplicity.

Compaction is performed at regular time interval if certain conditions are met. The compaction process looks at the uncompacted files in the same order as they were written, and based on the currently available free disk space, it determines which subset of these files can be compacted. For every uncompacted file, the compaction process checks if for any of the entries in these files, there exists an older version of those entries in an already compacted file, i.e. overwrite of entries with identical keys. The compaction also looks at the delete orders, and checks if it can find a file that contains the data for the deleted key. If such combinations of files are found, then these files will be merged into a single sequence of orders, and written to a sequence of new compacted files. In the mean time, the stale versions of the entries and the deleted entries, the uncompacted files, are discarded. When the new compacted version of the files have been written to disk, the uncompacted files are simply removed. The compaction process never overwrites any data, it always write data to a new file, at a new location, guaranteeing that if an error or a crash occurs, it will not cause any data loss. In addition, compaction is designed in such a way that if cursors or snapshots are requested by a client as a compaction is on-going, the files that the client may use are “locked” until the cursors and snapshots are released, at which point they will be removed.

During compaction, an important process also happens: key grouping. Whenever the compaction process encounters entries that have the same hashed keys but different keys — i.e they are not the successive versions of the same entry, they are effectively different entries — it writes them sequentially in the new sequence of files. This can incur a fair bit of rewrites, but it guarantees that after all files have gone through the compaction process, all entries that have the same hashed key will be found sequentially in files. Therefore, whenever hashed key collisions happen in the index and whenever the same hash points to different entries, only a single seek on disk will be necessary to access the whole set of entries having that hash: all entries can be found with a single random read. For SSDs there are no seeks of course, but this will still guarantee that data access can be made using the internal read-ahead buffers of the drives.

Figure 9.2 below shows what a compaction process looks like on an arbitrary set of HSTables.

> ‎KingDB 使用日志结构化存储将条目保存到磁盘。使用该解决方案，如果一个条目的多个版本（即具有相同键的条目）保存在数据库中，则所有这些版本都将存储在磁盘上，但必须保留最后一个版本。因此，必须对条目应用压缩过程，以便可以回收过时条目占用的空间。经常有报告称，当压缩过程开始时，存储系统会变慢。尽管如此，这是我选择用于KingDB的设计，因为它的简单性。
>
> ‎如果满足某些条件，则按固定的时间间隔执行压缩。压缩过程以与写入文件相同的顺序查看未压缩的文件，并根据当前可用的可用磁盘空间确定可以压缩这些文件的哪个子集。对于每个未压缩的文件，压缩过程会检查对于这些文件中的任何条目，在已压缩的文件中是否存在这些条目的旧版本，即覆盖具有相同键的条目。压缩还会查看删除订单，并检查它是否可以找到包含已删除密钥数据的文件。如果找到这样的文件组合，则这些文件将被合并到单个订单序列中，并写入一系列新的压缩文件。同时，条目的陈旧版本和已删除的条目（未压缩的文件）将被丢弃。将文件的新压缩版本写入磁盘后，只需删除未压缩的文件。压缩过程永远不会覆盖任何数据，它总是将数据写入新文件，在新位置，保证如果发生错误或崩溃，它不会造成任何数据丢失。此外，压缩的设计方式是，如果在压缩过程中客户端请求游标或快照，则客户端可能使用的文件将被“锁定”，直到释放游标和快照，此时它们将被删除。‎
>
> 在压缩过程中，还会发生一个重要的过程：键分组。每当压缩过程遇到具有相同哈希键但具有不同键的条目时 - 即它们不是同一条目的连续版本，它们实际上是不同的条目 - 它会按顺序将它们写入新的文件序列中。这可能会引起相当多的重写，但它可以保证在所有文件都经过压缩过程后，具有相同哈希键的所有条目都将在文件中按顺序找到。因此，每当索引中发生散列键冲突时，每当相同的散列指向不同的条目时，只需要在磁盘上进行一次搜索即可访问具有该散列的整个条目集：所有条目都可以通过一次随机读取找到。对于SSD，当然没有搜索，但这仍然可以保证可以使用驱动器的内部预读缓冲区进行数据访问。
>
> ‎下面的图 9.2 显示了任意一组 HSTables 上的压缩过程。‎

![compaction-kingdb-0.9.0](https://i0.wp.com/codecapsule.com/wp-content/uploads/2015/08/compaction-kingdb-0.9.0.png?resize=780%2C2187)

Figure 9.2: Compaction in KingDB v0.9.0

## Coming next

In the next article, I will explain how KingServer, the network server on top of KingDB, is implemented.

> ‎在下一篇文章中，我将解释KingServer（KingDB之上的网络服务器）是如何实现的

## References

[1] [Jeff Dean on group varints, from “Building Software Systems At Google and Lessons Learned”](https://www.youtube.com/watch?v=modXC5IWTJI&t=28m34s)
[2] [Variable-Length Integers from SQLite documentation](https://sqlite.org/src4/doc/trunk/www/varint.wiki)
[3] [The Linux Programming Interface by Michael Kerrisk, Chapter 13 “File I/O Buffering”](http://man7.org/tlpi/)
