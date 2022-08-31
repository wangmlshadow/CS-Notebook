# Implementing a Key-Value Store – Part 3: Comparative Analysis of the Architectures of Kyoto Cabinet and LevelDB

This is Part 3 of the IKVS series, “Implementing a Key-Value Store”. You can also check the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) for other parts.

In this article, I will walk through the architectures of Kyoto Cabinet and LevelDB, component by component. The goal, as stated in Part 2 of the IKVS series, is to get insights at how I should create the architecture my own key-value store by analyzing the architectures of existing key-value stores. This article will cover:

1. Intent and methodology of this architecture analysis
2. Overview of the Components of a Key-Value Store
3. Structural and conceptual analysis of Kyoto Cabinet and LevelDB
   3.1 Create a map of the code with Doxygen
   3.2 Overall architecture
   3.3 Interface
   3.4 Parametrization
   3.5 String
   3.6 Error Management
   3.7 Memory Management
   3.8 Data Storage
4. Code review
   4.1 Organization of declarations and definitions
   4.2 Naming
   4.3 Code duplication

> 这是 IKVS 系列的第 3 部分“实现键值存储”。您还可以在‎[‎目录中‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎查看其他部分。‎
>
> 在本文中，我将逐个组件地介绍Kyoto Cabinet和LevelDB的架构。如 IKVS 系列的第 2 部分所述，其目标是通过分析现有键值存储的体系结构，深入了解我应该如何创建自己的键值存储的体系结构。本文将介绍：
>
> 1. 本架构分析‎‎的意图和方法
> 2. 键值存储‎‎的组件概述
> 3. Kyoto Cabinet和LevelDB‎的结构和概念分析
>    1. 使用Doxygen‎创建代码映射
>    2. 整体架构‎
>    3. 接口‎
>    4. 参数化‎
>    5. 字符串‎
>    6. 错误管理‎
>    7. 内存管理‎
>    8. 数据存储‎
> 4. 代码审查‎
>    1. 声明和定义的‎‎组织
>    2. 命名‎
>    3. 代码重复‎

![](https://i0.wp.com/codecapsule.com/wp-content/uploads/2012/12/kvstore_leveldb_kyotocabinet_small.jpg?resize=770%2C465 "kvstore_leveldb_kyotocabinet_small")

## 1. Intent and methodology of this architecture analysis

I was thinking whether I should do two separate articles, one for LevelDB and another one for Kyoto Cabinet, or a combined article with both. I believe that software architecture is a craft where decision making plays a very important role, as an architect needs to consider and choose among many alternatives for every part of a system. Solutions are never evaluated by themselves in isolation, but weighted against other solutions. The analysis of the architecture of a software system has value only if it is made in context, and compared to other architectures. For that reason, I will go thrugh some of the main components encountered in a key-value store, and compare for each of them the solutions developed by existing key-value stores. I will use my own analyses for Kyoto Cabinet and LevelDB, but for other projects, I will use existing analyses. Here are the external analyses that I have chosen to use:

- BerkeleyDB, Chapter 4 in The Architecture of Open Source Applications, by Margo Seltzer and Keith Bostic (Seltzer being one of the two original authors of BerkeleyDB) [[1]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_1)
- Memcached for dummies, by Tinou Bao [[2]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_2)
- Memcached Internals [[3]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_3)
- MongoDB Architecture, by Ricky Ho [[4]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_4)
- Couchbase Architecture, by Ricky Ho [[5]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_5)
- The Architecture of SQLite [[6]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_6)
- Redis Documentation [[7]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_7)

> ‎我在想，我是否应该写两篇单独的文章，一篇是针对LevelDB，另一篇是针对Kyoto Cabinet，或者应该将两篇结合起来。我相信软件架构是一门决策过程，其中决策起着非常重要的作用，因为架构师需要考虑并在系统的每个部分的许多替代方案中进行选择。解决方案从不单独评估，而是与其他解决方案进行加权。只有当软件系统的架构是在上下文中进行并与其他架构进行比较时，它才有价值。因此，我将介绍键值存储中遇到的一些主要组件，并比较每个组件由现有键值存储开发的解决方案。我将对Kyoto Cabinet和LevelDB使用我自己的分析，但对于其他项目，我将使用现有的分析。以下是我选择使用的外部分析：‎
>
> - BerkeleyDB, Chapter 4 in The Architecture of Open Source Applications, by Margo Seltzer and Keith Bostic (Seltzer being one of the two original authors of BerkeleyDB) [[1]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_1)
> - Memcached for dummies, by Tinou Bao [[2]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_2)
> - Memcached Internals [[3]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_3)
> - MongoDB Architecture, by Ricky Ho [[4]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_4)
> - Couchbase Architecture, by Ricky Ho [[5]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_5)
> - The Architecture of SQLite [[6]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_6)
> - Redis Documentation [[7]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_7)

## 2. Overview of the Components of a Key-Value Store

In spite of major differences in their internal architectures, key-value stores have very similar components. Below is a list of the major components encountered in most key-value stores, along with shorts descriptions of their utility.

**Interface:** The set of methods and classes exposed to the clients of a key-value store so they can interact it. This is also referred as the API. The minimum API for a key-value store must include the methods Get(), Put() and Delete().

**Parametrization:** The way that options are being set and passed to components across the whole system.

**Data Storage:** The interface used to access the memory where the data, i.e. keys and values, are stored. If the data must be *persisted* on non-volatile storage such as hard drive or flash memory, then problems of *synchronization* and *concurrency* may arise.

**Data Structure:** The algorithms and methods being used to organize the data, and allow for efficient storage and retrieval. This is generally a hash table or B+ Tree. In the case of LevelDB, it is a Log-Structured Merge Tree. The choice of the data structure may depend on the internal structure of the data and the underlying data storage solution.

**Memory Management:** The algorithms and techniques being used to manage the memory used by the system. This is crucial as a data storage accessed with the wrong memory management technique can impact performance dramatically.

**Iteration:** The ways by which all the keys and values in a database can be enumerated and accessed sequentially. The solutions are mostly Iterators and Cursors.

**String:** The data structure used to represent and access strings of characters. This might seem like a detail, but for key-value stores, a great deal of time is being spent on passing and processing strings, and std::string from the STL might not be the best solution.

**Lock Management:** All the facilities related to the locking of concurrently accessed memory areas (with mutexes and semaphores), and the locking of files if the data storage is the file system. Also handles issues related to multithreading.

**Error Management:** The techniques used to intercept and handle errors encountered in the system.

**Logging:** The facilities that log the events happening in the system.

**Transaction Management:** Mechanism over a set of operations which ensures that all the operations are executed correctly, and in case of an error, that none of the operations is executed and the database is left unchanged.

 **Compression** : The algorithms used to compress the data.

**Comparators:** Comparators provide ways to order two keys with regard to each other.

**Checksum:** The methods used to test and ensure the integrity of the data.

**Snapshot:** A Snapshot provides a read-only view of the entire database as it was when the snapshot was created.

 **Partitioning** : Also referred to as Sharding, this consists in splitting the data set into multiple data storages, possibly distributed across multiple nodes on a network.

 **Replication** : In order to ensure durability in case of system or hardware failures, some key-value stores allow for multiple copies of the data – or of partitions of the data – to be maintained simultaneously, preferably on multiple nodes.

 **Testing Framework** : Framework being used to test the system, including unit and integration testing.

> ‎尽管其内部体系结构存在重大差异，但键值存储都具有非常相似的组件。以下是大多数键值存储中遇到的主要组件的列表，以及它们的用途的简短描述。
>
> **Interface:** 向键值存储的客户端公开的方法和类集，以便它们可以与之交互。这也称为 API。键值存储的最低 API 必须包括 Get（）、Put（） 和 Delete（） 方法
>
> **Parametrization:** 设置选项并将其传递给整个系统中的组件的方式。
>
> **Data Storage:** **‎**‎用于访问存储数据的存储器（即键和值）的接口。如果数据必须‎*‎保存在‎*‎非易失性存储（如硬盘驱动器或闪存）上，则可能会出现‎*‎同步‎*‎和‎*‎并发‎*‎问题。‎
>
> **Data Structure:** 用于组织数据的算法和方法，并允许高效的存储和检索。这通常是哈希表或 B+ 树。在LevelDB中，它是一个LSM Tree。数据结构的选择可能取决于数据的内部结构和底层数据存储解决方案。
>
> **Memory Management:** 用于管理系统使用的内存的算法和技术。这一点至关重要，因为使用错误的内存管理技术访问的数据存储可能会对性能产生巨大影响。‎
>
> **Iteration:** 按顺序枚举和访问数据库中所有键和值的方式。解决方案主要是迭代器和游标。‎
>
> **String:** 用于表示和访问字符串的数据结构。这似乎是一个细节，但对于键值存储，在传递和处理字符串上花费了大量时间，STL 中的 std::string 可能不是最佳解决方案。
>
> **Lock Management:** 所有与锁定并发访问的内存区域（使用互斥锁和信号量）以及锁定文件（如果数据存储是文件系统）相关的功能。还处理与多线程处理相关的问题。‎
>
> **Error Management:** **‎**‎用于拦截和处理在系统中遇到的错误的技术。
>
> **Logging:** **‎**‎记录系统中发生的事件的工具。日志机制
>
> **Transaction Management:** **‎**‎一组操作的机制，确保正确执行所有操作，并且在发生错误时，不执行任何操作并且数据库保持不变。原子执行一组指令
>
> **Compression** : 用于压缩数据的算法。
>
> **Comparators:** 比较器提供了对两个键比较的方法。
>
> **Checksum:** **‎**‎用于测试和确保数据完整性的方法。‎
>
> **Snapshot:** 快照提供整个数据库的只读视图，就像创建快照时一样。‎
>
> **Partitioning** : 也称为分片，这包括将数据集拆分为多个数据存储，可能分布在网络上的多个节点上。
>
> **Replication** : 为了确保在系统或硬件发生故障时的持久性，一些键值存储允许同时维护数据的多个副本或数据分区的副本，最好是在多个节点上。‎
>
> **Testing Framework** :用于测试系统的框架，包括单元和集成测试。

## 3. Structural and conceptual analysis of Kyoto Cabinet and LevelDB

The following analysis of LevelDB and Kyoto Cabinet will focus on the following components:  Parametrization, Data Storage, String and Error Management . The components Interface, Data Structure, Memory Management, Logging and Testing Framework will be covered in future articles of the IKVS series. As for the rest of the components, I have no plans to cover them at the moment I am writing this article. Other systems, like relational databases, have other components such as Command Processor, Query Parser, and Planner/Optimizer, but all these are way beyond the scope of the IKVS series.

Before I start my analysis, please note that I consider both Kyoto Cabinet and LevelDB as great pieces of software, and I highly respect their authors. Even if I say bad things about their designs, keep in mind that their code is still awesome, and that I do not have accomplished what those guys did. This being said, you’ll find below my two cents about the code of Kyoto Cabinet and LevelDB.

> ‎以下对LevelDB和Kyoto Cabinet的分析将集中在以下组件上：‎‎参数，数据存储，字符串和错误管理‎‎。‎‎接口、数据结构、内存管理、日志和测试框架‎‎等组件将在 IKVS 系列后续文章中介绍。至于其余的组件，在我写这篇文章的时候，我没有计划涵盖它们。其他系统，如关系数据库，有其他组件，如命令处理器，查询解析器和规划器/优化器，但所有这些都超出了IKVS系列的范围。‎
>
> 在我开始分析之前，请注意，我认为Kyoto Cabinet和LevelDB都是伟大的软件，我非常尊重它们的作者。即使我对他们的设计说了不好的话，请记住，他们的代码仍然很棒，而且我还没有完成那些家伙所做的事情。话虽如此，你会在下面找到关于Kyoto Cabinet和LevelDB代码的个人浅见。‎

### 3.1 Create a map of the code with Doxygen

In order to understand the architectures of Kyoto Cabinet and LevelDB, I had to dig into their source code. But I also used Doxygen, which is a very powerful tool to navigate through the hierarchies of modules and classes of an application. Doxygen is a documentation system for various programming languages, which can generate documentation directly form source code in the form of a report or HTML website. Most people add comments with a special format to their classes and methods, and then use Doxygen to generate a documentation that contains those special comments. However, Doxygen can also be used on code that does not contain any comment, and will generate an interface based the organization – files, namespaces, classes and methods – of the system.

You can get Doxygen on the official website [[8]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_8). After having installed Doxygen on your machine, just open a shell and go to the directory that contains all the source code that you want to analyze. Then type the following command to create a default configuration file:

```
$ doxygen -g
```

This will create a file called “Doxygen”. Open this file, and make sure that the following options are all set to “YES”: `EXTRACT_ALL, EXTRACT_PRIVATE, RECURSIVE, HAVE_DOT, CALL_GRAPH, CALLER_GRAPH`. These options will make sure that all entities are extracted from the code, even in sub-directories, and that call graphs are generated. Full descriptions of all the available options can be found in the online documentation of Doxygen [[9]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_9). To generate the documentation with the selected options, simply type:

```
$ doxygen Doxygen
```

The documentation will be generated in the “html” directory, and you can access it by opening the “index.html” file in any web browser. You can navigate through the code, see inheritance relationships between classes, and thanks to the graphs you can also see for every method which other methods it is calling.

> 为了理解Kyoto Cabinet和LevelDB的架构，我不得不深入研究它们的源代码。我使用了Doxygen，这是一个非常强大的工具，可以浏览应用程序的模块和类的层次结构。Doxygen是可应用在各种编程语言的文档系统，它可以以报告或HTML网站的形式直接生成文档。大多数人将具有特殊格式的注释添加到他们的类和方法中，然后使用Doxygen生成包含这些特殊注释的文档。但是，Doxygen也可以用于不包含任何注释的代码，并将基于系统的组织（文件，命名空间，类和方法）生成接口。
>
> ‎您可以在官方网站[‎[‎8]‎](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_8)‎上获得Doxygen。在您的计算机上安装Doxygen后，只需打开一个shell并转到包含要分析的所有源代码的目录即可。然后键入以下命令以创建默认配置文件：
>
> ```
> $ doxygen -g
> ```
>
> ‎这将创建一个名为“Doxygen”的文件。打开此文件，并确保以下选项（EXTRACT_ALL, EXTRACT_PRIVATE, RECURSIVE, HAVE_DOT, CALL_GRAPH, CALLER_GRAPH）都设置为“YES”：。这些选项将确保从代码中提取所有实体，即使在子目录中也是如此，并且生成调用图。所有可用选项的完整描述可以在Doxygen ‎[‎[9]‎](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_9)‎的在线文档中找到。要使用所选选项生成文档，只需键入：
>
> ```
> $ doxygen Doxygen
> ```
>
> ‎文档将在“html”目录中生成，您可以通过在任何Web浏览器中打开“index.html”文件来访问它。您可以浏览代码，查看类之间的继承关系，并且由于图形，您还可以看到它正在调用的其他方法的每个方法。‎

### 3.2 Overall architecture

Figure 3.1 and 3.2 represent the architecture of Kyoto Cabinet v1.2.76 and LevelDB 1.7.0, respectively. Classes are represented with the UML class diagram convention. Components are represented with corner-rounded rectangles, and black arrows represent the use of an entity by another. A black arrow from A to B means that A is using or accessing elements of B.

These diagrams try to represent the functional architecture as much as the structural architecture. For instance in Figure 3.1, many components are represented inside the HashDB class, because in the code those components are defined as part of the HashDB class.

In terms of internal component organization, there is no doubt that LevelDB is the big winner. The reason for this is that in Kyoto Cabinet the components for Iteration, Parametrization, Memory Management and Error Management are all defined as parts of the Core/Interface component, as shown in Figure 3.1. This creates a strong coupling of those components with the Core, and limits the modularity and future extensibility of the system. On the contrary, LevelDB is built in a very modular way, with only the Memory Management being part of the Core component.

> 图 3.1 和 3.2 分别表示 Kyoto Cabinet v1.2.76 和 LevelDB 1.7.0 的体系结构。类使用 UML 类图约定表示。组件用圆角矩形表示，黑色箭头表示另一个实体对一个实体的使用。从 A 到 B 的黑色箭头表示 A 正在使用或访问 B 的元素。‎
>
> 这些图试图像表示结构架构一样多的功能架构。例如，在图 3.1 中，许多组件在 HashDB 类中表示，因为在代码中，这些组件被定义为 HashDB 类的一部分。‎
>
> 在内部组件组织方面，毫无疑问，LevelDB是最大的赢家。其原因是，在Kyoto Cabinet中，用于迭代、参数化、内存管理和错误管理的组件都被定义为内核/接口组件的一部分，如图3.1所示。这创建了这些组件与核心的强耦合，并限制了系统的模块化和未来的可扩展性。相反，LevelDB是以一种非常模块化的方式构建的，只有内存管理是核心组件的一部分。

[![](https://i0.wp.com/codecapsule.com/wp-content/uploads/2012/12/kvstore_kyotocabinet.jpg?resize=770%2C930 "kvstore_kyotocabinet")](https://i0.wp.com/codecapsule.com/wp-content/uploads/2012/12/kvstore_kyotocabinet.jpg)

Figure 3.1

[![](https://i0.wp.com/codecapsule.com/wp-content/uploads/2012/12/kvstore_leveldb.jpg?resize=770%2C900 "kvstore_leveldb")](https://i0.wp.com/codecapsule.com/wp-content/uploads/2012/12/kvstore_leveldb.jpg)

Figure 3.2

### 3.3 Interface

The interface of the HashDB class of Kyoto Cabinet is exposing more than 50 methods, versus only 15 for the DBImpl class of LevelDB (and four of these 15 are for test purposes). This is a direct consequence of the strong coupling taking place in the Core/Interface component for Kyoto Cabinet, such as the definition of the Parametrization module inside the Core.

API design will be discussed in more details in a future article of the IKVS series.

> ‎Kyoto Cabinet 的 HashDB 类的接口公开了 50 多种方法，而 LevelDB 的 DBImpl 类只有 15 种方法（这 15 个中有 4 个用于测试目的）。这是Kyoto Cabinet内核/接口组件中发生强耦合的直接结果，例如内部参数化模块的定义。
>
> ‎API 设计将在 IKVS 系列后续文章中更详细地讨论。

### 3.4 Parametrization

In Kyoto Cabinet, the parameters are tuned by calling methods of the HashDB class. There are 15 methods like that, all with the prefix “tune_”.

In LevelDB, the parameters are defined into specialized objects. “Options” for the general parameters, and “ReadOptions” and “WriteOptions” for parameters of the Get() and Put() methods respectively, as represented in Figure 3.2. This decoupling enables a better extensibility of the options, without messing with the public interface of the Core like this is the case with Kyoto Cabinet.

> ‎在Kyoto Cabinet中，参数通过调用 HashDB 类的方法进行调整。有15种类似的方法，所有方法都带有前缀“tune_”。‎
>
> ‎在 LevelDB 中，参数被定义为专用对象。“Options”分别用于常规参数，“ReadOptions”和“WriteOptions”分别用于 Get（） 和 Put（） 方法的参数，如图 3.2 所示。这种解耦使选项具有更好的可扩展性，而不会像Kyoto Cabinet那样弄乱Core的公共接口。‎

### 3.5 String

In key-value stores, there is a lot of string processing going on. Strings are being iterated, hashed, compressed, passed, and returned. Therefore, a clever implementation of String is very important, as tiny savings in objects used on a large scale can have a dramatic impact globally.

LevelDB is using a specialized class called “Slice” [[10]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_10). A Slice holds a byte array along with the size that array. This allows to know the size of the string in time O(1), ~unlike std::string which would take O(n)~ unlike strlen() on C strings which would take O(n). Note that in C++, size() for std::string is also O(1). Having the size stored separately also allows for the ‘\0’ character to be stored, which means the keys and values can be real byte array and not just null-terminated strings. Finally and more importantly, the Slice class handles the copy by making a shallow copy, not a deep copy. Meaning, it simply copies the pointer to the byte array, and doesn’t make a full copy of the byte array like std::string. This avoids copying potentially very large keys and values.

Like LevelDB, Redis is using its own data structure to represent strings. The goal expressed is also to avoid an O(n) operation to retrieve the size of the string [[11]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_11).

Kyoto Cabinet is using std::string for its strings.

My opinion is that an implementation of String, adapted to the requirements of the key-value stores, is absolutely necessary. Why spend time copying strings and allocating memory if it can be avoided?

> ‎在键值存储中，进行了大量的字符串处理。字符串被迭代、哈希、压缩、传递和返回。因此，巧妙地实现 String 非常重要，因为大规模使用的对象的微小节省可以在全球范围内产生巨大影响。
>
> ‎LevelDB正在使用一个名为“Slice”‎[‎[10]的‎](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_10)‎专用类。Slice 包含一个字节数组以及该数组的大小。这允许知道字符串在时间O（1）中的大小，‎~‎不像std：：string会取O（n），‎~‎不像C字符串上的strlen（）会取O（n）。请注意，在 C++ 中，std：：string 的 size（） 也是 O（1）。单独存储大小还允许存储“\0”字符，这意味着键和值可以是实字节数组，而不仅仅是空终止的字符串。最后，更重要的是，Slice 类通过浅拷贝（而不是深拷贝）来处理副本。这意味着，它只是将指针复制到字节数组，而不会像std：：string那样制作字节数组的完整副本。这样可以避免复制可能非常大的键和值。
>
> ‎与LevelDB一样，Redis使用自己的数据结构来表示字符串。表达的目标也是为了避免O（n）操作来检索字符串‎[‎的大小[11]‎](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_11)‎。
>
> Kyoto Cabinet使用std：：string作为其字符串。‎
>
> ‎我的观点是，适应键值存储要求的 String 实现是绝对必要的。如果可以避免，为什么要花时间复制字符串和分配内存？

### 3.6 Error Management

In all the C++ source code that I have been looking at for key-value stores, I have not seen a single use of exceptions being used as the global error management system. In Kyoto Cabinet, the threading component in the kcthread.cc file is using the exceptions, but I think that this choice is more related to the handling of threads than a general architectural choice. Exceptions are dangerous, and should be avoided whenever possible.

BerkeleyDB has a nice C-style way to handle errors. Error message and error codes are all centralized in one file. All functions that return error codes have a integer local variable named “ret”, which is filled while processing and returned at the end. This approach is rolled out in all files, and in all modules: very polished, normalized error management. In some functions, a few forward jumping gotos are used, a technique widely used in serious C-based system such as the Linux kernel [[12]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_12). Even though this error management approach is very clear and clean, a C-style error management would not make much sense in a C++ application.

In Kyoto Cabinet, one Error object is stored in every database object such as HashDB. In the database classes, methods are calling set_error() to set the Error object in case an error occurs, and return true or false in very a C-style way. No local variable returned at the very end of the methods like in BerkeleyDB, return statements are placed wherever the errors occur.

LevelDB is not using exceptions at all, but a special class called Status. This class holds both an error value and an error message. This object is returned by all methods so that the error status can be either treated on the spot or passed to other methods higher up in the calling stack. This Status class is also implemented in a very clever way, as the error code is stored inside the string itself. My understanding of this choice is that most of the time, the methods will return a Status of “OK”, to say that no error was encountered. In that case, the message string is NULL, and the occurrence of the Status object is very light. Had the authors of LevelDB chosen to have one additional attribute to store the error code, this error code would have had to be filled even in the case of a Status of “OK”, which would have meant more space used on every method call. All components are using this Status class, and there is no need to go through a centralized method as with Kyoto Cabinet, as shown in Figure 3.1 and 3.2.

Of all the error management solutions presented above, I personally prefer the solution used in LevelDB. This solution avoids the use of exceptions, it is not a simple C-style error management which is too limited in my opinion, and it prevents any unnecessary coupling with the Core component like it is the case with Kyoto Cabinet.

> ‎在我一直在查看键值存储的所有C++源代码中，我没有看到异常被用作全局错误管理系统。在Kyoto Cabinet中，kcthread.cc 文件中的线程组件使用了exceptions，但我认为这种选择更多地与线程的处理有关，而不是一般的体系结构选择。使用exception是危险的，应尽可能避免。‎
>
> BerkeleyDB有一个很好的C风格方法来处理错误。错误消息和错误代码都集中在一个文件中。所有返回错误代码的函数都有一个名为“ret”的整数局部变量，该变量在处理时填充并在末尾返回。这种方法在所有文件和所有模块中被使用：是非常完善，规范化的错误管理。在某些函数中，使用了一些向前跳转的goto，这种技术广泛用于基于C的严肃系统，如Linux内核‎[‎[12]‎](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_12)‎。尽管这种错误管理方法非常清晰干净，但C风格的错误管理在C++应用程序中没有多大意义。
>
> 在Kyoto Cabinet中，每个数据库对象（如HashDB）中都存储了一个Error对象。在数据库类中，调用 set_error（）方法来设置 Error 对象，以防发生错误，并以C 风格的方式返回 true 或 false。在BerkeleyDB中，在方法的最末尾没有返回局部变量，返回语句放置在错误发生的位置。‎
>
> LevelDB 根本没有使用异常，而是使用一个名为 Status 的特殊类。此类同时包含错误值和错误消息。所有方法都返回此对象，以便可以当场处理错误状态，也可以将错误状态传递给调用堆栈中更高的其他方法。此 Status 类也以非常聪明的方式实现，因为错误代码存储在字符串本身内。我对这个选择的理解是，大多数时候，这些方法会返回“OK”的状态，以表示没有遇到错误。在这种情况下，消息字符串为 NULL，并且 Status 对象的出现非常轻。如果 LevelDB 的作者选择有一个额外的属性来存储错误代码，那么即使在状态为“OK”的情况下，也必须填写此错误代码，这意味着在每次方法调用上使用更多空间。所有组件都使用此 Status 类，无需像 Kyoto Cabinet 那样采用集中式方法，如图 3.1 和 3.2 所示。
>
> ‎在上面介绍的所有错误管理解决方案中，我个人更喜欢LevelDB中使用的解决方案。这个解决方案避免了异常的使用，在我看来，它不是一个简单的C型错误管理，它太有限了，它防止了与核心组件的任何不必要的耦合，就像Kyoto Cabinet的情况一样。

### 3.7 Memory Management

Both Kyoto Cabinet and LevelDB have the memory management defined inside the Core component. For Kyoto Cabinet, the memory management consists of keeping track of the block of contiguous free memory in the database file on disk, and selecting a block of adequate size whenever an item is being stored. The file itself is just memory mapped with the mmap() function. Note that MongoDB too is using a memory mapped file [[13]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_13).

For LevelDB, which implements a Log-Structured Merge Tree, there are no gaps of free space in the file as it is the case with hash tables stored on disk. The memory management consists in compacting the log files whenever they exceed a certain size [[14]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_14).

Other key-value stores, such as Redis, use memory allocation with malloc() — in the case of Redis, the memory allocation algorithm is not the one provided by the operating system like dlmalloc or ptmalloc3, but jemalloc [[15]](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_15).

Memory management will be described in details in a later article of the IKVS series.

> ‎Kyoto Cabinet 和 LevelDB 都在 Core 组件中定义了内存管理。对于‎Kyoto Cabinet，内存管理包括跟踪磁盘上数据库文件中的连续可用内存块，并在存储项目时选择足够大小的块。文件本身只是用 mmap（） 函数映射的内存。请注意，MongoDB也使用内存映射文件‎[‎[13]‎](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_13)‎。‎
>
> ‎对于实现日志结构化合并树的 LevelDB，文件中没有可用空间的间隙，就像存储在磁盘上的哈希表一样。内存管理包括在日志文件超过特定大小 ‎[‎[14]‎](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_14)‎ 时压缩它们。‎
>
> ‎其他键值存储，如 Redis，使用带有 malloc（） 的内存分配 — 在 Redis 的情况下，内存分配算法不是像 dlmalloc 或 ptmalloc3 这样的操作系统提供的算法，而是 jemalloc ‎[‎[15]‎](https://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/#ref_15)‎。‎
>
> 内存管理将在 IKVS 系列的后续文章中详细介绍。‎

### 3.8 Data Storage

Kyoto Cabinet, LevelDB, BerkeleyDB, MongoDB and Redis are using the file system to store the data. Memcached, on the contrary, is storing the data in memory (RAM).

Data storage will be described in details in a later article of the IKVS series.

> ‎Kyoto Cabinet，LevelDB，BerkeleyDB，MongoDB和Redis正在使用文件系统来存储数据。相反，Memcached将数据存储在内存（RAM）中。
>
> ‎数据存储将在 IKVS 系列的后续文章中详细介绍。‎

## 4. Code review

This section is a quick code review of Kyoto Cabinet and LevelDB. It is not thorough, and only contains elements that I judged remarkable when I was reading the source code.

> 本节是对Kyoto Cabinet和LevelDB的快速代码审查。它并不彻底，只包含我在阅读源代码时判断为可讨论的元素。‎

### 4.1 Organization of declarations and definitions

If the code is normally organized in LevelDB, with the declarations in the .h header files and the definitions in the .cc implementation files, I have found something shocking in Kyoto Cabinet. Indeed, for many classes, the .cc files do not contain any definition, and the methods are all being defined directly from the headers. In other files, some methods are defined in the .h and some others in the .cc files. While I understand that there might be a reason behind this choice, I still find that not following such a respected convention in a C++ application is fundamentally wrong. This is wrong because it makes me wonder with it is like that, and it makes me look into two different files for the implementations, after years of C++ and looking into just one type of files, the .cc files.

> ‎如果在LevelDB中，通常在.h头文件中使用声明，在.cc实现文件中使用定义，那么我在Kyoto Cabinet中发现了一些令人震惊的东西。实际上，对于许多类，.cc 文件不包含任何定义，并且所有方法都是直接从标头定义的。在其他文件中，某些方法在 .h 中定义，而其他一些方法在 .cc 文件中定义。虽然我知道这种选择背后可能有原因，但我仍然发现，在C++应用中不遵守这样一个受人尊敬的惯例从根本上是错误的。因为它让我想知道它是这样的，它让我在多年的C++并只研究了一种类型的文件，.cc文件之后，它让我研究了两个不同的实现文件。‎

### 4.2 Naming

First of all, the code of Kyoto Cabinet is a significant improvement compared to the code of Tokyo Cabinet. The overall architecture and naming conventions have been greatly improved. Nevertheless, I still find many of the names in Kyoto Cabinet to be very cryptic, with attribute and method names such as `embcomp, trhard, fmtver(), fpow()`. It feels like some C code got lost into some C++ code. On the other hand, the naming in LevelDB is very clear, except maybe for some temporary variables with names such as `mem, imm, and in`, but that’s very minimal and the code remains extremely readable.

> 首先，Kyoto Cabinet的代码与Tokyo Cabinet的代码相比有显著的改进。整体体系结构和命名约定已得到极大改进。尽管如此，我仍然发现Kyoto Cabinet中的许多名称都非常隐晦，如这些属性和方法名称(`embcomp, trhard, fmtver(), fpow()`)感觉就像一些C代码被用到一些C++代码中。另一方面，LevelDB 中的命名非常清晰，除了一些名称为 的临时变量，例如 （`mem, imm, in`），但这是非常少的，代码仍然非常可读。

### 4.3 Code duplication

I have seen quite a bit of code duplication in Kyoto Cabinet. The code that is used to defragment a file is repeated at least three times, and all the methods that require a branching between Unix and Windows versions all show a great deal of duplication. I have not found any significant piece of duplicated code in LevelDB. I am sure there must be some too, but I would have to dig a lot deeper to find it, proof that it is a lesser problem in LevelDB than it is in Kyoto Cabinet.

> ‎我在Kyoto Cabinet中看到了相当多的重复代码。用于对文件进行碎片整理的代码至少重复三次，并且所有需要在Unix和Windows版本之间进行选择的方法都显示出大量的重复。我没有在LevelDB中找到任何重要的重复代码。我相信一定也有一些，但我必须更深入地挖掘才能找到它，证明它在LevelDB中的问题比在Kyoto Cabinet中的问题要小。

## References

[]()[1] [http://www.aosabook.org/en/bdb.html](http://www.aosabook.org/en/bdb.html)
[]()[2] [http://work.tinou.com/2011/04/memcached-for-dummies.html](http://work.tinou.com/2011/04/memcached-for-dummies.html)
[]()[3] [http://code.google.com/p/memcached/wiki/NewUserInternals](http://code.google.com/p/memcached/wiki/NewUserInternals)
[]()[4] [http://horicky.blogspot.com/2012/04/mongodb-architecture.html](http://horicky.blogspot.com/2012/04/mongodb-architecture.html)
[]()[5] [http://horicky.blogspot.com/2012/07/couchbase-architecture.html](http://horicky.blogspot.com/2012/07/couchbase-architecture.html)
[]()[6] [http://www.sqlite.org/arch.html](http://www.sqlite.org/arch.html)
[]()[7] [http://redis.io/documentation](http://redis.io/documentation)
[]()[8] [http://doxygen.org](http://doxygen.org/)
[]()[9] [http://www.stack.nl/~dimitri/doxygen/config.html](http://www.stack.nl/~dimitri/doxygen/config.html)
[]()[10] [http://leveldb.googlecode.com/svn/trunk/doc/index.html](http://leveldb.googlecode.com/svn/trunk/doc/index.html)
[]()[11] [http://redis.io/topics/internals-sds](http://redis.io/topics/internals-sds)
[]()[12] [http://news.ycombinator.com/item?id=3883310](http://news.ycombinator.com/item?id=3883310)
[]()[13] [http://www.briancarpio.com/2012/05/03/mongodb-memory-management/](http://www.briancarpio.com/2012/05/03/mongodb-memory-management/)
[]()[14] [http://leveldb.googlecode.com/svn/trunk/doc/impl.html](http://leveldb.googlecode.com/svn/trunk/doc/impl.html)
[]()[15] [http://oldblog.antirez.com/post/everything-about-redis-24.html](http://oldblog.antirez.com/post/everything-about-redis-24.html)
