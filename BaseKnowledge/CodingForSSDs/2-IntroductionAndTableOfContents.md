# Coding for SSDs – Part 1: Introduction and Table of Contents

## Introduction

I want to make solid-state drives (SSDs) the optimal storage solution for [my key-value store project](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/). For that reason, I had to make sure I fully understood how SSDs work, so that I can optimize my hash table implementation to suit their internal characteristics. There is a lot of incomplete and contradictory information out there, and finding trustworthy information about SSDs was not an easy task. I had to do some substantial reading to find the proper publications and benchmarks in order to convince myself that, if I had to be coding for SSDs, I would know what I was doing.

Then I figured that since I had done all the research, it would be useful to share the conclusions I had reached. My intent was to transform all the information already available into practical knowledge. I ended up writing a 30-page article, not very suitable for the format of a blog. I have therefore decided to split what I had written into logical parts that can be digested independently. The full Table of Contents is available at the bottom of this article.

The most remarkable contribution is [Part 6](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/), a summary of the whole “ *Coding for SSDs* ” article series, that I am sure programmers who are in a rush will appreciate. This summary covers the basics of SSDs along with all of the recommended access patterns on how reads and writes should be implemented to get the best performance out of solid-state drives.

Another important detail is that “ *Coding for SSDs* ” is independent from my key-value store project ([IKVS series](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)), and therefore, no prior knowledge of the IKVS articles is needed. I am planning on writing an article for the IKVS series, on how hash table can be implemented to take advantage of the internal characteristics of SSDs, though I have no precise date for that yet.

My only regret is not to have produced any code of my own to prove that the access patterns I recommend are actually the best. However even with such code, I would have needed to perform benchmarks over a large array of different models of solid-state drives to confirm my results, which would have required more time and money than I can afford. I have cited my sources meticulously, and if you think that something is not correct in my recommendations, please leave a comment to shed light on that. And of course, feel free to drop a comment as well if you have questions or would like to contribute in any way.

Finally, remember to subscribe to the newsletter to receive a notification email every time a new article is posted on Code Capsule. The subscription panel is available at the top right corner of the blog.

> ‎我想使固态硬盘 （SSD） 成为‎[‎我的键值存储项目‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎的最佳存储解决方案。出于这个原因，我必须确保我完全了解SSD的工作原理，以便我可以优化我的哈希表实现以适应它们的内部特征。那里有很多不完整和矛盾的信息，找到有关SSD的可靠信息并非易事。我必须做一些实质性的阅读来找到合适的出版物和基准，以便说服自己，如果我必须为SSD编码，我会知道我在做什么。‎
>
> ‎然后我想，既然我已经完成了所有的研究，那么分享我得出的结论将是有用的。我的目的是将所有已经可用的信息转化为实践知识。我最终写了一篇30页的文章，不太适合博客的格式。因此，我决定将我所写的内容分成可以独立消化的逻辑部分。完整的目录位于本文底部。
>
> ‎最显着的贡献是第‎[‎6部分‎](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/)‎，这是整个“‎*‎SSD编码‎*‎”系列文章的摘要，我相信赶时间的程序员会喜欢。本摘要介绍了 SSD 的基础知识，以及有关如何实现读取和写入以获得固态硬盘最佳性能的所有推荐访问模式。
>
> ‎另一个重要的细节是“SSD‎*‎编码‎*‎”独立于我的键值存储项目（‎[‎IKVS系列‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎），因此不需要IKVS文章的先验知识。我计划为IKVS系列写一篇文章，关于如何实现哈希表以利用SSD的内部特征，尽管我还没有确切的日期。‎
>
> ‎我唯一的遗憾是没有生成我自己的任何代码来证明我推荐的访问模式实际上是最好的。然而，即使使用这样的代码，我也需要在大量不同型号的固态硬盘上执行基准测试来确认我的结果，这将需要更多的时间和金钱。我已经一丝不苟地引用了我的消息来源，如果你认为我的建议中有什么地方不正确，请留下评论来阐明这一点。当然，如果您有任何疑问或想以任何方式做出贡献，请随时发表评论。‎
>
> ‎最后，请记住订阅时事通讯，以便在每次在Code Capsule上发布新文章时收到通知电子邮件。订阅面板位于博客的右上角。‎

## Table of Content

[Part 1: Introduction and Table of Contents](http://codecapsule.com/2014/02/12/coding-for-ssds-part-1-introduction-and-table-of-contents/)
[Part 2: Architecture of an SSD and Benchmarking](http://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/)

    1. Structure of an SSD

    1.1 NAND-flash memory cells
        1.2 Organization of an SSD
        1.3 Manufacturing process

    2. Benchmarking and performance metrics

    2.1 Basic benchmarks
        2.2 Pre-conditioning
        2.3 Workloads and metrics

[Part 3: Pages, Blocks, and the Flash Translation Layer](http://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)

    3. Basic operations

    3.1 Read, write, erase
        3.2 Example of a write
        3.3 Write amplification
        3.4 Wear leveling

    4. Flash Translation Layer (FTL)

    4.1 On the necessity of having an FTL
        4.2 Logical block mapping
        4.3 Notes on the state of the industry
        4.4 Garbage collection

[Part 4: Advanced Functionalities and Internal Parallelism](http://codecapsule.com/2014/02/12/coding-for-ssds-part-4-advanced-functionalities-and-internal-parallelism/)

    5. Advanced functionalities

    5.1 TRIM
        5.2 Over-provisioning
        5.3 Secure Erase
        5.4 Native Command Queueing (NCQ)
        5.5 Power-loss protection

    6. Internal Parallelism in SSDs

    6.1 Limited I/O bus bandwidth
        6.2 Multiple levels of parallelism
        6.3 Clustered blocks

[Part 5: Access Patterns and System Optimizations](http://codecapsule.com/2014/02/12/coding-for-ssds-part-5-access-patterns-and-system-optimizations/)

    7. Access patterns

    7.1 Defining sequential and random I/O operations
        7.2 Writes
        7.3 Reads
        7.4 Concurrent reads and writes

    8. System optimizations

    8.1 Partition alignment
        8.2 Filesystem parameters
        8.3 Operating system I/O scheduler
        8.4 Swap
        8.5 Temporary files

[Part 6: A Summary – What every programmer should know about solid-state drives](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/)

## What’s next

Part 2 is available [here](http://codecapsule.com/2014/02/12/coding-for-ssds-part-2-architecture-of-an-ssd-and-benchmarking/). If you’re in a rush, you can also go directly to [Part 6](http://codecapsule.com/2014/02/12/coding-for-ssds-part-6-a-summary-what-every-programmer-should-know-about-solid-state-drives/), which is summarizing the content from all the other parts.
