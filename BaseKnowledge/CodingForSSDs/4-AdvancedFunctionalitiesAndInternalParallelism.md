# Coding for SSDs – Part 4: Advanced Functionalities and Internal Parallelism

This is Part 4 over 6 of “Coding for SSDs”, covering Sections 5 and 6. For other parts and sections, you can refer to the [Table to Contents](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/). This is a series of articles that I wrote to share what I learned while documenting myself on SSDs, and on how to make code perform well on SSDs. If you’re in a rush, you can also go directly to [Part 6](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/), which is summarizing the content from all the other parts.

In this part, I cover briefly some of the main SSD functionalities such as TRIM and over-provisioning. I am also presenting the different levels of internal parallelism in an SSD, and the concept of clustered block.

> ‎这是“SSD 编码”的第 4 部分，第 6 部分，涵盖第 5 节和第 6 节。对于其他部分和节，可以参考‎[‎目录‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)‎。这是我写的一系列文章，旨在分享我在SSD上记录自己时学到的东西，以及如何使代码在SSD上表现良好。如果您赶时间，也可以直接转到‎[‎第6部分‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/)‎，该部分总结了所有其他部分的内容。‎
>
> ‎在本部分中，我将简要介绍一些主要的 SSD 功能，例如 TRIM 和过度配置。我还介绍了SSD中不同级别的内部并行性，以及集群块的概念。‎

![ssd-presentation-04](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/02/ssd-presentation-04.jpg?resize=360%2C434)

## 5. Advanced functionalities

### 5.1 TRIM

Let’s imagine that a program write files to all the logical block addresses of an SSD: this SSD would be considered full. Now let’s assume that all those files gets deleted. The filesystem would report 100% free space, although the drive would still be full, because an SSD controller has no way to know when logical data is deleted by the host. The SSD controller will see the free space only when the logical block addresses that used to be holding the files get overwritten. At that moment, the garbage collection process will erase the blocks associated with the deleted files, providing free pages for incoming writes. As a consequence, instead of erasing the blocks as soon as they are known to be holding stale data, the erasing is being delayed, which hurts performance badly.

Another concern is that, since the pages holding deleted files are unknown to the SSD controller, the garbage collection mechanism will continue move them around to ensure wear leveling. This increases write amplification, and interferes with the foreground workload of the host for no good reason.

A solution to the problem of delayed erasing is the TRIM command, which can be sent by the operating system to notify the SSD controller that pages are no longer in use in the logical space. With that information, the garbage collection process knows that it doesn’t need to move those pages around, and also that it can erase them whenever needed. The TRIM command will only work if the SSD controller, the operating system, and the filesystem are supporting it.

The Wikipedia page for the TRIM command is listing the operating systems and filesystems that support TRIM [[16]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref). Under Linux, support for the ATA TRIM was added in version 2.6.33. Although the ext2 and ext3 filesystems do not support TRIM, ext4 and XFS, among others, do support it. Under Mac OS 10.6.8, HFS+ supports the TRIM operation. As for Windows 7, it only supports TRIM for SSD using a SATA interface and not PCI-Express.

The majority of recent drives support TRIM, and indeed, allowing the garbage collection to work as early as possible significantly improves future performance. Therefore, it is strongly preferable to use SSDs that support TRIM, and to make sure that support is enabled both at the operating system and filesystem levels.

> ‎让我们想象一下，一个程序将文件写入SSD的所有逻辑块地址：这个SSD将被视为已满。现在，让我们假设所有这些文件都被删除了。文件系统将报告 100% 的可用空间，尽管驱动器仍将已满，因为 SSD 控制器无法知道主机何时删除逻辑数据。仅当过去保存文件的逻辑块地址被覆盖时，SSD 控制器才会看到可用空间。此时，垃圾回收过程将擦除与已删除文件关联的块，为传入的写入提供空闲页面。因此，一旦知道块保存了陈旧数据，就不会立即擦除块，而是延迟擦除，这会严重损害性能。‎
>
> ‎另一个问题是，由于SSD控制器不知道保存已删除文件的页面，垃圾回收机制将继续移动它们以确保磨损均衡。这会增加写入放大，并无缘无故地干扰主机的前景工作负载。‎
>
> ‎延迟擦除问题的解决方案是TRIM命令，该命令可以由操作系统发送，以通知SSD控制器逻辑空间中不再使用页面。有了这些信息，垃圾回收过程就知道它不需要移动这些页面，并且可以随时擦除它们。仅当 SSD 控制器、操作系统和文件系统支持 TRIM 命令时，它才有效。‎
>
> ‎TRIM 命令的维基百科页面列出了支持 TRIM ‎[‎[16]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎ 的操作系统和文件系统。在 Linux 下，在版本 2.6.33 中添加了对 ATA TRIM 的支持。虽然 ext2 和 ext3 文件系统不支持 TRIM，但 ext4 和 XFS 等文件确实支持它。在 Mac OS 10.6.8 下，HFS+ 支持 TRIM 操作。至于Windows 7，它只支持使用SATA接口的SSD TRIM，而不是PCI-Express。‎
>
> ‎最近的大多数驱动器都支持 TRIM，实际上，允许垃圾回收尽早工作可以显著提高未来的性能。因此，强烈建议使用支持 TRIM 的 SSD，并确保在操作系统和文件系统级别都启用了支持。‎

### 5.2 Over-provisioning

Over-provisioning is simply having more physical blocks than logical blocks, by keeping a ratio of the physical blocks reserved for the controller and not visible to the user. Most manufacturers of professional SSDs already include some over-provisioning, generally in the order of 7 to 25% [[13]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref). Users can create more over-provisioning simply by partitioning a disk to a lower logical capacity than its maximum physical capacity. For example, one could create a 90 GB partition in a 100 GB drive, and leave the remaining 10 GB for over-provisioning. Even if the over-provisioned space is not visible at the level of the operation system, the SSD controller can still see it. The main reason for which manufacturers are offering over-provisioning is to cope with the inherent limited lifespan of NAND-flash cells. The invisible over-provisioned blocks are here to seamlessly replace the blocks wearing off in the visible space.

AnandTech has an interesting article showing the impact of over-provisioning on the life-span and performance of SSDs [[34]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref). For the disk they studied, the conclusion was that performance increased dramatically simply by making sure that 25% of the space was reserved for over-provisioning — summing up all levels of over-provisioning. Another interesting result was presented in an article by Percona, in which they tested an Intel 320 SSD and showed that the write throughput decreased as the disk was getting filled up [[38]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref).

Here is my explanation on what is happening. The garbage collection is using idle time to erase stale pages in the background. But since the erase operation has a higher latency than the write operation, i.e. erasing takes more time than writing, an SSD under a heavy workload of continuous random writes would use up all of its free blocks before the garbage collection would have time to erase the stale pages. At that point, the FTL would be unable to catch up with the foreground workload of random writes, and the garbage collection process would have to erase blocks at the same time as write commands are coming in. This is when performance drops and the SSD appears to be performing badly in benchmarks, as shown in Figure 7 below. Therefore, over-provisioning can act as a buffer to absorb high throughput write workloads, leaving enough time to the garbage collection to catch up and erase blocks again. How much over-provisioning is needed depends mostly on the workload in which the SSD will be used and how much writes it will need to absorb. As a rule of thumb, somewhere around 25% of over-provisioning is recommended for sustained workload of random writes [[34]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref). If the workload is not so heavy, somewhere around 10-15% may be largely enough.

> ‎过度配置只是通过保持为控制器保留的物理块的比率并且对用户不可见，从而拥有比逻辑块更多的物理块。大多数专业 SSD 制造商已经包含一些过度配置，通常约为 7% 到 25% ‎[‎[13]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。用户只需将磁盘分区到低于其最大物理容量的逻辑容量，即可创建更多的过度配置。例如，可以在 100 GB 的驱动器中创建一个 90 GB 的分区，并将剩余的 10 GB 留给预留空间。即使超额配置的空间在操作系统级别不可见，SSD 控制器仍能看到它。制造商提供过度配置的主要原因是应对NAND闪存单元固有的有限寿命。这里提供了不可见的过度配置块，以无缝替换可见空间中磨损的块。‎
>
> ‎AnandTech有一篇有趣的文章，展示了过度配置对SSD寿命和性能的影响‎[‎[34]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。对于他们研究的磁盘，结论是，只需确保为过度配置保留 25% 的空间（总结了所有级别的过度配置），性能就会显著提高。Percona在一篇文章中提出了另一个有趣的结果，他们在其中测试了Intel 320 SSD，并表明随着磁盘被填满，写入吞吐量降低‎[‎[38]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。‎
>
> ‎以下是我对正在发生的事情的解释。垃圾回收正在使用空闲时间在后台擦除过时的页面。但是，由于擦除操作的延迟高于写入操作，即擦除比写入花费更多的时间，因此在连续随机写入的繁重工作负载下，SSD将在垃圾回收有时间擦除过时页面之前耗尽其所有可用块。此时，FTL 将无法赶上随机写入的前台工作负载，垃圾回收过程必须在写入命令传入的同时擦除块。这是性能下降并且 SSD 在基准测试中表现不佳的情况，如下面的图 7 所示。因此，过度预配可以充当缓冲区来吸收高吞吐量写入工作负荷，从而为垃圾回收留出足够的时间来赶上并再次擦除块。需要多少过度配置主要取决于使用 SSD 的工作负载以及它需要吸收的写入量。根据经验，对于随机写入的持续工作负载，建议大约 25% 的过度配置 ‎[‎[34]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。如果工作量不是那么重，那么大约10-15%可能就足够了。‎

#### Over-provisioning is useful for wear leveling and performance

A drive can be over-provisioned simply by formatting it to a logical partition capacity smaller than the maximum physical capacity. The remaining space, invisible to the user, will still be visible and used by the SSD controller. Over-provisioning helps the wear leveling mechanisms to cope with the inherent limited lifespan of NAND-flash cells. For workloads in which writes are not so heavy, 10% to 15% of over-provisioning is enough. For workloads of sustained random writes, keeping up to 25% of over-provisioning will improve performance. The over-provisioning will act as a buffer of NAND-flash blocks, helping the garbage collection process to absorb peaks of writes.

From there, it can also be deduced that over-provisioning offers even greater improvements for setups in which the TRIM command is not supported — note that I am just making an assumption here, and that I have yet to find a reference to support this idea. Let’s imagine that only 75% of the drive is used by the operating system and the remaining 25% is reserved for over-provisioning. Because the SSD controller can see the whole drive, 100% of the blocks are rotating and alternating between being used, stale, and erased, although only 75% of the physical NAND-flash memory is actually used at any single moment in time. This means that the remaining 25% of physical memory should be safely assumed not to be holding any data, since it is not mapped to any logical block addresses. Therefore, the garbage collection process should be able to erase blocks from the over-provisioned space in advance, and this even in the absence of TRIM support.

> ‎只需将驱动器格式化为小于最大物理容量的逻辑分区容量，即可对其进行过度置备。对用户不可见的剩余空间仍将可见并由 SSD 控制器使用。过度配置有助于磨损均衡机制应对NAND闪存单元固有的有限寿命。对于写入量不大的工作负载，10% 到 15% 的过度预配就足够了。对于持续随机写入的工作负载，保留高达 25% 的过度预配将提高性能。过度配置将充当 NAND 闪存块的缓冲区，帮助垃圾回收过程吸收写入峰值。‎
>
> ‎从那里还可以推断出，过度配置为不支持TRIM命令的设置提供了更大的改进 - 请注意，我只是在这里做一个假设，我还没有找到支持这个想法的参考。假设操作系统仅使用驱动器的 75%，其余 25% 用于过度预配。由于SSD控制器可以看到整个驱动器，因此100%的块在使用，过时和擦除之间旋转和交替，尽管在任何一个时刻实际上只有75%的物理NAND闪存被使用。这意味着应安全地假定剩余的 25% 的物理内存不保存任何数据，因为它未映射到任何逻辑块地址。因此，垃圾回收过程应该能够提前从过度预配的空间中删除块，即使在没有 TRIM 支持的情况下也是如此。‎

### 5.3 Secure Erase

Some SSD controllers offer the *ATA Secure Erase* functionality, the goal being to restore the performance of the drive back to its fresh out-of-box state. This command erases all data written by the user and resets the FTL mapping tables, but obviously cannot overcome the physical limitations of the limited P/E cycles. Even though this functionality looks very promising in the specs, it’s up to each manufacturer to implement it correctly. In their review of the secure erase command, Wei et al., 2011, have shown that over the 12 models of SSDs studied, only eight offered the ATA Secure Erase functionality, and over those eight drives, three had buggy implementations [[11]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref).

The implications for performance are important, and they are all the more so important for security, but it is not my intent to cover this topic here. There are a couple of discussions on Stack Overflow which explain in more details how to reliably erase data from an SSD [[48, 49]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref).

> ‎某些 SSD 控制器提供 ‎*‎ATA 安全擦除‎*‎功能，目标是将驱动器的性能恢复到其全新的全新状态。此命令擦除用户写入的所有数据并重置 FTL 映射表，但显然无法克服有限 P/E 周期的物理限制。尽管此功能在规格中看起来非常有前途，但每个制造商都有责任正确实现它。Wei等人在2011年对安全擦除命令的审查中表明，在研究的12种SSD型号中，只有八种提供ATA安全擦除功能，而在这八个驱动器上，三个驱动器有错误的实现‎[‎[11]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。‎
>
> ‎对性能的影响很重要，它们对安全性来说也更加重要，但我不打算在这里介绍这个主题。关于Stack Overflow有一些讨论，更详细地解释了如何可靠地擦除SSD中的数据‎[‎[48，49]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。‎

### 5.4 Native Command Queueing (NCQ)

Native Command Queueing (NCQ) is a feature of Serial ATA that allows for an SSD to accept multiple commands from the host in order to complete them concurrently using the internal parallelism [[3]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref). In addition to reducing latency due to the drive, some newer drives also use NCQ to cope with latency from the host. For example, NCQ can prioritize incoming commands to ensure that the drive always has commands to process while the host CPU is busy [[39]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref).

> ‎本机命令队列 （NCQ） 是串行 ATA 的一项功能，它允许 SSD 接受来自主机的多个命令，以便使用内部并行度 ‎[‎[3]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎ 同时完成这些命令。除了减少驱动器引起的延迟外，一些较新的驱动器还使用 NCQ 来应对来自主机的延迟。例如，NCQ 可以确定传入命令的优先级，以确保驱动器在主机 CPU 繁忙时始终具有要处理的命令 ‎[‎[39]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。‎

### 5.5 Power-loss protection

Whether it is at home or in datacenter, power loss will happen. Some manufacturers include a supercapacitor in their SSD architecture, which is supposed to hold enough power in order to commit the I/O requests in the bus in case of a power outage, and leave the drive in a consistent state. The problem is that not all SSD manufacturer include a supercapacitor or some sort of power-fault data protection for their drives, and that those who include it do not always mention it in their specifications. Then, like for the secure erase command, it is not clear whether or not the power-fault mechanisms are correctly implemented and will indeed protect the drive from data corruption when a power outage occurs.

A study by Zheng et al., 2013, tested 15 SSDs, without revealing their brands [[72]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref). They stressed the drives with various power faults, and found that 13 out of the 15 tested SSDs ended up losing some data or being massively corrupted. Another article about power fault by Luke Kenneth Casson Leighton showed that three out of four tested drives were left in a corrupted state, and that the fourth was fine (an Intel drive) [[73]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref).

SSDs are still a very young technology and I am convinced that their resistance to data corruption under power fault will improve over the next generations. Nevertheless for the time being, it is probably worth it to invest in an uninterruptible power supply (UPS) in datacenter setups. And as with any other storage solution, backup sensitive data regularly.

> ‎无论是在家中还是在数据中心，都会发生断电。一些制造商在其SSD架构中包括一个超级电容器，它应该保持足够的功率，以便在断电时在总线中提交I / O请求，并使驱动器处于一致状态。问题在于，并非所有SSD制造商都为其驱动器提供超级电容器或某种电源故障数据保护，并且包含它的人并不总是在其规格中提及它。然后，与安全擦除命令一样，尚不清楚电源故障机制是否正确实现，并且在发生断电时确实可以保护驱动器免受数据损坏。‎
>
> ‎Zheng等人2013年的一项研究测试了15种SSD，但没有透露它们的品牌‎[‎[72]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。他们向驱动器施加了各种电源故障的压力，并发现15个测试的SSD中有13个最终丢失了一些数据或被大规模损坏。Luke Kenneth Casson Leighton的另一篇关于电源故障的文章显示，四个测试驱动器中有三个处于损坏状态，第四个驱动器很好（英特尔驱动器）‎[‎[73]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。‎
>
> ‎SSD仍然是一项非常年轻的技术，我相信它们在电源故障下对数据损坏的抵抗力将在下一代中得到改善。尽管如此，就目前而言，在数据中心设置中投资不间断电源（UPS）可能是值得的。与任何其他存储解决方案一样，请定期备份敏感数据。‎

## 6. Internal Parallelism in SSDs

### 6.1 Limited I/O bus bandwidth

Due to physical limitations, an asynchronous NAND-flash I/O bus cannot provide more than 32-40 MB/s of bandwidth [[5]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref). The only way for SSD manufacturers to increase performance is to design their drives in such a way that multiple packages can be parallelized or interleaved. A good explanation of interleaving can be found in Section 2.2 of [[2]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref).

By combining all the levels of internal parallelism inside an SSD, multiple blocks can be accessed simultaneously across separate chips, as a unit called a  **clustered block** . Explaining all the details regarding the inner parallelism of an SSD is not my intent here, therefore I am just covering briefly the levels of parallelism and the clustered block. To learn more about these topics and more generally about parallelism inside SSDs, two great starting points are the papers [[2, 3]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref). In addition, the advanced commands such as *copyback* and *inter-plane transfer* are presented in [[5]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref).

> ‎由于物理限制，异步 NAND 闪存 I/O 总线无法提供超过 32-40 MB/s 的带宽 ‎[‎[5]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。SSD制造商提高性能的唯一方法是以这样的方式设计其驱动器，即多个封装可以并行化或交错。交错的一个很好的解释可以在‎[‎[2]的第2.2节中找到‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。‎
>
> ‎通过将SSD内部的所有内部并行性级别组合在一起，可以跨单独的芯片同时访问多个块，作为称为‎**‎集群块的‎**‎单元。解释有关SSD内部并行性的所有细节并不是我在这里的意图，因此我只是简要介绍并行度和集群块的级别。要了解有关这些主题的更多信息以及更广泛地了解SSD内部的并行性，两个很好的起点是论文‎[‎[2，3]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。此外，高级命令，如‎*‎复制‎*‎和‎*‎平面间传输‎*‎，在‎[‎[5]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎中显示。‎

#### Internal parallelism

Internally, several levels of parallelism allow to write to several blocks at once into different NAND-flash chips, to what is called a “clustered block”.

> ‎在内部，几个级别的并行性允许同时将多个块写入不同的NAND闪存芯片，即所谓的“集群块”。‎

### 6.2 Multiple levels of parallelism

Figure 6 below shows the internals of a NAND-flash package, which is organized as a hierarchical structure. The levels are channel, package, chip, plane, block, and page. As exposed in [[3]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref), those different levels offer parallelism as follows:

* **Channel-level parallelism** . The flash controller communicates with the flash packages through multiple channels. Those channels can be accessed independently and simultaneously. Each individual channel is shared by multiple packages.
* **Package-level parallelism** . The packages on a channel can be accessed independently. Interleaving can be used to run commands simultaneously on the packages shared by the same channel.
* **Chip-level parallelism** . A package contains two or more chips, which can be accessed independently in parallel. Note: chips are also called “dies”.
* **Plane-level parallelism** . A chip contains two or more planes. The same operation (read, write or erase) can be run simultaneously on multiple planes inside a chip. Planes contain blocks, which themselves contains pages. The plane also contains registers (small RAM buffers), which are used for plane-level operations.

> ‎下面的图6显示了NAND闪存封装的内部结构，它被组织为分层结构。级别包括通道、封装、芯片、平面、块和页。如 ‎[‎[3]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎ 所示，这些不同的级别提供并行性，如下所示：‎
>
> * **‎通道级并行性‎**‎。闪存控制器通过多个通道与闪存包进行通信。这些通道可以独立且同时访问。每个单独的通道由多个包共享。‎
> * **‎包级并行性‎**‎。可以独立访问通道上的包。交错可用于在同一通道共享的包上同时运行命令。‎
> * **‎芯片级并行性‎**‎。一个封装包含两个或多个芯片，可以并行独立访问。注意：芯片也称为“芯片”。‎
> * **‎平面级并行性‎**‎。芯片包含两个或多个平面。相同的操作（读取、写入或擦除）可以在芯片内部的多个平面上同时运行。平面包含块，块本身包含页面。该平面还包含寄存器（小 RAM 缓冲区），用于平面级操作。‎

![ssd-package](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/02/ssd-package.jpg?resize=720%2C867)

Figure 6: NAND flash package

### 6.3 Clustered blocks

Multiple blocks accessed across multiple chips are called a **clustered block** [[2]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref). The idea is similar to the concept of **striping** encountered in RAID systems [[1, 5]](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref).

Logical block addresses accessed at once are striped over different SSD chips in distinct flash packages. This is done thanks to the mapping algorithm of the FTL, and it is independent of whether or not those addresses are sequential. Striping blocks allows to use multiple channels simultaneously and combine their bandwidths, and also to perform multiple read, write and erase operations in parallel. This means that I/O operations that are both *aligned* and *multiple* of the clustered block size guarantee an optimal use of all the performance offered by the various levels of internal parallelism in an SSD. See Sections 8.2 and 8.3 for more information about clustered blocks.

> ‎跨多个芯片访问的多个块称为‎**‎集群块‎**[‎[2]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。这个想法类似于 RAID 系统中遇到的‎**‎条带化‎**‎概念 ‎[‎[1， 5]‎](https://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/#ref)‎。‎
>
> ‎一次访问的逻辑块地址在不同的闪存包中的不同SSD芯片上条带化。这要归功于FTL的映射算法，并且与这些地址是否是顺序的无关。条带化块允许同时使用多个通道并组合其带宽，还可以并行执行多个读取、写入和擦除操作。这意味着，既‎*‎对齐‎*‎又是群集块大小的‎*‎倍数‎*‎的 I/O 操作，可确保最佳地利用 SSD 中各种级别的内部并行性提供的所有性能。参阅第 8.2 节和第 8.3 节，了解有关集群块的更多信息。‎

## What’s next

Part 5 is available [here](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/). You can also go to the [Table of Content](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/) for this series of articles, and if you’re in a rush, you can also directly go to [Part 6](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/), which is summarizing the content from all the other parts.

> ‎第 5 部分可‎[‎在此处获取‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/)‎。您还可以转到本系列文章的‎[‎目录‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)‎，如果您赶时间，也可以直接转到‎[‎第6部分‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/)‎，该部分总结了所有其他部分的内容。‎

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
