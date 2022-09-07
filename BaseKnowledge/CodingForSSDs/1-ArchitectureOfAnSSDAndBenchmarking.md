# Coding for SSDs – Part 2: Architecture of an SSD and Benchmarking

This is Part 2 over 6 of “Coding for SSDs”, covering Sections 1 and 2. For other parts and sections, you can refer to the [Table to Contents](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/). This is a series of articles that I wrote to share what I learned while documenting myself on SSDs, and on how to make code perform well on SSDs. If you’re in a rush, you can also go directly to [Part 6](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/), which is summarizing the content from all the other parts.

In this part, I am explaining the basics of NAND-flash memory, cell types, and basic SSD internal architecture. I am also covering SSD benchmarking and how to interpret those benchmarks.

> ‎这是“SSD 编码”的第 2 部分，共有 6 部分，涵盖第 1 节和第 2 节。对于其他部分和节，可以参考‎[‎目录‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)‎。这是我写的一系列文章，旨在分享我在SSD上记录自己时学到的东西，以及如何使代码在SSD上表现良好。如果您赶时间，也可以直接转到‎[‎第6部分‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/)‎，该部分总结了所有其他部分的内容。‎
>
> ‎在本部分中，我将解释NAND闪存，单元类型和基本SSD内部架构的基础知识。我还介绍了SSD基准测试以及如何解释这些‎‎基准。‎

![ssd-presentation-02](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/02/ssd-presentation-02.jpg?resize=720%2C420)

## 1. Structure of an SSD

### 1.1 NAND-flash memory cells

A solid-state drives (SSD) is a flash-memory based data storage device. Bits are stored into cells, which are made of floating-gate transistors. SSDs are made entirely of electronic components, there are no moving or mechanical parts like in hard drives.

Voltages are applied to the floating-gate transistors, which is how bits are being read, written, and erased. Two solutions exist for wiring transistors: the NOR flash memory, and the NAND flash memory. I will not go into more details regarding the difference between NOR and NAND flash memory. This article only covers NAND flash memory, which is the solution chosen by the majority of the manufacturers. For more information on the difference between NOR and NAND, you can refer to this article by Lee Hutchinson [[31]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref).

An important property of NAND-flash modules is that their cells are wearing off, and therefore have a limited lifespan. Indeed, the transistors forming the cells store bits by holding electrons. At each P/E cycle (i.e. Program/Erase, “Program” here means write), electrons might get trapped in the transistor by mistake, and after some time, too many electrons will have been trapped and the cells would become unusable.

> 固态硬盘 （SSD） 是一种基于闪存的数据存储设备。比特被存储到由浮栅晶体管制成的单元中。SSD完全由电子元件制成，没有像硬盘驱动器那样的移动或机械部件。‎
>
> ‎电压被施加到浮栅晶体管上，这就是位被读取、写入和擦除的方式。布线晶体管有两种解决方案：NOR闪存和NAND闪存。我不会详细介绍NOR和NAND闪存之间的区别。本文仅介绍NAND闪存，这是大多数制造商选择的解决方案。有关NOR和NAND之间差异的更多信息，您可以参考Lee Hutchinson‎[‎[31]的‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎这篇文章。‎
>
> ‎NAND闪存模块的一个重要特性是它们的电池正在磨损，因此寿命有限。事实上，形成电池的晶体管通过保持电子来存储位。在每个P / E周期（即程序/擦除，这里的“程序”意味着写入），电子可能会被错误地困在晶体管中，一段时间后，太多的电子将被捕获，电池将变得不可用。‎

#### Limited lifespan

Each cell has a maximum number of P/E cycles (Program/Erase), after which the cell is considered defective. NAND-flash memory wears off and has a limited lifespan. The different types of NAND-flash memory have different lifespans [[31]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref).

Recent research has shown that by applying very high temperatures to NAND chips, trapped electrons can be cleared out [[14, 51]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref). The lifespan of SSDs could be tremendously increased, though this is still research and there is no certainty that this will one day reach the consumer market.

The types of cells currently present in the industry are:

* Single level cell (SLC), in which transistors can store only 1 bit but have a long lifespan
* Multiple level cell (MLC), in which transistors can store 2 bits, at the cost of a higher latency and reduced lifespan compared to SLC
* Triple-level cell (TLC), in which transistors can store 3 bits, but at an even higher latency and reduced lifespan

> ‎每个单元都有最大 P/E 周期数（编程/擦除），之后该单元将被视为有缺陷。NAND闪存磨损，寿命有限。不同类型的NAND闪存具有不同的寿命‎[‎[31]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎。‎
>
> ‎最近的研究表明，通过对NAND芯片施加非常高的温度，可以清除被捕获的电子‎[‎[14，51]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎。SSD的寿命可以大大延长，尽管这仍然是研究，并且不确定有一天会进入消费市场。‎
>
> ‎目前业界存在的电池类型有：‎
>
> * ‎单电平单元（SLC），其中晶体管只能存储1位，但使用寿命长‎
> * ‎多电平单元（MLC），其中晶体管可以存储2位，与SLC相比，其代价是延迟更高，寿命更短‎
> * ‎三电平单元（TLC），其中晶体管可以存储3位，但延迟更高，寿命更短‎

#### Memory cell types

A solid-state drives (SSD) is a flash-memory based data storage device. Bits are stored into cells, which exist in three types: 1 bit per cell (single level cell, SLC), 2 bits per cell (multiple level cell, MLC), and 3 bits per cell (triple-level cell, TLC).

Table 1 below shows detailed information for each NAND-flash cell type. For the sake of comparison, I have also added the average latencies of hard drives, main memory (RAM), and L1/L2 caches.

> ‎固态硬盘 （SSD） 是一种基于闪存的数据存储设备。位存储在单元中，这些单元存在三种类型：每个单元1位（单级单元，SLC），每个单元2位（多级单元，MLC）和每个单元3位（三级单元，TLC）。‎
>
> ‎下表1显示了每种NAND闪存单元类型的详细信息。为了进行比较，我还添加了硬盘驱动器，主内存（RAM）和L1 / L2缓存的平均延迟。‎

|                     | <br />SLC                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 | MLC  | TLC  | HDD       | RAM      | L1 cache | L2 cache |
| ------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---- | ---- | --------- | -------- | -------- | -------- |
| P/E cycles          | 100k                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      | 10k  | 5k   | *         | *        | *        | *        |
| Bits per cell       | 1                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         | 2    | 3    | *         | *        | *        | *        |
| Seek latency (μs)  | *                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         | *    | *    | 9000      | *        | *        | *        |
| Read latency (μs)  | 25                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        | 50   | 100  | 2000-7000 | 0.04-0.1 | 0.001    | 0.004    |
| Write latency (μs) | 250                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       | 900  | 1500 | 2000-7000 | 0.04-0.1 | 0.001    | 0.004    |
| Erase latency (μs) | 1500                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      | 3000 | 5000 | *         | *        | *        | *        |
| *Notes*           | * metric is not applicable for that type of memory                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |      |      |           |          |          |          |
| *Sources*         | P/E cycles[[20]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)``SLC/MLC latencies [[1]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)``TLC latencies [[23]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)``Hard disk drive latencies [[18, 19, 25]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)``RAM latencies [[30, 52]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)``L1 and L2 cache latencies [[52]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref) |      |      |           |          |          |          |

Table 1: Characteristics and latencies of NAND-flash memory types compared to other memory components

Having more bits for the same amount of transistors reduces the manufacturing costs. SLC-based SSDs are known to be more reliable and have a longer life expectancy than MLC-based SSDs, but at a higher manufacturing cost. Therefore, most general public SSDs are MLC- or TLC-based, and only professional SSDs are SLC-based. Choosing the right memory type depends on the workload the drive will be used for, and how often the data is likely to be updated. For high-update workloads, SLC is the best choice, whereas for high-read and low-write workloads (ex: video storage and streaming), then TLC will be perfectly fine. Moreover, benchmarks of TLC drives under a workload of realistic usage show that the lifespan of TLC-based SSDs is not a concern in practice [[36]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref).

> ‎为相同数量的晶体管提供更多位可以降低制造成本。众所周知，基于SLC的SSD比基于MLC的SSD更可靠，预期寿命更长，但制造成本更高。因此，大多数一般公共 SSD 都是基于 MLC 或 TLC 的，只有专业的 SSD 是基于 SLC 的。选择正确的内存类型取决于驱动器将用于的工作负载以及数据可能更新的频率。对于高更新工作负载，SLC 是最佳选择，而对于高读取和低写入工作负载（例如：视频存储和流），则 TLC 将完全没问题。此外，在实际使用工作负载下，TLC驱动器的基准测试表明，基于TLC的SSD的使用寿命在实践中不是问题‎[‎[36]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎。‎

#### NAND-flash pages and blocks

Cells are grouped into a grid, called a  **block** , and blocks are grouped into planes. The smallest unit through which a block can be read or written is a  **page** . Pages cannot be erased individually, only whole blocks can be erased. The size of a NAND-flash page size can vary, and most drive have pages of size 2 KB, 4 KB, 8 KB or 16 KB. Most SSDs have blocks of 128 or 256 pages, which means that the size of a block can vary between 256 KB and 4 MB. For example, the Samsung SSD 840 EVO has blocks of size 2048 KB, and each block contains 256 pages of 8 KB each. The way pages and blocks can be accessed is covered in details in Section 3.1.

> ‎单元格被分组到一个网格中，称为‎**‎块‎**‎，块被分组到平面中。可以读取或写入块的最小单位是‎**‎页面‎**‎。页面不能单独擦除，只能擦除整个块。NAND 闪存页面大小的大小可能会有所不同，并且大多数驱动器的页面大小为 2 KB、4 KB、8 KB 或 16 KB。大多数 SSD 的块为 128 或 256 页，这意味着块的大小可以在 256 KB 到 4 MB 之间变化。例如，三星SSD 840 EVO具有大小为2048 KB的块，每个块包含256页，每个页面8 KB。访问页面和块的方式在第 3.1 节中有详细说明。‎

### 1.2 Organization of an SSD

Figure 1 below is representing an SSD drive and its main components. I have simply reproduced the basic schematic already presented in various papers [[2, 3, 6]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref).

> ‎下面的图 1 表示 SSD 驱动器及其主要组件。我只是简单地复制了各种论文中已经提出的基本原理‎[‎图[2，3，6]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎。‎

![ssd-architecture](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/02/ssd-architecture.jpg?resize=720%2C414)

Figure 1: Architecture of a solid-state drive

Commands come from the user through the host interface. At the moment I am writing this article, the two most common interfaces for newly released SSDs are Serial ATA (SATA), PCI Express (PCIe). The processor in the SSD controller takes the commands and pass them to the flash controller. SSDs also have embedded RAM memory, generally for caching purposes and to store mapping information. Section 4 covers mapping policies in more details. The packages of NAND flash memory are organized in gangs, over multiple channels, which is covered in Section 6.

Figure 2 and 3 below, reproduced from StorageReview.com [[26, 27]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref), show what SSDs look like in real life. Figure 2 shows the 512 GB version of the Samsung 840 Pro SSD, released in August 2013. As it can be seen on the circuit board, the main components are:

* 1 SATA 3.0 interface
* 1 SSD controller (Samsung MDX S4LN021X01-8030)
* 1 RAM module (256 MB DDR2 Samsung K4P4G324EB-FGC2)
* 8 MLC NAND-flash modules, each offering 64 GB of storage (Samsung K9PHGY8U7A-CCK0)

> ‎命令通过主机接口来自用户。在我写这篇文章的时候，新发布的SSD的两个最常见的接口是串行ATA（SATA），PCI Express（PCIe）。SSD 控制器中的处理器接收命令并将其传递给闪存控制器。SSD还具有嵌入式RAM内存，通常用于缓存目的和存储映射信息。第 4 节更详细地介绍了映射策略。NAND闪存的封装通过多个通道分组组织，这将在第6节中介绍。
>
> ‎下面的图2和3，转载自 StorageReview.com‎[‎[26，27]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎，显示了SSD在现实生活中的样子。图 2 显示了 2013 年 8 月发布的 512 GB 版本的三星 840 Pro SSD。从电路板上可以看出，主要部件有：‎
>
> * ‎1 个 SATA 3.0 接口‎
> * ‎1 个固态硬盘控制器（三星 MDX S4LN021X01-8030）‎
> * ‎1 内存模块 （256 MB DDR2 三星 K4P4G324EB-FGC2）‎
> * ‎8 个 MLC NAND 闪存模块，每个模块提供 64 GB 的存储空间 （三星 K9PHGY8U7A-CCK0）‎

![samsungssd840pro-01](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/01/samsungssd840pro-01.jpg?resize=720%2C366)

![samsungssd840pro-02](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/01/samsungssd840pro-02.jpg?resize=720%2C482)

Figure 2: Samsung SSD 840 Pro (512 GB) — *Pictures courtesy of StorageReview.com [[26]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)*

Figure 3 is a Micron P420m Enterprise PCIe, released late 2013. The main components are:

* 8 lanes of a PCI Express 2.0 interface
* 1 SSD controller
* 1 RAM module (DRAM DDR3)
* 64 MLC NAND-flash modules over 32 channels, each module offering 32 GB of storage (Micron 31C12NQ314 25nm)

The total memory is 2048 GB, but only 1.4 TB are available after over-provisioning.

> 图3是2013年底发布的Micron P420m Enterprise PCIe。主要组件有：‎
>
> * ‎PCI Express 2.0 接口的 8 个通道‎
> * ‎1 个固态硬盘控制器‎
> * ‎1 个内存模块 （内存 DDR3）‎
> * ‎64 个 MLC NAND 闪存模块，超过 32 个通道，每个模块提供 32 GB 存储（Micron 31C12NQ314 25nm）‎
>
> ‎总内存为 2048 GB，但过度预配后只有 1.4 TB 可用。‎

![micron-p420m-01](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/01/micron-p420m-01.jpg?resize=720%2C208)

![micron-p420m-02](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/01/micron-p420m-02.jpg?resize=720%2C448)

Figure 3: Micron P420m Enterprise PCIe (1.4 TB) — *Pictures courtesy of StorageReview.com [[27]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)*

### 1.3 Manufacturing process

Many SSD manufacturers use surface-mount technology (SMT) to produce SSDs, a production method in which electronic components are placed directly on top of printed circuit boards (PCBs). SMT lines are composed of a chain of machines, each machine being plugged into the next and having a specific task to perform in the process, such as placing components or melting the solder. Multiple quality checks are also performed throughout the entire process. Photos and videos of SMT lines can be seen in two articles by Steve Burke [[67, 68]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref), in which he visited the production facilities of Kingston Technologies in Fountain Valley, California, and in an article by Cameron Wilmot about the Kingston installations in Taiwan [[69]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref).

Other interesting resources are two videos, the first one about the Crucial SSDs by Micron [[70]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref) and the second one about Kingston [[71]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref). In the latter, which is part of Steve Burke’s articles and that I also have embedded below, Mark Tekunoff from Kingston is giving a tour of one of their SMT lines. Interesting detail, everyone in the video is wearing a cute antistatic pyjama and seems to be having a lot of fun!

> 许多SSD制造商使用表面贴装技术（SMT）来生产SSD，这是一种将电子元件直接放置在印刷电路板（PCB）顶部的生产方法。SMT生产线由一系列机器组成，每台机器都插入下一台机器，并在该过程中执行特定的任务，例如放置组件或熔化焊料。在整个过程中还执行多项质量检查。SMT生产线的照片和视频可以在Steve Burke的两篇文章中看到‎[‎[67，68]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎，其中他参观了位于加利福尼亚州喷泉谷的Kingston Technologies的生产设施，以及Cameron Wilmot关于台湾Kingston装置的文章‎[‎[69]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎。‎
>
> ‎其他有趣的资源是两个视频，第一个是关于美光‎[‎[70]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎的关键SSD，第二个是关于金士顿‎[‎[71]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎。在后者中，这是史蒂夫·伯克文章的一部分，我也在下面嵌入，来自金斯敦的马克·特库诺夫（Mark Tekunoff）正在参观他们的SMT系列之一。有趣的细节，视频中的每个人都穿着可爱的防静电睡衣，似乎玩得很开心！‎

## 2. Benchmarking and performance metrics

### 2.1 Basic benchmarks

Table 2 below shows the throughput for sequential and random workloads on different solid-state drives. For the sake of comparison, I have included SSDs released in 2008 and 2013, along with one hard drive, and one RAM memory chip.

> ‎下表 2 显示了不同固态硬盘上顺序和随机工作负载的吞吐量。为了进行比较，我包括了2008年和2013年发布的SSD，以及一个硬盘驱动器和一个RAM内存芯片。‎

|                           | Samsung 64 GB                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         | Intel X25-M                   | Samsung 840 EVO             | Micron P420m | HDD                            | RAM                    |
| ------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------- | --------------------------- | ------------ | ------------------------------ | ---------------------- |
| Brand/Model               | Samsung (MCCDE64G5MPP-OVA)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            | Intel X25-M (SSDSA2MH080G1GC) | Samsung (SSD 840 EVO mSATA) | Micron P420m | Western Digital Black 7200 rpm | Corsair Vengeance DDR3 |
| Memory cell type          | MLC                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | MLC                           | TLC                         | MLC          | *                              | *                      |
| Release year              | 2008                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  | 2008                          | 2013                        | 2013         | 2013                           | 2012                   |
| Interface                 | SATA 2.0                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              | SATA 2.0                      | SATA 3.0                    | PCIe 2.0     | SATA 3.0                       | *                      |
| Total capacity            | 64 GB                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 | 80 GB                         | 1 TB                        | 1.4 TB       | 4 TB                           | 4 x 4 GB               |
| Pages per block           | 128                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | 128                           | 256                         | 512          | *                              | *                      |
| Page size                 | 4 KB                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  | 4 KB                          | 8 KB                        | 16 KB        | *                              | *                      |
| Block size                | 512 KB                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                | 512 KB                        | 2048 KB                     | 8196 KB      | *                              | *                      |
| Sequential reads (MB/s)   | 100                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | 254                           | 540                         | 3300         | 185                            | 7233                   |
| Sequential writes (MB/s)  | 92                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    | 78                            | 520                         | 630          | 185                            | 5872                   |
| 4KB random reads (MB/s)   | 17                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    | 23.6                          | 383                         | 2292         | 0.54                           | 5319 **                |
| 4KB random writes (MB/s)  | 5.5                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | 11.2                          | 352                         | 390          | 0.85                           | 5729 **                |
| 4KB Random reads (KIOPS)  | 4                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     | 6                             | 98                          | 587          | 0.14                           | 105                    |
| 4KB Random writes (KIOPS) | 1.5                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | 2.8                           | 90                          | 100          | 0.22                           | 102                    |
| *Notes*                 | * metric is not applicable for that storage solution``** measured with 2 MB chunks, not 4 KB                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |                               |                             |              |                                |                        |
| *Metrics*               | MB/s: Megabytes per Second``KIOPS: Kilo IOPS, i.e 1000 Input/Output Operations Per Second                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |                               |                             |              |                                |                        |
| *Sources*               | Samsung 64 GB[[21]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)``Intel X25-M [[2, 28]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)``Samsung SSD 840 EVO [[22]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)``Micron P420M [[27]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)``Western Digital Black 4 TB [[25]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)``Corsair Vengeance DDR3 RAM [[30]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref) |                               |                             |              |                                |                        |

Table 2: Characteristics and throughput of solid-state drives compared to other storage solutions

An important factor for performance is the host interface. The most common interfaces for newly released SSDs are SATA 3.0, PCI Express 3.0. On a SATA 3.0 interface, data can be transferred up to 6 Gbit/s, which in practice gives around 550 MB/s, and on a PCIe 3.0 interface, data can be transferred up to 8 GT/s per lane, which in practice is roughly 1 GB/s (GT/s stands for Gigatransfers per second). SSDs on the PCIe 3.0 interface are more than a single lane. With four lanes, PCIe 3.0 can offer a maximum bandwidth of 4 GB/s, which is eight times faster than SATA 3.0. Some enterprise SSDs also offer a Serial Attached SCSI interface (SAS) which in its latest version can offer up to 12 GBit/s, although at the moment SAS is only a tiny fraction of the market.

Most recent SSDs are fast enough internally to easily reach the 550 MB/s limitation of SATA 3.0, therefore the interface is the bottleneck for them. The SSDs using PCI Express 3.0 or SAS offer tremendous performance increases [[15]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref).

> ‎影响性能的一个重要因素是主机接口。新发布的 SSD 最常见的接口是 SATA 3.0、PCI Express 3.0。在SATA 3.0接口上，数据传输速度最高可达6 Gbit/s，实际上约为550 MB/s，而在PCIe 3.0接口上，数据传输速度最高可达每通道8 GT/s，实际上约为1 GB/s（GT/s代表每秒千兆传输）。PCIe 3.0 接口上的 SSD 不仅仅是单通道。PCIe 3.0 具有四个通道，可提供 4 GB/s 的最大带宽，比 SATA 3.0 快 8 倍。一些企业SSD还提供串行连接SCSI接口（SAS），在其最新版本中可以提供高达12 GBit / s的速率，尽管目前SAS只是市场的一小部分。‎
>
> 大多数最新的SSD在内部足够快，可以轻松达到SATA 3.0的550 MB / s限制，因此接口是它们的瓶颈。使用 PCI Express 3.0 或 SAS 的 SSD 提供了巨大的性能提升 ‎[‎[15]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎。

#### PCI Express and SAS are faster than SATA

The two main host interfaces offered by manufacturers are SATA 3.0 (550 MB/s) and PCI Express 3.0 (1 GB/s per lane, using multiple lanes). Serial Attached SCSI (SAS) is also available for enterprise SSDs. In their latest versions, PCI Express and SAS are faster than SATA, but they are also more expensive.

> ‎制造商提供的两个主要主机接口是 SATA 3.0 （550 MB/s） 和 PCI Express 3.0（每通道 1 GB/s，使用多个通道）。串行连接 SCSI （SAS） 也可用于企业 SSD。在最新版本中，PCI Express和SAS比SATA更快，但它们也更昂贵。‎

### 2.2 Pre-conditioning

> If you torture the data long enough, it will confess.
> — Ronald Coase

The data sheets provided by SSD manufacturers are filled with amazing performance values. And indeed, by banging whatever random operations for long enough, manufacturers seem to always find a way to show shinny numbers in their marketing flyers. Whether or not those numbers really mean anything and allow to predict the performance of a production system is a different problem.

In his articles about common flaws in SSD benchmarking [[66]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref), Marc Bevand mentioned that for instance it is common for the IOPS of random write workloads to be reported without any mention of the span of the LBA, and that many IOPS are also reported for queue depth of 1 instead of the maximum value for the drive being tested. There are also many cases of bugs and misuses of the benchmarking tools.

Correctly assessing the performance of SSDs is not an easy task. Many articles from hardware reviewing blogs run ten minutes of random writes on a drive and claim that the drive is ready to be tested, and that the results can be trusted. However, the performance of SSDs only drops under a sustained workload of random writes, which depending on the total size of the SSD can take just 30 minutes or up to three hours. This is why the more serious benchmarks start by applying such a sustained workload of random writes, also called “ *pre-conditioning* ” [[50]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref). Figure 7 below, reproduced from an article on StorageReview.com [[26]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref), shows the effect of pre-conditioning on multiple SSDs. A clear drop in performance can be observed after around 30 minutes, where the throughput decreases and the latency increases for all drives. It then takes another four hours for the performance to slowly decay to a constant minimum.

> ‎SSD制造商提供的数据表充满了惊人的性能值。事实上，通过长时间的随机操作，制造商似乎总能找到一种方法在他们的营销传单中展示闪亮的数字。这些数字是否真的意味着什么，并允许预测生产系统的性能，这是一个不同的问题。
>
> ‎Marc Bevand 在他关于 SSD 基准测试常见缺陷的文章中‎[‎提到‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎，例如，在报告随机写入工作负载的 IOPS 时，通常只不提及 LBA 的跨度，并且许多 IOPS 也报告队列深度为 1，而不是所测试驱动器的最大值。还有许多错误和滥用基准测试工具的情况。‎
>
> ‎正确评估 SSD 的性能并非易事。来自硬件审查博客的许多文章在驱动器上运行十分钟的随机写入，并声称驱动器已准备好进行测试，并且结果可以信任。但是，SSD的性能仅在随机写入的持续工作负载下下降，具体取决于SSD的总大小，仅需30分钟或长达3个小时。这就是为什么更严肃的基准测试从应用这种持续的随机写入工作负载开始，也称为“‎*‎预调节‎*‎”‎[‎[50]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎。下面的图7，转载自 StorageReview.com ‎[‎的文章[26]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎，显示了预处理对多个SSD的影响。大约 30 分钟后，可以观察到性能明显下降，其中所有驱动器的吞吐量降低，延迟增加。然后，再过四个小时，性能才能慢慢衰减到恒定的最小值。‎

![writes-preconditioning](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/01/writes-preconditioning.jpg?resize=720%2C829)

Figure 7: Effect of pre-conditioning on multiple SSD models — *Pictures courtesy of StorageReview.com [[26]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)*

What is happening in Figure 7 essentially is that, as explained in Section 5.2, the amount of random writes is so large and applied in such a sustained way that the garbage collection process is unable to keep up in background. The garbage collection must erase blocks as write commands arrive, therefore competing with the foreground operations from the host. People using pre-conditioning claim that the benchmarks it produces accurately represent how a drive will behave in its worst possible state. Whether or not this is a good model for how a drive will behave under all workloads is arguable.

In order to compare various models coming from different manufacturers, a common ground must be found, and the worst possible state is a valid one. But picking the drive that performs best under the worst possible workload does not always guarantee that it will perform best under the workload of a production environment. Indeed, in most production environments, an SSD drive will serve one and only one system. That system has a specific workload due to its internal characteristics, and therefore a better and more accurate way to compare different drives would be to run the same replay of this workload on those drives, and then compare their respective performance. This is why, even though a pre-conditioning using a sustained workload of random writes allows for a fair comparison of different SSDs, one has to be careful and should, whenever possible, run in-house benchmarks based on the target workload. Benchmarking in-house also allows not to over-allocate resources, by avoiding using the “best” SSD model when a cheaper one would be enough and save a lot of money.

> ‎图 7 中发生的情况基本上是，如第 5.2 节中所述，随机写入量非常大，并且以如此持续的方式应用，以至于垃圾回收过程无法在后台跟上。垃圾回收必须在写入命令到达时擦除块，因此与主机的前台操作竞争。使用预制的人声称，它产生的基准测试准确地代表了驱动器在最差状态下的行为方式。这是否是驱动器在所有工作负载下行为的良好模型，这是有争议的。‎
>
> ‎为了比较来自不同制造商的各种型号，必须找到一个共同点，而最坏的状态是有效的状态。但是，选择在尽可能差的工作负载下表现最佳的驱动器并不总是保证它在生产环境的工作负载下表现最佳。实际上，在大多数生产环境中，SSD驱动器将服务于一个且只有一个系统。由于其内部特征，该系统具有特定的工作负载，因此比较不同驱动器的更好，更准确的方法是在这些驱动器上运行此工作负载的相同重播，然后比较它们各自的性能。这就是为什么，即使使用持续的随机写入工作负载的预调节允许对不同的SSD进行公平的比较，也必须小心，并且应该尽可能根据目标工作负载运行内部基准测试。内部基准测试还允许不要过度分配资源，避免使用“最佳”SSD型号，而更便宜的SSD型号就足够了，并节省大量资金。‎

#### Benchmarking is hard

Testers are humans, therefore not all benchmarks are exempt of errors. Be careful when reading the benchmarks from manufacturers or third parties, and use multiple sources before trusting any numbers. Whenever possible, run your own in-house benchmarking using the specific workload of your system, along with the specific SSD model that you want to use. Finally, make sure you look at the performance metrics that matter most for the system at hand.

> ‎测试人员是人类，因此并非所有基准测试都可以免于错误。在阅读制造商或第三方的基准时要小心，并在信任任何数字之前使用多个来源。只要有可能，就使用系统的特定工作负载以及要使用的特定SSD型号运行自己的内部基准测试。最后，请务必查看对手头系统最重要的性能指标。‎

### 2.3 Workloads and metrics

Performance benchmarks all share the same varying parameters and provide results using the same metrics. In this section, I wish to give some insights as to how to interpret those parameters and metrics.

The parameters used are generally the following:

* The type of workload: can be a specific benchmark based on data collected from users, or just only sequential or random accesses of the same type (ex: only random writes)
* The percentages of reads and writes performed concurrently (ex: 30% reads and 70% writes)
* The queue length: this is the number of concurrent execution threads running commands on a drive
* The size of the data chunks being accessed (4 KB, 8 KB, etc.)

Benchmark results are presented using different metrics. The most common are:

* Throughput: The speed of transfer, generally in KB/s or MB/s, respectively kilobytes per second, and megabytes per second. This is the metric chosen for sequential benchmarks.
* IOPS: the number of Input/Output Operations Per Second, each operations being of the same data chunk size (generally 4 KB/s). This is the metrics chosen for the random benchmarks.
* Latency: the response time of a device after a command is emitted, generally in μs or ms, respectively microseconds or milliseconds.

While the throughput is easy to understand and relate to, the IOPS is more difficult to grasp. For example, if a disk shows a performance for random writes at 1000 IOPS for 4 KB chunks, this means that the throughput is of 1000 x 4096 = 4 MB/s. Consequently, a high IOPS will translate into a high throughput only if the size of the chunks is the largest possible. A high IOPS at a low average chuck size will translate into a low throughput.

To illustrate this point, let’s imagine that we have a logging system performing tiny updates over thousands of different files per minute, giving a performance of 10k IOPS. Because the updates are spread over so many different files, the throughput could be close to something like 20 MB/s, whereas writing sequentially to only one file with the same system could lead to an increased throughput of 200 MB/s, which is a tenfold improvement. I am making up those numbers for the sake of this example, although they are close to production systems I have encountered.

Another concept to grasp is that a high throughput does not necessarily means a fast system. Indeed, if the latency is high, no matter how good is the throughput, the overall system will be slow. Let’s take the example of a hypothetical single-threaded process that requires connections to 25 databases, each connection having a latency of 20 ms. Because the connection latencies are cumulative, obtaining the 25 connections will require 25 x 20 ms = 500 ms. Therefore, even if the machines running the database queries have fast network cards, let’s say 5 GBits/s of bandwidth, the script will still be slow due to the latency.

The takeaway from this section is that it is important to keep an eye on all the metrics, as they will show different aspects of the system and will allow to identify the bottlenecks when they come up. When looking at the benchmarks of SSDs and deciding which model to pick, keeping in mind which metric is the most critical to the system in which those SSDs will used is generally a good rule of thumb. Then of course, nothing will replace proper in-house benchmarking as explained in Section 2.2.

An interesting follow-up on the topic is the article “ *IOPS are a scam* ” by Jeremiah Peschka [[46]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref).

> ‎性能基准测试都共享相同的参数，并使用相同的指标提供结果。在本节中，我希望就如何解释这些参数和指标提供一些见解。‎
>
> ‎使用的参数一般如下：‎
>
> * ‎工作负载类型：可以是基于从用户收集的数据的特定基准，也可以只是相同类型的顺序或随机访问（例如：仅随机写入）‎
> * ‎同时执行的读取和写入的百分比（例如：30% 的读取和 70% 的写入）‎
> * ‎队列长度：这是在驱动器上运行命令的并发执行线程数‎
> * ‎正在访问的数据块的大小（4 KB、8 KB 等）‎
>
> ‎基准测试结果使用不同的指标显示。最常见的是：‎
>
> * ‎吞吐量：传输速度，通常以 KB/s 或 MB/s 为单位，分别为千字节/秒和兆字节/秒。这是为顺序基准测试选择的指标。‎
> * ‎IOPS：每秒输入/输出操作数，每个操作具有相同的数据块大小（通常为 4 KB/s）。这是为随机基准选择的指标。‎
> * ‎时延：设备发出命令后的响应时间，一般以μs或ms为单位，分别为微秒或毫秒。‎
>
> ‎虽然吞吐量易于理解和关联，但 IOPS 更难掌握。例如，如果磁盘显示 4 KB 块的随机写入性能为 1000 IOPS，则意味着吞吐量为 1000 x 4096 = 4 MB/s。因此，仅当块的大小尽可能大时，高 IOPS 才会转化为高吞吐量。在低平均卡盘尺寸下的高 IOPS 将转化为低吞吐量。
>
> ‎为了说明这一点，让我们想象一下，我们有一个日志记录系统，每分钟对数千个不同的文件执行微小的更新，从而提供 10k IOPS 的性能。由于更新分布在许多不同的文件上，因此吞吐量可能接近 20 MB/s，而按顺序仅写入具有相同系统的一个文件可能会导致吞吐量增加 200 MB/s，这是十倍的改进。为了这个例子，我正在编造这些数字，尽管它们与我遇到的生产系统很接近。‎
>
> ‎另一个需要掌握的概念是，高吞吐量并不一定意味着快速的系统。事实上，如果延迟很高，无论吞吐量有多好，整个系统都会很慢。让我们以一个假设的单线程进程为例，该进程需要连接到 25 个数据库，每个连接的延迟为 20 毫秒。由于连接延迟是累积的，因此获取 25 个连接将需要 25 x 20 ms = 500 ms。因此，即使运行数据库查询的计算机具有快速网卡（假设 5 GBits/s 的带宽），脚本仍会因延迟而变慢。‎
>
> ‎本节的结论是，重要的是要关注所有指标，因为它们将显示系统的不同方面，并允许在出现瓶颈时识别瓶颈。在查看SSD的基准并决定选择哪种型号时，请记住哪个指标对使用这些SSD的系统最关键通常是一个很好的经验法则。当然，没有什么可以取代适当的内部基准测试，如第2.2节所述。‎
>
> ‎关于这个话题的一个有趣的后续是Jeremiah Peschka的文章“‎*‎IOPS是一个骗局‎*‎”‎[‎[46]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/#ref)‎。‎

## What’s next

Part 3 is available [here](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/). You can also go to the [Table of Content](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/) for this series of articles, and if you’re in a rush, you can also directly go to Part 6, which is summarizing the content from all the other parts.

> 第 3 部分可‎[‎在此处获取‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)‎。您还可以转到本系列文章的‎[‎目录‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)‎，如果您赶时间，也可以直接转到第6部分，该部分总结了所有其他部分的内容。‎

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
