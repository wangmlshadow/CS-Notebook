# Implementing a Key-Value Store – Part 5: Hash table implementations

This is Part 5 of the IKVS series, “Implementing a Key-Value Store”. You can also check the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) for other parts.

In this article, I will study the actual implementations of hash tables in C++ to understand where are the bottlenecks. Hash functions are CPU-intensive and should be optimized for that. However, most of the inner mechanisms of hash tables are just about efficient memory and I/O access, which will be the main focus of this article. I will study three different hash table implementations in C++, both in-memory and on-disk, and take a look at how the data are organized and accessed. This article will cover:

1. Hash tables
   1.1 Quick introduction to hash tables
   1.2 Hash functions
2. Implementations
   2.1 unordered_map from TR1
   2.2 dense_hash_map from SparseHash
   2.3 HashDB from Kyoto Cabinet
3. Conclusion
4. References

> ‎这是 IKVS 系列的第 5 部分“实现键值存储”。您还可以在‎[‎目录中‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎查看其他部分。‎
>
> ‎在本文中，我将研究哈希表在C++的实际实现，以了解瓶颈在哪里。哈希函数是CPU密集型的，应该为此进行优化。但是，哈希表的大多数内部机制只是关于高效内存和 I/O 访问，这将是本文的主要重点。我将研究C++中的三种不同的哈希表实现，包括内存中和磁盘上，并查看数据的组织和访问方式。本文将介绍：‎
>
> 1. 哈希表‎
>    1. 哈希表‎‎快速介绍
>    2. 哈希函数‎
> 2. ‎实现
>    1. TR1‎‎ unordered_map
>    2. SparseHash的dense_hash_map
>    3. Kyoto Cabinet的HashDB。
> 3. 结论‎
> 4. 引用‎

![](https://i0.wp.com/codecapsule.com/wp-content/uploads/2013/05/kvstore-part5-intro.jpg?resize=770%2C315)

## 1. Hash tables

### 1.1 Quick introduction to hash tables

> Hashtables are arguably the single most important data structure known to mankind.
>
> ‎哈希表可以说是人类已知的最重要的数据结构。
> — Steve Yegge

A hash table allows to efficiently access  *associative data* . Each *entry* is a pair of a *key* and a  *value* , and can be quickly retrieved or assigned just by knowing its key. For that, the key is hashed using a  *hash function* , to transform that key from its original representation into an integer. This integer is then used as an index to identify the *bucket* in the *bucket array* from which the entry’s value can be accessed. Many keys can hash to the same values, meaning that these keys will be in *collision* in the bucket array. To resolve collisions, various techniques can be used, such as *separate chaining* with linked-lists or self-balanced trees, or *open addressing* with linear or quadratic probing.

From now on, I will assume that you know what hash tables are. If you think you need to brush up your knowledge a bit, good references are either the “Hash table” article on Wikipedia [[1]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_1) (and the external links section at the bottom of the page), or the Hash table chapter in the book “ *Introduction to Algorithms* ” by Cormen et. al [[2]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_2).

> ‎哈希表允许有效地访问‎*‎关联数据‎*‎。每个‎*‎条目‎*‎都是一个*键‎*‎和*值‎对*‎，只需知道其键即可快速检索或分配。为此，使用哈希函数对键进行‎*‎哈希处理‎*‎，以将该键从其原始表示形式转换为整数。然后，此整数用作索引，以标识‎*‎存储桶数组‎*‎中可从中访问条目值的存储桶。‎‎许多键可以散列为相同的值，这意味着这些键将在存储桶数组中‎*‎发生冲突‎*‎。为了解决冲突，可以使用各种技术，例如使用链表或自平衡树‎*‎进行单独链接‎*‎，或者使用线性或二次探测进行‎*‎开放寻址‎*‎。‎
>
> 从现在开始，我将假设您知道哈希表是什么。如果你认为你需要稍微了解一下什么是Hash Table，可以查看参考文献或者维基百科上的“哈希表”相关文章‎[‎[1]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_1)‎（以及页面底部的外部链接部分），要么是Cormen等人的“‎*‎算法导论‎*‎”一书中的哈希表章节。‎[‎al [2]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_2)‎.‎

### 1.2 Hash functions

The choice of the hash function is extremely important. The basic requirement for a good hash function is that the output hashed values should be distributed uniformly. That way, the chances of collisions are minimized, along with the average number of colliding entries in a bucket.

There are many possible hash functions, and unless you know exactly what the data are going to be, the safest option is to go for a hash function that distributes random data uniformly on average, and if possible that fits the *avalanche effect* [[3]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_3). A few people have already worked on hash function comparison [[4]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_4) [[5]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_5) [[6]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_6) [[7]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_7), and from their conclusions, it is clear that MurmurHash3 [[8]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_8) and CityHash [[9]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_9) are the best hash functions to use for hash tables at the time this article is being written.

> 哈希函数的选择非常重要。一个好的哈希函数的基本要求是输出哈希值应均匀分布。这样，冲突的可能性就会最小化，并且存储桶中碰撞条目的平均数量也会降至最低。‎
>
> ‎有许多可用的哈希函数，除非您确切地知道数据将是什么，否则最安全的选择是使用平均均匀分布随机数据的哈希函数，如果可能的话，这符合‎*‎雪崩效应‎*[‎[3]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_3)‎。一些人已经研究了哈希函数比较‎[‎[4]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_4)‎ ‎[‎[5]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_5)‎ ‎[‎[6]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_6)‎ ‎[‎[7]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_7)‎，从他们的结论来看，很明显MurmurHash3 ‎[‎[8]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_8)‎和CityHash ‎[‎[9]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_9)‎是撰写本文时用于哈希表的最佳哈希函数。‎

## 2. Implementations

Like for the comparisons of hash functions, there are a few blog articles that already compare the performance of in-memory C++ hash table libraries. The most notables I have encountered are “ *Hash Table Benchmarks* ” by Nick Welch [[10]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_10) and “ *Hash Table Performance Tests* ” by Jeff Preshing [[11]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_11), but other articles also deserve a glance [[12]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_12) [[13]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_13) [[14]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_14). From these comparisons, I have derived that unordered_map from TR1 in GCC along with dense_hash_map from the SparseHash library — formerly called Google SparseHash — are two interesting pieces to study, and I will cover them below. In addition, I will also describe the data structures inside HashDB from Kyoto Cabinet. Obviously, unordered_map and dense_hash_map won’t be as relevant as HashDB for my key-value store project, since they are in-memory hash tables. Nevertheless, having a glance at how their inner data structures are organized and what are the memory patterns can only be interesting.

For the descriptions of the three hash table libraries below, I will take as a common example a set of city names as keys, and their GPS coordinates as values. The source code for unordered_map can be found in GCC’s code, as part of libstdc++-v3. I’ll be looking at libstdc++-v3 release 6.0.18 from GCC v4.8.0 [[15]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_15), dense_hash_map from SparseHash v2.0.2 [[16]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_16), and HashDB from Kyoto Cabinet v1.2.76 [[17]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_17).

Interesting implementation discussions can also be found in “ *A Proposal to Add Hash Tables to the Standard Library (revision 4)* ” by Matthew Austern [[18]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_18) and in the “ *Implementation notes* ” page of SparseHash [[19]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_19).

> ‎与哈希函数的比较一样，有一些博客文章已经比较了内存中C++哈希表库的性能。我遇到的最著名的是Nick Welch的“‎*‎Hash Table Benchmarks‎*‎”‎[‎[10]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_10)‎和Jeff Preshing的“‎*‎Hash Table Performance Tests‎*‎”‎[‎[11]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_11)‎，但其他文章也值得一看‎[‎[12]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_12)[‎[13]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_13)[‎[14]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_14)‎。从这些比较中，我得出了来自GCC TR1 unordered_map以及来自SparseHash库（以前称为Google SparseHash）dense_hash_map是两个有趣的研究部分，我将在下面介绍它们。此外，我还将描述Kyoto Cabinet的HashDB内部的数据结构。显然，unordered_map和dense_hash_map不会像HashDB那样与我的键值存储项目相关，因为它们是内存中的哈希表。然而，了解它们的内部数据结构是如何组织的，以及记忆模式是什么，这只会很有趣。‎
>
> 对于下面三个哈希表库的描述，我将以一组城市名称作为键，并将其GPS坐标作为值作为常见示例。unordered_map的源代码可以在GCC的代码中找到，作为libstdc++-v3的一部分。我将查看来自GCC v4.8.0 ‎[‎[15]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_15)‎的libstdc++-v3版本6.0.18，来自SparseHash v2.0.2 ‎[‎[16]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_16)‎的dense_hash_map，以及来Kyoto Cabinet v1.2.76的HashDB ‎[‎[17]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_17)‎。‎
>
> ‎有趣的实现讨论也可以在Matthew Austern ‎[‎[18]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_18)‎的“‎*‎将哈希表添加到标准库的建议（修订版4）‎*‎”和SparseHash ‎[‎[19]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_19)‎的“‎*‎实现说明‎*‎”页面中找到。‎

### 2.1 unordered_map from TR1

TR1’s unordered_map provides a hash table that handles collisions with linked lists (separate chaining). The bucket array is allocated on the heap, and scales up or down automatically based on the load factor of the hash table. A node struct named `_Hash_node` is used to create the linked lists for the buckets:

> ‎TR1 的unordered_map提供了一个哈希表，通过链接列表来处理hash的冲突（单独的链接）。存储桶数组在堆上分配，并根据哈希表的负载因子自动向上或向下扩展。名为 `_Hash_node`的节点结构用于为存储桶创建链接列表：

<pre class="wp-block-preformatted"><b>/* from gcc-4.8.0/libstdc++-v3/include/tr1/hashtable_policy.h */</b> 
template
  struct _Hash_node<_Value, false>
  {
    _Value       _M_v;
    _Hash_node*  _M_next;
  };
</pre>

If the keys and values are of integral types, they can be stored directly inside this struct in `_M_v`. Otherwise pointers will be stored and some extra memory will be necessary. The bucket array is allocated at once on the heap, but it’s not the case of the Nodes, which are allocated with individual calls to the C++ memory allocator:

> 如果键和值为整数类型，则可以将它们直接存储在 `_M_v`结构中。否则，将存储指针，并且需要一些额外的内存。存储桶数组在堆上一次分配，但节点不一样，节点是通过对C++内存分配器的单独调用来分配的：‎

<pre class="wp-block-preformatted"><b>/* from gcc-4.8.0/libstdc++-v3/include/tr1/hashtable.h */</b> 
Node* _M_allocate_node(const value_type& __v)
    {
      _Node* __n = _M_node_allocator.allocate(1);
      __try
	{
	  _M_get_Value_allocator().construct(&__n->_M_v, __v);
	  __n->_M_next = 0;
	  return __n;
	}
      __catch(...)
	{
	  _M_node_allocator.deallocate(__n, 1);
	  __throw_exception_again;
	}
    }
</pre>

Because nodes are allocated individually, a lot of memory may be wasted on every node allocation. This depends of course on the memory allocator of the compiler and operating system being used. And I am not even talking about all the system calls being performed for each allocation. The original implementation of the SGI hash table was doing some resource pre-allocation for the nodes, but this solution has not been kept for the unordered_map implementation of TR1.

Figure 5.1 below offers a representation of the memory and access patterns for unordered_map from TR1. Let’s see what happens if we look for the GPS coordinates associated with the key “Johannesburg”. This key is hashed and mapped to the bucket #0. From there we jump to the first node of the linked list for that bucket (orange arrow on the left of bucket #0), and we can access the memory area in the heap that holds the data for the key “Johannesburg” (black arrow on the right of the node). If the key were to be invalid at this first node, we would have had to navigate throw other nodes.

As for CPU performance, one cannot expect to have all the data in the same cache line in the processor. Indeed, given the size of the bucket array, the initial bucket and the initial node will not be in the same cache line, and the external data associated with a node is also unlikely to be found on the same cache line. Subsequent nodes and associated data will also not be in the same cache line and will have to be retrieved from RAM. If you are not familiar with CPU optimizations and cache lines, the “ *CPU Cache* ” article on Wikipedia is a good introduction [[20]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_20).

> ‎由于节点是单独分配的，因此在每个节点分配上可能会浪费大量内存。当然这取决于正在使用的编译器和操作系统的内存分配器。我甚至不是在谈论为每个分配执行的所有系统调用。SGI哈希表的原始实现是为节点进行一些资源预分配，但是对于TR1的unordered_map实现，此解决方案尚未保留。
>
> 下面的图 5.1 提供了来自 TR1 的unordered_map的内存和访问模式的表示形式。让我们看看如果我们寻找与键“约翰内斯堡”关联的GPS坐标会发生什么。此键经过哈希处理并映射到存储桶 #0。从那里，我们跳转到该存储桶的链接列表的第一个节点（存储桶 #0 左侧的橙色箭头），我们可以访问堆中的内存区域，该区域保存键“Johannesburg”的数据（节点右侧的黑色箭头）。如果密钥在第一个节点上无效，我们将不得不搜索其他节点。
>
> ‎至于CPU性能，不能期望所有数据都位于处理器的同一高速缓存行中。实际上，考虑到存储桶数组的大小，初始存储桶和初始节点将不在同一缓存行中，并且与节点关联的外部数据也不太可能在同一缓存行上找到。后续节点和关联的数据也不会在同一缓存行中，并且必须从 RAM 中检索。如果您不熟悉CPU优化和缓存行，维基百科上的“‎‎CPU缓存‎‎”文章是一个很好的介绍‎‎[20]‎.‎

![kvstore_unordered_map_web](https://i0.wp.com/codecapsule.com/wp-content/uploads/2013/04/kvstore_unordered_map_web.jpg?resize=770%2C948)

Figure 5.1

### 2.2 dense_hash_map from SparseHash

The SparseHash library offers two hash table implementations, *sparse_hash_map* and  *dense_hash_map* . sparse_hash_map offers amazing memory footprint at the cost of being slow, and uses a specific data structure to achieve such results, a  *sparsetable* . More information about sparsetables and sparse_hash_map can be found in the “ *Implementation notes* ” page of SparseHash [[19]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_19). Here I will only cover dense_hash_map.

dense_hash_map handles collisions with quadratic internal probing. Like for unordered_map, the bucket array is allocated on the heap at once, and scales up or down automatically based on the load factor of the hash table. Elements of the bucket array are instances of `std::pair` where `Key` are `T` are the template parameters for the keys and values, respectively. On a 64-bit architecture and for storing strings, an instance of pair will be 16 bytes.

Figure 5.2 below is a representation of the memory and access patterns for dense_hash_map. If we look for the GPS coordinates of “Johannesburg”, we would fall in bucket #0 at first, which has data for “Paris” (black arrow at the right of bucket #0). So we would have to probe and jump at bucket (i + 1) = (0 + 1) = 1 (orange arrow at the left of bucket #0), and then we would find the data for “Johannesburg” from bucket #1 (black arrow at the right of bucket #1). This seems similar to what was going on with unordered_map, but it is actually very different. Sure, the keys and values will have to be stored in memory allocated on the heap just like for unordered_map, which means that the key and value lookups will invalidate the cache line. But navigating among the entries in collision for a bucket is going to be rather fast. Indeed, given that each pair is 16 bytes and that the cache line is 64 bytes on most processors, the probing steps are very likely to be on the same cache line, which is going to speed things up dramatically, as opposed to the linked list in unordered_map which required jumping in the RAM to get the following nodes.

This cache line optimization offered as by the quadratic internal probing is what makes dense_hash_map the winner of all the performance tests for in-memory hash tables (as least those I have read so far). You should take a moment to review the “Hash Table Benchmarks” article by Nick Welch [[10]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_10).

> ‎SparseHash 库提供两种哈希表实现：‎*‎sparse_hash_map‎*‎和‎*‎dense_hash_map‎*‎。sparse_hash_map以速度慢为代价提供了惊人的内存占用量，并使用特定的数据结构来实现这样的结果，这是一种‎*‎可分割的‎*‎。有关 sparsetables 和 sparse_hash_map的更多信息，可以在 SparseHash ‎[‎[19]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_19)‎ 的“‎*‎实现说明‎*‎”页面中找到。在这里，我只介绍dense_hash_map。‎
>
> ‎dense_hash_map处理具有二次内部探测的碰撞。与unordered_map一样，存储桶数组一次在堆上分配，并根据哈希表的负载因子自动向上或向下扩展。存储桶数组的元素是其中分别是键和值的模板参数的实例。在 64 位体系结构上，为了存储字符串，pair 的实例将为 16 个字节。‎
>
> 下面的图 5.2 表示dense_hash_map的内存和访问模式。如果我们查找“约翰内斯堡”的GPS坐标，我们首先会落入桶#0中，其中包含“巴黎”的数据（桶#0右侧的黑色箭头）。因此，我们必须探测并跳到桶（i + 1）= （0 + 1）= 1（桶#0左侧的橙色箭头），然后我们将从桶#1中找到“约翰内斯堡”的数据（桶#1右侧的黑色箭头）。这似乎与unordered_map的情况相似，但实际上非常不同。当然，键和值必须存储在堆上分配的内存中，就像unordered_map一样，这意味着键和值查找将使缓存行无效。但是，在存储桶的冲突条目之间导航将相当快。实际上，假设每个对是16个字节，并且大多数处理器上的缓存行是64个字节，探测步骤很可能在同一缓存行上，这将大大加快速度，而不是unordered_map中的链接列表需要跳入RAM才能获得以下节点。‎
>
> 这种由二次内部探测提供的缓存行优化使dense_hash_map成为内存中哈希表的所有性能测试的赢家（至少是我到目前为止所读到的那些）。你应该花点时间回顾一下Nick Welch‎[‎[10]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_10)‎的“哈希表基准测试”文章。‎

![kvstore_hash_dense_hash_map_web](https://i0.wp.com/codecapsule.com/wp-content/uploads/2013/04/kvstore_hash_dense_hash_map_web.jpg?resize=770%2C910)

Figure 5.2

### 2.3 HashDB from Kyoto Cabinet

Kyoto Cabinet implements many data structures, among which a hash table. This hash table, HashDB, was designed to be persistent on-disk, even though there is an option to use it as an in-memory replacement for `std::map`. The hash table metadata along with the user’s data are all stored sequentially in a unique file on disk using the file system.

Kyoto Cabinet handles collisions with separate chaining through a binary search tree for each bucket. The bucket array has a fixed length and is never resized, regardless of the state of the load factor. This has been a major drawback of the hash table implementation of Kyoto Cabinet. Indeed, if the size of the bucket array defined at the creation of the database is below its actual needs, then performance suffers badly when entries start colliding.

It is very difficult to allow the bucket array to be resized for an on-disk hash table implementation. First, that would require the bucket array and the entries to be stored into two separate files, so that they could grow independently. Second, since resizing the bucket array requires to re-hash the keys to their new locations in the new bucket array, that would require reading from disk all the keys for all the entries, which would be very costly or almost impossible in the case of very large databases. One way to avoid this re-hashing process would be to store the hashed keys, but that would mean 4 or 8 more bytes of structural data for each entry (depending on whether the hash is 32- or 64-bit long). Because of all these complications, having a fixed-length bucket array is simpler, and it is the solution that was adopted for HashDB in Kyoto Cabinet.

Figure 5.3 shows the structure of a HashDB stored in a file. I have derived this internal structure from the code in the `calc_meta()` method, and from the comments of the attributes of the HashDB class at the end of kchashdb.h. The file is organized in sections as follows:

* The headers with all the metadata for the database
* The FreeBlock pool that holds the free space in the data area
* The bucket array
* The records (data area)

A record holds an entry (key/value pair), along with a node of the binary search tree for the separate chaining. Here is the Record struct:

> Kyoto Cabinet实现了许多数据结构，其中包括哈希表。此哈希表 HashDB 被设计为永久保存在磁盘上，即使可以选择将其用作‎`std::map`内存中的替代项。哈希表元数据以及用户的数据都使用文件系统按顺序存储在磁盘上的唯一文件中。‎
>
> ‎Kyoto Cabinet通过每个存储桶单独的二叉搜索树链接来处理冲突。存储桶数组具有固定的长度，并且无论负载因子的状态如何，都不会调整大小。这是‎Kyoto Cabinet哈希表实现的一个主要缺点。实际上，如果在创建数据库时定义的存储桶数组的大小低于其实际需求，则当条目开始冲突时，性能会受到严重影响。‎
>
> 允许为磁盘上的哈希表实现调整存储桶数组的大小是非常困难的。首先，这需要将存储桶数组和条目存储到两个单独的文件中，以便它们可以独立增长。其次，由于调整存储桶数组的大小需要将key重新散列到新存储桶数组中的新位置，因此需要从磁盘读取所有条目的key，这在非常大的数据库的情况下成本非常昂贵或几乎是不可能的。避免这种重新散列过程的一种方法是存储散列的密钥，但这意味着每个条目多4或8个字节的结构数据（取决于散列的长度是32位还是64位）。由于所有这些复杂性，拥有固定长度的桶阵列更简单，这是‎Kyoto Cabinet中HashDB采用的解决方案。
>
> ‎图 5.3 显示了存储在文件中的HashDB的结构。我从方法‎`calc_meta()`中的代码以及kchashdb.h末尾的HashDB类属性的注释中派生了此内部结构。该文件按以下部分进行组织：
>
> * ‎包含数据库所有元数据的标头‎
> * ‎在数据区域中保存可用空间的 FreeBlock 池‎
> * ‎存储桶数组‎
> * ‎记录（数据区域）
>
> ‎记录保存一个条目（键/值对），以及用于单独链接的二叉搜索树的节点。下面是记录结构：‎

<pre class="wp-block-preformatted"><b>/* from kyotocabinet-1.2.76/kchashdb.h */</b> 
  /**
   * Record data.
   */
  struct Record {
    int64_t off;                         ///< offset
    size_t rsiz;                         ///< whole size
    size_t psiz;                         ///< size of the padding
    size_t ksiz;                         ///< size of the key
    size_t vsiz;                         ///< size of the value
    int64_t left;                        ///< address of the left child record
    int64_t right;                       ///< address of the right child record
    const char* kbuf;                    ///< pointer to the key
    const char* vbuf;                    ///< pointer to the value
    int64_t boff;                        ///< offset of the body
    char* bbuf;                          ///< buffer of the body
  };
</pre>

The on-disk organization of a record can be observed on Figure 5.4. I derived this organization from the code in the `write_record()` method in kchashdb.h. Note that this is different from the Record struct: the goal of the on-disk representation is to minimize space on disk, while the struct aims at making the record easy to use programmatically. All the fields in Figure 5.4 have a fixed length, except for `key`, `value`, and `padding`, which of course depend on the size of the data being held by the entry. The `left` and `right` fields are part of the node of the binary search tree, and hold the offset to other records in the file.

> ‎可以在图 5.4 中观察到记录的磁盘组织。我从kchashdb.h中write_record()方法中的代码中派生了这个组织。请注意，这与 Record 结构不同：磁盘上表示的目标是最小化磁盘上的空间，而结构旨在使记录易于以编程方式使用。图 5.4 中的所有字段都有固定的长度，除了 `key value padding`，这当然取决于条目所保存的数据的大小。left和right字段是二叉搜索树节点的一部分，并保持与文件中其他记录的偏移量。

![kvstore_hash_kyoto_cabinet_web](https://i0.wp.com/codecapsule.com/wp-content/uploads/2013/04/kvstore_hash_kyoto_cabinet_web.jpg?resize=770%2C910)

Figure 5.3

![kvstore_hash_kyoto_cabinet_record_web](https://i0.wp.com/codecapsule.com/wp-content/uploads/2013/04/kvstore_hash_kyoto_cabinet_record_web.jpg?resize=329%2C362)

Figure 5.4

If we wanted to access the value for the key “Paris”, we would start by getting the offset of the initial record for the associated bucket, which happens to be bucket #0. We would then jump to the head node of the binary search tree for that bucket (orange arrow on the left of bucket #0), which holds the data for the key “Johannesburg”. The data for the key “Paris” can then be accessed through the right child of the current node (black arrow at the right of the record for “Johannesburg”). Binary search trees need a “comparable” type in order to classify nodes. The comparable type used here is simply the hashed keys shrunk into a smaller representation using the `fold_hash()` method:

> ‎如果我们想访问键“Paris”的值，我们将首先获取关联存储桶的初始记录的偏移量，该偏移恰好是存储桶 #0。然后，我们将跳转到该存储桶的二叉搜索树的头节点（存储桶 #0 左侧的橙色箭头），其中包含键“约翰内斯堡”的数据。然后，可以通过当前节点的右子节点（“约翰内斯堡”记录右侧的黑色箭头）访问键“Paris”的数据。二叉搜索树需要“可比较”类型才能对节点进行分类。此处使用的可比类型只是使用该方法缩小为较小表示形式的散列键：‎`fold_hash()`

<pre class="wp-block-preformatted"><b>/* from kyotocabinet-1.2.76/kchashdb.h */</b> 
uint32_t fold_hash(uint64_t hash) {
  _assert_(true);
  return (((hash & 0xffff000000000000ULL) >> 48) | ((hash & 0x0000ffff00000000ULL) >> 16)) ^
      (((hash & 0x000000000000ffffULL) << 16) | ((hash & 0x00000000ffff0000ULL) >> 16));
}
</pre>

Storing the entries and nodes together into a single record might seem like a design mistake at first, but it is actually very clever. In order to store the data for an entry, one will always need to manage three different data: bucket, collision, and entry. Given that buckets in the bucket array must be stored sequentially per definition, they will be stored as such and there is nothing to improve there. Then assuming we are not storing integral types but strings or variable-length byte arrays that cannot be stored in the buckets themselves, another memory access will have to be made outside of the area of the bucket array. Therefore when adding a new entry, one would need to store data for the collision data structure and for the entry’s key and value.

If the collision and entry data were stored separately, that would require accessing the disk twice, in addition to the access already required for the bucket. In the case of setting a value, that would make a total of three writes on disk, at potentially very distant locations. This means a pattern of random writes on disk, which is as far as I/O is concerned the worst possible thing ever. Now since in Kyoto Cabinet’s HashDB the node data and entry data are stored together, they can be committed to disk with just one write instead of two. Sure, the bucket still has to be accessed, but if the bucket array is small enough, then chances are that it will be cached from disk into RAM by the operating system anyway, which is one of the major assumption of Kyoto Cabinet, as stated in the Section “Effective Implementation of Hash Database” of the specs [[17]](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_).

There is however one concern to be raised with having the binary search tree nodes stored with the entries on disk, which is that it slows down the reads, at least when collisions start kicking in. Indeed, since the nodes are stored with the entries, resolving a collision in a bucket means finding the record holding the valid entry in the binary search tree, which requires possibly many random reads on the disk. This gives a better understanding as to why Kyoto Cabinet shows such drops in performance when the number of entries exceeds the number of buckets.

Finally, because everything is stored in a file, memory management is being handled by Kyoto Cabinet itself, and is not left to the operating system like it is the case for unordered_map and dense_hash_map. The FreeBlock struct holds information regarding free space in the file, which is basically the offset and size, as it can be seen here:

> ‎将条目和节点一起存储到单个记录中乍一看似乎是一个设计错误，但实际上非常聪明。为了存储条目的数据，始终需要管理三个不同的数据：存储桶、冲突和条目。鉴于存储桶数组中的存储桶必须按照定义按顺序存储，因此它们将按此存储，并且没有任何需要改进的地方。然后，假设我们存储的不是整数类型，而是存储无法存储在存储桶本身中的字符串或可变长度字节数组，则必须在存储桶数组区域之外进行另一次内存访问。因此，在添加新条目时，需要存储碰撞数据结构以及条目的键和值的数据。
>
> ‎如果碰撞和条目数据是分开存储的，那么除了存储桶已经需要的访问之外，还需要访问磁盘两次。在设置值的情况下，这将在磁盘上总共写入三次，可能位于非常遥远的位置。这意味着磁盘上的随机写入模式，就I / O而言，这是有史以来最糟糕的事情。现在，由于在Kyoto Cabinet的HashDB中，节点数据和条目数据存储在一起，因此只需一次写入而不是两次写入即可将它们提交到磁盘。当然，仍然必须访问存储桶，但是如果存储桶数组足够小，那么操作系统无论如何都有可能将其从磁盘缓存到RAM中，这是Kyoto Cabinet的主要假设之一，如规范的“哈希数据库的有效实现”一节‎[‎所述[17]‎](https://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/#ref_)‎。‎
>
> ‎然而，将二叉搜索树节点与磁盘上的条目一起存储存在一个问题，即它至少当冲突开始启动时会减慢读取速度。实际上，由于节点与条目一起存储，因此在存储桶中解决冲突意味着在二叉搜索树中找到保存有效条目的记录，这可能需要在磁盘上进行多次随机读取。这样可以更好地理解为什么当条目数量超过桶数时，Kyoto Cabinet的显示性能如此下降。‎
>
> ‎最后，由于所有内容都存储在文件中，因此内存管理由Kyoto Cabinet本身处理，而不是像unordered_map和dense_hash_map那样留给操作系统。FreeBlock 结构保存有关文件中可用空间的信息，这基本上是偏移量和大小，如下所示：‎

<pre class="wp-block-preformatted"><b>/* from kyotocabinet-1.2.76/kchashdb.h */</b> 
  /**
   * Free block data.
   */
  struct FreeBlock {
    int64_t off;                         ///< offset
    size_t rsiz;                         ///< record size
    /** comparing operator */
    bool operator <(const FreeBlock& obj) const {
      _assert_(true);
      if (rsiz < obj.rsiz) return true;
      if (rsiz == obj.rsiz && off > obj.off) return true;
      return false;
    }
  };
</pre>

All the FreeBlock instances are loaded in a std::set, which allows free memory blocks to be retrieved using the upper_bound() method of std::set as seen in the `fetch_free_block()` method, making the memory allocation strategy a “best fit”. When the free space appears to be too fragmented or that no space is left in the FreeBlock pool, the file is defragmented. This defragmentation process moves records around to reduce the overall size of the database file.

> 所有 FreeBlock 实例都加载到 std：：set 中，这允许使用 std：：set 的 upper_bound（） 方法检索可用内存块，如 `fetch_free_block()`方法所示，使内存分配策略成为“最佳选择”。当可用空间看起来过于碎片化或 FreeBlock 池中没有剩余空间时，将对文件进行碎片整理。此碎片整理过程会移动记录以减小数据库文件的总体大小。‎

## 3. Conclusion

In this article, I have presented the data organization and memory access patterns for three different hash table libraries. The unordered_map from TR1 and dense_hash_map from SparseHash are in memory, and HashDB from Kyoto Cabinet is on disk. All three make use of different solutions for handling collisions, with different effects on performance. Separating the bucket data, collision data and entry data will impact performance, which is what happens with unordered_map. Speed can be improved greatly by storing the collision data with either the buckets, as it is the case with dense_hash_map and its quadratic internal probing, or with the entries as it is the case with HashDB. Both solutions improve the speed for the writes, but storing the collision data with the buckets will also makes the reads faster.

If there is one thing that I have learned from studying those hash table libraries, it is that when designing the data organization of a hash table, the preferred solution should be to store the collision data with the buckets and not with the entries. This is because even if the hash table is on disk, the bucket array and collision data will be small enough so that they can be stored in the RAM, where random reads are a lot cheaper than on disk.

> ‎在本文中，我介绍了三个不同哈希表库的数据组织和内存访问模式。来自TR1的unordered_map和来自SparseHash的dense_hash_map在内存中，来自Kyoto Cabinet的HashDB在磁盘上。这三者都使用不同的解决方案来处理碰撞，对性能有不同的影响。将存储桶数据、碰撞数据和条目数据分开将影响性能，而unordered_map就会发生这种情况。通过使用存储桶存储碰撞数据（如dense_hash_map及其二次内部探测）或条目（如 HashDB）存储碰撞数据，可以大大提高速度。这两种解决方案都提高了写入速度，但将冲突数据存储在存储桶中也会使读取速度更快。
>
> ‎如果我从研究这些哈希表库中学到了一件事，那就是在设计哈希表的数据组织时，首选的解决方案应该是将冲突数据存储在存储桶而不是条目中。这是因为即使哈希表位于磁盘上，存储桶数组和冲突数据也将足够小，以便它们可以存储在RAM中，其中随机读取比在磁盘上便宜得多。

## 4. References

[]()[1] [http://en.wikipedia.org/wiki/Hash_table](http://en.wikipedia.org/wiki/Hash_table)
[]()[2] [http://www.amazon.com/Introduction-Algorithms-Thomas-H-Cormen/dp/0262033844/](http://www.amazon.com/Introduction-Algorithms-Thomas-H-Cormen/dp/0262033844/)
[]()[3] [http://en.wikipedia.org/wiki/Avalanche_effect](http://en.wikipedia.org/wiki/Avalanche_effect)
[]()[4] [http://blog.reverberate.org/2012/01/state-of-hash-functions-2012.html](http://blog.reverberate.org/2012/01/state-of-hash-functions-2012.html)
[]()[5] [http://www.strchr.com/hash_functions](http://www.strchr.com/hash_functions)
[]()[6] [http://programmers.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed/145633#145633](http://programmers.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed/145633#145633)
[]()[7] [http://blog.aggregateknowledge.com/2012/02/02/choosing-a-good-hash-function-part-3/](http://blog.aggregateknowledge.com/2012/02/02/choosing-a-good-hash-function-part-3/)
[]()[8] [https://sites.google.com/site/murmurhash/](https://sites.google.com/site/murmurhash/)
[]()[9] [http://google-opensource.blogspot.fr/2011/04/introducing-cityhash.html](http://google-opensource.blogspot.fr/2011/04/introducing-cityhash.html)
[]()[10] [http://incise.org/hash-table-benchmarks.html](http://incise.org/hash-table-benchmarks.html)
[]()[11] [http://preshing.com/20110603/hash-table-performance-tests](http://preshing.com/20110603/hash-table-performance-tests)
[]()[12] [http://attractivechaos.wordpress.com/2008/08/28/comparison-of-hash-table-libraries/](http://attractivechaos.wordpress.com/2008/08/28/comparison-of-hash-table-libraries/)
[]()[13] [http://attractivechaos.wordpress.com/2008/10/07/another-look-at-my-old-benchmark/](http://attractivechaos.wordpress.com/2008/10/07/another-look-at-my-old-benchmark/)
[]()[14] [http://blog.aggregateknowledge.com/2011/11/27/big-memory-part-3-5-google-sparsehash/](http://blog.aggregateknowledge.com/2011/11/27/big-memory-part-3-5-google-sparsehash/)
[]()[15] [http://gcc.gnu.org/](http://gcc.gnu.org/)
[]()[16] [https://code.google.com/p/sparsehash/](https://code.google.com/p/sparsehash/)
[]()[17] [http://fallabs.com/kyotocabinet/spex.html](http://fallabs.com/kyotocabinet/spex.html)
[]()[18] [http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2003/n1456.html](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2003/n1456.html)
[]()[19] [http://sparsehash.googlecode.com/svn/trunk/doc/implementation.html](http://sparsehash.googlecode.com/svn/trunk/doc/implementation.html)
[]()[20] [http://en.wikipedia.org/wiki/CPU_cache](http://en.wikipedia.org/wiki/CPU_cache)
