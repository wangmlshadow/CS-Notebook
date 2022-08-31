# Implementing a Key-Value Store – Part 4: API Design

This is Part 4 of the IKVS series, “Implementing a Key-Value Store”. You can also check the [Table of Contents](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/) for other parts.

I finally settled on a name for this whole key-value store project, which from now on will be referred as ~FelixDB~  **[KingDB](http://kingdb.org/)** .

In this article, I will take a look at the APIs of four key-value stores and database systems: LevelDB, Kyoto Cabinet, BerkekeyDB and SQLite3. For each major functionality in their APIs, I will compare the naming conventions and method prototypes, to balance the pros and cons and design the API for the key-value store I am currently developing, KingDB. This article will cover:

1. General principles for API design
2. Defining the functionalities for the public API of KingDB
3. Comparing the APIs of existing databases
   3.1 Opening and closing a database
   3.2 Reads and Writes
   3.3 Iteration
   3.4 Parametrization
   3.5 Error management
4. Conclusion
5. References

> ‎这是 IKVS 系列的第 4 部分“实现键值存储”。您还可以在‎[‎目录中‎](http://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)‎查看其他部分。‎
>
> 我最终为整个键值存储项目确定了一个名称，从现在开始，它将被称为‎~‎FelixDB‎~‎ ‎[‎KingDB‎](http://kingdb.org/)‎。‎
>
> ‎在本文中，我将介绍四个键值存储和数据库系统的API：LevelDB，Kyoto Cabinet，BerkekeyDB和SQLite3。对于其API中的每个主要功能，我将比较命名约定和方法原型，以平衡优缺点，并为我目前正在开发的键值存储KingDB设计API。本文将介绍：
>
> 1. API设计的‎‎一般原则
> 2. 定义 KingDB‎的公共 API 的功能。
> 3. 比较现有数据库‎‎的 API
>    1. 打开和关闭数据库‎
>    2. 读取和写入‎
>    3. 迭代‎
>    4. 参数化
>    5. 错误管理‎
> 4. .结论‎
> 5. 引用

## 1. General principles for API design

Designing a good API is hard. Really hard. But I am not saying anything new here, just repeating what has been told by many before me. The best resource I have found so far about API Design is Joshua Bloch’s talk “How to Design a Good API & Why it Matters” [[1]](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_1), along with its bumper-sticker version [[2]](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_2). If you haven’t had a chance to watch this talk yet, I strongly advise you to take the time to watch it. During the talk, Bloch clearly states the two most important things the audience should remember. I’ve copied those points from the bumper-sticker version and added some comments:

1. **When in doubt, leave it out.** When not sure if a functionality, class, method or parameter should be included in the API, do not include it.
2. **Don’t make the client do anything the library could do.** If your API makes the client execute a series of function calls and plug the outputs of each function into the input of the next one, then just add a function to your API that does this series of function calls.

Other good resources for API design are Chapter 4 “Designs and Declarations” from “ *Effective C++* ” by Scott Meyers [[3]](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_3), and “ *Effective Java* ” by Joshua Bloch [[4]](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_4).

Those resources are very important at this stage of the key-value store project, though I think it is important to consider an important factor that is not covered by those resources: user expectations. API design is hard when is it done for a library being built from scratch, but in the case of a key-value store, there is a history. Users have been playing with the APIs of other key-value stores and database systems. Thus, when facing the API of a new key-value store, they will expect to be on familiar ground, and not respecting this untold convention will only increase the learning curve for this new API and make them angry.

For this reason, even though I keep in mind all the good advice presented in the resources I listed above, I will also consider that I have to copy as much as possible the APIs of already existing libraries, because it will make it simpler for users to take advantage of the API that I am creating.

> 设计一个好的API是很困难的。真的很难。但我在这里并没有说任何新的东西，只是重复在我之前的许多人所说的话。到目前为止，我找到的关于API设计的最好的资源是Joshua Bloch的演讲“如何设计一个好的API和为什么它很重要”‎[‎[1]‎](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_1)‎，以及它的文字版‎[‎[2]‎](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_2)‎。如果你还没有机会看这个演讲，我强烈建议你花时间看一下。在演讲中，布洛赫清楚地陈述了观众应该记住的两件最重要的事情。我从文字版中复制了这些要点，并添加了一些评论：‎
>
> ‎1. ‎**‎如有疑问，请将其排除在外。‎**‎如果不确定 API 中是否应包含功能、类、方法或参数，请不要将其包括在内。‎
>
> ‎2‎**‎. 不要让客户端做任何库可以做的事情。‎**‎如果您的 API 使客户端执行一系列函数调用，并将每个函数的输出插入到下一个函数的输入中，则只需向 API 添加一个执行这一系列函数调用的函数即可。‎
>
> ‎API设计相关的其他好的学习资源是Scott Meyers的“‎*‎Effective C++‎*‎”‎[‎[3]‎](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_3)‎的第4章“Designs and Declarations”，以及Joshua Bloch的“‎*‎Effective Java‎*‎”‎[‎[4]‎](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_4)‎。‎
>
> 这些资源在键值存储项目的这个阶段非常重要，尽管我认为重要的是要考虑这些资源未涵盖的重要因素：用户期望。对于从头开始构建的库，API设计很难，但是在键值存储的情况下，有一个历史记录。用户一直在玩其他键值存储和数据库系统的API。因此，当面对新的键值存储的API时，他们会期望在熟悉的基础上，不尊重这个不为人知的惯例只会增加这个新API的学习曲线，并让用户难以接受。
>
> ‎出于这个原因，即使我牢记上面列出的资源中提供的所有好建议，我也会考虑我必须尽可能多地复制现有库的API，因为它将使用户更容易利用我正在创建的API。‎

## 2. Defining the functionalities for the public API of KingDB

Given that as a first step I want to implement a minimal but trustable key-value store, I will for sure not include all of the most advanced functionalities offered by more mature projects such as Kyoto Cabinet and LevelDB. I want to get the basic functionalities right, and then I will add others incrementally. For me, the basic functionalities are strictly limited to:

* Opening and closing a database
* Reading and writing data to a database
* Iterating over the full collection of keys and values in a database
* Offer a way to tune parameters
* Offer a decent error notification interface

I am aware that these functionalities are too limited for some use cases, but this will be enough for now. I have decided to not include any transaction mechanisms, grouped queries, or atomic operations. Also, for now I will not provide a snapshot feature.

> ‎鉴于作为第一步，我想实现一个最小但可信赖的键值存储，我不会实现现有成熟的项目（如Kyoto Cabinet和LevelDB）提供的所有的功能。我想先实现基本功能，然后我将逐步添加其他功能。对我来说，基本功能严格限于：‎
>
> * ‎打开和关闭数据库‎
> * ‎在数据库中读取和写入数据‎
> * ‎循环访问数据库中键和值的完整集合‎
> * ‎提供一种调整参数的方法‎
> * ‎提供人性化的错误通知界面‎
>
> ‎我知道这些功能对于某些用例来说太有限了，但现在这已经足够了。我决定不包括任何事务机制，分组查询或原子操作。另外，现在我不会提供快照功能。‎

## 3. Comparing the APIs of existing databases

In order to compare the C++ APIs of existing databases, I will compare samples of codes for each functionality. Those code samples have been inspired or directly taken from official sources: “Fundamental Specifications of Kyoto Cabinet” [[5]](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_5), “LevelDB’s Detailed Documentation” [[6]](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_6), “Getting Started with Berkeley DB” [[7]](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_7), and “SQLite in 5 minutes or less” [[8]](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_8). I am also using a color convention so that it’s easier to differentiate code from the various APIs.

### 3.1 Opening and closing a database

Below are code samples showing how to open a database for the systems being studied. For clarity, option tuning and error management are not shown here, and will be explained in more details in their respective sections below.

> 为了比较现有数据库的C++ API，我将比较每个功能的代码示例。这些代码示例来自官方文档：“Kyoto Cabinet的基本规范”‎[‎[5]‎](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_5)‎，“LevelDB的详细文档”‎[‎[6]‎](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_6)‎，“Berkeley DB入门”‎[‎[7]‎](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_7)‎和“在5分钟或更短的时间内了解SQLite”‎[‎[8]‎](https://codecapsule.com/2013/04/03/implementing-a-key-value-store-part-4-api-design/#ref_8)‎。我还使用了颜色约定，以便更轻松地将代码与各种 API 区分开来。‎

<pre class="wp-block-preformatted"><b>/* LevelDB */</b>
leveldb::DB* db;
leveldb::DB::Open(leveldb::Options, "/tmp/testdb", &db);
...
delete db;
</pre>

<pre class="wp-block-preformatted"><b>/* Kyoto Cabinet */</b>
HashDB db;
db.open("dbfile.kch", HashDB::OWRITER | HashDB::OCREATE);
...
db.close()
</pre>

<pre class="wp-block-preformatted"><b>/* SQLite3 */</b>
sqlite3 *db;
sqlite3_open("askyb.db", &db);
...
sqlite3_close(db);
</pre>

<pre class="wp-block-preformatted"><b>/* Berkeley DB */</b>
Db db(NULL, 0);
db.open(NULL, "my_db.db", NULL, DB_BTREE, DB_CREATE, 0);
...
db.close(0);
</pre>

There are two clear patterns emerging for the opening part. On the one hand, the APIs of LevelDB and SQLite3 require to create a pointer (handle) to a database object, and then call an opening function with a pointer to this pointer, to allocate the memory for it and setup the database. On the other hand, the APIs of Kyoto Cabinet and Berkeley DB start by instantiating a database object, and then call the open() method on that object to setup the database.

Now regarding the closing part, LevelDB just requires to delete the pointer, whereas for SQLite3, a closing function has to be called. Kyoto Cabinet and BerkeleyDB have a close() method on the database object itself.

I believe that forcing the use of a pointer to a database object as LevelDB and SQLite3 are doing it, and then passing that pointer to an opening function, is very “C-style”. In addition, I think that the way LevelDB handles the closing — by deleting the pointer — is a design flaw, because it creates  **asymmetry** . In an API, function calls should be symmetric as much as possible, because it is more intuitive and logical. “If I call open(), then I should call close()” is infinitely more logical than “If I call open(), then I should delete the pointer.”

**Design decision**
So my take on this is that for KingDB, I’d like to use something similar to Kyoto Cabinet and Berkeley DB, which is to instantiate a database object, then call the Open() and Close() methods on it. As for naming, I’ll stick to the conventional Open() and Close().

> ‎示例部分出现了两种明显的模式。一方面，LevelDB 和 SQLite3 的 API 需要创建一个指向数据库对象的指针（句柄），然后使用指向该指针的指针调用open函数，为其分配内存并设置数据库。另一方面，Kyoto Cabinet 和 Berkeley DB 的 API 首先实例化数据库对象，然后在该对象上调用 open（） 方法来设置数据库。‎
>
> ‎现在关于close部分，LevelDB只需要删除指针，而对于SQLite3，必须调用close函数。Kyoto Cabinet 和 BerkeleyDB 在数据库对象本身上有一个 close（） 方法。‎
>
> ‎我相信，强制使用指向数据库对象的指针，就像LevelDB和SQLite3所做的那样，然后将该指针传递给打开函数，是非常“C风格”的。此外，我认为LevelDB处理关闭的方式 - 通过删除指针 - 是一个设计缺陷，因为它会产生‎**‎不对称性‎**‎。在 API 中，函数调用应尽可能对称，因为它更直观、更合乎逻辑。“如果我调用open（），那么我应该调用close（）”比“如果我调用open（），那么我应该删除指针”更合乎逻辑。‎
>
> ‎因此，我对此的看法是，对于KingDB，我想使用类似于Kyoto Cabinet 和 BerkeleyDB的方式，即实例化数据库对象，然后在其上调用Open（）和Close（）方法。至于命名，我将坚持传统的Open（）和Close（）。‎

### 3.2 Reads and Writes

In this section, I compare the APIs at the level of their read/write functionalities.、

> 在本节中，我将比较 读/写功能的API。‎

<pre class="wp-block-preformatted"><b>/* LevelDB */</b>
std::string value;
db->Get(leveldb::ReadOptions(), "key1", &value);
db->Put(leveldb::WriteOptions(), "key2", value);
</pre>

<pre class="wp-block-preformatted"><b>/* Kyoto Cabinet */</b>
string value;
db.get("key1", &value);
db.set("key2", "value");
</pre>

<pre class="wp-block-preformatted"><b>/* SQLite3 */</b>
int szErrMsg;
char *query = “INSERT INTO table col1, col2 VALUES (‘value1’, ‘value2’)”;
sqlite3_exec(db, query, NULL, 0, &szErrMsg);
</pre>

<pre class="wp-block-preformatted"><b>/* Berkeley DB */</b>
/* reading */
Dbt key, data;

key.set_data(&money);
key.set_size(sizeof(float));

data.set_data(description);
data.set_ulen(DESCRIPTION_SIZE + 1);
data.set_flags(DB_DBT_USERMEM);

db.get(NULL, &key, &data, 0);


/* writing */
char *description = "Grocery bill.";
float money = 122.45;

Dbt key(&money, sizeof(float));
Dbt data(description, strlen(description) + 1);

db.put(NULL, &key, &data, DB_NOOVERWRITE);

int const DESCRIPTION_SIZE = 199;
float money = 122.45;
char description[DESCRIPTION_SIZE + 1];
</pre>

I am not going to consider SQLite3 here, because it is SQL-based and therefore the reads and writes are being operated through SQL queries, not method calls. Berkeley DB requires the creation of objects of class  *Dbt* , and setting a lot of options on them, so I am not going to consider that either.

This leaves us with LevelDB and Kyoto Cabinet, and they have nice getter/setter symmetrical interfaces. LevelDB has Get() and Put(), and Kyoto Cabinet has get() and set(). The prototypes for the setter methods, Put() and set() are very similar: the key is  *passed by value* , and the value is *passed as a pointer* so it can be updated by the call. The values are not returned by the calls, the returns are for error management here.

**Design decision**
For KingDB, I am going to take the same approach as LevelDB and Kyoto Cabinet, with a similar prototype for the setter method, i.e. passing the key by value, and passing a pointer to the value so it can be filled. Regarding the naming, at first I thought that Get() and Set() would be the best choice here, but after thinking a bit longer I am more inclined to do it as LevelDB, with Get() and Put(). The reason for this is that Get/Set and Get/Put are just as symmetric, only the words “Get” and “Set” are very similar as they differ by only one letter. Therefore when reading code it will be clearer and require less cognitive work to read “Get” and “Put” instead, so I’ll go with Get/Put.

> 我不打算在这里考虑SQLite3，因为它是基于SQL的，因此读和写是通过SQL查询而不是方法调用来操作的。Berkeley DB需要创建类‎*‎Dbt‎*‎的对象，并在它们上设置很多选项，所以我也不会考虑它。‎
>
> 这就只剩下LevelDB和Kyoto Cabinet，它们有很好的getter/setter对称接口。LevelDB有Get（）和Put（），Kyoto Cabinet有get（）和set（）。setter 方法的原型 Put（） 和 set（） 非常相似：键‎*‎按值传递‎*‎，值‎*‎作为指针传递‎*‎，以便可以通过调用进行更新。这些值不是由调用返回的，返回值用于此处的错误管理。‎
>
> ‎对于 KingDB ‎**‎的设计决策‎**
> ‎我将采用与 LevelDB 和 Kyoto Cabinet 相同的方法，使用类似的 setter 方法原型，即按值传递键，并传递指向值的指针，以便可以填充它。关于命名，起初我认为Get（）和Set（）将是这里的最佳选择，但是在考虑了更长的时间之后，我更倾向于类似于LevelDB，使用Get（）和Put（）。这样做的原因是Get/Set和Get/Put是对称的，只有单词“Get”和“Set”非常相似，因为它们只有一个字母不同。因此，在阅读代码时，阅读“Get”和“Put”会更清晰，并且需要更少的认知工作，所以我将使用Get/Put。‎

### 3.3 Iteration

<pre class="wp-block-preformatted"><b>/* LevelDB */</b>
leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
for (it->SeekToFirst(); it->Valid(); it->Next()) {
  cout << it->key().ToString() << ": "  << it->value().ToString() << endl;
}
delete it;
</pre>

<pre class="wp-block-preformatted"><b>/* Kyoto Cabinet */</b>
DB::Cursor* cur = db.cursor();
cur->jump();
string ckey, cvalue;
while (cur->get(&ckey, &cvalue, true)) {
  cout << ckey << ":" << cvalue << endl;
}
delete cur;
</pre>

<pre class="wp-block-preformatted"><b>/* SQLite3 */</b>
static int callback(void *NotUsed, int argc, char **argv, char **szColName) {
  for(int i = 0; i < argc; i++) {
    printf("%s = %s\n", szColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

char *query = “SELECT * FROM table”;
sqlite3_exec(db, query, callback, 0, &szErrMsg);
</pre>

<pre class="wp-block-preformatted"><b>/* Berkeley DB */</b>
Dbc *cursorp;
db.cursor(NULL, &cursorp, 0);
Dbt key, data;
while (cursorp->get(&key, &data, DB_NEXT) == 0) {
  // do things
}
cursorp->close();
</pre>

As in the previous section, SQLite3 will not be considered here because it does not fit the requirements of a key-value store. It is interesting though to see that for a SELECT query sent to a database, a callback function is then being called on any row being retrieved by this query. Most if the APIs for MySQL and PostgreSQL use a loop and a function call that fills local variables, and there is no such use of a callback. I find the callback here to be tricky, because it makes things more complex for users in case they want to perform some aggregate operations or computation on the stream of rows as they are being retrieved. But well, that’s another discussion, now back to our key-value store!

There are two ways of doing things here: using a cursor or using an iterator. Kyoto Cabinet and BerkeleyDB use a cursor, which starts by creating a pointer to a cursor object and instantiating that object, then calling the get() method of the cursor repeatedly in a *while loop* in order to retrieve all the values in the database. LevelDB uses the iterator design pattern, which starts by creating a pointer to an iterator object and instantiating that object (so far just like the cursor), but then uses a *for loop* to iterate over the set of items in the collection. Note that the while vs. for loop is just a convention: cursors could be used with for loops and iterators with while loops. The main difference is that with the cursor, the keys and values are passed as pointers and being filled by the get() method of the cursor, whereas with the iterator, the keys and values are accessed as the return values of methods of the iterator.

**Design decision**
The cursor and its while loop are, again, very “C-style”. I find the iterator approach to be cleaner and more “C++ compliant”, because it is how the collections of the STL are being accessed in C++. Therefore for KingDB, I choose to use an iterator as LevelDB does. As for naming, I will simply copy the method names from LevelDB.

> ‎与上一节一样，这里将不考虑 SQLite3，因为它不符合键值存储的要求。有趣的是，对于发送到数据库的 SELECT 查询，然后在此查询检索的任何行上调用回调函数。大多数情况下，如果MySQL和PostgreSQL的API使用循环和填充局部变量的函数调用，并且没有这种回调的使用。我发现这里的回调很棘手，因为它使用户在检索行流时对行流执行一些聚合操作或计算时变得更加复杂。但是，好吧，这是另一个讨论，现在回到我们的键值存储！‎
>
> 此处有两种操作方法：使用游标或使用迭代器。Kyoto Cabinet 和 BerkeleyDB 使用游标，游标首先创建指向游标对象的指针并实例化该对象，然后在 ‎*‎while 循环‎*‎中重复调用游标的 get（） 方法，以便检索数据库中的所有值。LevelDB 使用迭代器设计模式，该模式首先创建指向迭代器对象的指针并实例化该对象（到目前为止，就像游标一样），然后使用 ‎*‎for 循环‎*‎来循环访问集合中的项集。请注意，while vs. for 循环只是一个约定：游标可以与 for 循环一起使用，而迭代器可以与 while 循环一起使用。主要区别在于，对于游标，键和值作为指针传递并由游标的get（）方法填充，而使用迭代器，键和值作为迭代器方法的返回值进行访问。
>
> **设计决策‎**
> ‎光标及其 while 循环同样非常“C 风格”。我发现迭代器方法更清晰，更“C++兼容”，因为这是C++访问STL集合的方式。因此，对于 KingDB，我选择像 LevelDB 一样使用迭代器。至于命名，我将简单地从LevelDB复制方法名称。‎

### 3.4 Parametrization

Parametrization was presented quickly in Part 3 of the IKVS series, Section 3.4, but I also wanted to cover it here.

> ‎参数化在 IKVS 系列的第 3 部分第 3.4 节中很快呈现，但我也想在这里介绍它。‎

<pre class="wp-block-preformatted"><b>/* LevelDB */</b>
leveldb::DB* db;
leveldb::Options options;
options.create_if_missing = true;
options.compression = leveldb::kNoCompression;
leveldb::DB::Open(options, "/tmp/testdb", &db);
...
leveldb::WriteOptions write_options;
write_options.sync = true;
db->Put(write_options, "key", "value");
</pre>

<pre class="wp-block-preformatted"><b>/* Kyoto Cabinet */</b>
db.tune_options(GrassDB::TCCOMPESS);
db.tune_buckets(500LL * 1000);
db.tune_page(32768);
db.tune_page_cache(1LL << 20);
db.open(...);
</pre>

<pre class="wp-block-preformatted"><b>/* SQLite3 */</b> 
sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
sqlite3_config(SQLITE_CONFIG_MEMSTATUS, 1);
sqlite3_config(SQLITE_CONFIG_LOG, SqliteLogger, NULL);
sqlite3_initialize();
sqlite3_open(...);
</pre>

<pre class="wp-block-preformatted"><b>/* Berkeley DB */</b>
db.set_flags(DB_DUPSORT);
db.set_bt_compare(compare_fct);
db.open(NULL, file_name, NULL, DB_BTREE, DB_CREATE, 0);
</pre>

SQLite3 is modifying global parameters through sqlite3_config(), which then apply on all subsequent connections created. Kyoto Cabinet and Berkeley DB, options are set by calling methods on the database object before the call to open(), which is very similar to what SQlite3 is doing. On top of these methods, more general options are also set through parameters of the open() method (See section 3.1 above). This means that options are split into two sets, those that can be modified by method calls, and those that are set when calling open().

LevelDB is doing it differently. Options are defined in their own classes, all in one place, and parameters are modified through attributes of these classes. Objects of these option classes are then passed as parameters of the various methods, always as the first argument. For instance, the first argument of the open() method of LevelDB’s database object is of type leveldb::Options, and the first arguments of the Get() and Put() methods are leveldb::ReadOptions and leveldb::WriteOptions, respectively. A good point about this design is that options can be easily shared in case multiple databases are created at the same time, although in the case of Kyoto Cabinet and Berkeley DB, it would just as simple to create a method that set specific options, and just call that method to share configurations. The real advantage of having options in specific classes, as LevelDB does, is that the interfaces are more stable, since extending options will only modify the option classes, and not any method of the database object.

Even though I like having such option classes, I have to say that I am not comfortable with the way those options are passed to other methods in LevelDB, always as the first argument. If no options need to be changed, then it leads to code using the default options, such as:

```
db.Put(leveldb::WriteOptions, "key", "value");
```

This may create code bloats, and another possibility would have been to have options as the last argument, and to set a default value for that argument so it could be omitted if no options were to be set. Yet another solution that comes to mind for C++ would be to use method overloading, and have versions of the methods with prototypes that simply omit the option objects. It just seems more logical to me to have the options at the end, since they could be omitted, but I am sure that the authors of LevelDB had very good reason for choosing to have the options are the first parameter.

**Design decision**

For parametrization, I think that keeping the options in their own classes is the cleanest way to go, as it fits the bill for good object-oriented design.

For KingDB, I will have the options in separate classes as LevelDB is doing it, only I will pass those options as the last parameters of methods that need them. I may have a moment of enlightenment later and realize that having the options first is the only true way to do it — or maybe someone will explain it to me — but right now I’ll stick to having them at the end. Finally, names are not very important here, so  *Options* , *ReadOptions* and *WriteOptions* will do just fine.

> ‎SQLite3 通过 sqlite3_config（） 修改全局参数，然后将其应用于后续创建的所有连接。Kyoto Cabinet 和 Berkeley DB，选项是在调用 open（） 之前通过在数据库对象上调用方法来设置的，这与 SQlite3 正在做的事情非常相似。除了这些方法之外，还通过 open（） 方法的参数设置了更通用的选项（参见上面的 3.1 节）。这意味着选项被拆分为两个集合，一个是可以通过方法调用修改的，另一个是调用 open（） 时设置的。
>
> ‎LevelDB通过不同的方式做到这一点。选项在它们自己的类中定义，全部在一个地方，参数作为这些类的属性进行修改。然后，这些选项类的对象将作为各种方法的参数进行传递，始终作为第一个参数传递。例如，LevelDB 数据库对象的 open（） 方法的第一个参数是 leveldb：：Options 类型，Get（） 和 Put（） 方法的第一个参数分别是 leveldb：：ReadOptions 和 leveldb：：WriteOptions。关于此设计的一个好处是，如果同时创建多个数据库，可以轻松共享选项，尽管在Kyoto Cabinet and Berkeley DB的中，创建一个设置特定选项的方法并调用该方法共享配置同样简单。但像LevelDB一样，在特定类中使用选项的真正优点是接口更稳定，因为扩展选项只会修改选项类，而不会修改数据库对象的任何方法。‎
>
> ‎尽管我喜欢这样的选项类，但我不得不说，我对将这些选项传递给LevelDB中其他方法的方式感到不舒服，这些方法总是作为第一个参数。如果没有需要更改的选项，则会导致使用默认选项的代码，例如：‎
>
> 这可能会造成代码膨胀，另一种可能性是将选项作为最后一个参数，并为该参数设置默认值，以便在未设置选项时可以省略它。对于C++来说，另一个解决方案是使用方法重载，并具有原型的方法版本，这些原型只是省略了选项对象。在我看来，将选项放在最后似乎更合乎逻辑，因为它们可以被省略，但我确信LevelDB的作者有很好的理由选择将选项作为第一个参数。
>
> **‎设计决策‎**
>
> ‎对于参数化，我认为将选项保留在自己的类中是最干净的方式，因为它符合良好的面向对象设计的要求。‎
>
> ‎对于 KingDB，我将在单独的类中提供选项，因为 LevelDB 正在这样做，只是我将把这些选项作为需要它们的方法的最后一个参数传递。我可能稍后会有更多地认识，并意识到将选项作为第一个参数是唯一真正的方法。或者也许有人会向我解释 ，但现在我会坚持在最后拥有它们。最后，名称在这里不是很重要，所以‎*‎选项‎*‎，‎*‎ReadOptions‎*‎和‎*‎WriteOptions‎*‎会做得很好。‎

### 3.5 Error management

In Part 3 of the IKVS series, Section 3.6, there was a discussion about error management, which was about how errors are managed in the code that the user does not see. The current section overlaps this topic but differs slightly because it is not exactly about errors inside the library, but about how errors are being reported to the user when they occur, as she uses the public interfaces. Below are some code sample of how errors are notified to the user for the database systems being studied.

> ‎在 IKVS 系列的第 3 部分第 3.6 节中，有一个关于错误管理的讨论，即如何在用户看不到的代码中管理错误。当前部分与本主题重叠，但略有不同，因为它不完全是关于库内的错误，而是关于当用户使用公共接口时，当用户使用公共接口时，如何向用户报告错误。下面是一些代码示例，说明如何向正在研究的数据库系统的用户通知错误。

<pre class="wp-block-preformatted"><b>/* LevelDB */</b>
leveldb::Status s = db->Put(leveldb::WriteOptions(), "key", "value");
if (!s.ok()) {
  cerr << s.ToString() << endl;
}
</pre>

<pre class="wp-block-preformatted"><b>/* Kyoto Cabinet */</b>
if (!db.set("baz", "jump")) {
  cerr << "set error: " << db.error().name() << endl;
}
</pre>

<pre class="wp-block-preformatted"><b>/* SQLite3 */</b>
int rc = sqlite3_exec(db, query, callback, 0, &zErrMsg);
if (rc != SQLITE_OK) {
  fprintf(stderr, "SQL error: %s\n", zErrMsg);
  sqlite3_free(zErrMsg);
}
</pre>

<pre class="wp-block-preformatted"><b>/* Berkeley DB */</b>
int ret = my_database.put(NULL, &key, &data, DB_NOOVERWRITE);
if (ret == DB_KEYEXIST) {
  my_database.err(ret, "Put failed because key %f already exists", money);
}
</pre>

Kyoto Cabinet, Berkeley DB and SQLite3 are all handling errors in the same way, by having their methods return an integer error code. As discussed in Section 3.6 from Part 3 of IKVS, internally Kyoto Cabinet is setting a value in the database object itself, which is why in the code sample above, the error message is taken from  *db.error().name()* .

LevelDB has a special Status class which contains the error type and a message giving more information about the error. Objects of this class are returned by all methods in the LevelDB library, and it makes it really easy to test for errors and pass errors to sub-parts of the system for further examination.

**Design decision**

Returning error codes and avoiding the use of C++ exceptions is the right thing to do, however integers are not enough to carry meaningful information around. Kyoto Cabinet, Berkeley DB and SQLite3 all have their own ways to store messages with more meaning, however this always adds an additional step to get the message, and in the case of Kyoto Cabinet and Berkeley, even creates a strong coupling between error management and the database class. Using a Status class like LevelDB is doing it allows to avoid C++ exceptions, while preventing any coupling to other parts of the architecture.

For KingDB, I will use an error management and notification approach similar to the one used by LevelDB, with the same naming conventions.

> ‎Kyoto Cabinet，Berkeley DB和SQLite3都以相同的方式处理错误，让它们的方法返回整数错误代码。如 IKVS 第 3 部分的第 3.6 节所述，Kyoto Cabinet 在内部设置了一个值，这就是为什么在上面的代码示例中，错误消息取自 ‎*‎db.error（）.name（）‎*‎ 的原因。
>
> ‎LevelDB 有一个特殊的 Status 类，其中包含错误类型和一条消息，提供有关错误的详细信息。此类的对象由 LevelDB 库中的所有方法返回，这使得测试错误并将错误传递给系统的子部分以进行进一步检查变得非常容易。
>
> **设计决策‎**
>
> ‎返回错误代码并避免使用C++异常是正确的做法，但是整数不足以携带有意义的信息。Kyoto Cabinet, Berkeley DB和SQLite3都有自己的方法来存储具有更多含义的消息，但是这总是增加了获取消息的额外步骤，并且在Kyoto Cabinet和Berkeley DB的情况下，甚至在错误管理和数据库类之间建立了强大的耦合。使用像LevelDB这样的Status类可以避免C++异常，同时防止与体系结构的其他部分进行任何耦合。‎
>
> ‎对于 KingDB，我将使用类似于 LevelDB 使用的错误管理和通知方法，具有相同的命名约定。‎

## 4. Conclusion

This API walkthrough has been fun, as it’s always interesting to see how different engineers solved the same problems. This also made me realize how similar the APIs of Kyoto Cabinet and Berkeley DB really were. Mikio Hirabayashi, the author of Kyoto Cabinet, clearly stated that his key-value store was based on Berkeley DB, and this is even clearer now after looking at the API similarities.

LevelDB is extremely well designed, though I did argue about some details I thought could be done differently, like database opening and closing, and method prototypes.

I have taken little bits from every system, and now I feel more confident with the choices I have to make for the design of KingDB’s API.

> ‎这个 API 设计分析很有趣，因为看到不同的工程师如何解决相同的问题总是很有趣的。这也让我意识到Kyoto Cabinet和Berkeley DB的API到底有多相似。Kyoto Cabinet的作者Mikio Hirabayashi明确表示，他的键值存储是基于Berkeley DB的，在查看API的相似性之后，现在这一点更加清楚了。‎
>
> ‎LevelDB的设计非常好，尽管我确实讨论了一些我认为可以不同方式完成的细节，比如数据库打开和关闭，以及方法原型。‎
>
> ‎我从每个系统中都学到了一些东西，现在我对KingDB的API设计所必须做出的选择更有信心了。‎

## 5. References

[]()[1] [http://www.infoq.com/presentations/effective-api-design](http://www.infoq.com/presentations/effective-api-design)
[]()[2] [http://www.infoq.com/articles/API-Design-Joshua-Bloch](http://www.infoq.com/articles/API-Design-Joshua-Bloch)
[]()[3] [http://www.amazon.com/Effective-Specific-Improve-Programs-Designs/dp/0321334876](http://www.amazon.com/Effective-Specific-Improve-Programs-Designs/dp/0321334876)
[]()[4] [http://www.amazon.com/Effective-Java-Edition-Joshua-Bloch/dp/0321356683](http://www.amazon.com/Effective-Java-Edition-Joshua-Bloch/dp/0321356683)
[]()[5] [http://fallabs.com/kyotocabinet/spex.html](http://fallabs.com/kyotocabinet/spex.html)
[]()[6] [http://leveldb.googlecode.com/svn/trunk/doc/index.html](http://leveldb.googlecode.com/svn/trunk/doc/index.html)
[]()[7] [http://docs.oracle.com/cd/E17076_02/html/gsg/CXX/index.html](http://docs.oracle.com/cd/E17076_02/html/gsg/CXX/index.html)
[]()[8] [http://www.sqlite.org/quickstart.html](http://www.sqlite.org/quickstart.html)
