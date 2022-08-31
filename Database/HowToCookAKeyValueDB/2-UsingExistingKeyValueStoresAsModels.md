# Implementing a Key-Value Store – Part 2: Using existing key-value stores as models

This is Part 2 of the IKVS series, “Implementing a Key-Value Store”. You can also check the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) for other parts.

In this article, I will start by explaining why I think it is important to use models for this project and not start completely from scratch. I will describe a set of criteria for selecting key-value store models. Finally, I will go over some well-known key-value store projects, and select a few of them as models using the presented criteria. This article will cover:

1. Not reinventing the wheel
2. Model candidates and selection criteria
3. Overview of the selected key-value stores

> ‎这是 IKVS 系列的第 2 部分“实现键值存储”。您还可以在‎[‎目录中‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎查看其他部分。‎
>
> ‎在本文中，我将首先解释为什么我认为为这个项目使用已有的模型很重要，而不是完全从头开始。我将描述一组用于选择键值存储模型的标准。最后，我将介绍一些众所周知的键值存储项目，并使用所提出的标准选择其中一些作为模型。本文将介绍：‎
>
> ‎1. 不重新发明轮子‎
> ‎ 2.模型候选和选择标准‎
> ‎ 3.所选键值存储概述‎

## 1. Not reinventing the wheel

Key-value stores have been out there for at least a good 30 years [[1]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_1). One of the most memorable projects is DBM, the initial database manager coded by Kenneth Thompson for Unix version 7 and released in 1979 [[2]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_2). Engineers have faced issues related to these database systems, and have either adopted or rejected various design and data structure ideas. They have learned through experiments with real-world problems. It would be foolish to not consider their work and start from scratch, only to repeat errors they have done before.

Gall’s law, from John Gall’s Systemantics: [[3]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_3)

> A complex system that works is invariably found to have evolved from a simple system that worked. The inverse proposition also appears to be true: A complex system designed from scratch never works and cannot be made to work. You have to start over, beginning with a working simple system.

This quote brings in two fundamental ideas for the development of my key-value store project:

 **1. Use models** . I need to identify key-value stores that have been out there for a while, and even better, which are successors of previously successful key-value stores. This would be the proof that their design is solid, and has been refined over time on multiple iterations. These selected key-value stores shall then be used as models for the project I am currently working on.

 **2. Start small** . The first version of this project should be small and simple, so its design can be easily tested and approved. Improvements and additional features should only come in later versions, if needed.

> ‎键值存储已经存在了至少30年‎[‎[1]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_1)‎。最令人难忘的项目之一是DBM，这是由Kenneth Thompson为Unix版本7编码并于1979年发布的最早的数据库管理器‎[‎[2]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_2)‎。工程师们面临着与这些数据库系统相关的问题，他们要么采用要么拒绝了各种设计和数据结构的想法。他们从现实世界问题的实践中学习。这实在是太蠢了，不考虑他们的工作仅仅是从头开始，重复他们以前犯过的错误。‎
>
> ‎加尔定律，来自约翰·加尔的系统证据：‎[‎[3]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_3)
>
>> ‎一个运行良好的复杂系统总是从一个简单系统演变而来的。相反的命题似乎也是正确的：一个从头开始设计的复杂系统总会出问题，而不能很好的运行起来。你必须重新开始，从一个工作的简单系统开始。‎
>>
>
> ‎这句话为我的键值存储项目的发展带来了两个基本思想：‎
>
> **‎1. 使用模型‎**‎。我需要确定已经存在了一段时间的键值存储，甚至，它们是以前成功的键值存储的继承者。这将证明他们的设计是可靠的，并且随着时间的推移在多次迭代中得到了改进。然后，这些选定的键值存储将用作我当前正在处理的项目的模型。‎
>
> **‎2. 从简单系统开始‎**‎。该项目的第一个版本应该小而简单，因此其设计可以更容易测试和实现。如果需要，改进和附加功能应仅在更高版本中提供。‎

## 2. Model candidates and selection criteria

After doing a bit of research around key-value stores and NoSQL databases, I have decided to consider the following ones for a more thorough selection:

* DBM
* Berkeley DB
* Kyoto Cabinet
* Memcached and MemcacheDB
* LevelDB
* MongoDB
* Redis
* OpenLDAP
* SQLite

My criteria are the following:

* I want to create a key-value store using Object-Oriented Programming, so for the design, I will have to draw inspiration from projects coded in an OOP language.
* For the underlying data structure, I want to have an on-disk hash table, so I will need to select projects that offer a way to read and write information to disk.
* I also want to have a network access to the data store.
* I do not need a query engine or ways to access the data in a structured way.
* I do not need to support the full ACID paradigm.
* Given that I am pursuing this project by myself, I want to take for model projects that have been implemented by small teams, ideally one or two persons.

> ‎在对键值存储和NoSQL数据库进行了一些研究之后，我决定考虑以下数据库以进行更全面的选择：‎
>
> * DBM
> * Berkeley DB
> * Kyoto Cabinet
> * Memcached and MemcacheDB
> * LevelDB
> * MongoDB
> * Redis
> * OpenLDAP
> * SQLite
>
> 我的标准如下：‎
>
> * ‎我想使用面向对象编程创建一个键值存储，因此对于设计，我必须从用OOP语言编码的项目中获取灵感。‎
> * ‎对于基础数据结构，我希望有一个磁盘上的哈希表，因此我需要选择提供在磁盘读取和写入数据接口的项目。‎
> * ‎我还希望通过网络访问数据存储。‎
> * ‎我不需要查询引擎或以结构化方式访问数据的方法。‎
> * ‎我不需要支持完整的ACID范式。‎
> * ‎鉴于我自己正在从事这个项目，我想采取由小团队实施的模型项目，最好是一两个人。‎

## 3. Overview of the selected key-value stores

The three big winners are Berkeley DB, Kyoto Cabinet and LevelDB. Berkeley DB and Kyoto Cabinet share a common history as successors to DBM. In addition, Berkeley DB and Kyoto Cabinet are not “first versions”, they are the N-th versions of their authors. This means that they are more solid compared to projects done by people who would be implementing a key-value store for the first time. LevelDB is more recent and is based on LSM Tree data structure, which is of no use as a model for a hash table. However, the code itself is one of the cleanest I have seen ever. All three projects have been developed by one or two persons. Below are more details about each of them.

> ‎最终选出来的三个数据存储分别是Berkeley DB，Kyoto Cabinet和LevelDB。Berkeley DB和Kyoto Cabinet作为DBM的继任者有着共同的历史。此外，Berkeley DB和Kyoto Cabinet不是“第一版”，它们是其作者的第N个版本。这意味着与首次实现键值存储的人完成的项目相比，它们更加可靠。LevelDB是较新的，它基于LSM树数据结构，它并没有实现哈希表的存储模型。但是，它的代码本身是我见过的最干净的代码之一。而且这三个项目都是由一两个人开发的。以下是有关其中每个的更多详细信息。

### Berkeley DB

The development of Berkeley DB started in 1986, which means that at the moment I am writing this article, it has been out there for 26 years. Berkeley DB was developed as a successor to DBM, and implements a hash table. The first version was coded by Margo Seltzer [[22]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_22) and Ozan Yigit [[23]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_23), while they were at The University of California, Berkeley. The project was later acquired by Oracle, which continued to develop it.

Berkeley DB was originally implemented in C, and continues to be a C-only nowadays. The development went through an incremental process, which added features with each major version. From a simple key-value store, Berkeley DB went on managing access concurrency, transactions and recovery, and replication [[4]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_4). Berkeley DB has been used extensively with hundreds of millions of deployed copies [[5]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_5), which is the proof that its architecture can be trusted as extremely solid. More information about its design can be found in the Introduction of the “ *Berkeley DB Programmer’s Reference Guide* ” [[6]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_6), and the entry of “ *The Architecture of Open Source Applications, Volume 1* ” [[5]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_5).

> ‎Berkeley DB的开发始于1986年，这意味着在我写这篇文章的那一刻，它已经存在了26年。Berkeley DB是作为DBM的继承者开发的，并实现了哈希表。第一个版本是由Margo Seltzer ‎[‎[22]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_22)‎和Ozan Yigit ‎[‎[23]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_23)‎编写的，当时他们在加州大学伯克利分校。该项目后来被甲骨文收购，甲骨文继续开发它。‎
>
> ‎Berkeley DB最初是用C语言实现的，现在仍然是C语言。开发经历了一个渐进的过程，每个主要版本都增加了新功能。从一个简单的键值存储，Berkeley DB增加了访问并发管理，事务和恢复以及复制功能‎[‎[4]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_4)‎。Berkeley DB已被应用在数亿个部署的副本上‎[‎[5]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_5)‎，这证明了它的架构非常可靠。有关其设计的更多信息，请参阅“‎*‎Berkeley DB Programmer's Reference Guide‎*‎”‎[‎[6]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_6)‎的介绍，以及“‎*‎The Architecture of Open Source Applications， Volume 1‎*‎”‎[‎[5]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_5)‎的条目。‎

### Kyoto Cabinet

Kyoto Cabinet was introduced in 2009 by Mikio Hirabayashi [[24]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_24). It is still in active development at the moment. Kyoto Cabinet is a successor to other key-value stores by the same author, which are Tokyo Cabinet (released in 2007), and QDBM (released in 2003, started in 2000). QDBM was intended as a successor to DBM with higher performances [[7]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_7). Kyoto Cabinet is particularly interesting because there is a clear lineage to DBM, and because its author has been working on key-value stores for over 12 years now. After going through the implementation of three key-value stores during so many years, there is no doubt that the author has acquired a solid understanding of the structural needs, along with a strong sense of the causes of performance bottlenecks.

Kyoto Cabinet was implemented in C++, and implements a hash table, a B+ Tree, and other more esoteric data structures. It also offers outstanding performance [[16]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_16). Nevertheless, there seems to be some performance problems due to the initial parameters. Indeed, many people have reported that performance is good as long as the number of items remains below a certain threshold which is proportional to the bucket array size, as defined by the parameters at the creation of a database file. Passed that threshold, performance seems to decrease dramatically [[18]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_18) [[19]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_19). Similar problems are present with Tokyo Cabinet [[20]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_20) [[21]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_21). This means that if the requirements of a project are changing while the database is being used, you could run into serious troubles. And we all know how much change is constant in software.

> ‎Kyoto Cabinet于2009年由Mikio Hirabayashi‎[‎[24]提出‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_24)‎。它目前仍在积极开发中。Kyoto Cabinet是该作者开发的其他键值存储的继承者，这些键值存储分别是Tokyo Cabinet（2007年发布）和QDBM（2003年发布，2000年开始）。QDBM旨在作为DBM的继任者，具有更高的性能‎[‎[7]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_7)‎。Kyoto Cabinet特别有趣，因为DBM有明确的血统，并且因为它的作者已经在键值存储领域工作了12年以上。在经历了这么多年的三个键值存储的实际开发之后，毫无疑问，作者已经对结构需求有了深刻的理解，并且对造成性能瓶颈的原因有了强烈的认识。‎
>
> ‎Kyoto Cabinet是由C++开发的，并实现了哈希表，B +树和其他更复杂的数据结构。它还具有出色的性能 ‎[‎[16]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_16)‎。但是，由于初始参数的不同，似乎会造成一些性能问题。事实上，许多人都报告说，只要项目数保持在与存储桶数组大小成比例的某个阈值（由创建数据库文件时的参数定义）以下，性能就很好。超过这个阈值后，性能似乎急剧下降‎[‎[18]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_18)[‎[19]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_19)‎。Kyoto Cabinet也存在类似的问题‎[‎[20]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_20)[‎[21]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_21)‎。这意味着，如果在使用数据库时项目的要求发生变化，则可能会遇到严重的麻烦。我们都知道软件中有多少变化是恒定的。

### LevelDB

LevelDB is a key-value store developed by Google fellows Jeffrey Dean [[8]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_8) and Sanjay Ghemawat [[9]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_9), who worked on Google’s mythical infrastructure projects, MapReduce and BigTable. Given the experience on large-scale problems Dean and Ghemawat have had while working at Google, there are good chances that they know what they are doing. An interesting difference from most key-value store projects is that LevelDB is not using a hash table or a B-tree as underlying data structure, but is based on a Log-Structured Merge Tree [[12]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_12). LSM structures are allegedly optimized for SSD drives [[13]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_13). A ton of information about LevelDB can be found on the blog High Scalability blog [[17]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_17).

Released in 2011, LevelDB was implemented in C++, and was designed to be useful as a building block for higher-level storage systems [[10]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_10). The implementation of the IndexedDB HTML5 API in future versions of Chrome will be using LevelDB [[10]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_10) [[11]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_11). The performance is mind blowing under a  *certain workload* , as shows the benchmark provided by the authors [[14]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_14). However, another benchmark on commodity SSDs made by Andy Twigg at Acunu has shown that if the number of items is increased over 1e6 (one million), and going towards 1e9 entries (one billion), then performance drops dramatically [[15]](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_15). So it seems that LevelDB might not be the best choice for heavy load or large databases as often required by serious back-end projects.

But this does not matter really, as to me, the best part of LevelDB does not lay in its performance but in its architecture. Looking at the source code and the way things are organized, it is just pure beauty. Everything is clear, simple, and logical. Having access to LevelDB’s source code and using it as a model is an amazing opportunity to create great code.

> ‎LevelDB是由Google研究员Jeffrey Dean ‎[‎[8]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_8)‎和Sanjay Ghemawat ‎[‎[9]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_9)‎开发的键值存储，他们参与了Google神话般的基础设施项目MapReduce和BigTable。鉴于Dean和Ghemawat在谷歌工作时得到的在大型系统问题上的经验，他们肯定了解他们在做什么。与大多数键值存储项目的一个有趣的区别是，LevelDB不使用哈希表或B树作为基础数据结构，而是基于日志结构化合并树‎[‎[12]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_12)‎。据称，LSM结构针对SSD驱动器进行了优化‎[‎[13]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_13)‎。有关LevelDB的大量信息可以在博客High Scalability博客‎[‎[17]上找到‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_17)‎。‎
>
> ‎LevelDB于2011年发布，通过C++实现，旨在作为更高级别存储系统的底层存储引擎‎[‎[10]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_10)‎。IndexedDB HTML5 API在未来版本的Chrome中的实现将使用LevelDB ‎[‎[10]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_10)‎ ‎[‎[11]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_11)‎。在一‎‎定的工作负载下，性能表现很好，正如作者提供的基准‎[‎[14]所示‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_14)‎。然而，Acunu的Andy Twigg对SSD的另一个基准表明，如果存储数据量超过1e6（100万），并达到1e9个条目（10亿个），那么性能就会急剧下降‎[‎[15]‎](https://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/#ref_15)‎。因此，LevelDB似乎可能不是重负载或大型数据库的最佳选择，而这些常常是的后端项目需要的特性。‎
>
> ‎但这对我来说并不重要，LevelDB最好的部分不在于它的性能，而在于它的架构。看看源代码的组织方式，它只是纯粹的美。一切都是清晰，简单和合乎逻辑的。访问LevelDB的源代码并将其用作模型是创建出色代码的绝佳机会。‎

### What about the non-selected key-value stores?

The fact that I did not select the other key-value stores does not mean that I will just discard them fully. I will keep them in mind and might use some elements of their architectures sporadically. However, the current project will not be influenced by those key-value stores as much as it will be by the ones that have been selected.

> ‎事实上我没有选择其他的键值存储并不意味着我将完全丢弃它们。我会记住它们，并可能偶尔使用它们体系结构的某些元素。但是，当前项目不仅会受到这些键值存储的影响，而且会受到所选键值存储的影响。‎

## References

[]()[1] [http://blog.knuthaugen.no/2010/03/a-brief-history-of-nosql.html](http://blog.knuthaugen.no/2010/03/a-brief-history-of-nosql.html)
[]()[2] [http://en.wikipedia.org/wiki/Dbm](http://en.wikipedia.org/wiki/Dbm)
[]()[3] [http://en.wikipedia.org/wiki/Systemantics](http://en.wikipedia.org/wiki/Systemantics)
[]()[4] [http://en.wikipedia.org/wiki/Berkeley_DB#Origin](http://en.wikipedia.org/wiki/Berkeley_DB#Origin)
[]()[5] [http://www.aosabook.org/en/bdb.html](http://www.aosabook.org/en/bdb.html)
[]()[6] [http://docs.oracle.com/cd/E17076_02/html/programmer_reference/intro.html](http://docs.oracle.com/cd/E17076_02/html/programmer_reference/intro.html)
[]()[7] [http://fallabs.com/qdbm/](http://fallabs.com/qdbm/)
[]()[8] [http://research.google.com/people/jeff/](http://research.google.com/people/jeff/)
[]()[9] [http://research.google.com/pubs/SanjayGhemawat.html](http://research.google.com/pubs/SanjayGhemawat.html)
[]()[10] [http://google-opensource.blogspot.com/2011/07/leveldb-fast-persistent-key-value-store.html](http://google-opensource.blogspot.com/2011/07/leveldb-fast-persistent-key-value-store.html)
[]()[11] [http://www.w3.org/TR/IndexedDB/](http://www.w3.org/TR/IndexedDB/)
[]()[12] [http://www.igvita.com/2012/02/06/sstable-and-log-structured-storage-leveldb/](http://www.igvita.com/2012/02/06/sstable-and-log-structured-storage-leveldb/)
[]()[13] [http://www.acunu.com/2/post/2011/04/log-file-systems-and-ssds-made-for-each-other.html](http://www.acunu.com/2/post/2011/04/log-file-systems-and-ssds-made-for-each-other.html)
[]()[14] [http://leveldb.googlecode.com/svn/trunk/doc/benchmark.html](http://leveldb.googlecode.com/svn/trunk/doc/benchmark.html)
[]()[15] [http://www.acunu.com/2/post/2011/08/benchmarking-leveldb.html](http://www.acunu.com/2/post/2011/08/benchmarking-leveldb.html)
[]()[16] [http://blog.creapptives.com/post/8330476086/leveldb-vs-kyoto-cabinet-my-findings](http://blog.creapptives.com/post/8330476086/leveldb-vs-kyoto-cabinet-my-findings)
[]()[17] [http://highscalability.com/blog/2011/8/10/leveldb-fast-and-lightweight-keyvalue-database-from-the-auth.html](http://highscalability.com/blog/2011/8/10/leveldb-fast-and-lightweight-keyvalue-database-from-the-auth.html)
[]()[18] [http://stackoverflow.com/questions/13054852/kyoto-cabinet-berkeley-db-hash-table-size-limitations](http://stackoverflow.com/questions/13054852/kyoto-cabinet-berkeley-db-hash-table-size-limitations)
[]()[19] [https://groups.google.com/forum/#!topic/tokyocabinet-users/Bzp4fLbmcDw/discussion](https://groups.google.com/forum/#!topic/tokyocabinet-users/Bzp4fLbmcDw/discussion)
[]()[20] [http://stackoverflow.com/questions/1051847/why-does-tokyo-tyrant-slow-down-exponentially-even-after-adjusting-bnum](http://stackoverflow.com/questions/1051847/why-does-tokyo-tyrant-slow-down-exponentially-even-after-adjusting-bnum)
[]()[21] [https://groups.google.com/forum/#!topic/tokyocabinet-users/1E06DFQM8mI/discussion](https://groups.google.com/forum/#!topic/tokyocabinet-users/1E06DFQM8mI/discussion)
[]()[22] [http://www.eecs.harvard.edu/margo/](http://www.eecs.harvard.edu/margo/)
[]()[23] [http://www.cse.yorku.ca/~oz/](http://www.cse.yorku.ca/~oz/)
[]()[24] [http://fallabs.com/mikio/profile.html](http://fallabs.com/mikio/profile.html)
