# Coding for SSDs – Part 3: Pages, Blocks, and the Flash Translation Layer

This is Part 3 over 6 of “Coding for SSDs”, covering Sections 3 and 4. For other parts and sections, you can refer to the [Table to Contents](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/). This is a series of articles that I wrote to share what I learned while documenting myself on SSDs, and on how to make code perform well on SSDs. If you’re in a rush, you can also go directly to [Part 6](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/), which is summarizing the content from all the other parts.

In this part, I am explaining how writes are handled at the page and block level, and I talk about the fundamental concepts of write amplification and wear leveling. Moreover, I describe what is a Flash Translation Layer (FTL), and I cover its two main purposes, logical block mapping and garbage collection. More particularly, I explain how write operations work in the context of a hybrid log-block mapping.

> ‎这是“SSD 编码”的第 3 部分，第 6 部分，涵盖第 3 节和第 4 节。对于其他部分和节，可以参考‎[‎目录‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)‎。这是我写的一系列文章，旨在分享我在SSD上记录自己时学到的东西，以及如何使代码在SSD上表现良好。如果您赶时间，也可以直接转到‎[‎第6部分‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/)‎，该部分总结了所有其他部分的内容。‎
>
> ‎在这一部分中，我将解释如何在页面和块级别处理写入，并讨论写入放大和磨损均衡的基本概念。此外，我描述了什么是Flash转换层（FTL），并介绍了它的两个主要目的，逻辑块映射和垃圾回收。更具体地说，我将解释写入操作如何在混合日志块映射的上下文中工作。‎

![ssd-presentation-03](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/02/ssd-presentation-03.jpg?resize=720%2C505)

## 3. Basic operations

### 3.1 Read, write, erase

Due to the organization of NAND-flash cells, it is not possible to read or write single cells individually. Memory is grouped and is accessed with very specific properties. Knowing those properties is crucial for optimizing data structures for solid-state drives and for understanding their behavior. I am describing below the basic properties of SSDs regarding the read, write and erase operations.

> ‎由于NAND闪存单元的组织，不可能单独读取或写入单个单元。内存被分组，并使用非常特定的属性进行访问。了解这些属性对于优化固态硬盘的数据结构和了解其行为至关重要。我在下面描述了SSD关于读取，写入和擦除操作的基本属性。‎

#### Reads are aligned on page size

It is not possible to read less than one page at once. One can of course only request just one byte from the operating system, but a full page will be retrieved in the SSD, forcing a lot more data to be read than necessary.

> ‎一次阅读少于一页是不可能的。当然，人们只能从操作系统请求一个字节，但是在SSD中将检索到一整页，从而强制读取比必要更多的数据。‎

#### Writes are aligned on page size

When writing to an SSD, writes happen by increments of the page size. So even if a write operation affects only one byte, a whole page will be written anyway. Writing more data than necessary is known as write amplification, a concept that is covered in Section 3.3. In addition, writing data to a page is sometimes referred to as “to program” a page, therefore the terms “write” and “program” are used interchangeably in most publications and articles related to SSDs.

> ‎写入 SSD 时，写入按页面大小的增量进行。因此，即使写入操作仅影响一个字节，也会写入整个页面。写入超出必要数量的数据称为写入放大，这一概念在第 3.3 节中介绍。此外，将数据写入页面有时被称为“编程”页面，因此术语“写入”和“程序”在大多数与SSD相关的出版物和文章中可以互换使用。‎

#### Pages cannot be overwritten

A NAND-flash page can be written to only if it is in the “free” state. When data is changed, the content of the page is copied into an internal register, the data is updated, and the new version is stored in a “free” page, an operation called “read-modify-write”. The data is not updated in-place, as the “free” page is a different page than the page that originally contained the data. Once the data is persisted to the drive, the original page is marked as being “stale”, and will remain as such until it is erased.

> ‎只有当NAND闪存页面处于“空闲”状态时，才能写入该页面。当数据更改时，页面的内容被复制到内部寄存器中，数据被更新，新版本存储在“空闲”页面中，这种操作称为“读-修改-写”。数据不会就地更新，因为“空闲”页面与最初包含数据的页面不同。一旦数据保存到驱动器，原始页面将被标记为“过时”，并将保持原样，直到它被擦除。‎

#### Erases are aligned on block size

Pages cannot be overwritten, and once they become stale, the only way to make them free again is to erase them. However, it is not possible to erase individual pages, and it is only possible to erase whole blocks at once. From a user perspective, only read and write commands can be emitted when data is accessed. The erase command is triggered automatically by the garbage collection process in the SSD controller when it needs to reclaim stale pages to make free space.

> ‎页面不能被覆盖，一旦它们变得过时，使它们再次空闲的唯一方法就是擦除它们。但是，无法擦除单个页面，并且只能一次擦除整个块。从用户的角度来看，访问数据时只能发出读写命令。擦除命令由 SSD 控制器中的垃圾回收进程自动触发，当它需要回收过时的页面以腾出可用空间时。‎

### 3.2 Example of a write

Let’s illustrate the concepts from Section 3.1. Figure 4 below shows an example of data being written to an SSD. Only two blocks are shown, and those blocks contain only four pages each. This is obviously a simplified representation of a NAND-flash package, created for the sake of the reduced examples I am presenting here. At each step in the figure, bullet points on the right of the schematics explain what is happening.

> ‎让我们来说明第 3.1 节中的概念。下面的图 4 显示了写入 SSD 的数据示例。仅显示两个块，每个块仅包含四个页面。这显然是NAND闪存包的简化表示，是为了我在这里介绍的简化示例而创建的。在图中的每个步骤中，原理图右侧的项目符号点都会解释正在发生的事情。‎

![ssd-writing-data](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/02/ssd-writing-data.jpg?resize=720%2C761)

Figure 4: Writing data to a solid-state drive

### 3.3 Write amplification

Because writes are aligned on the page size, any write operation that is not both aligned on the page size and a multiple of the page size will require more data to be written than necessary, a concept called **write amplification** [[13]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref). Writing one byte will end up writing a page, which can amount up to 16 KB for some models of SSD and be extremely inefficient.

But this is not the only problem. In addition to writing more data than necessary, those writes also trigger more internal operations than necessary. Indeed, writing data in an unaligned way causes the pages to be read into cache before being modified and written back to the drive, which is slower than directly writing pages to the disk. This operations is known as  **read-modify-write** , and should be avoided whenever possible [[2, 5]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref).

> ‎由于写入操作在页面大小上对齐，因此任何既不与页面大小对齐又与页面大小成倍数的写入操作都需要写入比所需数据更多的数据，这一概念称为‎**‎写入放大‎**‎ ‎[‎[13]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。写入一个字节最终会写入一个页面，对于某些型号的SSD，该页面可能高达16 KB，并且效率极低。
>
> ‎但这不是唯一的问题。除了写入不必要的数据之外，这些写入还会触发不必要的内部操作。实际上，以未对齐的方式写入数据会导致页面在修改并写回驱动器之前被读入缓存，这比直接将页面写入磁盘要慢。此操作称为‎**‎读取-修改-写入‎**‎，应尽可能避免 ‎[‎[2， 5]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。‎

#### Never write less than a page

Avoid writing chunks of data that are below the size of a NAND-flash page to minimize write amplification and prevent read-modify-write operations. The largest size for a page at the moment is 16 KB, therefore it is the value that should be used by default. This size depends on the SSD models and you may need to increase it in the future as SSDs improve.

> ‎避免写入小于 NAND 闪存页面大小的数据块，以最大程度地减少写入放大并防止读取-修改-写入操作。当前页面的最大大小为 16 KB，因此默认情况下应使用该值。此大小取决于 SSD 型号，随着 SSD 的改进，将来可能需要增加此大小。‎

#### Align writes

Align writes on the page size, and write chunks of data that are multiple of the page size.

> ‎根据页面大小对齐写入，并写入页面大小的倍数数据块。‎

#### Buffer small writes

To maximize throughput, whenever possible keep small writes into a buffer in RAM and when the buffer is full, perform a single large write to batch all the small writes.

> ‎为了最大限度地提高吞吐量，请尽可能将小写入操作保留在 RAM 中的缓冲区中，当缓冲区已满时，执行一次大写入操作以对所有小写入进行批处理。‎

### 3.4 Wear leveling

As discussed in Section 1.1, NAND-flash cells have a limited lifespan due to their limited number of P/E cycles. Let’s imagine that we had an hypothetical SSD in which data was always read and written from the same exact block. This block would quickly exceed its P/E cycle limit, wear off, and the SSD controller would mark it as being unusable. The overall capacity of the disk would then decrease. Imagine buying a 500 GB drive and being left with at 250 GB a couple of years later, that would be outrageous!

For that reason, one of the main goals of an SSD controller is to implement  **wear leveling** , which distributes P/E cycles as evenly as possible among the blocks. Ideally, all blocks would reach their P/E cycle limits and wear off at the same time [[12, 14]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref).

In order to achieve the best overall wear leveling, the SSD controller will need to choose blocks judiciously when writing, and may have to move around some blocks, a process which in itself incurs an increase of the write amplification. Therefore, block management is a trade-off between maximizing wear leveling and minimizing write amplification.

Manufacturers have come up with various functionalities to achieve wear leveling, such as garbage collection, which is covered in the next section.

> ‎如第1.1节所述，NAND闪存单元由于其有限的P/E周期数而具有有限的寿命。让我们想象一下，我们有一个假设的SSD，其中数据总是从同一个完全相同的块中读取和写入。该块将很快超过其P / E周期限制，磨损，SSD控制器将其标记为不可用。然后，磁盘的总体容量将减小。想象一下，购买一个500 GB的驱动器，几年后留在250 GB，这将是离谱的！‎
>
> ‎因此，SSD控制器的主要目标之一是实现‎**‎磨损均衡‎**‎，从而在块之间尽可能均匀地分配P / E周期。理想情况下，所有模块都将达到其P / E周期限制并同时磨损‎[‎[12，14]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。‎
>
> ‎为了实现最佳的整体磨损均衡，SSD控制器在写入时需要明智地选择块，并且可能必须在某些块周围移动，这一过程本身就会导致写入放大的增加。因此，模块管理是在最大化磨损均衡和最小化写入放大之间进行的权衡。‎
>
> ‎制造商已经提出了各种功能来实现磨损均衡，例如垃圾收集，这将在下一节中介绍。‎

#### Wear leveling

Because NAND-flash cells are wearing off, one of the main goals of the FTL is to distribute the work among cells as evenly as possible so that blocks will reach their P/E cycle limit and wear off at the same time.

> ‎由于NAND闪存单元正在磨损，FTL的主要目标之一是尽可能均匀地在单元之间分配工作，以便模块达到其P / E周期极限并同时磨损。‎

## 4. Flash Translation Layer (FTL)

### 4.1 On the necessity of having an FTL

The main factor that made adoption of SSDs so easy is that they use the same host interfaces as HDDs. Although presenting an array of Logical Block Addresses (LBA) makes sense for HDDs as their sectors can be overwritten, it is not fully suited to the way flash memory works. For this reason, an additional component is required to hide the inner characteristics of NAND flash memory and expose only an array of LBAs to the host. This component is called the *Flash Translation Layer* (FTL), and resides in the SSD controller. The FTL is critical and has two main purposes: logical block mapping and garbage collection.

> ‎使SSD采用如此容易的主要因素是它们使用与HDD相同的主机接口。虽然呈现逻辑块地址（LBA）数组对于HDD有意义，因为它们的扇区可以被覆盖，但它并不完全适合闪存的工作方式。因此，需要一个额外的组件来隐藏NAND闪存的内部特性，并且仅向主机公开一个LBA阵列。此组件称为‎*‎闪存转换层‎*‎ （FTL），驻留在 SSD 控制器中。FTL 至关重要，它有两个主要用途：逻辑块映射和垃圾回收。‎

### 4.2 Logical block mapping

The logical block mapping translates logical block addresses (LBAs) from the host space into physical block addresses (PBAs) in the physical NAND-flash memory space. This mapping takes the form of a table, which for any LBA gives the corresponding PBA. This mapping table is stored in the RAM of the SSD for speed of access, and is persisted in flash memory in case of power failure. When the SSD powers up, the table is read from the persisted version and reconstructed into the RAM of the SSD [[1, 5]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref).

The naive approach is to use a **page-level mapping** to map any logical page from the host to a physical page. This mapping policy offers a lot of flexibility, but the major drawback is that the mapping table requires a lot of RAM, which can significantly increase the manufacturing costs. A solution to that would be to map blocks instead of pages, using a  **block-level mapping** . Let’s assume that an SSD drive has 256 pages per block. This means that block-level mapping requires 256 times less memory than page-level mapping, which is a huge improvement for space utilization. However, the mapping still needs to be persisted on disk in case of power failure, and in case of workloads with a lot of small updates, full blocks of flash memory will be written whereas pages would have been enough. This increases the write amplification and makes block-level mapping widely inefficient [[1, 2]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref).

The tradeoff between page-level mapping and block-level mapping is the one of performance versus space. Some researcher have tried to get the best of both worlds, giving birth to the so-called “ *hybrid* ” approaches [[10]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref). The most common is the  **log-block mapping** , which uses an approach similar to log-structured file systems. Incoming write operations are written sequentially to log blocks. When a log block is full, it is merged with the data block associated to the same logical block number (LBN) into a free block. Only a few log blocks need to be maintained, which allows to maintain them with a page granularity. Data blocks on the contrary are maintained with a block granularity [[9, 10]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref).

Figure 5 below shows a simplified representation of a hybrid log-block FTL, in which each block only has four pages. Four write operations are handled by the FTL, all having the size of a full page. The logical page numbers of 5 and 9 both resolve to LBN=1, which is associated to the physical block #1000. Initially, all the physical page offsets are null at the entry where LBN=1 is in the  *log-block mapping table* , and the log block #1000 is entirely empty as well. The first write, b’ at LPN=5, is resolving to LBN=1 by the log-block mapping table, which is associated to PBN=1000 (log block #1000). The page b’ is therefore written at the physical offset 0 in block #1000. The metadata for the mapping now needs to be updated, and for this, the physical offset associated to the logical offset of 1 (arbitrary value for this example) is updated from null to 0.

The write operations go on and the mapping metadata is updated accordingly. When the log block #1000 is entirely filled, it is merged with the data block associated to the same logical block, which is block #3000 in this case. This information can be retrieved from the  *data-block mapping table* , which maps logical block numbers to physical block numbers. The data resulting from the merge operation is written to a free block, #9000 in this case. When this is done, both blocks #1000 and #3000 can be erased and become free blocks, and block #9000 becomes a data block. The metadata for LBN=1 in the data-block mapping table is then updated from the initial data block #3000 to the new data block #9000.

An important thing to notice here is that the four write operations were concentrated on only two LPNs. The log-block approach enabled to hide the b’ and d’ operations during the merge, and directly use the more up-to-date b” and d” versions, allowing to achieve better write amplification.

Finally, if a read command is requesting a page that was recently updated and for which the merge step on the blocks has not occurred yet, then the page will be in a log block. Otherwise, the page will be found a data block. This is why read requests need to check both the *log-block mapping table* and the  *data-block mapping table* , as shown in Figure 5.

> ‎逻辑块映射将主机空间中的逻辑块地址 （LBA） 转换为物理 NAND 闪存空间中的物理块地址 （PBA）。此映射采用表的形式，对于任何 LBA，表都提供相应的 PBA。此映射表存储在 SSD 的 RAM 中以提高访问速度，并在断电时保留在闪存中。当 SSD 通电时，将从持久化版本中读取表，并将其重建为 SSD 的 RAM ‎[‎[1， 5]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。‎
>
> ‎一般的方法是使用‎**‎页面级映射‎**‎将任何逻辑页面从主机映射到物理页面。这种映射策略提供了很大的灵活性，但主要缺点是映射表需要大量的RAM，这可能会显着增加制造成本。解决这个问题的一个方法是使用块级映射来‎**‎映射块‎**‎而不是页面。假设 SSD 驱动器每个块有 256 页。这意味着块级映射需要的内存比页面级映射少 256 倍，这对空间利用率来说是一个巨大的改进。但是，在发生电源故障时，映射仍需要保留在磁盘上，并且在具有大量小更新的工作负载的情况下，将写入完整的闪存块，而页面就足够了。这增加了写入放大，并使块级映射效率低下‎[‎[1，2]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。
>
> 页面级映射和块级映射之间的权衡是性能与空间之间的权衡。一些研究人员试图两全其美，催生了所谓的“‎*‎混合‎*‎”方法‎[‎[10]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。最常见的是‎**‎日志块映射‎**‎，它使用类似于日志结构化文件系统的方法。传入的写入操作按顺序写入日志块。当日志块已满时，它将与同一逻辑块号 （LBN） 关联的数据块合并到一个可用块中。只需要维护几个日志块，这允许以页面粒度维护它们。相反，数据块以块粒度‎[‎[9，10]进行‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎维护。‎
>
> 下面的图 5 显示了混合日志块 FTL 的简化表示形式，其中每个块只有四个页面。FTL 处理四个写入操作，所有操作的大小都为一整页。逻辑页码 5 和 9 都解析为 LBN=1，这与物理块 #1000 相关联。最初，所有物理页偏移量在 LBN=1 位于‎*‎日志块映射表中‎*‎的条目处为 null，日志块 #1000 也完全为空。第一个写入操作 b' at LPN=5，通过与 PBN=1000（日志块 #1000）关联的日志块映射表解析为 LBN=1。因此，页面 b' 写在块 #1000 中的物理偏移量 0 处。映射的元数据现在需要更新，为此，与逻辑偏移量 1（此示例为任意值）关联的物理偏移量将从 null 更新为 0。‎
>
> ‎写入操作继续进行，映射元数据将相应地更新。当日志块 #1000 完全填满时，它将与与同一逻辑块关联的数据块合并，在本例中为块 #3000。可以从‎*‎数据块映射表中‎*‎检索此信息，该表将逻辑块编号映射到物理块编号。合并操作产生的数据将写入一个空闲块，在本例中为 #9000。完成此操作后，块 #1000 和 #3000 都可以擦除并成为可用块，块 #9000 成为数据块。然后，数据块映射表中 LBN=1 的元数据将从初始数据块 #3000 更新为新的数据块 #9000。‎
>
> ‎这里需要注意的一件重要事情是，四个写入操作仅集中在两个LPN上。日志块方法能够在合并期间隐藏b'和d'操作，并直接使用更新的b“和d”版本，从而实现更好的写入放大。‎
>
> ‎最后，如果读取命令请求的页面最近已更新，并且尚未对块执行合并步骤，则该页面将位于日志块中。否则，该页面将找到一个数据块。这就是读取请求需要同时检查‎*‎日志块映射表‎*‎和‎*‎数据块映射表‎*‎的原因，如图 5 所示。‎

![ssd-hybrid-ftl](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/02/ssd-hybrid-ftl.jpg?resize=720%2C1009)

Figure 5: Hybrid log-block FTL

The log-block FTL allows for optimizations, the most notable being the  **switch-merge** , sometimes referred to as “swap-merge”. Let’s imagine that all the addresses in a logical block were written at once. This would mean that all the new data for those addresses would be written to the same log block. Since this log block contains the data for a whole logical block, it would be useless to merge this log block with a data block into a free block, because the resulting free block would contain exactly the data as the log block. It would be faster to only update the metadata in the data block mapping table, and switch the the data block in the data block mapping table for the log block, this is a switch-merge.

The log-block mapping scheme has been the topic of many papers, which has lead to a series of improvements, such as FAST (Fully Associative Sector Translation), superblock mapping, and flexible group mapping [[10]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref). There are also other mapping schemes, such as the Mitsubishi algorithm, and SSR [[9]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref). Two great starting points to learn more about the FTL and mapping schemes are the two following papers:

* “ *A Survey of Flash Translation Layer* “, Chung et al., 2009 [[9]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)
* “ *A Reconfigurable FTL (Flash Translation Layer) Architecture for NAND Flash-Based Applications* “, Park et al., 2008 [[10]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)

> ‎日志块FTL允许优化，最值得注意的是‎**‎交换机合并‎**‎，有时称为“交换 - 合并”。让我们想象一下，逻辑块中的所有地址都是一次性写入的。这意味着这些地址的所有新数据都将写入同一日志块。由于此日志块包含整个逻辑块的数据，因此将此日志块与数据块合并到一个可用块中是没有用的，因为生成的可用块将包含与日志块完全相同的数据。仅更新数据块映射表中的元数据，并为日志块切换数据块映射表中的数据块会更快，这是一个切换合并。‎
>
> ‎日志块映射方案一直是许多论文的主题，这导致了一系列改进，如FAST（完全关联扇区转换），超级块映射和灵活的组映射‎[‎[10]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。还有其他映射方案，如三菱算法和SSR ‎[‎[9]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。以下两篇论文是了解有关 FTL 和映射方案的更多信息的两个很好的起点：‎
>
> * ‎“‎*‎Flash Translation Layer Survey of Flash Translation Layer‎*‎”， Chung et al.， 2009 ‎[‎[9]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)
> * ‎“‎*‎用于基于NAND闪存的应用的可重构FTL（闪存转换层）架构‎*‎”，Park等人，2008‎[‎年[10]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)

#### Flash Translation Layer

The Flash Translation Layer (FTL) is a component of the SSD controller which maps Logical Block Addresses (LBA) from the host to Physical Block Addresses (PBA) on the drive. Most recent drives implement an approach called “hybrid log-block mapping” or one of its derivatives, which works in a way that is similar to log-structured file systems. This allows random writes to be handled like sequential writes.

> ‎闪存转换层 （FTL） 是 SSD 控制器的一个组件，它将主机上的逻辑块地址 （LBA） 映射到驱动器上的物理块地址 （PBA）。最新的驱动器实现了一种称为“混合日志块映射”或其衍生方法之一的方法，其工作方式类似于日志结构化文件系统。这允许像顺序写入一样处理随机写入。‎

### 4.3 Notes on the state of the industry

As of February 2, 2014, there are 70 manufacturers of SSDs listed on Wikipedia [[64]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref), and interestingly, there are only 11 manufacturers of controllers [[65]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref). Out of the 11 controller manufacturers, only four are “captive”, i.e. they use their controllers only for their own products (this is the case of Intel and Samsung), and the remaining seven are “independent”, i.e. they sell their controllers to other drive manufacturers. What those numbers are saying is that seven companies are providing controllers for 90% of the solid-state drive market.

I have no data on which controller manufacturers is selling to which drive manufacturer from these 90%, but following the Pareto principle, I would bet that only two or three controller manufacturers are sharing most of the cake. The direct consequence is that SSDs from non-captive drive manufacturers are extremely likely to behave similarly, since they are essentially running the same controllers, or at least controllers using the same general design and underlying ideas.

Mapping schemes, which are part of the controller, are critical components of SSDs because they will often entirely define the performance a drive. This explains why, in an industry with so much competition, SSD controller manufacturers do not share the details of their FTL implementations. Therefore, even though there is a lot of publicly available research regarding FTL algorithm, it is always unclear how much of that research is being used by controller manufacturers, and what are the exact implementations for specific brands and models.

The authors of [[3]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref) claim that by analyzing workloads, they can reverse engineer the mapping policies of a drive. I would argue that unless the binary code itself is being reversed engineered from the chip, there is no way to be completely sure what the mapping policy is really doing inside a specific drive. And it is even harder to predict how the mapping would behave under a specific workload.

There is a great deal of different mapping policies and it would take a considerable amount of time to reverse engineer all of the firmwares available in the wild. Then, even after getting the source code of all possible mapping policies, what would be the benefit? The system requirements of new projects are often to produce good overall results, using generic and inter-changeable commodity hardware. Therefore, it would be worthless to optimize for only one mapping policy, because the solution would be likely to perform poorly on all other mapping policies. The only reason why one would want to optimize for only one type of policy is when developing for an embedded system that is guaranteed to have consistent hardware.

For the reasons exposed above, I would argue that knowing the exact mapping policy of an SSD does not matter. The only important thing to know is that mapping schemes are the translation layer between LBAs and PBAs, and that it is very likely that the approach being used is the hybrid log-block or one of its derivatives. Consequently, writing chunks of data of at least the size of the NAND-flash block is more efficient, because for the FTL, it minimizes the overhead of updating the mapping and its metadata.

> ‎截至2014年2月2日，维基百科上列出了70家SSD制造商‎[‎[64]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎，有趣的是，只有11家控制器制造商‎[‎[65]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。在11家控制器制造商中，只有四家是“自保”的，即他们只将控制器用于自己的产品（英特尔和三星就是这种情况），其余七家是“独立的”，即他们将控制器出售给其他驱动器制造商。这些数字表明，有七家公司正在为90%的固态硬盘市场提供控制器。‎
>
> ‎我没有关于哪些控制器制造商从这90%中向哪个驱动器制造商销售的数据，但按照帕累托原理，我敢打赌只有两三个控制器制造商分享了大部分蛋糕。直接后果是，来自非强制驱动制造商的SSD极有可能表现得类似，因为它们基本上运行相同的控制器，或者至少使用相同的通用设计和基本思想的控制器。‎
>
> ‎映射方案是控制器的一部分，是 SSD 的关键组件，因为它们通常完全定义了驱动器的性能。这就解释了为什么在竞争如此激烈的行业中，SSD控制器制造商不分享其FTL实现的细节。因此，尽管有很多关于FTL算法的公开研究，但始终不清楚控制器制造商使用了多少研究，以及特定品牌和型号的确切实现是什么。‎
>
> [‎[3]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎的作者声称，通过分析工作负载，他们可以对驱动器的映射策略进行逆向工程。我认为，除非二进制代码本身是从芯片逆向工程的，否则没有办法完全确定映射策略在特定驱动器中真正在做什么。而且，预测映射在特定工作负载下的行为更加困难。‎
>
> ‎有大量不同的映射策略，并且需要花费大量时间来对现有的所有固件进行反向工程。那么，即使获得了所有可能的映射策略的源代码，又有什么好处呢？新项目的系统要求往往是使用通用和可互换的商品硬件，产生良好的整体结果。因此，仅针对一个映射策略进行优化是毫无价值的，因为该解决方案在所有其他映射策略上的性能可能很差。人们只想针对一种类型的策略进行优化的唯一原因是，在开发保证具有一致硬件的嵌入式系统时。‎
>
> ‎出于上述原因，我认为了解SSD的确切映射策略并不重要。唯一需要知道的是，映射方案是LBA和PBA之间的转换层，并且所使用的方法很可能是混合对数块或其衍生物之一。因此，写入至少与 NAND 闪存块大小相同的数据块效率更高，因为对于 FTL，它可以最大限度地减少更新映射及其元数据的开销。‎

### 4.4 Garbage collection

As explained in Sections 4.1 and 4.2, pages cannot be overwritten. If the data in a page has to be updated, the new version is written to a *free* page, and the page containing the previous version is marked as  *stale* . When blocks contain stale pages, they need to be erased before they can be written to.

> ‎如第 4.1 节和第 4.2 节所述，不能覆盖页面。如果必须更新页面中的数据，则会将新版本写入空闲页面，并且包含以前版本的页面将标记为‎*‎过时‎*‎。当块包含陈旧页面时，需要先擦除它们，然后才能写入它们。‎

#### Garbage collection

The garbage collection process in the SSD controller ensures that “stale” pages are erased and restored into a “free” state so that the incoming write commands can be processed.

Because of the high latency required by the erase command compared to the write command — which are respectively 1500-3500 μs and 250-1500 μs as described in Section 1 — this extra erase step incurs a delay which makes the writes slower. Therefore, some controllers implement a  *background garbage collection process* , also called  *idle garbage collection* , which takes advantage of idle time and runs regularly in the background to reclaim stale pages and ensure that future foreground operations will have enough free pages available to achieve the highest performance [[1]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref). Other implementations use a *parallel garbage collection* approach, which performs garbage collection operations in parallel with write operations from the host [[13]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref).

It is not uncommon to encounter workloads in which the writes are so heavy that the garbage collection needs to be run on-the-fly, at the same time as commands from the host. In that case, the garbage collection process supposed to run in background could be interfering with the foreground commands [[1]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref). The TRIM command and over-provisioning are two great ways to reduce this effect, and are covered in more details in Sections 6.1 and 6.2.

> ‎SSD 控制器中的垃圾回收过程可确保擦除“过时”页面并将其还原到“可用”状态，以便可以处理传入的写入命令。‎
>
> ‎由于与写入命令相比，erase 命令需要高延迟（如第 1 节所述，分别为 1500-3500 μs 和 250-1500 μs），因此此额外的擦除步骤会导致延迟，从而使写入速度变慢。因此，一些控制器实现了‎*‎后台垃圾回收过程‎*‎，也称为‎*‎空闲垃圾回收‎*‎，它利用空闲时间并在后台定期运行以回收过时的页面，并确保将来的前景操作将有足够的可用页面来实现最高性能‎[‎[1]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。其他实现使用‎*‎并行垃圾回收‎*‎方法，该方法与来自主机 ‎[‎[13]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎ 的写入操作并行执行垃圾回收操作。‎
>
> ‎遇到写入非常繁重的工作负载，以至于需要动态运行垃圾回收，同时运行来自主机的命令，这种情况并不少见。在这种情况下，应该在后台运行的垃圾回收过程可能会干扰前台命令 ‎[‎[1]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。TRIM 命令和过度预配是减少这种影响的两种很好的方法，第 6.1 节和第 6.2 节中对此进行了更详细的介绍。‎

#### Background operations can affect foreground operations

Background operations such as garbage collection can impact negatively on foreground operations from the host, especially in the case of a sustained workload of small random writes.

A less important reason for blocks to be moved is the  *read disturb* . Reading can change the state of nearby cells, thus blocks need to be moved around after a certain number of reads have been reached [[14]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref).

The rate at which data is changing is an important factor. Some data changes rarely, and is called *cold* or *static* data, while some other data is updated frequently, which is called *hot* or *dynamic* data. If a page stores partly cold and partly hot data, then the cold data will be copied along with the hot data during garbage collection for wear leveling, increasing write amplification due to the presence of cold data. This can be avoided by splitting cold data from hot data, simply by storing them in separate pages. The drawback is then that the pages containing the cold data are less frequently erased, and therefore the blocks storing cold and hot data have to be swapped regularly to ensure wear leveling.

Since the hotness of data is defined at the application level, the FTL has no way of knowing how much of cold and hot data is contained within a single page. A way to improve performance in SSDs is to split cold and hot data as much as possible into separate pages, which will make the job of the garbage collector easier [[8]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref).

> ‎垃圾回收等后台操作可能会对来自主机的前台操作产生负面影响，尤其是在小型随机写入的持续工作负载的情况下。‎
>
> ‎移动块的一个不太重要的原因是‎*‎读取干扰‎*‎。读取可以改变附近单元格的状态，因此在达到一定数量的读取后，块需要移动‎[‎[14]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。‎
>
> ‎数据变化的速率是一个重要因素。有些数据很少变化，称为‎*‎冷‎*‎数据或‎*‎静态‎*‎数据，而其他一些数据更新频繁，称为‎*‎热‎*‎数据或‎*‎动态‎*‎数据。如果页面存储部分冷数据和部分热数据，则在垃圾回收期间，冷数据将与热数据一起复制以进行磨损均衡，从而由于存在冷数据而增加写入放大。这可以通过将冷数据与热数据分开来避免，只需将它们存储在单独的页面中即可。缺点是包含冷数据的页面被擦除的频率较低，因此存储冷数据和热数据的块必须定期交换以确保磨损均衡。‎
>
> ‎由于数据的热度是在应用程序级别定义的，因此 FTL 无法知道单个页面中包含多少冷数据和热数据。提高SSD性能的一种方法是尽可能多地将冷数据和热数据拆分到单独的页面中，这将使垃圾回收器的工作更容易‎[‎[8]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/#ref)‎。‎

#### Split cold and hot data

Hot data is data that changes frequently, and cold data is data that changes infrequently. If some hot data is stored in the same page as some cold data, the cold data will be copied along every time the hot data is updated in a read-modify-write operation, and will be moved along during garbage collection for wear leveling. Splitting cold and hot data as much as possible into separate pages will make the job of the garbage collector easier.

> ‎热数据是频繁更改的数据，冷数据是不经常更改的数据。如果某些热数据与某些冷数据存储在同一页面中，则每次在读-修改-写操作中更新热数据时，冷数据都会被复制，并在垃圾回收期间移动以进行磨损均衡。将冷数据和热数据尽可能多地拆分到单独的页面中将使垃圾回收器的工作更容易。‎

#### Buffer hot data

Extremely hot data should be buffered as much as possible and written to the drive as infrequently as possible.

> ‎应尽可能缓冲极热的数据，并尽可能少地写入驱动器。‎

#### Invalidate obsolete data in large batches

When some data is no longer needed or need to be deleted, it is better to wait and invalidate it in a large batches in a single operation. This will allow the garbage collector process to handle larger areas at once and will help minimizing internal fragmentation.

> ‎当不再需要或需要删除某些数据时，最好等待并在单个操作中大批量使其失效。这将允许垃圾回收器进程一次处理更大的区域，并将有助于最大限度地减少内部碎片。‎

## What’s next

Part 4 is available [here](http://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/). You can also go to the [Table of Content](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/) for this series of articles, and if you’re in a rush, you can also directly go to [Part 6](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/), which is summarizing the content from all the other parts.

> ‎第 4 部分可‎[‎在此处获取‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/)‎。您还可以转到本系列文章的‎[‎目录‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)‎，如果您赶时间，也可以直接转到‎[‎第6部分‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/)‎，该部分总结了所有其他部分的内容。‎

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
