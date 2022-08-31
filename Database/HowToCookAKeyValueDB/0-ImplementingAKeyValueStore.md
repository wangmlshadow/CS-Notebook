> 这是一个关于如何设计一个key-value存储的系列文章的翻译

# Implementing a Key-Value Store

> Published by [Emmanuel Goossaert](https://codecapsule.com/author/admin/) on [November 7, 2012](https://codecapsule.com/2012/11/)

***UPDATE July 21, 2016** : This article series is still on-going, and the key-value store, KingDB, has already been released: [http://kingdb.org](http://kingdb.org/). Over the coming weeks I will publish the last articles for the IKVS series, which will cover the architecture and data format of KingDB. To get an update when it’s done, you can subscribe to the newsletter from the top-right corner of the blog!*

> ***‎2016 年 7 月 21 日更新‎**‎：本系列文章仍在进行中，键值存储 KingDB 已经发布：‎[‎http://kingdb.org‎](http://kingdb.org/)‎。在接下来的几周里，我将发布IKVS系列的最后几篇文章，这些文章将涵盖KingDB的架构和数据格式。要在完成后获得更新，您可以从博客的右上角订阅新闻通讯！‎*

This post is the main article for the series “Implementing a Key-Value Store” (IKVS) that I am starting today. It aims at summing up all the articles of the series in a Table of Contents, and might later hold some general notes on the project.

Its content will change over time until the series is completed. In particular, in the Table of Contents, the titles of the parts that have not been written yet and their ordering might change. Some parts might also be removed and some others added as the writing advances.

More information on the project can be found in Section 1.3 of “ *Part 1: What are key-value stores, and why implement one?* “

**Enjoy, and if you have any questions, post a comment!**

> ‎这篇文章是我今天开始的“实现键值存储”（IKVS）系列的主要文章。它旨在将该系列的所有文章总结在目录中，并可能在以后包含有关该项目的一些一般注释。‎
>
> ‎其内容将随着时间的推移而变化，直到该系列完成。特别是，在目录中，尚未编写的部分的标题及其顺序可能会更改。随着写作的推进，一些部分也可能被删除，而另一些部分则被添加。
>
> ‎有关该项目的更多信息，请参见“‎*‎第 1 部分：什么是键值存储，以及为什么要实现一个键值存储‎*‎？”的第 1.3 节‎

## Table of Contents

[1 – What are key-value stores, and why implement one?](http://codecapsule.com/2012/11/07/ikvs-part-1-what-are-key-value-stores-and-why-implement-one/)

    1.1 – A quick overview of key-value stores
    1.2 – Key-value stores versus relational databases
    1.3 – Why implement a key-value store
    1.4 – The plan

[2 – Using existing key-value stores as models](http://codecapsule.com/2012/12/03/implementing-a-key-value-store-part-2-using-existing-key-value-stores-as-models/)

    2.1 – Not reinventing the wheel
    2.2 – Model candidates and selection criteria
    2.3 – Overview of the selected key-value stores

[3 – Comparative Analysis of the Architectures of Kyoto Cabinet and LevelDB](http://codecapsule.com/2012/12/30/implementing-a-key-value-store-part-3-comparative-analysis-of-the-architectures-of-kyoto-cabinet-and-leveldb/)

    3.1 – Intent and methodology of this architecture analysis
    3.2 – Overview of the Components of a Key-Value Store
    3.3 – Structural and conceptual analysis of Kyoto Cabinet and LevelDB
    3.4 – Code review

[4 – API Design](http://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/)

    4.1 – General principles for API design
    4.2 – Defining the functionalities for the public API of KingDB
    4.3 – Comparing the APIs of existing databases

[5 – Hash table implementations](http://codecapsule.com/2013/05/13/implementing-a-key-value-store-part-5-hash-table-implementations/ "Implementing a Key-Value Store – Part 5: Hash table implementations")

    5.1 – Hash tables
    5.2 – Implementations

[6 – Open-Addressing Hash Tables](http://codecapsule.com/2014/05/07/implementing-a-key-value-store-part-6-open-addressing-hash-tables/ "Implementing a Key-Value Store – Part 6: Open-Addressing Hash Tables")

    6.1 – Open-addressing hash tables
    6.2 – Metrics
    6.3 – Experimental Protocol
    6.4 – Results and Discussion

[7 – Optimizing Data Structures for SSDs](http://codecapsule.com/2014/10/18/implementing-a-key-value-store-part-7-optimizing-data-structures-for-ssds/ "Implementing a Key-Value Store – Part 7: Open-Addressing Hash Tables")

    7.1 – Fast data structures on SSDs
    7.2 – File I/O optimizations
    7.3 – Done is better than perfect

[8 – Architecture of KingDB](http://codecapsule.com/2015/05/25/implementing-a-key-value-store-part-8-architecture-of-kingdb/ "Implementing a Key-Value Store – Part 8: Architecture of KingDB")

[9 – Data Format and Memory Management in KingDB](http://codecapsule.com/2015/08/03/implementing-a-key-value-store-part-9-data-format-and-memory-management-in-kingdb/ "Implementing a Key-Value Store – Part 9: Data Format and Memory Management in KingDB")

[10 – High-Performance Networking: KingServer vs. Nginx](http://codecapsule.com/2016/07/21/implementing-a-key-value-store-part-10-high-performance-networking-kingserver-vs-nginx/ "Implementing a Key-Value Store – Part 10: High-Performance Networking: KingServer vs. Nginx")

Coming next, the final article:

11 – Mistakes and learnings
