# Implementing a Key-Value Store – Part 1: What are key-value stores, and why implement one?


This is Part 1 of the IKVS series, “Implementing a Key-Value Store”. You can also check the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) for other parts.

In this article, I will start with a short description of what key-value stores are. Then, I will explain the reasons behind this project, and finally I will expose the main goals for the key-value store that I am planning to implement. Here is the list of the things I will cover in this article:

1. A quick overview of key-value stores
2. Key-value stores versus relational databases
3. Why implement a key-value store
4. The plan

> ‎这是 IKVS 系列“实现键值存储”的第 1 部分。您还可以在‎[‎目录中‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎查看其他部分。‎
>
> ‎在本文中，我将首先简要介绍什么是键值存储。然后，我将解释此项目背后的原因，最后我将公开我计划实现的键值存储的主要目标。以下是我将在本文中介绍的内容列表：‎
>
> 1. 键值存储‎的快速概述
> 2. 键值存储与关系数据库‎
> 3. 为什么要实现键值存储‎
> 4. 计划

## 1. A quick overview of key-value stores

This section is intended to be a very short introduction to key-value stores, as many more detailed articles have been written already. I have selected a few ones that you will find in the References section at the bottom of this article.

Key-value stores are one of the simplest forms of database. Almost all programming languages come with in-memory key-value stores. The map container from the C++ STL is a key-value store, just like the HashMap of Java, and the dictionary type in Python. Key-value stores generally share the following interface:

**Get( key ):** Get some data previously saved under the identifier “key”, or fail if no data was stored for “key”.

**Set( key, value ):** Store the “value” in memory under the identifier “key”, so we can access it later referencing the same “key”. If some data was already present under the “key”, this data will be replaced.

**Delete( key ):** Delete the data that was stored under the “key”.

Most underlying implementations are using either hash tables or some kind of self-balancing trees, like B-Trees or Red-black trees. Sometimes, the data is too big to fit in memory, or the data must be persisted in case the system crashes for any reason. In that case, using the file system becomes mandatory.

Key-value stores are part of the NoSQL movement, which regroup all the database systems that do no make use of all the concepts coined by relational databases. The [Wikipedia article on NoSQL](http://en.wikipedia.org/wiki/NoSQL) sums up very well what are the main characteristics of those databases:

* Do not use the SQL query language
* May not provide full support of the [ACID paradigm](http://en.wikipedia.org/wiki/ACID) (atomicity, consistency, isolation, durability)
* May offer a distributed, fault-tolerant architecture.

> ‎本节旨在对键值存储进行非常简短的介绍，因为已经编写了许多更详细的文章。我选择了一些，您可以在本文底部的“参考”部分找到它们。
>
> ‎键值存储是最简单的数据库形式之一。几乎所有编程语言都带有内存中的键值存储。C++ STL中的map是一个键值存储，就像Java的HashMap，Python中的dict类型。键值存储通常都有以下接口：
>
> **Get( key ):获取以键“key”对应的value，如果“key”不存在，则获取失败。‎**
>
> **Set( key, value ):**‎**‎存储一个键值对key-value，如果key已经存在，则替换对应的value值**
>
> **Delete( key ):删除指定key对应的键值对**
>
> 大多数底层实现都使用哈希表或某种自平衡树，如B树或红黑树。有时，数据太大而无法放入内存，或者必须保留数据，以防系统因任何原因崩溃。在这种情况下，必须使用文件系统。‎
>
> 键值存储是NoSQL的一部分，它重新组合了所有不使用关系数据库概念的数据库系统（非关系型数据库）。‎[‎关于NoSQL的维基百科文章‎](http://en.wikipedia.org/wiki/NoSQL)‎很好地总结了这些数据库的主要特征：‎
>
> * ‎没有 SQL 结构式查询语言‎
> * ‎可能无法完全支持 ‎[‎ACID 范式‎](http://en.wikipedia.org/wiki/ACID)‎（原子性、一致性、隔离性、持久性）‎
> * ‎可能提供分布式、容错体系结构。‎

## 2. Key-value stores and relational databases

Unlike relational databases, key-value stores have no knowledge of the data in the values, and do not have any schema like in MySQL or PostgreSQL. This also means that it is impossible to query only part of the data by doing any kind of filtering, as it can be done in SQL with the WHERE clause. If you do not know where to look for, you will have to iterate over all the keys, get their corresponding values, apply whatever filtering that you need on those values, and keep only the ones you need. This can be very computationally intensive, which implies that for full performance can only be attained in the cases where the keys are known, otherwise key-value stores turn up to be simply inadequate (Note: some key-value stores can store structured data and have fields indexed).

Therefore, even if key-value stores often outperform relational database systems by several orders of magnitude in terms of sheer access speed, the requirement to know the keys restricts the possible applications.

> 与关系数据库不同，键值存储对值中的数据信息一无所知，并且没有任何像MySQL或PostgreSQL那样的模式。这也意味着不可能通过执行某种类型的筛选操作来仅查询部分数据，虽然它可以在SQL中使用WHERE子句完成。如果您不知道在何处查找，则必须遍历所有键，获取其相应的值，对这些值应用所需的所有筛选机制，从而保留所需的值。这可能是一个计算密集型的操作，这意味着只有在键已知的情况下才能获得最高性能，否则键值存储根本不高效（注意：一些键值存储可以存储结构化数据并索引字段）。
>
> 因此，即使键值存储在纯粹的访问速度方面通常比关系数据库系统高出几个数量级，但知道键的要求限制了可能的应用程序。‎

## 3. Why implement a key-value store

I am starting this project as a way to refresh my knowledge of some fundamentals of hardcore back-end engineering. Reading books and Wikipedia articles is boring and exempt of practice, so I thought it would be better to get down to business and get my hands dirty with some code. I was looking for a project that would allow me to review:

* The C++ programming language
* Object-oriented design
* Algorithmics and data structures
* Memory management
* Concurrency control with multi-processors or multi-threading
* Networking with a server/client model
* I/O problems with disk access and use of the file system

A key-value store that is using the file system for permanent storage and which offers a networking interface would cover the whole range of topics listed above. This is just the perfect project that deals with all domains of back-end engineering.

> 我开始这个项目是为了回顾后端开发的一些基础知识。阅读书籍和维基百科文章很无聊，并且没有实际练习，所以我认为最好的方式是阅读一些实际的代码。我正在寻找一个允许我review的项目：‎
>
> * ‎C++编程语言‎
> * ‎面向对象设计‎
> * ‎算法和数据结构‎
> * ‎内存管理‎
> * ‎使用多处理器或多线程进行并发控制‎
> * ‎与服务器/客户端模型联网‎
> * ‎磁盘访问和文件系统使用的 I/O 问题‎
>
> ‎使用文件系统进行永久存储并提供网络接口，该key-value数据库将涵盖上面列出的所有主题。这是一个完美的项目，涉及后端开发的所有领域。‎

But let’s be realistic for a minute. There are already a lot of key-value stores out there, some of which have been implemented by really smart guys, and that are already in use in production in very big companies. These include names like Redis, MongoDB, memcached, BerkeleyDB, Kyoto Cabinet and LevelDB.

In addition to that, there appears to be a trend around key-value stores these days. It seems like everybody has one and wants to show how awesome and fast their key-value stores are. This issue is also described in [Leonard Lin’s blog article on key-value stores](http://randomfoo.net/2009/04/20/some-notes-on-distributed-key-stores). The majority of these projects at the moment are not mature and really not ready for production, but still, people like to show them off. It is very common to find blog articles or conference slides showing a comparison of the performance of some obscure key-value stores. Those charts are generally worth nothing, and only a test on your own hardware and with your own data and application will tell you which ones of these key-value stores are the most adapted to solve your problem. This is because performance depends on:

* The hardware
* The file system being used
* The actual application and order in which the keys are being accessed ([locality of reference](http://en.wikipedia.org/wiki/Locality_of_reference))
* The data set, and in particular the lengths of keys and values and the possibility for key collisions when hash tables are used

> 但是，让我们现实一点。已经有很多键值数据库，其中一些已经由一些真正杰出的人实现，并且已经在大型公司的实际生产中使用。其中包括Redis，MongoDB，memcached，BerkeleyDB，Kyoto Cabinet和LevelDB等。
>
> 除此之外，如今键值数据库似乎出现了一种趋势。似乎每个人都有一个，并希望展示他们的键值数据库有棒的性能。‎[‎这个问题在Leonard Lin关于键值存储的博客文章‎](http://randomfoo.net/2009/04/20/some-notes-on-distributed-key-stores)‎中也有描述。目前，这些项目中的大多数尚未成熟，实际上还没有准备好投入生产，但是，人们仍然喜欢炫耀它们。在博客文章或会议幻灯片中通过比较一些晦涩难懂的指标来展示键值存储的性能很常见。这些图表通常一文不值，只有在您自己的硬件上以及您自己的数据和应用程序上进行测试，才能告诉您这些键值存储中的哪些最适合解决您的问题。这是因为性能取决于：‎
>
> * ‎硬件‎
> * ‎正在使用的文件系统‎
> * ‎实际应用程序和访问key的顺序（‎‎引用位置）‎)
> * ‎数据集，特别是键和值的长度以及使用哈希表时键冲突的可能性‎

Therefore, coding a key-value store that will have an impact is going to be relatively hard, because it is very likely to end up unnoticed under the weight of the best key-value stores already available, or to simply drown in the ocean of half-baked side projects that nobody cares about.

In order to be different, this project cannot run for speed like everybody else, and must aim at filling a gap left by the solutions already available. Here are a few ways I see to have a key-value store project stand out from the crowd:

* Adapt to a specific data representation (ex: graphs, geographic data, etc.)
* Adapt to a specific operation (ex: performing very well for reads only, or writes only, etc.)
* Adapt to a specific issue (ex: automatic parameter tuning, as many key-value stores have many options, and it’s sometimes a mess to find the best ones)
* Offer more data access options. For instance in LevelDB, data can be accessed forward or backward, with iterators, and it has sorting on the keys. Not all key-value stores can do that.
* Make the implementation more accessible: right now, very few key-value stores have their code fully documented. If you need to get a project running quickly and you have to customize a key-value store for that, one that has code which seems really accessible will be a possible choice, even if it’s not a very well-known project. The fact that one can understand the code and therefore trust it will compensate for that.
* Specific applications. Here is an example of a real problem: many web crawling frameworks (web spiders) have a poor interface to manage the URLs they have to crawl, and this is often left to the client to implement the logic using a key-value store. All web crawling frameworks could benefit from a unified key-value store optimized for URL management.

> ‎因此，编写一个有影响力的键值数据库很困难，因为它很可能在现有的众多杰出的键值数据库的光辉下被忽视，或者只是淹没在大量没有人关心的半生不熟的项目海洋中。‎
>
> 为了展现差异性，这个项目不能像其他的简直数据库一样关注运行速度，而必须致力于填补现有解决方案留下的空白。以下是我认为让键值数据库项目脱颖而出的几种方法：‎
>
> * ‎适应特定的数据格式（例如：图形、地理数据等）‎
> * ‎适应特定操作（例如：仅读取或仅写入等性能非常好）‎
> * ‎适应特定问题（例如：自动参数调整，因为许多键值存储都有很多选项，有时很难找到最佳选项）‎
> * ‎提供更多数据访问选项。例如，在LevelDB中，可以使用迭代器向前或向后访问数据，并且它对键进行排序。并非所有键值存储都可以做到这一点。‎
> * ‎使实现更易于访问：现在，很少有键值存储的代码完全记录在案。如果你需要让一个项目快速运行，并且你必须为此自定义一个键值存储，那么一个具有看起来真正可访问的代码的存储将是一个可能的选择，即使它不是一个非常知名的项目。一个人可以理解代码并因此信任它的事实将弥补这一点。‎
> * ‎特定应用。下面是一个实际问题的示例：许多 Web 爬网框架（网络爬虫）的接口很差，无法管理它们必须爬网的 URL，这通常留给客户端使用键值存储来实现逻辑。所有 Web 爬网框架都可以从针对 URL 管理优化的统一键值存储中受益。‎

## 4. The plan

The goal of this project is to develop a lightweight key-value store in understandable C++ code. As a matter of fact, I am planning to follow the [Google C++ style guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml) for this project. I will use a hash table for the underlying data structure, the data will be persistent on disk, and a network interface will also be implemented. I will not run for absolute speed, but for conciseness and clarity in both design and implementation. I will also try the best I can to minimize the memory footprint of the database file on disk.

I do not want to re-invent the wheel, so I will start by looking around for key-value store projects in C or C++, and select those that stand out due to their quality. I will proceed to study their architecture and code, and draw inspiration from them for my own implementation. As back-end engineering is one of my core skills, I already have most of the knowledge needed for this project, but I know that I am going to learn a lot of new things too, and that’s what makes it interesting to me. I am also very happy to be documenting this whole thing. In the past, I have enjoyed hardcore technical blogs such as the ones of [Alexander Sandler](http://www.alexonlinux.com/) and [Gustavo Duarte](http://duartes.org/gustavo/blog/best-of), and I wanted to contribute with something useful myself, as best as I can.

The result of my research and my work on key-value stores will be documented in this blog article series. Do not try to use the dates of the articles to estimate the time it takes to implement a key-value store: the articles may be published with significant delays compared to the research and actions they describe.

In Part 2, I will search for top key-value stores projects and explain why I choose some of them as models and not others. For other articles, you can refer to the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) of the IKVS article series.

You will find below in the References section a few articles and book chapters to learn more about key-value stores. Before reading Part 2, I strongly recommend you to read at least [The NoSQL Ecosystem](http://www.aosabook.org/en/nosql.html) and [Key Value Stores: A Practical Overview](http://blog.marc-seeger.de/2009/09/21/key-value-stores-a-practical-overview/).

> 此项目的目标是使用C++开发一个代码易理解的轻量级键值存储。事实上，我计划遵循‎[‎Google C++这个项目的风格指南‎](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml)‎。我将使用哈希表作为底层数据结构，数据将持久保存在磁盘上，并且还将实现网络接口。我不会将关注点完全放在键值存储的性能上，而是力求设计和实现的简洁和清晰。我还将尽最大努力最大限度地减少磁盘上数据库文件的内存占用。‎
>
> 我不想重新发明轮子，所以我将首先会寻找C或C++开发有价值可复用的项目，并选择那些由于其质量而脱颖而出的项目。我将继续研究它们的体系结构和代码，并从中汲取灵感用于我自己的实现。由于后端开发是我的核心技能之一，我已经拥有了这个项目所需的大部分知识，但我知道我也会学到很多新东西，这就是让我感兴趣的原因。我也很高兴能记录下这整件事。过去，我喜欢硬核技术博客，例如‎[‎Alexander Sandler‎](http://www.alexonlinux.com/)‎和‎[‎Gustavo Duarte‎](http://duartes.org/gustavo/blog/best-of)‎的博客，我想尽我所能为自己贡献一些有用的东西。
>
> 我的研究和我在键值存储方面的工作结果将记录在本博客文章系列中。不要试图使用文章的日期来估计实现键值存储所需的时间：与它们描述的研究和操作相比，文章的发布可能会有很大的延迟。‎
>
> ‎在第 2 部分中，我将搜索顶级键值存储项目，并解释为什么我选择其中一些作为模型而不是其他项目。对于其他文章，您可以参考IKVS系列文章的‎[‎目录‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎。‎
>
> 您将在下面的“参考”部分找到一些文章和书籍章节，以了解有关键值存储的更多信息。在阅读第 2 部分之前，我强烈建议您至少阅读 ‎[‎NoSQL 生态系统‎](http://www.aosabook.org/en/nosql.html)‎和‎[‎键值存储：实用概述‎](http://blog.marc-seeger.de/2009/09/21/key-value-stores-a-practical-overview/)‎。‎

## References

[The NoSQL Ecosystem](http://www.aosabook.org/en/nosql.html), from the book “Architecture of Open Source Applications, Volume 1”
[NoSQL Patterns](http://horicky.blogspot.com.es/2009/11/nosql-patterns.html), by Ricky Ho
[NoSQL Databases](http://nosql-database.org/), referencing all the NoSQL databases that matter at the moment
[NoSQL Data Modeling Techniques](http://highlyscalable.wordpress.com/2012/03/01/nosql-data-modeling-techniques/), by Ilya Katsov
[NoSQL databases benchmark: Cassandra, HBase, MongoDB, Riak](http://www.networkworld.com/cgi-bin/mailto/x.cgi?pagetosend=/news/tech/2012/102212-nosql-263595.html), and the discussion on [Hacker News](http://news.ycombinator.com/item?id=4733212)
[Wikipedia article on NoSQL](http://en.wikipedia.org/wiki/NoSQL)
[Wikipedia article on the ACID paradigm](http://en.wikipedia.org/wiki/ACID)[Key Value Stores: A Practical Overview](http://blog.marc-seeger.de/2009/09/21/key-value-stores-a-practical-overview/), by Marc Seeger
[Some Notes on Distributed Key Stores](http://randomfoo.net/2009/04/20/some-notes-on-distributed-key-stores), by Leonard Lin
