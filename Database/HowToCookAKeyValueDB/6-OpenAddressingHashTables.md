# Implementing a Key-Value Store – Part 6: Open-Addressing Hash Tables


This is Part 6 of the IKVS series, “Implementing a Key-Value Store”. You can also check the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) for other parts.

In this article, I will compare several open-addressing hash tables: Linear Probing, Hopscotch hashing, and Robin Hood hashing. I have already done some work on this topic, and in this article I want to gather data for more metrics in order to decide which hash table I will use for my key-value store.

The result section also contains an interesting observation about the maximum DIB for Robin Hood hashing, which originated from Kristofer Karlsson, a software engineer at Spotify and the author of the key-value store Sparkey.

This article will cover:

1. Open-addressing hash tables
2. Metrics
3. Experimental Protocol
4. Results and Discussion
5. Conclusion
6. References

> ‎这是 IKVS 系列的第 6 部分“实现键值存储”。您还可以在‎[‎目录中‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎查看其他部分
>
> ‎在本文中，我将比较几个开放寻址哈希表：线性探测、Hopscotch 哈希和罗宾汉哈希。我已经在这个主题上做了一些工作，在本文中，我想收集更多指标的数据，以便确定我将哪种形式用于键值存储。‎
>
> 结果部分还包含关于罗宾汉哈希的最大DIB的有趣观察，该观察结果源于Spotify的软件工程师Kristofer Karlsson，也是键值存储Sparkey的作者。‎
>
> ‎本文将介绍：‎
>
> 1. 开放寻址哈希表‎
> 2. 指标‎
> 3. 实验方案‎
> 4. 结果和讨论‎
> 5. 结论‎
> 6. 引用‎

## 1. Open-addressing hash tables

I have settled on open-addressing hash tables as the data structure for my key-value store, because such algorithms minimize the number of I/O operations to access entries, which is ideal when the data is persisted to a secondary storage such as HDD or SSD. However, one of the main drawbacks of hash tables compared to other data structures such as trees is that they make it difficult to access the entries sorted by keys. I am aware of this is a trade-off, and I make the choice to pursue with open-addressing hash tables anyway.

There are several open addressing algorithms that are great candidates — Linear Probing, Cuckoo hashing, Hopscotch hashing, and Robin Hood hashing — and which I have described in details in previous articles [[1, 3, 6, 7]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref). I have chosen not to pursue my tests any further with Cuckoo hashing, because even if it is an interesting approach, it forces data to be read from non-contiguous memory locations, which is not cache-friendly and incurs additional I/O accesses when persisted to a secondary storage. The rest of this article will therefore only consider Linear Probing, Hopscotch hashing, and Robin Hood hashing.

> 我已确定使用开放寻址哈希表作为键值存储的数据结构，因为此类算法可以最大程度地减少访问条目的 I/O 操作数，这在数据保存到辅助存储（如 HDD 或 SSD）时是理想的选择。但是，与其他数据结构（如树）相比，哈希表的主要缺点之一是它们难以访问按键排序的条目。我知道这是一种权衡，无论如何，我都会选择使用开放寻址哈希表。‎
>
> 有几种开放寻址算法是很好的候选者 - 线性探测，Cuckoo哈希，Hopscotch哈希和罗宾汉哈希 - 我在之前的文章中详细描述了它们‎[‎[1，3，6，7]‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎。我选择不再使用Cuckoo哈希进行进一步的测试，因为即使它是一种有趣的方法，它也会强制从不连续的内存位置读取数据，这不是缓存友好的，并且在持久保存到辅助存储时会产生额外的I / O访问。因此，本文的其余部分将仅考虑线性探测，Hopscotch哈希和Robin Hood哈希。

## 2. Metrics

In my previous articles about hashing [[1, 3, 6, 7]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref), I was only using one metric to assess the performance of the hash tables: the distance to initial bucket (DIB). Since then I have collected data for more metrics to get a better understanding of the behavior of hash tables, and I am presenting those metrics in this section. In addition, Figure 1 below gives a quick visual representation of the DIB, DFB and DMB.

> ‎在我之前关于哈希 ‎[‎[1， 3， 6， 7]‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎ 的文章中，我只使用一个指标来评估哈希表的性能：到初始存储桶 （DIB） 的距离。从那时起，我收集了更多指标的数据，以便更好地了解哈希表的行为，我将在本节中介绍这些指标。此外，下面的图 1 给出了 DIB、DFB 和 DMB 的快速可视化表示。‎

### 2.1 DIB: Distance to Initial Bucket

The distance to the initial bucket (DIB) for a given entry is the distance between the bucket where an entry is stored and its initial bucket, which is the bucket to which its key was initially hashed. This metric is the most important of all because it shows how much work is required to perform an entry retrieval, which is used by all the operations of a hash table: exists(), get(), put() and remove(). Keeping that metric as low as possible is crucial for achieving high performance. Note that the original Robin Hood paper [[8]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref) is calling this metric the “ *Probe Sequence Length* ” (PSL).

> 对于给定条目，到初始存储桶 （DIB） 的距离是存储条目的存储桶与其初始存储桶（即其键最初被哈希到的存储桶）之间的距离。此指标是最重要的指标，因为它显示了执行条目检索所需的工作量，该检索由哈希表的所有操作使用：exists（）、get（）、put（） 和 remove（）。保持该指标尽可能低对于实现高性能至关重要。请注意，最初的罗宾汉论文‎[‎[8]‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎将此指标称为“‎*‎探测序列长度‎*‎”（PSL）。

### 2.2 DFB: Distance to Free Bucket

The distance to the free bucket (DFB) for a given entry is the distance that the put() method has to scan from the initial bucket until it can find a free bucket (i.e., an empty bucket).

> 对于给定条目，到可用存储桶 （DFB） 的距离是 put（） 方法必须从初始存储桶扫描的距离，直到它能够找到一个空存储桶（即空存储桶）。‎

### 2.3 DMB: Distance to Missing Bucket

The distance to the missing bucket (DMB) is the distance that the get() method has to scan from the initial bucket until it can conclude that an entry is not in the hash table.

> ‎到缺失存储桶 （DMB） 的距离是 get（） 方法必须从初始存储桶扫描的距离，直到它能够得出结论认为某个条目不在哈希表中。‎

### 2.4 DSB: Distance to Shift Bucket

The distance to the shift bucket (DSB) measures the size of the shift during a deletion for Robin Hood hashing with backward shift deletion (RH-backshift). During a deletion, the index of the entry to delete is retrieved, and from there the position of the bucket where the shift can stop is determined (shift bucket). The DSB is therefore calculated as the distance between the index of the entry to delete and the index of the shift bucket. In this aspect, the DSB differs from the DIB, DMB and DFB, which are computed from the initial bucket.

> ‎到移位桶 （DSB） 的距离测量罗宾汉哈希与向后移位删除 （RH-backshift） 的删除过程中的移位大小。在删除过程中，将检索要删除的条目的索引，并从中确定可以停止移位的存储桶的位置（移位存储桶）。因此，DSB 计算为要删除的条目的索引与 shift 存储桶的索引之间的距离。在这方面，DSB与DIB，DMB和DFB不同，后者是从初始桶计算得出的。

![kvstore-part6-metrics](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/05/kvstore-part6-metrics.png?resize=530%2C350)

Figure 1: Visual representation of the DIB, DFB and DMB

### 2.5 Number of bucket swaps

Hopscotch hashing and Robin Hood hashing are both using a reordering scheme, which swaps buckets during insertions. Each swap incurs a write operation in the bucket array, and thus it is important to look at the number of swaps performed to see if they impact performance.

> ‎Hopscotch 哈希和 Robin Hood 哈希都使用重新排序方案，该方案在插入期间交换存储桶。每次交换都会在存储桶数组中产生一个写入操作，因此查看执行的交换数以了解它们是否会影响性能非常重要

### 2.6 Aligned DIB, DFB, DMB, and DSB

Lower-level components have to deal with memory alignment. In order to get a better idea as to how much memory will actually be accessed for each operation, I have computed the aligned versions of all the distance metrics discussed above.

For example, if an entry is stored in the bucket just after its initial bucket — with indexes (i) and (i+1) — then the DIB for that entry is (i+1)-(i) = 1. Assuming 16-byte buckets, the difference between the offsets of these two indexes is 16 bytes. But to perform any operation on both buckets, both need to be read, and therefore 32 bytes of memory in total need to be accessed. If these two buckets happen to be at the edge of two 64-byte cache lines, with the initial bucket at the end of a cache line and the storage bucket at the beginning of the next one, then 128 bytes of memory in total need to be accessed.

Because I wanted to know the amount of memory accessed, I am not just aligning the distance metric data points, I am really calculating how much aligned memory is accessed. In that sense, it is a bit of a lie to call those metrics “aligned distances”, because they do not represent distances, but the amount of aligned memory required to access the data.

> ‎较低级别的组件必须处理内存对齐。为了更好地了解每个操作实际访问的内存量，我计算了上面讨论的所有距离指标的对齐版本。‎
>
> ‎例如，如果一个条目存储在其初始存储桶之后的存储桶中（索引为 （i） 和 （i+1），则该条目的 DIB 为 （i+1）-（i） = 1。假设有 16 字节的存储桶，这两个索引的偏移量之差为 16 个字节。但是，要对两个存储桶执行任何操作，都需要读取这两个存储桶，因此总共需要访问 32 个字节的内存。如果这两个存储桶恰好位于两个 64 字节缓存行的边缘，其中初始存储桶位于缓存行的末尾，而存储存储桶位于下一个缓存行的开头，则总共需要访问 128 字节的内存。
>
> ‎因为我想知道访问的内存量，所以我不仅仅是对齐距离度量数据点，而是在计算访问了多少对齐的内存。从这个意义上说，将这些指标称为“对齐距离”有点谎言，因为它们不表示距离，而是表示访问数据所需的对齐内存量。

### 2.7 Other metrics of interest

There are two other metrics that would be interesting to look at, but that I have decided to not cover in this article.

The first metric of interest is the number of exact collisions, i.e. the number of keys for which the hash functions outputs the exact same bits. For on-disk hash tables, it makes sense to store the hashes of the keys in the buckets. That way during a lookup, a quick comparison on the hashes can be done to only retrieve the full key data when the hashes match. If there are many exact collisions, then many unsuccessful retrieval of key data will be performed, wasting a lot of I/O. This metric depends only on the hash function, not on the hash table algorithm, which is why it is not included in this article.

The second metric of interest is the number of cache misses. Open-addressing hash tables are expected to perform well due to their cache-friendliness, although it is good to keep an eye on the actual number of cache misses in the CPU to verify that the algorithms deliver what they promise. I have decided not to include this metric here because it is mostly related to CPU architecture, thus it is something that will be best monitored directly on the end system where the hash tables will be running.

> 还有另外两个指标很有趣，但我决定不在本文中介绍。
>
> ‎第一个感兴趣的指标是精确冲突的数量，即哈希函数输出完全相同位的键的数量。对于磁盘上的哈希表，将密钥的哈希存储在存储桶中是有意义的。这样，在查找期间，可以对哈希进行快速比较，以仅在哈希匹配时才检索完整的密钥数据。如果存在许多精确的冲突，则将执行许多不成功的密钥数据检索，从而浪费大量的I / O。此指标仅取决于哈希函数，而不依赖于哈希表算法，这就是本文未包含它的原因。‎
>
> ‎第二个感兴趣的指标是缓存未命中数。由于开放寻址哈希表的缓存友好性，预计其性能良好，尽管最好密切关注CPU中缓存未命中的实际数量，以验证算法是否实现了它们的承诺。我决定不在此处包含此指标，因为它主要与CPU架构相关，因此最好直接在运行哈希表的最终系统上进行监视。‎

## 3. Experimental protocol

### 3.1 Test cases

I have used the same test cases that I used in my previous articles about Robin Hood hashing, the “loading”, “batch”, and “ripple” test cases. Below are full descriptions of what those test cases are doing:

**The “loading” test case:**
– Insert entries in the hash table until it full, up to a load factor of 0.98

– Measure statistics at every 0.02 increment of the load factor

**The “batch” test case:**
Uses two parameters, Load Factor Max (LFM) and Load Factor Remove (LFR)
– Insert entries in the table up to LFM (with a table of 10k entries and LFM=0.8, 8k entries would be inserted)
– Do the following operations over 50 iterations (for 1 <= i <= 50):

* Remove at once LFR entries of the total capacity (with a table of 10k entries and LFR=0.1, 1k entries would be removed at once)
* Insert at once LFR entries of the total capacity (with a table of 10k entries and LFR=0.1, 1k entries would be inserted at once)
* Measure statistics at the i-th iteration

**The “ripple” test case:**
Uses two parameters, Load Factor Max (LFM) and Load Factor Remove (LFR)
– Insert entries in the table up to LFM (with a table of 10k entries and LFM=0.8, 8k entries would be inserted)
– Do the following operations over 50 iterations (for 1 <= i <= 50):

* Remove a single entry, and immediately insert a single entry. Do this for LFR of the total capacity (with a table of 10k entries and LFR=0.1, a single entry would be removed, then a single entry would be inserted, and this, 1000 times)
* Measure statistics at the i-th iteration

> 我使用了与之前关于罗宾汉哈希，“加载”，“批处理”和“波纹”测试用例相同的测试用例。以下是这些测试用例正在执行的操作的完整描述：
>
> **“加载”测试用例：‎**
>
> - 在哈希表中插入条目，直到它满为止，负载因子最高为 0.98‎
> - 以负载系数每增加0.02次来测量统计数据‎
>
> **“批处理”测试用例：‎‎使用两个参数，最大负载因子 （LFM） 和负载因子删除 （LFR）**
>
> - 在表中插入条目，直到 LFM（表包含 10000 个条目，LFM=0.8，将插入 8k 个条目）
> - 在 50 次迭代中执行以下操作（对于 1 < = i <= 50）
>   - 一次删除总容量的 LFR 条目（如果表包含 10000 个条目且 LFR=0.1，则将一次删除 1000 个条目）‎
>   - ‎一次插入总容量的LFR条目（如果表包含10000个条目，LFR=0.1，则一次插入1000个条目）‎
>   - 在第 i 次迭代时测量统计信息
>
> **‎“纹波”测试用例：‎‎使用两个参数，最大负载因子 （LFM） 和负载系数移除 （LFR）**
>
> - 在表中插入条目，直到 LFM（表包含 10k 个条目，LFM=0.8，将插入 8k 个条目）‎
> - 在 50 次迭代中执行以下操作（对于 1 < = i <= 50）：‎
>   - ‎删除单个条目，然后立即插入单个条目。对总容量的 LFR 执行此操作（如果表包含 10k 个条目且 LFR=0.1，则将删除单个条目，然后插入单个条目，这是 1000 次）
>   - 在第 i 次迭代时测量统计信息

### 3.2 Algorithm comparison

For each algorithm that I am studying, I have run simulations of the test cases described above with the following parameters:

* “batch” test case, with LFM=0.8 and LFR=0.1
* “batch” test case, with LFM=0.8 and LFR=0.8
* “ripple” test case, with LFM=0.8 and LFR=0.1
* “loading” test case

The keys for the entries are generated using the random function from the C++ standard library. For each test case, 50 **instances** have been run. This means that for the “batch” and “ripple” test cases, I have run 50 times 50 iterations, then I have averaged the 50 values corresponding to each iteration. For the “loading” test case, I have also run 50 instances, but there is only one iteration, which is why the x-axis in the graphs is not “ *Iterations* ” but “ *Load factor* “. For each instance, the same seed value was used across all algorithms, in such a way that the k-th instance for a test case over any algorithm was run using the same random keys as for the other algorithms, making it a fair comparison. Finally for each metric, I have calculated the following statistical aggregates: mean, median, 95th percentile, maximum, and variance.

It is important to notice that the metrics are gathered at different moments for each test case. For example for the “loading” test case, each metric is computed after an increment of 2% and therefore over the 2% of the population that was the most recently inserted into the table. For the batch with LFM=0.8 and LFR=0.8, the metrics are computed after the table has been cleared and filled up to a load factor of 0.8, thus they represent the entire population of entries in that hash table.

> 对于我正在研究的每个算法，我都使用以下参数对上述测试用例进行了模拟：‎
>
> * ‎“批处理”测试用例，LFM=0.8，LFR=0.1‎
> * ‎“批处理”测试用例，LFM=0.8，LFR=0.8‎
> * ‎“纹波”测试用例，LFM=0.8，LFR=0.1‎
> * ‎“加载”测试用例‎
>
> 条目的键是使用C++标准库中的随机函数生成的。对于每个测试用例，已运行 50 ‎**‎个实例‎**‎。这意味着对于“批处理”和“波纹”测试用例，我已经运行了50次50次迭代，然后我平均了每次迭代对应的50个值。对于“加载”测试用例，我也运行了50个实例，但只有一次迭代，这就是为什么图中的x轴不是“‎*‎迭代‎*‎”而是“‎*‎负载因子‎*‎”。对于每个实例，在所有算法中都使用相同的种子值，使得任何算法上的测试用例的第 k 个实例都使用与其他算法相同的随机键运行，从而使其成为公平的比较。最后，对于每个指标，我计算了以下统计聚合：平均值、中位数、第 95 百分位、最大值和方差。‎
>
> ‎请务必注意，指标是在每个测试用例的不同时刻收集的。例如，对于“加载”测试用例，每个指标都是在增量为 2% 之后计算的，因此超过最近插入到表中的 2% 的总体。对于 LFM=0.8 和 LFR=0.8 的批处理，将在清除表并填充到负载因子 0.8 后计算指标，因此它们表示该哈希表中的整个条目填充。‎

### 3.3 Robin Hood hashing table size comparison

To determine if the table size has an effect on the performance of Robin Hood hashing, I have run the “loading” test case for different sizes of the hash table: 10k, 100k, 1M and 10M. Each table size is run over 10 iterations.

> 为了确定表大小是否对罗宾汉哈希的性能有影响，我对不同大小的哈希表运行了“加载”测试用例：10k、100k、1M 和 10M。每个表大小在 10 次迭代中运行。

## 4. Results and Discussion

### 4.1 Raw data

The source code for all the hash tables presented here is available in a public repository [[10]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref).

The raw data from the simulations that I have run can be downloaded from the data repository [[11]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref).

From this data, I have plotted a significant amount of graphs, taking into account the various test cases and metrics discussed above, along with the multiple statistical aggregates. All the graphs are available from the data repository [[11]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref). For the sake of length, I am not presenting all the graphs in this article, only the ones that I think are the most meaningful.

> 此处提供的所有哈希表的源代码可在公共存储库‎[‎中找到 [10]‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎。
>
> 我运行的模拟中的原始数据可以从数据存储库‎[‎[11]下载‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎。
>
> ‎根据这些数据，我绘制了大量的图形，考虑了上面讨论的各种测试用例和指标，以及多个统计聚合。所有图形均可从数据存储库 ‎[‎[11] 获得‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎。为了长度起见，我在本文中不介绍所有图表，而只介绍我认为最有意义的图表。

### 4.2 Robin Hood hashing has the best DIB and DMB, and an acceptable DFB

In a hash table, all the operations — get(), put(), exists() and remove() — need to determine the position of the entry on which they are operating, which efficiency is measured by the DIB. For the 95th percentile of aligned DIB, as shown in Figures 2(a) and 2(d) below, Robin Hood hashing with backward shift deletion (RH-backshift) tops at 256 bytes, whereas Linear Probing and both Hopscotch hashing algorithms go up to 512 bytes and 1 KB respectively. Having a low DIB gives RH-backshift a clear advantage over other algorithms.

> ‎在哈希表中，所有操作 （ get（）、 put（）、 exists（） 和 remove（） — 都需要确定它们正在运行的条目的位置，该效率由 DIB 度量。对于对齐的DIB的第95百分位，如下图2（a）和2（d）所示，罗宾汉散列与向后移位删除（RH-backshift）的最高值为256字节，而线性探测和Hopscotch散列算法分别高达512字节和1 KB。与其他算法相比，较低的DIB使RH反移具有明显的优势。‎

![aligned dib_perc95](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/05/aligned-dib_perc95.png?resize=704%2C651)

Figure 2: 95th percentile of the aligned distance to initial bucket (DIB) for Linear Probing, Hopscotch Hashing, and Robin Hood hashing, over four test cases

For the distance to missing bucket (DMB), RH-backshift clearly outperforms all the other algorithms and minimizes the cost of looking up keys that are absent from the hash table, and this even if the hash table is filled up to a high load factor, here 0.8. In Figure 3 below, the mean of the aligned DMB shows that the cost of looking up absent keys will require to scan at most 32 bytes for RH-backshift. Compared to the bitmap Hopscotch or the Linear Probing which require 512 bytes and 16 KB respectively, this is impressive.

> ‎对于到缺失桶（DMB）的距离，RH-backshift明显优于所有其他算法，并最大限度地降低了查找哈希表中不存在的键的成本，即使哈希表被填充到高负载因子，这里为0.8。在下面的图 3 中，对齐 DMB 的平均值显示，查找缺失密钥的成本最多需要扫描 32 个字节的 RH-backshift。与位图Hopscotch或线性探测分别需要512字节和16 KB相比，这令人印象深刻

![aligned dmb_mean](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/05/aligned-dmb_mean.png?resize=704%2C651)

Figure 3: Mean of the aligned distance to missing bucket (DMB) for Linear Probing, Hopscotch Hashing, and Robin Hood hashing, over four test cases

The distance to free bucket (DFB) is the metric for which RH-backshift isn’t so shinny. The mean and median DFB of RH-backshift are higher than the mean and median for Hopscotch and Linear Probing. However, looking at the aligned DFB, RH-backshift shows acceptable performance with the mean of the aligned DFB at around 64 bytes as seen in Figure 4, thus within the critical limitation of a 64-byte cache line size.

> ‎DFB距离是 RH-backshift 不太亮眼的指标。RH背移的均值和中位数DFB高于Hopscotch和线性探测的均值和中位数。然而，从对齐的DFB来看，RH-backshift显示出可接受的性能，对齐的DFB的平均值约为64字节，如图4所示，因此在64字节高速缓存行大小的关键限制范围内。‎

![aligned dfb_mean](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/05/aligned-dfb_mean.png?resize=704%2C651)

Figure 4: Mean of the aligned distance to free bucket (DFB) for Linear Probing, Hopscotch Hashing, and Robin Hood hashing, over four test cases

### 4.3 In Robin Hood hashing, the maximum DIB increases with the table size

Kristofer Karlsson, a software engineer at Spotify, has a lot of experience with Robin Hood hashing. He is the author of Sparkey, an on-disk key-value store that he wrote for Spotify and which makes use of Robin Hood hashing. The code of Sparkey has been made open source and is available in its public repository [[12]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref).

In an email discussion, Kristofer pointed out that the maximum DIB was growing with the size of the table. At first I found that result a bit strange, and since I only had been playing with tables of relatively small sizes — 10k to 100k items — I decided to run some tests with my code on larger tables, up to 50M entries. The results I got were showing a growth of the maximum DIB as a function of the table size, which confirmed what Kristofer had observed. Reading over Celis’s thesis again [[8]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref), I realized that Celis did mention the growth pattern of the maximum DIB, on page 46:

> For a fixed load factor, the longest probe sequence length grows slowly when the table size is increased […] the growth is at most logarithmic in the table size.

With this new data and the indication from Celis’s thesis, Kristofer and I were able to confirm that the maximum DIB was growing logarithmically with the table size, and that this was indeed the expected behavior.

This growth in the maximum DIB can be observed in Figure 5(d), which shows the “loading” test case for different table sizes of Robin Hood hashing. The same growth behavior can also be observed for the maximum DFB, DMB, and DSB. Nevertheless, it is interesting to notice that the mean, median and 95th percentile of the DIB are the same for any table size, as shown by Figure 5(a), 5(b) and 5(c). Even though the maximum is growing with the size of the table, the average performance is independent from it, and remains good for any table size.

> ‎Kristofer Karlsson是Spotify的软件工程师，在Robin Hood哈希方面拥有丰富的经验。他是Sparkey的作者，Sparkey是他为Spotify编写的磁盘键值存储，它利用了Robin Hood哈希。Sparkey的代码已经开源，可以在其公共存储库‎[‎[12]中找到‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎。‎
>
> ‎在一封电子邮件讨论中，Kristofer指出，最大DIB随着table的大小而增长。起初，我发现这个结果有点奇怪，而且由于我只玩过相对较小的表-10k到100k个项目-我决定在更大的表上运行一些测试，最多5000万个条目。我得到的结果显示最大DIB的增长是表大小的函数，这证实了Kristofer观察到的内容。再次阅读Celis的论文‎[‎[8]‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎，我意识到Celis确实在第46页上提到了最大DIB的增长模式：‎
>
> **‎对于固定负载因子，当表大小增加时，最长的探测序列长度增长缓慢[...] 表大小的增长最多是对数。‎**
>
> ‎有了这些新数据和Celis论文的指示，Kristofer和我能够确认最大DIB随着表的大小而对数增长，这确实是预期的行为。
>
> ‎最大DIB的增长可以在图5（d）中观察到，它显示了不同表大小的罗宾汉哈希的“加载”测试用例。对于最大DFB，DMB和DSB，也可以观察到相同的生长行为。尽管如此，值得注意的是，对于任何表大小，DIB的平均值，中位数和第95百分位数都是相同的，如图5（a），5（b）和5（c）所示。即使最大值随着表的大小而增长，平均性能也独立于表，并且对于任何表大小都保持良好状态。‎

![dib](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/05/dib.png?resize=704%2C795)

Figure 5: All statistics for the distance to the initial bucket (DIB) of Robin Hood hashing with backward shift deletion, with different table sizes over the “loading” test case

### 4.4 Robin Hood hashing has an acceptable rate of swapped buckets during insertions

My biggest concern with RH-backshift is the number of buckets swapped during insertion, which can rise up to 80 at peak, as it can be seen in Figure 6(a). Buckets are going to be swapped starting from the initial bucket until a free bucket is found. As mentioned above, the aligned DFB for RH-backshift is at around 64 bytes, the size of a cache line. During an insert, the cache line would need to be updated anyway, therefore as the aligned DFB is within 64 bytes, the swaps will be written to the cache line along with the update made for the insert, not causing any additional work.

> ‎我对RH-backshift的最大担忧是在插入过程中交换的桶数，在峰值时可能会上升到80个，如图6（a）所示。存储桶将从初始存储桶开始交换，直到找到空闲存储桶。如上所述，RH-backshift 的对齐 DFB 大约为 64 字节，相当于缓存行的大小。在插入期间，无论如何都需要更新缓存行，因此由于对齐的DFB在64字节以内，因此交换将与为插入进行的更新一起写入缓存行，而不会引起任何额外的工作。‎

![swap_maximum](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/05/swap_maximum.png?resize=704%2C651)

Figure 6: 95th percentile of the swaps for Linear Probing, Hopscotch Hashing, and Robin Hood hashing, over four test cases

### 4.5 Hopscotch hashing uses bucket swaps only at high load factors

Following my intuition, I would have expected bucket swaps to occur frequently in a Hopscotch hash table, but looking at the swap metric, it is not the case. Indeed, if for Robin Hood hashing the swaps occur at any load factor, for Hopscotch hashing they only start occurring in large numbers starting at load factors of 0.7. I suspected that this could be something due my own implementation, therefore to clear up my doubts I decided to use the code made by the authors of the Hopscotch hashing paper [[5]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref), and I dumped the number of swaps occurring in their bitmap neighbourhood implementation with the following parameters:

> 根据我的直觉，我本来预计桶交换会在Hopscotch哈希表中频繁发生，但查看交换指标，情况并非如此。事实上，如果对于罗宾汉散列，交换发生在任何负载因子，对于Hopscotch散列，它们只会从0.7的负载因子开始大量发生。我怀疑这可能是由于我自己的实现，因此为了消除我的疑虑，我决定使用Hopscotch哈希论文‎[‎[5]‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎的作者制作的代码，并且我使用以下参数转储了其位图邻域实现中发生的交换次数：

```
./test_intel32 bhop 1 2 100 90 102400 1 1
```

This gave me the following output:

```
ConcurrentHashMap Benchmark
---------------------------
numOfThreads:   2
Algorithm Name: bhop
NumProcessors:  1
testNo:         1
insert ops:     50
del ops:        50
constain ops:   0
loadFactor:     89
initialCount:   131072
throughputTime: 1
initialCapacity:   131072

START create random numbers.
END   creating random numbers.

START fill table. (117964)
BHOP Num elm: 117964
END   fill table.

START creating threads.
END   creating threads.

START threads.
END   threads.

STARTING test.
ENDING test.
```

I compared this to my own implementation [[10]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref) using the following parameters:

```
./hashmap --algo bitmap --num_buckets 140000 --testcase batch --load_factor_max 0.89 --load_factor_step 0.89
```

Looking at the distributions of the swap metric for both implementations, it appeared that they are very similar, which gives me confidence that my implementation is valid, at least as valid as the one of the original authors of the Hopscotch algorithm. The swap distributions are available in the data repository [[12]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref).

Hopscotch hashing will perform swaps only when it is unable to fill up the neighborhood of a bucket, around the time when the “neighborhood clustering” [[3]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref) effect starts to appear and renders the hash table unusable unless it is resized.

> ‎查看两个实现的交换指标的分布，它们似乎非常相似，这使我相信我的实现是有效的，至少与Hopscotch算法的原始作者之一一样有效。交换分布在数据存储库 ‎[‎[12]‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎ 中可用。‎
>
> ‎Hopscotch 哈希只有在无法填满存储桶的邻域时才会执行交换，大约在“邻域聚类”‎[‎[3]‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎ 效应开始出现并导致哈希表不可用时，除非调整其大小。

### 4.6 Robin Hood hashing has an acceptable DSB

Another concern for RH-backshift is the number of writes necessary to shift buckets backward after a bucket is removed, which is measured by the distance to the shift bucket (DSB). Note that if the DIB, DMB and DFB are computed starting from the initial bucket, the DSB is computed from the bucket where the removed entry used to be stored, because this is the location up to which data will be written.

Even if the 95th percentile and the maximum of the DSB can reach high values, looking at the mean of the aligned DSB, it appears that they stay at around 64 bytes for the test cases having the heaviest workloads, as seen in Figure 7(a) and 7(d). Therefore only one cache line needs to be updated on average during the deletion process of RH-backshift. Hopscotch hashing and Linear Probing need to mark the bucket as deleted in some way, requiring one cache line update as well. From this observation it can be concluded that even if RH-backshift has an apparent disadvantage during deletion compared to other algorithms, in the end it has a similar I/O usage than the other algorithms.

> ‎RH-backshift的另一个问题是在移除存储桶后向后移动存储桶所需的写入次数，这是通过到移位存储桶（DSB）的距离来衡量的。请注意，如果从初始存储桶开始计算 DIB、DMB 和 DFB，则 DSB 是从曾经存储已删除条目的存储桶计算的，因为这是写入数据的最高位置。
>
> 即使 DSB 的第 95 百分位和最大值可以达到高值，从对齐的 DSB 的平均值来看，对于工作负载最重的测试用例，它们似乎保持在 64 字节左右，如图 7（a） 和 7（d） 所示。因此，在 RH 反移的删除过程中，平均只需要更新一个缓存行。Hopscotch 哈希和线性探测需要以某种方式将存储桶标记为已删除，还需要一次缓存行更新。从这个观察可以得出结论，即使RH-backshift在删除过程中与其他算法相比具有明显的缺点，但最终它具有与其他算法相似的I / O用法。‎

![aligned dsb_mean](https://i0.wp.com/codecapsule.com/wp-content/uploads/2014/05/aligned-dsb_mean.png?resize=704%2C575)

Figure 7: 95th percentile of the aligned distance to shift bucket (DSB) for Linear Probing, Hopscotch Hashing, and Robin Hood hashing, over four test cases

## 5. Conclusion

I choose not to use Hopscotch hashing, because it has lower performance compared to Robin Hood hashing, and more importantly because the neighborhood clustering effect makes Hopscotch hashing unreliable in practice. Indeed, there is always a chance for the distribution of the hashed keys to create a cluster in one of the neighbourhoods at any load factor, making bucket swaps impossible unless the table is resized [[3]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref). Using other neighborhood representations such as linked-list or shadow will minimize the chances of neighbourhood clusters, but won’t completely get rid of the problem. Moreover, the linked-list and shadow representations both force the data to be stored in non-contiguous memory locations, losing cache locality which is one of the major advantages of open-addressing hash tables.

Based on the analysis of the data presented above, I have chosen to use Robin Hood hashing to be the algorithm for my key-value store project. I have also recently done some research on solid-state drives [[9]](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref), and with that knowledge, I am now ready to finally implement that key-value store, which I will be doing over the next few months. Once done, I will write another article to describe who the hash table is stored in a way that is optimized for solid-state drives.

> ‎我选择不使用Hopscotch哈希，因为它的性能低于Robin Hood哈希，更重要的是因为邻域聚类效应使得Hopscotch哈希在实践中不可靠。事实上，散列键的分布总是有机会在任何负载因子的一个邻域中创建集群，使得存储桶交换不可能，除非调整表的大小‎[‎[3]‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎。使用其他邻域表示（如链表或影子）将最大限度地减少邻域聚类的可能性，但不会完全解决问题。此外，链表和影子表示都强制将数据存储在不连续的内存位置，从而丢失缓存位置，这是开放寻址哈希表的主要优点之一。‎
>
> ‎基于对上面提供的数据的分析，我选择使用罗宾汉哈希作为我的键值存储项目的算法。我最近还对固态硬盘‎[‎[9]‎](https://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/#ref)‎进行了一些研究，有了这些知识，我现在准备最终实现键值存储，我将在未来几个月内完成这项工作。完成后，我将写另一篇文章来描述哈希表以针对固态驱动器优化的方式存储谁。

## 6. References

[1] [Cuckoo Hashing, Emmanuel Goossaert](http://codecapsule.com/2013/07/20/cuckoo-hashing/)
[2] [Cuckoo hashing, Pagh and Rodler, 2001](http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.25.4189)
[3] [Hopscotch Hashing, Emmanuel Goossaert](http://codecapsule.com/2013/08/11/hopscotch-hashing/)
[4] [Hopscotch Hashing, Herlihy et at., 2008](http://mcg.cs.tau.ac.il/papers/disc2008-hopscotch.pdf)
[5] [Source code from the authors of the original hopscotch hashing paper](https://sites.google.com/site/cconcurrencypackage/hopscotch-hashing/download-1)
[6] [Robin Hood Hashing, Emmanuel Goossaert](http://codecapsule.com/2013/11/11/robin-hood-hashing/)
[7] [Robin Hood Hashing: backward shift deletion, Emmanuel Goossaert](http://codecapsule.com/2013/11/17/robin-hood-hashing-backward-shift-deletion/)
[8] [Robin Hood Hashing, Pedro Celis, 1986](https://cs.uwaterloo.ca/research/tr/1986/CS-86-14.pdf)
[9] [Coding for SSDs](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)
[10] [https://github.com/goossaert/hashmap](https://github.com/goossaert/hashmap)
[11] [http://codecapsule.com/hashmap-data/](http://codecapsule.com/hashmap-data/)
[12] [https://github.com/spotify/sparkey](https://github.com/spotify/sparkey)
