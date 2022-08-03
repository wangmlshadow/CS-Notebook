# LevelDB

## 在整体学习完之后要能回答的问题

1. 数据的写入和读取的具体流程是什么
2. 数据库打开和关闭的时候后台做了哪些操作
3. leveldb的数据存储形式是什么，有哪些优点和不足，有哪些改进的方式
4. 数据读写的完整流程是什么
5. 数据在各个组件之间传递的流程

> references: https://github.com/google/leveldb/blob/main/doc/index.md

## Base Operations

### 打开数据库，并进行数据的写入、读取、删除操作

```c++
int main(int argc, char **argv) {
    // open a database and create if necessary
    leveldb::DB* db;
    leveldb::Options options;
    // set options if database file is not exist to create database
    options.create_if_missing = true;
    // if database file is exist to raise exception
    // options.error_if_exists = true;
    leveldb::Status status = leveldb::DB::Open(options, "./testdb", &db);

    // check that the status
    if (!status.ok()) {
        std::cerr << status.ToString() << std::endl;
    } else {
        std::cout << "success!" << std::endl;
    }
    /*
    ➜  build git:(main) ✗ ./LevelDBTest 
    Invalid argument: ./testdb: exists (error_if_exists is true)
    */

    // assert(status.ok());
    // std::cout << "create success" << std::endl;
    /*
    ➜  build git:(main) ✗ ls
    CMakeCache.txt  CMakeFiles  cmake_install.cmake  LevelDBTest  Makefile  testdb
    ➜  build git:(main) ✗ ls testdb   
    000003.log  CURRENT  LOCK  LOG  MANIFEST-000002
    ➜  build git:(main) ✗ ./LevelDBTest
    LevelDBTest: /root/pkg/leveldb/examples/_1_open_options/main.cc:14: int main(int, char**): Assertion `status.ok()' failed.
    [1]    93525 abort (core dumped)  ./LevelDBTest
    */

    // reads and writes
    std::string key1 = "key1";
    std::string val1 = "val1";

    leveldb::Status s = db->Put(leveldb::WriteOptions(), key1, val1);
    if (s.ok()) {
        s = db->Get(leveldb::ReadOptions(), key1, &val1);
    } else {
        std::cout << "get failed" << std::endl;
    }

    if (s.ok()) {   
        s = db->Delete(leveldb::WriteOptions(), key1);
    } else {
        std::cout << "delete failed" << std::endl;
    }

    s = db->Put(leveldb::WriteOptions(), key1, val1);

    delete db;
}
```

### 支持将多个操作作为一个atomic update

```c++
int main(int argc, char** argv) {
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status s = leveldb::DB::Open(options, "./testdb", &db);
    if (!s.ok()) { 
        std::cout << "open failed" << std::endl;
    } else {
        std::cout << "opened successfully" << std::endl;
    }

    // atomic updates
    // Note that if the process dies after the Put of key2 but before the delete of key1, the 
    // same value may be left stored under multiple keys. Such problems can be avoided by using
    // the WriteBatch class to atomically apply a set of updates
    std::string key1 = "key1", value1 = "value1";
    std::string key2 = "key2";
    s = db->Get(leveldb::ReadOptions(), key1, &value1);
    if (!s.ok()) { 
        std::cout << "get failed" << std::endl;
    } else {
        leveldb::WriteBatch batch;
        // the execution in a batch is ordered by the definition
        // delete before put, so that we don't get a error if key1 is equal to key2
        batch.Delete(key1);
        batch.Put(key2, value1);
        s = db->Write(leveldb::WriteOptions(), &batch);
        if (!s.ok()) { 
            std::cout << "write batch failed" << std::endl;
        }
    }
}
```

### 设置同步开关

> leveldb数据写入时默认是一个异步操作，并不会直接写到磁盘，而是写到操作系统内存后写操作就会直接返回，设置此选项后会是的写动作写入到磁盘完成才返回

```c++
int main(int argc, char** argv) {
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status s = leveldb::DB::Open(options, "./testdb", &db);
    if (!s.ok()) { 
        std::cout << "open failed" << std::endl;
    } else {
        std::cout << "opened successfully" << std::endl;
    }

    // open the flag for synchronous write
    // By default, each write to leveldb is asynchronous: it returns after pushing the write from 
    // the process into the operating system. The transfer from operating system memory to the 
    // underlying persistent storage happens asynchronously. The sync flag can be turned on for 
    // a particular write to make the write operation not return until the data being written has
    // been pushed all the way to persistent storage. 
    leveldb::WriteOptions write_options;
    write_options.sync = true;

    std::string key1 = "key1", value1 = "value1";
    s = db->Put(write_options, key1, value1);
    if (!s.ok()) {
        std::cout << "write error: " << s.ToString() << std::endl;
    }
}
```

### 遍历数据库内元素的方式

```c++
int main(int argc, char** argv) {
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status s = leveldb::DB::Open(options, "./testdb", &db);
    if (!s.ok()) { 
        std::cout << "open failed" << std::endl;
    } else {
        std::cout << "opened successfully" << std::endl;
    }

    // Iteration
    // print key value
    leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
    std::cout << "print by order" << std::endl;
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        std::cout << it->key().ToString() << ":" << it->value().ToString() << std::endl;
    }

    std::cout << "print reverse order" << std::endl;
    for (it->SeekToLast(); it->Valid(); it->Prev()) {
        std::cout << it->key().ToString() << ":" << it->value().ToString() << std::endl;
    }

    /*
    ➜  build git:(main) ✗ ./LevelDBTest   
    opened successfully
    print by order
    key1:value1
    key2:value2
    key3:value3
    print reverse order
    key3:value3
    key2:value2
    key1:value
    */

    // print value just the key in range [start, limit)
    // std::cout << "range:" << std::endl;
    // auto start, limit; // Slice
    // for (it->Seek(start); it->Valid() && it->key().ToString() < limit; it->Next()) {
    //     std::cout << it->key().ToString() << ":" << it->value().ToString() << std::endl;
    // }

    assert(it->status().ok());
    delete it;
}
```

### Snapshot

```c++
int main(int argc, char** argv) {
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status s = leveldb::DB::Open(options, "./testdb", &db);
    if (!s.ok()) { 
        std::cout << "open failed" << std::endl;
    } else {
        std::cout << "opened successfully" << std::endl;
    }

    // Snapshot
    // Snapshots provide consistent read-only views over the entire state of the key-value store.
    // ReadOptions::snapshot may be non-NULL to indicate that a read should operate on a particular
    // version of the DB state. If ReadOptions::snapshot is NULL, the read will operate on an 
    // implicit snapshot of the current state.
    leveldb::ReadOptions r_options;
    r_options.snapshot = db->GetSnapshot();
    std::string key5 = "key5", value5 = "value5";
    s = db->Put(leveldb::WriteOptions(), key5, value5);
    if (!s.ok()) {
        std::cout << "Error inserting" << std::endl;
    }

    leveldb::Iterator *iter = db->NewIterator(r_options);
    std::cout << "Transaction snapshot" << std::endl;
    for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
        std::cout << iter->key().ToString() << ":" << iter->value().ToString() << std::endl;
    }

    leveldb::Iterator *iter2 = db->NewIterator(leveldb::ReadOptions());
    std::cout << "Transaction database" << std::endl;
    for (iter2->SeekToFirst(); iter2->Valid(); iter2->Next()) {
        std::cout << iter2->key().ToString() << ":" << iter2->value().ToString() << std::endl;
    }

    db->ReleaseSnapshot(r_options.snapshot);
    /*
    ➜  build git:(main) ✗ ./LevelDBTest   
    opened successfully
    Transaction snapshot
    key1:value1
    key2:value2
    key3:value3
    key4:value4
    Transaction database
    key1:value1
    key2:value2
    key3:value3
    key4:value4
    key5:value5
    */
}
```

### Slice

```c++
int main(int argc, char** argv) {
    // Slice
    // Slice is a simple structure that contains a length and a pointer to an external byte array.
    // Returning a Slice is a cheaper alternative to returning a std::string since we do not need
    // to copy potentially large keys and values. In addition, leveldb methods do not return
    // null-terminated C-style strings since leveldb keys and values are allowed to contain '\0' bytes.
    leveldb::Slice s1 = "test";

    // string to Slice
    std::string str("test");
    leveldb::Slice s2(str);

    // Slice to string
    std::string str2 = s1.ToString();
    assert(str == str2);

    // be careful when using Slices since it is up to the caller to ensure that the external byte
    // array into which the Slice points remains live while the Slice is in use. For example, the following is buggy
    leveldb::Slice s3;
    if (true) {
        // When the if statement goes out of scope, str will be destroyed and the backing storage for slice will disappear.
        std::string str3("asdad");
        s3 = str3;
    }

    // user s3
    // std::cout << s3.ToString() << std::endl;
}
```

### Comparator

> define a proper subclass of leveldb::Comparator that expresses your rule.

```c++
class TwoPartComparator : public leveldb::Comparator {
public:
    int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const {
        int a1, a2, b1, b2;
        ParseKey(a, &a1, &a2);
        ParseKey(b, &b1, &b2);
        if (a1 < b1) return -1;
        if (a1 > b1) return 1;
        if (a2 > b2) return -1;
        if (a2 < b2) return 1;
        return 0;
    }

    void ParseKey(const leveldb::Slice& a, int* a1, int* a2) const {

    }

    const char* Name() const { return "TwoPartComparator"; }
    void FindShortestSeparator(std::string*, leveldb::Slice&) const {}
    void FindShortSuccessor(std::string* key) const {}

    /**
     * use
     * 
     * TwoPartComparator cmp;
     * leveldb::DB* db;
     * leveldb::Options options;
     * options.comparator = cmp;
     * ...
     * 
     */
};
```

## Performance

> 可以通过修改leveldb的某些选项值达到改变性能的目的，这些选项在include/options.h

### Block size

> leveldb groups adjacent keys together into the same block and such a block is the unit of transfer to and from persistent storage. The default block size is approximately 4096 uncompressed bytes. Applications that mostly do bulk scans over the contents of the database may wish to increase this size. Applications that do a lot of point reads of small values may wish to switch to a smaller block size if performance measurements indicate an improvement. There isn't much benefit in using blocks smaller than one kilobyte, or larger than a few megabytes. Also note that compression will be more effective with larger block sizes.

### 在必要时设定kNoCompression

> Each block is individually compressed before being written to persistent storage. Compression is on by default since the default compression method is very fast, and is automatically disabled for uncompressible data. In rare cases, applications may want to disable compression entirely, but should only do so if benchmarks show a performance improvement:

```c++
leveldb::Options options;
options.compression = leveldb::kNoCompression;
... leveldb::DB::Open(options, name, ...) ....
```

### Cache

> 通过使用缓存将常用数据的未压缩形式放在缓存中以此提高性能
> The contents of the database are stored in a set of files in the filesystem and each file stores a sequence of compressed blocks. If options.block_cache is non-NULL, it is used to cache frequently used uncompressed block contents.
> 这里的缓存指的是leveldb client提供的，而非操作系统自身的缓存

```c++
#include "leveldb/cache.h"

leveldb::Options options;
options.block_cache = leveldb::NewLRUCache(100 * 1048576);  // 100MB cache
leveldb::DB* db;
leveldb::DB::Open(options, name, &db);
... use the db ...
delete db
delete options.block_cache;
```

> 在进行数据库的数据遍历读操作时，可能希望此次操作是针对整体的数据库数据，而非缓存，因此在读的时候需要关闭缓存的选项

```C++
leveldb::ReadOptions options;
options.fill_cache = false;
leveldb::Iterator* it = db->NewIterator(options);
for (it->SeekToFirst(); it->Valid(); it->Next()) {
  ...
}
delete it;
```

### Key Layout

> 根据不通类型的数据的访问频率，将不同类型的数据放在缓存和磁盘不通的位置，从而是的访问更加迅速
> Note that the unit of disk transfer and caching is a block. Adjacent keys (according to the database sort order) will usually be placed in the same block. Therefore the application can improve its performance by placing keys that are accessed together near each other and placing infrequently used keys in a separate region of the key space.
> For example, suppose we are implementing a simple file system on top of leveldb. The types of entries we might wish to store are:
> filename -> permission-bits, length, list of file_block_ids
> file_block_id -> data
> We might want to prefix filename keys with one letter (say '/') and the file_block_id keys with a different letter (say '0') so that scans over just the metadata do not force us to fetch and cache bulky file contents.

### filter

> 通过filter减少在执行Get操作时不必要的磁盘读
> Because of the way leveldb data is organized on disk, a single Get() call may involve multiple reads from disk. The optional FilterPolicy mechanism can be used to reduce the number of disk reads substantially.

```c++
leveldb::Options options;
options.filter_policy = NewBloomFilterPolicy(10);
leveldb::DB* db;
leveldb::DB::Open(options, "/tmp/testdb", &db);
... use the database ...
delete db;
delete options.filter_policy;
```

> 使用布隆过滤器从而减少不必要的快访问，但同时会增加内存使用
> The preceding code associates a Bloom filter based filtering policy with the database. Bloom filter based filtering relies on keeping some number of bits of data in memory per key (in this case 10 bits per key since that is the argument we passed to NewBloomFilterPolicy). This filter will reduce the number of unnecessary disk reads needed for Get() calls by a factor of approximately a 100. Increasing the bits per key will lead to a larger reduction at the cost of more memory usage.** We recommend that applications whose working set does not fit in memory and that do a lot of random reads set a filter policy.**
> If you are using a custom comparator, you should ensure that the filter policy you are using is compatible with your comparator. For example, consider a comparator that ignores trailing spaces when comparing keys. NewBloomFilterPolicy must not be used with such a comparator. Instead, the application should provide a custom filter policy that also ignores trailing spaces. For example:

```c++
class CustomFilterPolicy : public leveldb::FilterPolicy {
 private:
  leveldb::FilterPolicy* builtin_policy_;

 public:
  CustomFilterPolicy() : builtin_policy_(leveldb::NewBloomFilterPolicy(10)) {}
  ~CustomFilterPolicy() { delete builtin_policy_; }

  const char* Name() const { return "IgnoreTrailingSpacesFilter"; }

  void CreateFilter(const leveldb::Slice* keys, int n, std::string* dst) const {
    // Use builtin bloom filter code after removing trailing spaces
    std::vector<leveldb::Slice> trimmed(n);
    for (int i = 0; i < n; i++) {
      trimmed[i] = RemoveTrailingSpaces(keys[i]);
    }
    builtin_policy_->CreateFilter(trimmed.data(), n, dst);
  }
};
```

> filter的更多信息可以在这里看leveldb/filter_policy.h

## Ckecksums

> leveldb对所有存储在文件系统的数据提供checksum，通过设置不同的option，可能控制在每次读之前进行checksum或者打开数据库的时候进行checksum
> ReadOptions::verify_checksums may be set to true to force checksum verification of all data that is read from the file system on behalf of a particular read. By default, no such verification is done.
> Options::paranoid_checks may be set to true before opening a database to make the database implementation raise an error as soon as it detects an internal corruption. Depending on which portion of the database has been corrupted, the error may be raised when the database is opened, or later by another database operation. By default, paranoid checking is off so that the database can be used even if parts of its persistent storage have been corrupted.
> If a database is corrupted (perhaps it cannot be opened when paranoid checking is turned on), the leveldb::RepairDB function may be used to recover as much of the data as possible

## Approximate Size

> 获取指定key范围内的数据在文件系统中占据的空间大小，单位时byte
> The GetApproximateSizes method can used to get the approximate number of bytes of file system space used by one or more key ranges.

```c++
leveldb::Range ranges[2];
ranges[0] = leveldb::Range("a", "c");
ranges[1] = leveldb::Range("x", "z");
uint64_t sizes[2];// 分别存储上述两个指定范围的大小估计值
db->GetApproximateSizes(ranges, 2, sizes);
```

## Environment

> leveldb 实现发出的所有文件操作（和其他操作系统调用）都通过 leveldb::Env 对象进行路由。 成熟的客户可能希望提供自己的 Env 实现以获得更好的控制。 例如，应用程序可能会在文件 IO 路径中引入人为延迟，以限制 leveldb 对系统中其他活动的影响。
> All file operations (and other operating system calls) issued by the leveldb implementation are routed through a leveldb::Env object. Sophisticated clients may wish to provide their own Env implementation to get better control. For example, an application may introduce artificial delays in the file IO paths to limit the impact of leveldb on other activities in the system.

```c++
class SlowEnv : public leveldb::Env {
  ... implementation of the Env interface ...
};

SlowEnv env;
leveldb::Options options;
options.env = &env;
Status s = leveldb::DB::Open(options, ...);
```

## Port

> 平台移植相关
> leveldb may be ported to a new platform by providing platform specific implementations of the types/methods/functions exported by leveldb/port/port.h. See leveldb/port/port_example.h for more details.
> In addition, the new platform may need a new default leveldb::Env implementation. See leveldb/util/env_posix.h for an example.

## Files

> references: https://github.com/google/leveldb/blob/main/doc/impl.md

### Log File

> 最近的更新会被追加写到日志文件中，当日志文件大小达到大约4MB的时候，日志文件会被转化为一个有序表，并创建一个新的日志文件
> A **log file** (*.log) stores a sequence of recent updates. Each update is appended to the current log file. When the log file reaches a pre-determined size (approximately 4MB by default), it is converted to a **sorted table** (see below) and a new log file is created for future updates.
> A copy of the current log file is kept in an in-memory structure (the memtable). This copy is consulted on every read so that read operations reflect all logged updates.

## Sorted tables

> A sorted table (*.ldb) stores a sequence of entries sorted by key. Each entry is either a value for the key, or a deletion marker for the key. (Deletion markers are kept around to hide obsolete values present in older sorted tables).
> **The set of sorted tables are organized into a sequence of levels**(why it is named leveldb). The sorted table generated from a log file is placed in a special young level (also called level-0). When the number of young files exceeds a certain threshold (currently four), all of the young files are merged together with all of the overlapping level-1 files to produce a sequence of new level-1 files (we create a new level-1 file for every 2MB of data.)
> Files in the young level may contain overlapping keys. However files in other levels have distinct non-overlapping key ranges. Consider level number L where L >= 1. When the combined size of files in level-L exceeds (10^L) MB (i.e., 10MB for level-1, 100MB for level-2, ...), one file in level-L, and all of the overlapping files in level-(L+1) are merged to form a set of new files for level-(L+1). These merges have the effect of gradually migrating new updates from the young level to the largest level using only bulk reads and writes (i.e., minimizing expensive seeks).

### Manifest

> 对于当前数据有哪些SSTable，这些SSTable属于哪一层，每一个SSTable的键范围和文件大小等信息，需要持久化到磁盘上，下一次打开数据库的时候，就可以从磁盘上读取到这些元数据，恢复内存里的数据结构，这个持久化数据就存储在MANIFEST文件中。
> A MANIFEST file lists the set of sorted tables that make up each level, the corresponding key ranges, and other important metadata. A new MANIFEST file (with a new number embedded in the file name) is created whenever the database is reopened. The MANIFEST file is formatted as a log, and changes made to the serving state (as files are added or removed) are appended to this log.

### Current

> CURRENT is a simple text file that contains the name of the latest MANIFEST file.

### Info logs

> Informational messages are printed to files named LOG and LOG.old.

### Others

> Other files used for miscellaneous purposes may also be present (LOCK, *.dbtmp).

## More+

### Level 0

> When the log file grows above a certain size (4MB by default): Create a brand new memtable and log file and direct future updates here.
> In the background:

1. Write the contents of the previous memtable to an sstable.
2. Discard the memtable.
3. Delete the old log file and the old memtable.
4. Add the new sstable to the young (level-0) level.

### Compactions

> A compaction merges the contents of the picked files to produce a sequence of level-(L+1) files. We switch to producing a new level-(L+1) file after the current output file has reached the target file size (2MB). We also switch to a new output file when the key range of the current output file has grown enough to overlap more than ten level-(L+2) files. This last rule ensures that a later compaction of a level-(L+1) file will not pick up too much data from level-(L+2).
> The old files are discarded and the new files are added to the serving state.
> Compactions for a particular level rotate through the key space. In more detail, for each level L, we remember the ending key of the last compaction at level L. The next compaction for level L will pick the first file that starts after this key (wrapping around to the beginning of the key space if there is no such file).
> Compactions drop overwritten values. They also drop deletion markers if there are no higher numbered levels that contain a file whose range overlaps the current key.

### Timing

> Level-0 compactions will read up to four 1MB files from level-0, and at worst all the level-1 files (10MB). I.e., we will read 14MB and write 14MB.
> Other than the special level-0 compactions, we will pick one 2MB file from level L. In the worst case, this will overlap ~ 12 files from level L+1 (10 because level-(L+1) is ten times the size of level-L, and another two at the boundaries since the file ranges at level-L will usually not be aligned with the file ranges at level-L+1). The compaction will therefore read 26MB and write 26MB. Assuming a disk IO rate of 100MB/s (ballpark range for modern drives), the worst compaction cost will be approximately 0.5 second.
> If we throttle the background writing to something small, say 10% of the full 100MB/s speed, a compaction may take up to 5 seconds. If the user is writing at 10MB/s, we might build up lots of level-0 files (~50 to hold the 5*10MB). This may significantly increase the cost of reads due to the overhead of merging more files together on every read.
> Solution 1: To reduce this problem, we might want to increase the log switching threshold when the number of level-0 files is large. Though the downside is that the larger this threshold, the more memory we will need to hold the corresponding memtable.
> Solution 2: We might want to decrease write rate artificially when the number of level-0 files goes up.
> Solution 3: We work on reducing the cost of very wide merges. Perhaps most of the level-0 files will have their blocks sitting uncompressed in the cache and we will only need to worry about the O(N) complexity in the merging iterator.

### Number of files

> Instead of always making 2MB files, we could make larger files for larger levels to reduce the total file count, though at the expense of more bursty compactions. Alternatively, we could shard the set of files into multiple directories.
> An experiment on an ext3 filesystem on Feb 04, 2011 shows the following timings to do 100K file opens in directories with varying number of files:
> Files in directory	Microseconds to open a file
> 1000	                9
> 10000	                10
> 100000	            16
> So maybe even the sharding is not necessary on modern filesystems?

### Recovery

1. Read CURRENT to find name of the latest committed MANIFEST
2. Read the named MANIFEST file
3. Clean up stale files
4. We could open all sstables here, but it is probably better to be lazy...
5. Convert log chunk to a new level-0 sstable
6. Start directing new writes to a new log file with recovered sequence#

### GC

> RemoveObsoleteFiles() is called at the end of every compaction and at the end of recovery. It finds the names of all files in the database. It deletes all log files that are not the current log file. It deletes all table files that are not referenced from some level and are not the output of an active compaction.

## LevelDB File Format

```bash
<beginning_of_file>
[data block 1]
[data block 2]
...
[data block N]
[meta block 1]
...
[meta block K]
[metaindex block]
[index block]
[Footer]        (fixed size; starts at file_size - sizeof(Footer))
<end_of_file>
```

> The file contains internal pointers. Each such pointer is called a BlockHandle and contains the following information:
>
> - offset:   varint64
> - size:     varint64

### varints

1. The sequence of key/value pairs in the file are stored in sorted order and partitioned into a sequence of data blocks. These blocks come one after another at the beginning of the file. Each data block is formatted according to the code in block_builder.cc, and then optionally compressed.
2. After the data blocks we store a bunch of meta blocks. The supported meta block types are described below. More meta block types may be added in the future. Each meta block is again formatted using block_builder.cc and then optionally compressed.
3. A "metaindex" block. It contains one entry for every other meta block where the key is the name of the meta block and the value is a BlockHandle pointing to that meta block.
4. An "index" block. This block contains one entry per data block, where the key is a string >= last key in that data block and before the first key in the successive data block. The value is the BlockHandle for the data block.
5. At the very end of the file is a fixed length footer that contains the BlockHandle of the metaindex and index blocks as well as a magic number.

```bash
 metaindex_handle: char[p];     // Block handle for metaindex
 index_handle:     char[q];     // Block handle for index
 padding:          char[40-p-q];// zeroed bytes to make fixed length
                                // (40==2*BlockHandle::kMaxEncodedLength)
 magic:            fixed64;     // == 0xdb4775248b80fb57 (little-endian)
```

### "filter"Meta Block

> If a FilterPolicy was specified when the database was opened, a filter block is stored in each table. The "metaindex" block contains an entry that maps from filter.`<N>` to the BlockHandle for the filter block where `<N>` is the string returned by the filter policy's Name() method.
> The filter block stores a sequence of filters, where filter i contains the output of FilterPolicy::CreateFilter() on all keys that are stored in a block whose file offset falls within the range
> [ i*base ... (i+1)*base-1 ]
> Currently, "base" is 2KB. So for example, if blocks X and Y start in the range [ 0KB .. 2KB-1 ], all of the keys in X and Y will be converted to a filter by calling FilterPolicy::CreateFilter(), and the resulting filter will be stored as the first filter in the filter block.
> The filter block is formatted as follows:

```bash
[filter 0]
[filter 1]
[filter 2]
...
[filter N-1]

[offset of filter 0]                  : 4 bytes
[offset of filter 1]                  : 4 bytes
[offset of filter 2]                  : 4 bytes
...
[offset of filter N-1]                : 4 bytes

[offset of beginning of offset array] : 4 bytes
lg(base)                              : 1 byte
```

> The offset array at the end of the filter block allows efficient mapping from a data block offset to the corresponding filter.

### "stats"Meta Block

> This meta block contains a bunch of stats. The key is the name of the statistic. The value contains the statistic.

```bash
data size
index size
key size (uncompressed)
value size (uncompressed)
number of entries
number of data blocks
```

## LevelDB Log Format

The log file contents are a sequence of 32KB blocks. The only exception is that the tail of the file may contain a partial block.

Each block consists of a sequence of records:

```bash
block := record* trailer?
record :=
  checksum: uint32     // crc32c of type and data[] ; little-endian
  length: uint16       // little-endian
  type: uint8          // One of FULL, FIRST, MIDDLE, LAST
  data: uint8[length]
```

A record never starts within the last six bytes of a block (since it won't fit). Any leftover bytes here form the trailer, which must consist entirely of zero bytes and must be skipped by readers.

Aside: if exactly seven bytes are left in the current block, and a new non-zero length record is added, the writer must emit a FIRST record (which contains zero bytes of user data) to fill up the trailing seven bytes of the block and then emit all of the user data in subsequent blocks.

More types may be added in the future. Some Readers may skip record types they do not understand, others may report that some data was skipped.

```bash
FULL == 1
FIRST == 2
MIDDLE == 3
LAST == 4
```

The FULL record contains the contents of an entire user record.

FIRST, MIDDLE, LAST are types used for user records that have been split into multiple fragments (typically because of block boundaries). FIRST is the type of the first fragment of a user record, LAST is the type of the last fragment of a user record, and MIDDLE is the type of all interior fragments of a user record.

Example: consider a sequence of user records:

```bash
A: length 1000
B: length 97270
C: length 8000
```

A will be stored as a FULL record in the first block.

B will be split into three fragments: first fragment occupies the rest of the first block, second fragment occupies the entirety of the second block, and the third fragment occupies a prefix of the third block. This will leave six bytes free in the third block, which will be left empty as the trailer.

C will be stored as a FULL record in the fourth block.

## 基本概念

### Slice

> 数据的长度信息和内容信息被包装成一个整体结构，叫做Slice
> include/leveldb/slice.h

```c++
class Slice {
// ... other
private:
    const char* data_;
    size_t size_;
};
```

### Option

> leveldb的配置文件，包括启动时的配置，读写数据的配置
> include/leveldb/option.h

```c++
// Options to control the behavior of a database (passed to DB::Open)
struct LEVELDB_EXPORT Options {
  // Create an Options object with default values for all fields.
  Options();

  // -------------------
  // Parameters that affect behavior

  // Comparator used to define the order of keys in the table.
  // Default: a comparator that uses lexicographic byte-wise ordering
  //
  // REQUIRES: The client must ensure that the comparator supplied
  // here has the same name and orders keys *exactly* the same as the
  // comparator provided to previous open calls on the same DB.
  // 传入的比较器
  const Comparator* comparator;

  // If true, the database will be created if it is missing.
  bool create_if_missing = false;

  // If true, an error is raised if the database already exists.
  bool error_if_exists = false;

  // If true, the implementation will do aggressive checking of the
  // data it is processing and will stop early if it detects any
  // errors.  This may have unforeseen ramifications: for example, a
  // corruption of one DB entry may cause a large number of entries to
  // become unreadable or for the entire DB to become unopenable.
  // 是否保存中间的错误状态(RecoverLog/compact),compact 时是否读到的 block 做检验。
  bool paranoid_checks = false;

  // Use the specified object to interact with the environment,
  // e.g. to read/write files, schedule background work, etc.
  // Default: Env::Default()
  // 传入的ENV
  Env* env;

  // Any internal progress/error information generated by the db will
  // be written to info_log if it is non-null, or to a file stored
  // in the same directory as the DB contents if info_log is null.
  // 传入的打印日志的Logger
  Logger* info_log = nullptr;

  // -------------------
  // Parameters that affect performance

  // Amount of data to build up in memory (backed by an unsorted log
  // on disk) before converting to a sorted on-disk file.
  //
  // Larger values increase performance, especially during bulk loads.
  // Up to two write buffers may be held in memory at the same time,
  // so you may wish to adjust this parameter to control memory usage.
  // Also, a larger write buffer will result in a longer recovery time
  // the next time the database is opened.
  // memtable的最大大小
  size_t write_buffer_size = 4 * 1024 * 1024;

  // Number of open files that can be used by the DB.  You may need to
  // increase this if your database has a large working set (budget
  // one open file per 2MB of working set).
  // db 中打开的文件最大个数
  // db 中需要打开的文件包括基本的 CURRENT/LOG/MANIFEST/LOCK, 以及打开的 sstable 文件。
  // sstable 一旦打开，就会将 index 信息加入 TableCache，所以把
  // (max_open_files - 10)作为 table cache 的最大数量.
  int max_open_files = 1000;

  // Control over blocks (user data is stored in a set of blocks, and
  // a block is the unit of reading from disk).

  // If non-null, use the specified cache for blocks.
  // If null, leveldb will automatically create and use an 8MB internal cache.
  // 传入的 block 数据的 cache 管理
  Cache* block_cache = nullptr;

  // Approximate size of user data packed per block.  Note that the
  // block size specified here corresponds to uncompressed data.  The
  // actual size of the unit read from disk may be smaller if
  // compression is enabled.  This parameter can be changed dynamically.
  // sstable 中 block 的 size
  size_t block_size = 4 * 1024;

  // Number of keys between restart points for delta encoding of keys.
  // This parameter can be changed dynamically.  Most clients should
  // leave this parameter alone.
  // block 中对 key 做前缀压缩的区间长度
  int block_restart_interval = 16;

  // Leveldb will write up to this amount of bytes to a file before
  // switching to a new one.
  // Most clients should leave this parameter alone.  However if your
  // filesystem is more efficient with larger files, you could
  // consider increasing the value.  The downside will be longer
  // compactions and hence longer latency/performance hiccups.
  // Another reason to increase this parameter might be when you are
  // initially populating a large database.
  size_t max_file_size = 2 * 1024 * 1024;

  // Compress blocks using the specified compression algorithm.  This
  // parameter can be changed dynamically.
  //
  // Default: kSnappyCompression, which gives lightweight but fast
  // compression.
  //
  // Typical speeds of kSnappyCompression on an Intel(R) Core(TM)2 2.4GHz:
  //    ~200-500MB/s compression
  //    ~400-800MB/s decompression
  // Note that these speeds are significantly faster than most
  // persistent storage speeds, and therefore it is typically never
  // worth switching to kNoCompression.  Even if the input data is
  // incompressible, the kSnappyCompression implementation will
  // efficiently detect that and will switch to uncompressed mode.
  // 压缩数据使用的压缩类型（默认支持 snappy，其他类型需要使用者实现）
  CompressionType compression = kSnappyCompression;

  // EXPERIMENTAL: If true, append to existing MANIFEST and log files
  // when a database is opened.  This can significantly speed up open.
  //
  // Default: currently false, but may become true later.
  bool reuse_logs = false;

  // If non-null, use the specified filter policy to reduce disk reads.
  // Many applications will benefit from passing the result of
  // NewBloomFilterPolicy() here.
  const FilterPolicy* filter_policy = nullptr;
};

// Options that control read operations
struct LEVELDB_EXPORT ReadOptions {
  // If true, all data read from underlying storage will be
  // verified against corresponding checksums.
  // 是否对读到的 block 做校验
  bool verify_checksums = false;

  // Should the data read for this iteration be cached in memory?
  // Callers may wish to set this field to false for bulk scans.
  // 读到的 block 是否加入 block cache
  bool fill_cache = true;

  // If "snapshot" is non-null, read as of the supplied snapshot
  // (which must belong to the DB that is being read and which must
  // not have been released).  If "snapshot" is null, use an implicit
  // snapshot of the state at the beginning of this read operation.
  // 指定读取的 SnapShot
  const Snapshot* snapshot = nullptr;
};

// Options that control write operations
struct LEVELDB_EXPORT WriteOptions {
  WriteOptions() = default;

  // If true, the write will be flushed from the operating system
  // buffer cache (by calling WritableFile::Sync()) before the write
  // is considered complete.  If this flag is true, writes will be
  // slower.
  //
  // If this flag is false, and the machine crashes, some recent
  // writes may be lost.  Note that if it is just the process that
  // crashes (i.e., the machine does not reboot), no writes will be
  // lost even if sync==false.
  //
  // In other words, a DB write with sync==false has similar
  // crash semantics as the "write()" system call.  A DB write
  // with sync==true has similar crash semantics to a "write()"
  // system call followed by "fsync()".
  // write 时，记 binlog 之后，是否对 binlog 做 sync。
  bool sync = false;
};
```

**编译时常量**

```c++
namespace config {  //  db/dbformat.h
// level 的最大值
static const int kNumLevels = 7;

// Level-0 compaction is started when we hit this many files.
// level-0 中 sstable 的数量超过这个阈值，触发 compact
static const int kL0_CompactionTrigger = 4;

// Soft limit on number of level-0 files.  We slow down writes at this point.
// level-0 中 sstable 的数量超过这个阈值, 慢处理此次写(sleep1ms）
static const int kL0_SlowdownWritesTrigger = 8;

// Maximum number of level-0 files.  We stop writes at this point.
 // level-0 中 sstable 的数量超过这个阈值, 阻塞至 compact memtable 完成。
static const int kL0_StopWritesTrigger = 12;

// Maximum level to which a new compacted memtable is pushed if it
// does not create overlap.  We try to push to level 2 to avoid the
// relatively expensive level 0=>1 compactions and to avoid some
// expensive manifest file operations.  We do not push all the way to
// the largest level since that can generate a lot of wasted disk
// space if the same key space is being repeatedly overwritten.
// memtable dump 成的 sstable，允许推向的最高 level
static const int kMaxMemCompactLevel = 2;

// Approximate gap in bytes between samples of data read during iteration.
static const int kReadBytesPeriod = 1048576;

}  // namespace config

// db/version_set.cc
// compact 过程中，level-0 中的 sstable 由 memtable 直接 dump 生成，不做大小限制
// 非 level-0 中的 sstable 的大小设定为 TargetFileSize
static size_t TargetFileSize(const Options* options) {
  return options->max_file_size;
}

// Maximum bytes of overlaps in grandparent (i.e., level+2) before we
// stop building a single file in a level->level+1 compaction.
// compact level-n 时，与 level-n+2 产生 overlap 的数据 size （参见 Compaction）
static int64_t MaxGrandParentOverlapBytes(const Options* options) {
  return 10 * TargetFileSize(options);
}
```

### ENV

> include/leveldb/env.h util/env_posix.h
> 考虑到移植以及灵活性，leveldb 将系统相关的处理（文件/进程/时间之类）抽象成 Env，用户可以自己实现相应的接口，作为 Option 传入。默认使用自带的实现。

### varint

> util/coding.h
> leveldb 采用了 protocalbuffer 里使用的变长整形编码方法，节省空间

### ValueType

> db/dbformat.h
> leveldb 更新（put/delete）某个 key 时不会操控到 db 中的数据，每次操作都是直接新插入一份 kv 数据，具体的数据合并和清除由后台的 compact 完成。所以，每次 put，db 中就会新加入一份 KV 数据，即使该 key 已经存在；而 delete 等同于 put 空的 value。为了区分真实 kv 数据和删除操作的 mock 数据，使用 ValueType 来标识：\

```c++
// Value types encoded as the last component of internal keys.
// DO NOT CHANGE THESE ENUM VALUES: they are embedded in the on-disk
// data structures.
enum ValueType { kTypeDeletion = 0x0, kTypeValue = 0x1 };
```

### SequenceNumber

> db/dbformat.h
> leveldb 中的每次更新（put/delete)操作都拥有一个版本，由 SequnceNumber 来标识，整个 db 有一个全局值保存着当前使用到的 SequnceNumber。SequnceNumber 在 leveldb 有重要的地位，key 的排序，compact 以及 snapshot 都依赖于它。typedef uint64_t SequenceNumber;存储时，SequnceNumber 只占用 56 bits, ValueType 占用 8 bits，二者共同占用 64bits（uint64_t).

```c++
typedef uint64_t SequenceNumber;

// We leave eight bits empty at the bottom so a type and sequence#
// can be packed together into 64-bits.
static const SequenceNumber kMaxSequenceNumber = ((0x1ull << 56) - 1);
```

### user key

> 用户层面传入的 key，使用 Slice 格式。

### ParsedInternalKey

> db/dbformat.h
> db 内部操作的 key。db 内部需要将 user key 加入元信息(ValueType/SequenceNumber)一并做处理。

```c++
struct ParsedInternalKey {
  Slice user_key;
  SequenceNumber sequence;
  ValueType type;

  ParsedInternalKey() {}  // Intentionally left uninitialized (for speed)
  ParsedInternalKey(const Slice& u, const SequenceNumber& seq, ValueType t)
      : user_key(u), sequence(seq), type(t) {}
  std::string DebugString() const;
};
```

### InternalKey

> db/dbformat.h
> db 内部，包装易用的结构，包含 userkey 与 SequnceNumber/ValueType

### LookupKey

> db/dbformat.h
> db 内部在为查找 memtable/sstable 方便，包装使用的 key 结构，保存有 userkey 与SequnceNumber/ValueType dump 在内存的数据。

```c++
// A helper class useful for DBImpl::Get()
class LookupKey {
 public:
  // Initialize *this for looking up user_key at a snapshot with
  // the specified sequence number.
  LookupKey(const Slice& user_key, SequenceNumber sequence);

  LookupKey(const LookupKey&) = delete;
  LookupKey& operator=(const LookupKey&) = delete;

  ~LookupKey();

  // Return a key suitable for lookup in a MemTable.
  Slice memtable_key() const { return Slice(start_, end_ - start_); }

  // Return an internal key (suitable for passing to an internal iterator)
  Slice internal_key() const { return Slice(kstart_, end_ - kstart_); }

  // Return the user key
  Slice user_key() const { return Slice(kstart_, end_ - kstart_ - 8); }

 private:
  // We construct a char array of the form:
  //    klength  varint32               <-- start_
  //    userkey  char[klength]          <-- kstart_
  //    tag      uint64
  //                                    <-- end_
  // The array is a suitable MemTable key.
  // The suffix starting with "userkey" can be used as an InternalKey.
  const char* start_;
  const char* kstart_;
  const char* end_;
  char space_[200];  // Avoid allocation for short keys
};
// 对 memtable 进行 lookup 时使用 [start,end], 对 sstable lookup 时使用[kstart, end]。
```

### Comparator

> include/leveldb/comparator.h util/comparator.cc
> 对 key 排序时使用的比较方法。leveldb 中 key 为升序。用户可以自定义 user key 的 comparator（user-comparator)，作为 option 传入，默认采用 bytecompare（memcmp）。comparator 中有 FindShortestSeparator（）/ FindShortSuccessor（）两个接口，FindShortestSeparator（start，limit）是获得大于 start 但小于 limit 的最小值。FindShortSuccessor（start）是获得比 start 大的最小值。比较都基于 user-commparator，二者会被用来确定 sstable 中 block 的 end-key。

### InternalKeyComparator

> db/dbformat.h
> db 内部做 key 排序时使用的比较方法。排序时，会先使用 user-comparator 比较 user-key，如果user-key 相同，则比较 SequnceNumber，SequnceNumber 大的为小。因为 SequnceNumber 在 db 中全局递增，所以，对于相同的 user-key，最新的更新（SequnceNumber 更大）排在前面，在查找的时候，会被先找到。InternalKeyComparator 中 FindShortestSeparator（）/ FindShortSuccessor（）的实现，仅从传入的内部 key 参数，解析出 user-key，然后再调用 user-comparator 的对应接口。

### WriteBatch

> db/write_batch.cc
> 对若干数目 key 的 write 操作（put/delete）封装成 WriteBatch。它会将 userkey 连同SequnceNumber 和 ValueType 先做 encode，然后做 decode，将数据 insert 到指定的 Handler（memtable）上面。上层的处理逻辑简洁，但 encode/decode 略有冗余。

1. SequnceNumber: WriteBatch 中开始使用的 SequnceNumber。
2. count: 批量处理的 record 数量
3. record：封装在 WriteBatch 内的数据。
   如果 ValueType 是 kTypeValue,则后面有 key 和 value
   如果 ValueType 是 kTypeDeletion，则后面只有 key。

### Memtable

> db/memtable.cc db/skiplist.h
> db 数据在内存中的存储格式。写操作的数据都会先写到 memtable 中。memtable 的 size 有限制最大值（write_buffer_size)。memtable 的实现是 skiplist。当一个 memtable size 到达阈值时，会变成只读的 memtable（immutable memtable），同时生成一个新的 memtable 供新的写入。后台的 compact 进程会负责将 immutable memtable dump 成 sstable。所以，同时最多会存在两个 memtable（正在写的 memtable 和 immutable memtable）。

### Sstable

> table/table.cc
> db 数据持久化的文件。文件的 size 有限制最大值（target_file_size)。文件前面为数据，后面是索引元信息

### FileMetaData

> db/version_edit.h
> sstable 文件的元信息封装成 FileMetaData

```c++
struct FileMetaData {
  FileMetaData() : refs(0), allowed_seeks(1 << 30), file_size(0) {}

  int refs;  // 引用计数
  int allowed_seeks;  // compact之前允许的seek次数
  uint64_t number;    // File number
  uint64_t file_size;    // File size
  InternalKey smallest;  // sstable文件的最小key
  InternalKey largest;   // sstable文件的最大key
};
```

### block

> table/block.cc
> sstable 的数据由一个个的 block 组成。当持久化数据时，多份 KV 聚合成 block 一次写入；当读取时，也是以 block 单位做 IO。sstable 的索引信息中会保存符合 key-range 的 block 在文件中的offset/size（BlockHandle）。

### BlockHandle

> table/format.h
> block 的元信息（位于 sstable 的 offset/size）封装成 BlockHandle

### FileNumber

> db/dbformat.h
> db 创建文件时会按照规则将 FileNumber 加上特定后缀作为文件名。所以，运行时只需要记录FileNumber（uint64_t)即可定位到具体的文件路径，省掉了字符串的麻烦。FileNumber 在 db 中全局递增

### filename

> db/filename.h

```c++
enum FileType {
  kLogFile,
  kDBLockFile,
  kTableFile,
  kDescriptorFile,
  kCurrentFile,
  kTempFile,
  kInfoLogFile  // Either the current one, or an old one
};
```

1. kLogFile 日志文件：[0-9]+.logleveldb 的写流程是先记 binlog，然后写 sstable，该日志文件即是 binlog。前缀数字为FileNumber。
2. kDBLockFile，lock 文件：LOCK一个 db 同时只能有一个 db 实例操作，通过对 LOCK 文件加文件锁（flock）实现主动保护。
3. kTableFile，sstable 文件：[0-9]+.sst保存数据的 sstable 文件。前缀为 FileNumber。
4. kDescriptorFile，db 元信息文件：MANIFEST-[0-9]+每当 db 中的状态改变（VersionSet），会将这次改变（VersionEdit）追加到 descriptor 文件中。后缀数字为 FileNumber。
5. kCurrentFile，：CURRENTCURRENT 文件中保存当前使用的 descriptor 文件的文件名。
6. kTempFile，临时文件：[0-9]+.dbtmp对 db 做修复（Repairer）时，会产生临时文件。前缀为 FileNumber。
7. kInfoLogFile，db 运行时打印日志的文件：LOGdb 运行时，打印的 info 日志保存在 LOG 中。每次重新运行，如果已经存在 LOG 文件，会先将 LOG文件重名成 LOG.old

### level-n

> db/version_set.h
> 为了均衡读写的效率，sstable 文件分层次（level）管理，db 预定义了最大的 level 值。compact 进程负责 level 之间的均衡

### compact

> db/db_impl.cc db/version_set.cc
> db 中有一个 compact 后台进程，负责将 memtable 持久化成 sstable，以及均衡整个 db 中各 level 的sstable。 Comapct 进程会优先将已经写满的 memtable dump 成 level-0 的 sstable（不会合并相同key 或者清理已经删除的 key）。然后，根据设计的策略选取 level-n 以及 level-n+1 中有 key-rangeoverlap 的几个 sstable 进行 merge(期间会合并相同的 key 以及清理删除的 key），最后生成若干个level-(n+1)的 ssatble。随着数据不断的写入和 compact 的进行，低 level 的 sstable 不断向高level 迁移。level-0 中的 sstable 因为是由 memtable 直接 dump 得到，所以 key-range 可能 overlap，而 level-1 以及更高 level 中的 sstable 都是做 merge 产生，保证了位于同 level 的 sstable 之间，key-range 不会 overlap，这个特性有利于读的处理。

### Compaction

> db/version_set.cc

```c++
// A Compaction encapsulates information about a compaction.
class Compaction {
  //...
  int level_;  // 要compact的level
  uint64_t max_output_file_size_;  // 生成sstable的最大size
  Version* input_version_;  // compact时当前的version
  VersionEdit edit_;  // 记录compact过程中的操作

  // Each compaction reads inputs from "level_" and "level_+1"
  std::vector<FileMetaData*> inputs_[2];  // The two sets of inputs   inputs_[0]是level-n的sstable文件信息   inputs_[1]是level-n+1的sstable信息

  // 位于 level-n+2，并且与 compact 的 key-range 有 overlap 的 sstable。
  // 保存 grandparents_是因为 compact 最终会生成一系列 level-n+1 的 sstable，
  // 而如果生成的 sstable 与 level-n+2 中有过多的 overlap 的话，当 compact
  // level-n+1 时，会产生过多的 merge，为了尽量避免这种情况，compact 过程中
  // 需要检查与 level-n+2 中产生 overlap 的 size 并与
  // 阈值 kMaxGrandParentOverlapBytes 做比较，
  // 以便提前中止 compact
  // State used to check for number of overlapping grandparent files
  // (parent == level_ + 1, grandparent == level_ + 2)
  std::vector<FileMetaData*> grandparents_;
  size_t grandparent_index_;  // Index in grandparent_starts_  记录 compact 时 grandparents_中已经 overlap 的 index
  // 记录是否已经有 key 检查 overlap
  // 如果是第一次检查，发现有 overlap，也不会增加 overlapped_bytes_.
  // (没有看到这样做的意义）
  bool seen_key_;             // Some output key has been seen
  int64_t overlapped_bytes_;  // Bytes of overlap between current output  记录已经 overlap 的累计 size
                              // and grandparent files

  // State for implementing IsBaseLevelForKey

  // compact 时，当 key 的 ValueType 是 kTypeDeletion 时，
  // 要检查其在 level-n+1 以上是否存在（IsBaseLevelForKey()）
  // 来决定是否丢弃掉该 key。因为 compact 时，key 的遍历是顺序的，
  // 所以每次检查从上一次检查结束的地方开始即可，
  // level_ptrs_[i]中就记录了 input_version_->levels_[i]中，上一次比较结束的sstable 的容器下标。
  // level_ptrs_ holds indices into input_version_->levels_: our state
  // is that we are positioned at one of the file ranges for each
  // higher level than the ones involved in this compaction (i.e. for
  // all L >= level_ + 2).
  size_t level_ptrs_[config::kNumLevels];
};
```

### Version

> db/version_set.cc
> 将每次 compact 后的最新数据状态定义为 Version，也就是当前 db 元信息以及每个 level 上具有最新数据状态的 sstable 集合。compact 会在某个 level 上新加入或者删除一些 sstable，但可能这个时候，那些要删除的 sstable 正在被读，为了处理这样的读写竞争情况，基于 sstable 文件一旦生成就不会改动的特点，每个 Version 加入引用计数，读以及解除读操作会将引用计数相应加减一。这样， db 中可能有多个 Version 同时存在（提供服务），它们通过链表链接起来。当 Version 的引用计数为 0 并且不是当前最新的 Version 时，它会从链表中移除，对应的，该 Version 内的 sstable 就可以删除了（这些废弃的 sstable 会在下一次 compact 完成时被清理掉）

```c++
class Version {
  // ...
  VersionSet* vset_;  // VersionSet to which this Version belongs  属于的 VersionSet
  Version* next_;     // Next version in linked list  链表指针 next
  Version* prev_;     // Previous version in linked list
  int refs_;          // Number of live refs to this version  引用计数
 
  // 每个 level 的所有 sstable 元信息。
  // files_[i]中的 FileMetaData 按照 FileMetaData::smallest 排序，
  // 这是在每次更新都保证的。（参见 VersionSet::Builder::Save()）
  // List of files per level
  std::vector<FileMetaData*> files_[config::kNumLevels];

  // Next file to compact based on seek stats.  需要 compact 的文件（allowed_seeks 用光）
  FileMetaData* file_to_compact_;
  int file_to_compact_level_;

  // 当前最大的 compact 权重以及对应的 level
  // Level that should be compacted next and its compaction score.
  // Score < 1 means compaction is not strictly needed.  These fields
  // are initialized by Finalize().
  double compaction_score_;
  int compaction_level_;
};
```

> Version 中与 compact 相关的有 file_to_compact_/ file_to_compact_level_，compaction_score_/compaction_level_，这里详细说明他们的意义。

1. compaction_score_: leveldb 中分 level 管理 sstable，对于写，可以认为与 sstable 无关。而基于 get 的流程（参见get 流程），各 level 中的 sstable 的 count，size 以及 range 分布，会直接影响读的效率。可以预想的最佳情形可能是 level-0 中最多有一个 sstable，level-1 以及之上的各 level 中 keyrange 分布均匀，期望更多的查找可以遍历最少的 level 即可定位到。将这种预想的最佳状态定义成: level 处于均衡的状态。当采用具体的参数量化，也就量化了各个level 的不均衡比重，即 compact 权重： score。score 越大，表示该 level 越不均衡，需要更优先进行 compact。
   每个 level 的具体均衡参数及比重计算策略如下：
   a. 因为 level-0 的 sstable range 可能 overlap，所以如果 level-0 上有过多的 sstable，在做查找时，会严重影响效率。同时，因为 level-0 中的 sstable 由 memtable 直接 dump 得到，并不受kTargetFileSize（生成 sstable 的 size）的控制，所以 sstable 的 count 更有意义。基于此，对于 level-0，均衡的状态需要满足：sstable 的 count < kL0_CompactionTrigger。score = sstable 的 count/ kL0_CompactionTrigger。为了控制这个数量， 另外还有 kL0_SlowdownWritesTrigger/kL0_StopWritesTrigger 两个阈值来主动控制写的速率（参见 put 流程）。
   b. 对于 level-1 及以上的 level，sstable 均由 compact 过程产生，生成的 sstable 大小被kTargetFileSize 控 制 ， 所 以 可 以 限 定 sstable 总 的 size 。当前的策略是设置初始值kBaseLevelSize，然后以 10 的指数级按 level 增长。每个 level 可以容纳的 quota_size =kBaseLevelSize * 10^(level_number-1)。所以 level-1 可以容纳总共 kBaseLevelSize 的sstable，level-2 允许 kBaseLevelSize*10……基于此，对于 level-1 及以上的 level均衡的状态需要满足：sstable 的 size < quota_size。score = sstable 的 size / quota_size。每次 compact 完成，生效新的 Version 时（VersionSet::Finalize()），都会根据上述的策略，计算出每个 level 的 score,取最大值作为当前 Version 的 compaction_score_,同时记录对应的level(compaction_level_)。
2. file_to_compact_: leveldb 对单个 sstable 文件的 IO 也做了细化的优化，设计了一个巧妙的策略。首先，一个 sstable 如果被 seek 到多次（一次 seek 意味找到这个 sstable 进行 IO），可以认为它处在不最优的情况（尤其处于高 level），而我们认为 compact 后会倾向于均衡的状态，所以在一个 sstable 的 seek 次数达到一定阈值后，主动对其进行 compact 是合理的。这个具体 seek 次数阈值(allowed_seeks)的确定，依赖于 sas 盘的 IO 性能：
   a. 一次磁盘寻道 seek 耗费 10ms。
   b. 读或者写 1M 数据耗费 10ms （按 100M/s IO 吞吐能力）。
   c. compact 1M 的数据需要 25M 的 IO：从 level-n 中读 1M 数据，从 level-n+1 中读 10～12M 数据，写入 level-n+1 中 10～12M 数据。所以，compact 1M 的数据的时间相当于做 25 次磁盘 seek，反过来说就是，1 次 seek 相当于compact 40k 数据。那么，可以得到 seek 阈值 allowed_seeks=sstable_size / 40k。保守设置，当前实际的 allowed_seeks = sstable_size / 10k。每次 compact 完成，构造新的 Version 时（Builder::Apply()）,每个 sstable 的 allowed_seeks 会计算出来保存在 FileMetaData。在每次 get 操作的时候，如果有超过一个 sstable 文件进行了 IO，会将最后一个 IO 的 sstable 的allowed_seeks 减一，并检查其是否已经用光了 allowed_seeks,若是，则将该 sstable 记录成当前Version 的 file_to_compact_,并记录其所在的 level(file_to_compact_level_)。

### VersionSet

> db/version_set.h
> 整个 db 的当前状态被 VersionSet 管理着，其中有当前最新的 Version 以及其他正在服务的 Version链表；全局的 SequnceNumber，FileNumber；当前的 manifest_file_number; 封装 sstable 的TableCache。 每个 level 中下一次 compact 要选取的 start_key 等等。

```c++
class VersionSet {
  // ...
  Env* const env_;  // 实际的ENV
  const std::string dbname_;  // db的数据路径
  const Options* const options_;  // 传入的option
  TableCache* const table_cache_;  // 操作 sstable 的 TableCache
  const InternalKeyComparator icmp_; // comparator
  uint64_t next_file_number_; // 下一个可用的 FileNumber  
  uint64_t manifest_file_number_;// manifest 文件的 FileNumber
  uint64_t last_sequence_;// 最后用过的 SequnceNumber
  uint64_t log_number_; // log 文件的 FileNumber
  uint64_t prev_log_number_;  // 0 or backing store for memtable being compacted   辅助 log 文件的 FileNumber，在 compact memtable 时，置为 0

  // Opened lazily
  WritableFile* descriptor_file_;  // manifest 文件的封装
  log::Writer* descriptor_log_;  // manifest 文件的 writer
  Version dummy_versions_;  // Head of circular doubly-linked list of versions.   正在服务的 Version 链表
  Version* current_;        // == dummy_versions_.prev_   当前最新的的 Version

  // 为了尽量均匀 compact 每个 level，所以会将这一次 compact 的 end-key 作为
  // 下一次 compact 的 start-key。compactor_pointer_就保存着每个 level
  // 下一次 compact 的 start-key.
  // 除了 current_外的 Version，并不会做 compact，所以这个值并不保存在 Version 中。
  // Per-level key at which the next compaction at that level should start.
  // Either an empty string, or a valid InternalKey.
  std::string compact_pointer_[config::kNumLevels];
};
```

### VersionEdit

> db/version_edit.cc
>
> compact 过程中会有一系列改变当前 Version 的操作（FileNumber 增加，删除 input 的 sstable，增加输出的 sstable……），为了缩小 Version 切换的时间点，将这些操作封装成 VersionEdit，compact完成时，将 VersionEdit 中的操作一次应用到当前 Version 即可得到最新状态的 Version。
>
> 每次 compact 之后都会将对应的 VersionEdit encode 入 manifest 文件。

```c++

 /
 /class VersionEdit {
  // ...

  typedef std::set<std::pair<int, uint64_t>> DeletedFileSet;

  std::string comparator_;// db 一旦创建，排序的逻辑就必须保持兼容，用 comparator 的名字做凭证
  uint64_t log_number_;// log 的 FileNumber
  uint64_t prev_log_number_;// 辅助 log 的 FileNumber
  uint64_t next_file_number_;// 下一个可用的 FileNumber
  SequenceNumber last_sequence_;// 用过的最后一个 SequnceNumber
  // 标识是否存在，验证使用
  bool has_comparator_;
  bool has_log_number_;
  bool has_prev_log_number_;
  bool has_next_file_number_;
  bool has_last_sequence_;

  std::vector<std::pair<int, InternalKey>> compact_pointers_;// 要更新的 level ==》 compact_pointer。
  DeletedFileSet deleted_files_;// 要删除的 sstable 文件（compact 的 input）
  std::vector<std::pair<int, FileMetaData>> new_files_;// 新的文件（compact 的 output）
};
```

### VersionSet::Builder

> db/version_set.cc
>
> 将 VersionEdit 应用到 VersonSet 上的过程封装成 VersionSet::Builder.主要是更新Version::files_[]
>
> 以 base_->files_[level]为基准，根据 levels_中 LevelStat 的 deleted_files/added_files 做 merge，输出到新 Version 的 files_[level] (VersionSet::Builder::SaveTo()).
> 1） 对于每个 level n， base_->files_[n]与 added_files 做 merge，输出到新 Version 的 files_[n]中。过程中根据 deleted_files 将要删除的丢弃掉（VersionSet::Builder:: MaybeAddFile（）），。
> 2） 处理完成，新 Version 中的 files_[level]有了最新的 sstable 集合（FileMetaData）。

```cpp
class VersionSet::Builder {
 private:
  // Helper to sort by v->files_[file_number].smallest  处理 Version::files_[i]中 FileMetaData 的排序
  struct BySmallestKey {
    const InternalKeyComparator* internal_comparator;

    bool operator()(FileMetaData* f1, FileMetaData* f2) const {
      int r = internal_comparator->Compare(f1->smallest, f2->smallest);
      if (r != 0) {
        return (r < 0);
      } else {
        // Break ties by file number
        return (f1->number < f2->number);
      }
    }
  };

  typedef std::set<FileMetaData*, BySmallestKey> FileSet;  //  排序的 sstable（FileMetaData）集合
  struct LevelState {// 要添加和删除的 sstable 文件集合
    std::set<uint64_t> deleted_files;
    FileSet* added_files;
  };

  VersionSet* vset_;// 要更新的 VersionSet
  Version* base_; // 基准的 Version，compact 后，将 current_传入作为 base
  // 各个 level 上要更新的文件集合（LevelStat）
  // compact 时，并不是每个 level 都有更新（level-n/level-n+1）。
  LevelState levels_[config::kNumLevels];
// ...
}
```

### Manifest

> db/version_set.cc
>
> 为了重启 db 后可以恢复退出前的状态，需要将 db 中的状态保存下来，这些状态信息就保存在manifeest 文件中。当 db 出现异常时，为了能够尽可能多的恢复，manifest 中不会只保存当前的状态，而是将历史的状态都保存下来。又考虑到每次状态的完全保存需要的空间和耗费的时间会较多，当前采用的方式是，只在 manifest 开始保存完整的状态信息（VersionSet::WriteSnapshot（）），接下来只保存每次compact 产生的操作（VesrionEdit），重启 db 时，根据开头的起始状态，依次将后续的 VersionEdit replay，即可恢复到退出前的状态（Vesrion）。

### TableBuilder/BlockBuilder

> table/table_builder.cc table/block_builder.cc
>
> 生成 block 的过程封装成 BlockBuilder 处理。生出 sstable 的过程封装成 TableBuilder 处理。

### Iterator

> include/leveldb/iterator.h
>
> leveldb 中对 key 的查找和遍历，上层统一使用 Iterator 的方式处理，屏蔽底层的处理，统一逻辑。提供 RegisterCleanup()可以在 Iterator 销毁时，做一些清理工作（比如释放 Iterator 持有句柄的引用）。

## 存储结构的格式定义与操作

### memtable

> db/skiplist.h db/memtable
>
> 类似 BigTable 的模式，数据在内存中以 memtable 形式存储。leveldb 的 memtable 实现没有使用复杂的 B-树系列，采用的是更轻量级的 skip list。全局看来，skip list 所有的 node 就是一个排序的链表，考虑到操作效率，为这一个链表再添加若干不同跨度的辅助链表，查找时通过辅助链表可以跳跃比较来加大查找的步进。每个链表上都是排序的 node，而每个 node 也可能同时处在多个链表上。将一个 node 所属链表的数量看作它的高度，那么，不同高度的 node 在查找时会获得不同跳跃跨度的查找优化，图 1 是一个最大高度为 5 的skiplist。
> 换个角度，如果 node 的高度具有随机性，数据集合从高度层次上看就有了散列性，也就等同于树的平衡。相对于其他树型数据结构采用不同策略来保证平衡状态，Skip list 仅保证新加入 node 的高度随机即可（当然也可以采用规划计算的方式确定高度，以获得平摊复杂度。leveldb 采用的是更简单的随机方式）
> 如前所述，作为随机性的数据机构，skip list 的算法复杂度依赖于我们的随机假设，复杂度为 O（logn）.
> 基于下面两个特点，skiplist 中的操作不需要任何锁或者 node 的引用计数：
>
> 1) skip list 中 node 内保存的是 InternalKey 与相应 value 组成的数据，SequnceNumber 的全局唯
>    一保证了不会有相同的 node 出现，也就保证了不会有 node 更新的情况。
> 2) delete 等同于 put 操作，所以不会需要引用计数记录 node 的存活周期。

- skiplist的操作

  - 写入

    - insert: 先找到不小于该 key 的 node（FindGreaterOrEqual（）），随机产生新 node 的高度，对各个高度的链表做 insert 即可。
    - delete: 先找到 node，并对其所在各个高度的链表做相应的更新。leveldb 中 delete 操作相当于 insert，skiplist 代码中并未实现。
  - 读取

    > skiplist 提供了 Iterator 的接口方式，供查找和遍历时使用
    >

    - Seek（）找到不小 key 的节点（FindGreaterOrEqual（））。从根节点开始，高度从高向低与 node的 key 比较，直到找到或者到达链表尾。
    - SeekToFirst()定位到头节点最低高度的 node 即可。
    - SeekToLast()从头节点的最高开始，依次前进，知道达到链表尾。
    - Next()/Prev()在最低高度的链表上做 next 或者 prev 即可
- memtable中的数据格式

  > - key_size(varint32)
  > - key_data(key_size)
  > - value_size(varint32)
  > - value_data(value_data)
  >
- memtable的操作

  - 写入（MemTable::Add()）

    - 将传入的 key 和 value dump 成 memtable 中存储的数据格式。
    - SkipList::Insert()。
  - 读取(MemTable::Get()）

    > memtable 对 key 的查找和遍历封装成 MemTableIterator。底层直接使用 SkipList 的类 Iterator
    > 接口。
    >

    - 从传入的 LookupKey 中取得 memtable 中存储的 key 格式。
    - 做 MemTableIterator::Seek()。
    - c. seek 失败，返回 data not exist。seek 成功，则判断数据的 ValueType
      - kTypeValue，返回对应的 value 数据。
      - kTypeDeletion，返回 data not exist。
  - 使用内存（Arena util/arena.cc）

    > memtable 有阈值的限制（write_buffer_size）,为了便于统计内存的使用，也为了内存使用效率，对 memtable 的内存使用实现了比较简单的 arena 管理（Arena）。
    >

    ```cpp
    class Arena {
    // ...
      // Allocation state  当前空闲内存 block 内的可用地址
      char* alloc_ptr_;
      size_t alloc_bytes_remaining_; //  当前空闲内存 block 内的可用大小

      // Array of new[] allocated memory blocks  已经申请的内存 block
      std::vector<char*> blocks_;

      // Total memory usage of the arena.
      //
      // TODO(costan): This member is accessed via atomics, but the others are
      //               accessed without any locking. Is this OK?
      std::atomic<size_t> memory_usage_;  // 累计分配的内存大小 一个 memtable 对应一个 Arena, memtable 内的数据量就用这个值表示
    };

    ```

    > Arena 每次按 kBlockSize(4096)单位向系统申请内存，提供地址对齐的内存，记录内存使用。当 memtable 申请内存时，如果 size 不大于 kBlockSize 的四分之一，就在当前空闲的内存 block 中分配，否则，直接向系统申请（malloc）。这个策略是为了能更好的服务小内存的申请，避免个别大内存使用影响。
    >

### block of sstable

> table/block_builder.cc table/block.cc
>
> sstable 中的数据以 block 单位存储，有利于 IO 和解析的粒度，结构如下:
>
> - entry0 entry1 ......
> - restarts(uint32*num_of_restarts)
> - num_of_restarts(uint32)
> - trailer
>
> 其中每个entry的结构如下：
>
> - shared_bytes(varint)
> - unshared_bytes(varint)
> - value_bytes(varint)
> - unshared_key_data(unshared_bytes)
> - value_data(value_bytes)
>
> trailer的结构如下：
>
> - type (char)
> - crc (uint32)

- entry: 一份 key-value 数据作为 block 内的一个 entry。考虑节约空间，leveldb 对 key 的存储进行前缀压缩，每个 entry 中会记录 key 与前一个 key 前缀相同的字节（shared_bytes）以及自己独有的字节（unshared_bytes）。读取时，对 block 进行遍历，每个 key 根据前一个 key 以及shared_bytes/unshared_bytes 可以构造出来
- restarts: 如果完全按照 1）中所述处理，对每个 key 的查找，就都要从 block 的头开始遍历，所以进一步细化粒度，对 block 内的前缀压缩分区段进行。若干个（Option::block_restart_interval）key 做前缀压缩之后，就重新开始下一轮。每一轮前缀压缩的 block offset 保存在 restarts 中，num_of_restarts 记录着总共压缩的轮数
- trailer：每个 block 后面都会有 5 个字节的 trailer。1 个字节的 type 表示 block 内的数据是否进行了压缩（比如使用了 snappy 压缩），4 个字节的 crc 记录 block 数据的校验码。

> 含 trailer。持久化时，offset/size 均采用 varint64 encode

block的操作：

- 写入（BlockBuilder::Add()/BlockBuilder::Finish()）

  > block 写入时,不会对 key 做排序的逻辑，因为 sstable 的产生是由 memtable dump 或者 compact时 merge 排序产生，key 的顺序上层已经保证
  >

  - 检查上一轮前缀压缩是否已经完成（达到 restart_interval）完成，则记录 restarts 点，重新开始新一轮。该 key 不做任何处理（shared_bytes = 0）未完成，计算该 key 与保存的上一个 key 的相同前缀，确定 unshared_bytes/shared_bytes
  - 将 key/value 以 block 内 entry 的数据格式，追加到该 block 上(内存中)
  - BlockBuilder::Finish()在一个 block 完成（达到设定的 block_size）时，将 restarts 点的集合和数量追加到 block 上
- 读取（ReadBlock() table/format.cc）

  > 有了一个 block 的 BlockHandle，即可定位到该 block 在 sstable 中的 offset 及 size,从而读取出具体的 block（ReadBlock()）
  >

  - 根据 BlockHandle，将 block 从 sstable 中读取出来（包含 trailer）。
  - 可选校验 trailer 中的 crc(get 时由 ReadOption:: verify_checksums 控制，compact 时由Option:: paranoid_checks 控制)。
  - 根据 trailer 中的 type，决定是否要解压数据。
  - 将数据封装成 Block（block.cc），解析出 restarts 集合以及数量。

  > 上层对 Block 进行 key 的查找和遍历，封装成 Block::Iter 处理。
  >

  - Seek()
    - restarts 集合记录着每轮前缀压缩开始的 entry 在 block 中的 offset（restart_point），可以认为是所有位于 restart_point 的 key 的集合，并且是排序的。所以，用 seek 的 key 在 restarts 集合中做二分查找，找到它属于的前缀压缩区间的开始 offset（restart_point），位于 restarts 内的下标记为 restart_index。
    - 根据 restar_point 定位到 block 中的 entry(SeekToRestartPoint())。
    - 根据 entry 的格式，依次遍历（ParseNextKey()），直到找到不小于 key 的 entry。中间需要同步更新当前处于的 restart_index
  - SeekToFirst()
    - 定位到 restart_index 为 0 的 entry（SeekToRestartPoint()）
  - SeekToLast()
    - 定位到最后一个 restart_index 的 entry，然后遍历完该前缀压缩区间的 entry，即定位到该block 的最后一个 key。
  - Next（）
    - 根据 entry 的格式，遍历下一个 entry （ParseNextKey()）
    - 根据前一个 value，获得下一个 entry 的 offset（NextEntryOffset()）
    - 解析 entry（DecodeEntry（））
    - 根据 shared_bytes/unshared_bytes 以及前一个 key，构造出当前 entry 中的 key/value。
    - 如果到了这一轮前缀压缩的结束，更新 restart_index
  - Prev（）
    - 找到上一个 entry 属于的 restart_index，或者前一个 （前一个 entry 是一轮前缀压缩的开始时）
    - 定位到 restart_index 对应的 entry(SeekToRestartPoint())
    - 从当前位置开始依次遍历，直到上一个 entry 的前一个
- cache 的处理 (BlockCache)

  - block 的 cache 如果用户未指定自己的实现，使用内部的 ShardLRUCache。cache 中的 key 为 block 所在 sstable 加入 TableCache 时获得的 cacheID 加上 block 在 sstable中的 offset，value 为 未压缩的 block 数据。
- 统一处理 cache 与 IO (Table::BlockReader() table/table.cc)

  - 处理 BlockCache 以及实际的 block IO 的逻辑由 Table::BlockReader()处理
  - 如果不存在 block cache，直接调用 ReadBlock(), 否则，根据传入的 BlockHandle，构造出BlockCache 的 key 进行 lookup
    - 存在，则返回 cache 中的 block 数据（Block）
    - 不存在，调用 ReadBlock()从磁盘上获得，同时插入 BlockCache
  - 根据得到的 Block，构造出 Block::Iter 返回。

### sstable

> table/table_bulder.cc/table.cc
>
> sstable 是 leveldb 中持久化数据的文件格式。整体来看，sstable 由数据(data)和元信息(meta/index)组成。数据和元信息统一以 block 单位存储（除了文件最末尾的 footer 元信息），读取时也采用统一的读取逻辑。整体的数据格式如下：
>
> - data_block0 data_block1 ........data_blockN
> - meta_block0....meta_blockN
> - metaindex_block
> - index_block
> - footer
>
> footer的结构：
>
> - metaindex_block_handle
> - index_block_handle
> - padding_bytes magic(uint64)

- 构成介绍：

  - data_block: 实际存储的 kv 数据
  - meta_block：每个 data_block 对应一个 meta_block ，保存 data_block 中的 key size/valuesize/kv counts 之类的统计信息，当前版本未实现。
  - metaindex_block: 保存 meta_block 的索引信息。当前版本未实现
  - index_block : 保存每个 data_block 的 last_key 及其在 sstable 文件中的索引。block 中entry 的 key 即是 last_key(依赖于 FindShortestSeparator()/FindShortSuccessor()的实现)，value 即是该 data_block 的 BlockHandler（offset/size）
  - footer: 文件末尾的固定长度的数据。保存着 metaindex_block 和 index_block 的索引信息(BlockHandler),为达到固定的长度，添加 padding_bytes。最后有 8 个字节的 magic 校验。
- sstable的操作

  - 写入（TableBuilder::Add() TableBuilder::Finish()）
    - 同 sstable 中 block 的写入一样，不需要关心排序。
    - 如果是一个新 block 的开始，计算出上一个 block 的 end-key（Comparator::FindShortestSeparator()），连同 BlockHandle 添加到 index_block 中。考虑到 index_block 会 load 进内存，为了节约 index_block 中保存的 index 信息（每个block 对应的 end-key/offset/size），leveldb 中并没有直接使用 block 最后一个 key 做为它的 end-key，而是使用 Comparator::FindShortestSeparator（）得到。默认实现是将大于上一个 block 最后一个 key，但小于下一个 block 第一个 key 的最小 key 作为上一个 block的 end-key。用户可以实现自己的 Comparator 来控制这个策略
    - 将 key/value 加入当前 data_block（BlockBuilder::Add()）。
    - 如果当前 data_block 达到设定的 Option::block_size，将 data_block 写入磁盘(BlockBuilder::WriteBlock()）
    - BlockBuilder::Finish()。
    - 对 block 的数据做可选的压缩（snppy），append 到 sstable 文件。
    - 添加该 block 的 trailer（type/crc），append 到 sstable 文件
    - 记录该 block 的 BlockHandle。
    - TableBuilder::Finish()是在 sstable 完成时（dump memtable 完成或者达到kTargetFileSize）做的处理。
      - 将 meta_index_block 写入磁盘（当前未实现 meta_index_block 逻辑，meta_index_block没有任何数据）。
      - 计算最后一个 block 的 end-key（Comparator:: FindShortSuccessor()），连同其BlockHandle 添加到 index_block 中。
      - 将 index_block 写入磁盘
      - 构造 footer，作为最后部分写入 sstable
  - 读取(Table::Open() table/table.cc TwoLevelIterator table/two_level_iterator.cc)
    - 一个 sstable 需要 IO 时首先 open(Table::Open())
    - 根据传入的 sstable size（Version::files_保存的 FileMetaData），首先读取文件末尾的footer。
    - 解析 footer 数据(Footer::DecodeFrom() table/format.cc)，校验 magic,获得 index_block和 metaindex_block 的 BlockHandle.
    - 根据 index_block 的 BlockHandle，读取 index_block(ReadBlock() table/format.cc)。
    - 分配 cacheID(ShardedLRUCache::NewId(), util/cache.cc)。
    - 封装成 Table（调用者会将其加入 table cache， TableCache::NewIterator（））
    - 对 sstable 进行 key 的查找遍历封装成 TwoLevelIterator(参见 Iterator)处理
  - cache 的处理 （TableCache db/table_cache.cc）
    - 加快 block 的定位，对 sstable 的元信息做了 cache(TableCache),使用 ShardLRUCache。cache 的 key 为 sstable 的 FileNumber，value 是封装了元信息的 Table 句柄。每当新加入TableCache 时，会获得一个全局唯一 cacheId
    - 当 compact 完成，删除 sstable 文件的同时，会从 TableCache 中将其对应的 entry 清除。而属于该 sstable 的 BlockCache 可能有多个，需要遍历 BlockCache 才能得到（或者构造 sstable 中所有 block 的 BlockCache 的 key 做查询），所以基于效率考虑，BlockCache 中属于该 sstable的 block 缓存 entry 并不做处理，由 BlockCache 的 LRU 逻辑自行清除
  - 统一处理 cache 与 IO （TableCache::NewIterator（）db/table_cache.cc）
    - 处理 table cache 和实际 sstable IO 的逻辑由 TableCache::NewIterator（）控制
    - 构造 table cache 中的 key（FileNumber）,对 TableCache 做 Lookup，若存在，则直接获得对应的 Table。若不存在，则根据 FileNumber 构造出 sstable 的具体路径，Table::Open()，得到具体的 Table,并插入 TableCache
    - 返回 sstable 的 Iterator（Table::NewIterator()，TwoLevelIterator）。上层对 sstable 进行 key 的查找遍历都是用 TableCache::NewIterator（）获得 sstable 的Iterator,然后做后续操作，无需关心 cache 相关逻辑。

### block of log

> （db/log_format.h db/log_writer.cc db/log_reader.cc
>
> log 文件中的数据也是以 block 为单位组织。写日志时，一致性考虑，并没有按 block 单位写，每次更新均对 log 文件进行 IO，根据 WriteOption::sync 决定是否做强制 sync，读取时以 block 为单位做 IO 以及校验。

block的结构：

- record0 record1 ..... recordN
- trailer

record的组成：

- checksum (uint32)
- length (uint16)
- type (uint8)
- data (length)

1. record：每次更新写入作为一个 record。
2. checksum 记录的是 type 和 data 的 crc 校验。
3. length 是 record 内保存的 data 长度（little-endian）。
4. 为了避免 block 内部碎片的产生，一份 record 可能会跨 block，所以根据 record 内保存数据占更新写入数据的完整与否，当前分为 4 种 type：FULL，FIRST，MIDDLE，LAST, LAST，依次表示record 内保存的是完整数据的全部，开始，中间或者最后部分。
5. data 即是保存的数据。
6. trailer：如果 block 最后剩余的部分小于 record 的头长度(checksum/length/type 共 7bytes),则剩余的部分作为 block 的 trailer，填 0 不使用，record 写入下一个 block

log 的写入是顺序写，读取只会在启动时发生，不会是性能的瓶颈（每次写都 sync 会有影响），log中的数据也就没有进行压缩处理

### log

> db/log_format.h db/log_writer.cc db/log_reader.cc
>
> log格式：
>
> - init_data
> - block1 block2 ....blockN
>
> 1. init_data: log 文件开头可以添加一些信息，读取写入的时候，跳过这些数据。当前版本只在log_reader 中支持，log_writer 中并没有相关逻辑，所以当前 init_data 为空。
>
> 2) block:实际的数据。binlog 以及 MANIFEST 文件都使用了这种 log 的格式

log的操作：

- 写入（Writer::AddRecord() log_writer.cc）
  - 对 log 的每次写入作为 record 添加。
    - 如果当前 block 剩余的 size 小于 record 头长度，填充 trailer，开始下一个 block。
    - 根据当前 block 剩余的 size 和写入 size，划分出满足写入的最大 record，确定 record type。
    - 写入 record（Writer::EmitPhysicalRecord()）
      - 构造 record 头（checksum/size/type）
      - 追加写入 log 文件
    - 循环 a-c,直至写入处理完成。
    - 根据 option 指定的 sync 决定是否做 log 文件的强制 sync
- 读取（Reader::ReadRecord() db/log_reader.cc）
  - log 的读取仅发生在 db 启动的时候，每次读取出当时写入的一次完整更新。
    - 第一次读取，根据指定的 initial_offset_跳过 log 文件开始的init_data(Reader::SkipToInitialBlock())，如果从跳过的 offset 开始，当前 block 剩余的 size 小于 record 的头长度（是个 trailer），则直接跳过这个 block。当前实现中指定的initial_offset_为 0。
    - 从 log 文件中读一个 record（Reader::ReadPhysicalRecord()）.
      - 如果第一次读取或者当前 block 已经解析完成，从 log 文件中读取一个 block 的数据。
      - 从当前 block 解析到的 offset 开始，解析 reocord 头，根据选项决定是否校验 crc（当前一定校验），进而解析出完整的 record。
    - 根据读到 record 的 type 做进一步处理
      - kFullType，则直接返回。
      - kFirstType/ kMiddleType，保存读到的数据。
      - kLastType，与前面已经读到的数据合并，直接返回。
    - 非法的 type，返回错误。非法的 type 是指在读取 log 文件或者解析 block 数据时发生错误，诸如 block 中剩余的 size 不满足 record 头中的data_size，数据从 initial_offset开始，未完成完整的解析，log 文件就已经结束（eof）了之类。
    - 循环直至读取出当时写入的一个完整更新。

### cache

> util/cache.cc
>
> leveldb 中支持用户自己实现 block cache 逻辑，作为 option 传入。默认使用的是内部实现的 LRU。简单以及效率考虑，leveldb 中实现了一个简单的 hash table（HashHandle），采用定长数组存放node，链表解决 hash 冲突，每次 insert 后，如果 node 数量大于数组的容量（期望短的冲突链表长度），就将容量扩大 2 倍，做一次 rehash。LRU 的逻辑由 LRUCache 控制，insert 和 lookup 时更新链表即可。为了加速查找和减少冲突，又将 LRUCache 再做 shard（ShardedLRUCache）。整体来看，上层使用 cache 时，首先根据 key 做 shard，然后在 LRUCache 层对 HashHandle 做数据的操作，最后处理 lru 逻辑。

### snapshot

> include/leveldb/snapshot.h
>
> 依赖于 SequnceNumber 来标识时间点，leveldb 中 Snapshot 的实现很简单，只需要记录产生Snapshot 时的 SequnceNumber 即可，所有 Snapshot 用 double-linked list 组织，新加入的添加在列表头

### Iterator

> include/leveldb/iterator.h
>
> leveldb 中 key 的查找遍历，存储层面之上统一通过 Iterator 的方式处理。存储结构（memtable/sstable/block）都提供对应的 Iterator，另外还有为操作方便封装的特殊 Iterator

- memtable 的 Iterator（MemTableIterator db/memtable.cc）参看 memtable
- sstable 的 Iterator（TwoLevelIterator）。sstable 的 Iterator 使用 TwoLevelIterator，参看 TwoLevelIterator
- block of sstable 的 Iterator（Block::Iter table/block.cc）参看 block of sstable
- 非 level-0 的 sstable 元信息集合的 Iterator（LevelFileNumIterator db/version_set.cc）level-0 中的 sstable 可能存在 overlap，处理时每个 sstable 单独处理即可。非 level-0 的sstable 集合不会有 overlap，且 key-range 是排序的（Version::files_[level]），在非level-0 上进行 key 的查找遍历，可以根据排序的 FileMetaData 集合加速定位到 key 所在的sstable(FileMetaData)，将其封装成 LevelFileNumIterator

  ```cpp
  class Version::LevelFileNumIterator : public Iterator {
  // ...
   private:
    const InternalKeyComparator icmp_;// 对 key 做比较的 comparator
    const std::vector<FileMetaData*>* const flist_;// 当前 level 中 FileMetaData 集合，构造时取 Version::files_[]中的一项即可，其中的 FileMetaData 已经按照 sstable（FileMetaData）的 smallest 排序

    uint32_t index_;// 当前定位到的 sstable(FileMetaData)在 flist_中的 index

    // Backing store for value().  Holds the file number and size.
    mutable char value_buf_[16];// 保存上次取 Value（）时的实际数据，供 Slice 包装返回。这是为了避免每次 Value（）都要分配内存。

  };
  ```
- 相关操作

  - Seek()用要 Seek 的 key 在 flist_中做二分查找（与 FileMetaData::largest key 比较），可以定位到 key 所在 sstable 的元信息（FileMetaData）（FindFile()）
    - SeekToFirst（）/SeekToLast()定位到 flist_的开始/结束。
    - Next（）/Prev() flist_前进/后退一个
    - Key() 当前 FileMetaData 的 largest key
    - Value()将当前 FileMeta 的 filenumber 与 filesize encode 到 value_buf_,返回 value_buf_
- TwoLevelIterator(table/two_level_iterator.cc)对于类似 index ==> data 这种需要定位 index，然后根据 index 定位到具体 data 的使用方式，leveldb 封装成 TwoLevelIterator 使用。TwoLevelIterator 封装了 index Iterator (index_iter)，和根据 index 中的信息可以返回 dataIterator（data_iter）的 hook 函数。index_iter 以及 data_iter 需要支持同一个 key 的 seek。

  ```cpp
  class TwoLevelIterator : public Iterator {
  // ...
    BlockFunction block_function_;// 根据 index_value(index_iter->Value(), data 对应的 index 信息) 可以返回对应 data Iterator 的 hook
    void* arg_;// block_function_的参数
    const ReadOptions options_;// 传入的 option
    Status status_;// 记录过程中的 status
    IteratorWrapper index_iter_;// index 的 Iterator,根据 key 可以 Seek()到 key 所在 data 的元信息。
    IteratorWrapper data_iter_;  // data 的 Iterator，根据 key 可以 Seek()到 key 在 data 中的位置，进而获得对应的 value.
    // If data_iter_ is non-null, then "data_block_handle_" holds the
    // "index_value" passed to block_function_ to create the data_iter_.
    std::string data_block_handle_;//保存 index_value(data 的 index 信息
  };
  ```

  - Seek（）
    - index_iter->Seek（）,得到 index_iter->Value(),即 key 所在 data 的 index 信息data_block_handle_
    - InitDataBlock(),根据 index_block_handle，调用 hook 函数，获得对应 data 的data_iter
    - data_iter->Seek()。定位到要找的 key
    - SkipEmptyDataBlocksForward（）。如果获得的 data_iter 是无效，那么需要不断尝试下一个 data 并定位到其最开始(已经满足 Seek 条件)，直到找到合法的 data。（index_iter->Next()/InitDataBlock()/data_iter->SeekToFirst()）
  - SeekToFirst()/SeekToLast（）类似 Seek（），index_iter/data_iter 均做 SeekToFirst()/SeekToLast（）即可。最后同样 SkipEmptyDataBlocksForward
  - Next()/Prev()直接调用 data_iter->Next()/Prev（）即可。SkipEmptyDataBlocksForward（）
  - Key()/Value()即 data_iter->Key()/iter->Value()
  - TwoLevelIterator 的设计封装非常巧妙，使用在了两个地方
    - 作为 sstable 的 iterator 对 sstable 进行 key 的查找遍历。index_iter 为该 sstable 中的index_block_iter(Block::Iter), hook 函数为Table::BlockReader()(table/table.cc)。Seek 时，index_iter 做 Seek 得到 key 所在 block 的 index 信息（BlockHandle），BlockReader 根据 BlockHandle 获得具体的 block 数据，封装成 Block::Iter 作为 data_iter返回, data_iter 做 Seek 即可在 block 中定位到 key，进而得到对应的数据
    - 作为非 level-0 中 sstable 集合的 iterator 进行 key 的查找遍历。当遍历 db 或者 compact时，非 level-0 中已经排序的 sstable 集合封装成 TwoLevelIterator。index_iter 为LevelFileNumIterator, hook 函数为 GetFileIterator（）(db/version_set.cc)。Seek 时，LevelFileNumIterator 作为 index_iter 做 Seek，可以得到 key 所在 sstable 的index 信息（FileMetaData），GetFileIterator 根据 FileMeta，获得具体的 sstable 句柄（Table），封装成 sstable 的 iterator（也就是上面 a 使用的 TwoLevelIterator）作为data_iter 返回，data_iter 做 Seek 即可定位到 key，进而得到对应的数据
- IteratorWrapper (table/iterator_wrapper.h)IteratorWrapper 提供了稍作优化的 Iterator 包装，它会保存每次 Key（）/Valid()的值，从而避免每次调用 Iterator 接口产生的 virtural function 调用。另外，若频繁调用时，直接使用保存的值，比每次计算能有更好的 cpu cache locality。
- MergingIterator（table/merge.cc）MergingIterator 内部包含多个 Iterator 的集合（children_）,每个操作，对 children_中每个Iterator 做同样操作之后按逻辑取边界的值即可，负责边界值的 iterator 置为 current_。

  ```cpp
  class MergingIterator : public Iterator {
  // ...
    // We might want to use a heap in case there are lots of children.
    // For now we use a simple array since we expect a very small number
    // of children in leveldb.
    const Comparator* comparator_;// 对 key 做比较的 comparator
    IteratorWrapper* children_;// 包含的所有 Iterator， 这里采用简单的数组保存, 每次比较其中的当前值，会有 O(n)的遍历开销，n_较小时可以容忍。
    int n_;// children_中 Iterator 的数量
    IteratorWrapper* current_;// 当前定位到的 Iterator
    // 因为有多个 Iterator 存在，需要记录前一次做的是何种方向的操作,
    // 判断这一次操作的方向是否和前一次一致，来做不同的处理。
    // 比如，如果做了 Next()，current_定位到的一定是 children_中满足条件最小的，
    // 其他的 Iterator 已经定位到大于当前 key
    // 的位置(除非 Iterator 已经 end），这是，继续做 Next（），只需要
    // current_->Next()，然后在 children_中选出大于当前 key 且最小的即可.
    // 但如果做 Prev(),其他的 Iterator
    // 可能位于大于当前 key 的位置，所以必须先让所有的 Iterator 都定位到小于
    // 当前 key 的位置(Ierator 中不存在 key，就 SeekToLast()),然后选出小于
    // 当前 key 且最大的。
    Direction direction_;
  };
  ```

  - Seek（）children_中的所有 iterator 均做一次 seek，然后找到 Valid（）中最小的（FindSmallest()）
  - SeekToFirst()children_中的所有 iterator 均做一次 SeekToFirst()，FindSmallest()
  - SeekToLast() children_中的所有 iterator 均做一次 SeekToLast()，然后找到 Valid()中最大的（FindLargest()）
  - Next（）
    - 如果前一次的操作也是 Next()(direction_ == kForward),只需要 current_->Next（）,然后返回 FindSmallest().
    - 否则，对 children_中非 current_的 iterator 均做当前 key 的 Seek（），Seek 到则做相应的 Prev（），否则做 SeekToFirst（），这样保证除了不存在 key 的 Iterator，其他都处于大于当前 key 的下一个位置。
    - current_->Next()
    - FindSmallest()
  - Prev（）
    - 如果前一次的操作也是 Prev()(direction_ == kReverse),只需要 current_->Prev（）,然后返回 FindLargest().
    - 否则，对 children_中非 current_的 iterator 均做当前 key 的 Seek（），Seek 到则做相应的 Prev（），否则做 SeekToLast（），这样保证除了不存在 key 的 Iterator，其他都处于小于当前 key 的前一个位置。
    - current_->Prev()
    - FindLargest()
- 遍历 db 的 Iterator（DBIter db/db_iter.cc）对 db 遍历时，封装成 DBIter(NewDBIterator() db/db_iter.cc)

  - 整个 db 内部的 Iterator（DBImpl::NewInternalIterator（））

    - 获得 memtable 的 iterator（Memtable::NewIterator(), MemTableIterator）
    - 获得 immutable memtable 的 iterator(Memtable::NewIterator(), MemTableIterator)
    - 获得所有 sstable 的 Iterator(Version::AddIterators())
      - level-0 中所有 sstable 的 iterator (TableCache::NewIterator(),作为单个sstable iterator 的 TwoLevelIterator)
      - 每个非 level-0 的 leve 上 sstable 集合iterator(（VersionSet::NewConcatenatingIterator(), 作为 sstable 集合iterator 的 TwoLevelIterator）
    - 把获得的所有 Iterator 作为 children iterator 构造出 MergingIterator
  - 如果指定 Snapshot，将 SnapShot 的 SequnceNumber 作为最大值，否则将VersionSet::last_sequnce_作为最大值
  - 构造 DBIter。

    - ```cpp
      class DBIter : public Iterator {
      // ...
        DBImpl* db_;
        const Comparator* const user_comparator_;// 因为这是提供给使用者的 Iterator,需要对 user-key 进行比较验证，需要 user_comparator
        Iterator* const iter_; // DBImpl::NewInternalIterator()获得的封装整个 db Iterator 的 MergingIterator
        SequenceNumber const sequence_;// 通过 SequnceNumber 的比较来控制遍历数据的时间点。 如果指定了 Snapshot，则赋值为 Snapshot::sequncenumber, 只遍历出 Snapshot 确定之前的数据;否则赋值为 VersionSet::last_sequnce_number_,遍历出当前 db 中所有的数据
        Status status_;// 遍历过程中的 status
        // 遍历时需要跳过相同和删除的 key，反向遍历为了处理这个逻辑，操作完成时， iter_定位到的会是当前 key 的前一个位置，所以需要保存过程中获得的当前 key/value。 参见 FindPrevUserEntry()。
        std::string saved_key_;    // == current key when direction_==kReverse
        std::string saved_value_;  // == current raw value when direction_==kReverse
        Direction direction_;// 前一次遍历的方向，参见 MergingIterator
        bool valid_;// 标识是否遍历完成。
        Random rnd_;
        size_t bytes_until_read_sampling_;
      };
      ```
  - 存储层的 Iterator（iter_）不关心实际的数据，只需要做遍历，DBIter 是提供给用户的最外层Iterator，返回对应的 kv 数据，需要做逻辑上的解析，比如，遍历到相同或者删除的 key 要跳过，如果指定了 Snapshot，要跳过不属于 Snapshot 的数据等DBIter::FindNextUserEntry()/DBIter::FindPrevUserEntry处理这些解析逻辑。
  - - FindNextUserEntry()
      - 正向遍历，首次遇到的 key 就是 key 的最终状态（SequnceNumber 更大），处理简单
      - iter_->Next()直到 Key()不同于遍历最开始的 key
      - 解析 iter->Key()，判断是否可用（sequence <= DBIter::sequence_）
        - 是 kTypeValue，直接返回
        - 是 kTypeDeletion，说明该 key 已经删除,继续下去。
      - 此时，iter_->Key()/iter->Value()返回的就是遍历到的下一个合法的数据
    - FindPrevUserEntry（）
      - 反向遍历时，对于一个 key，最后遍历到的才是其最终状态，所以必须遍历到该 key 的前一个，才能确定该 key 已经全部处理过，并获得其最终状态。这时 iter_并不位于当前 key 的位置，所以需要 saved_key_/save_value_来保存当前的 key/value。
      - iter_->Prev()直到遍历到不同的 Key()，中间用 saved_key_/saved_value_保存已经遍历到的 key/value。
      - 解析 saved_key_，判断是否可用（sequence <= DBIter::sequence_）
        - 是 kTypeValue，则直接返回，saved_key_/saved_value_即是遍历到的key/value。
        - 是 kDeletion，说明该 key 已经删除，clear saved_key_/saved_value_,继续下去。
        - 其他的操作只需要底层 iterator 做相应操作，用 FindNextUserEntry()/FindPrevUserEntry()处理数据的判断逻辑即可。
    - Seek（）
      - iter_->Seek().
      - FindNextUserEntry().
    - SeekToFirst（）
      - iter_->SeekToFirst()
      - FindNextUserEntry（）
    - SeekToLast（)
      - iter_->SeekToLast()
      - FindPrevUserEntry（）
    - Next()
      - 如果与前一次操作 dirction_一致，直接 FindNextUserEntry()
      - 否则，前一次的 Next（）使 iter_定位在当前 key 上，先 iter->Prev()回退一个，然后再 FindPrevUserEntry()
    - Key()/Value（）
      - 如前所述，如果是正向遍历，就是 iter_->Key()中的 user-key 部分/iter_->Value()中的如果是反向遍历，则返回 saved_key/saved_value.

## 主要流程

### open

1. 基本检查
   1. 根据传入的 db 路径，对 LOCK 文件做 flock 来判断是否已经有 db 实例启动，一份数据同时只能有一个 db 实例操作
   2. 根据 option 内的 create_if_missing/error_if_exists，来确定当数据目录已经存在时要做的处理。
2. db 元信息检查 (VersionSet::recover())
   1. 从 CURRENT 文件中读取当前的 MANIFEST 文件
   2. 从 MANIFEST 文件中依次读取每个 record（VersionEdit::DecodeFrom）， 检查 Comparator是否一致，然后依次 replay
   3. 检查解析 MANIFEST 的最终状态中的基本的信息是否完整（log number, FileNumber,SequnceNumber），将其生效成 db 当前的状态。此时，整个 db 的各种元信息（FileNumber,SequnceNumber, 各 level 的文件数目，size，range，下一次 compact 的 start_key 等等）均 load 完成，db 恢复成上一次退出前的状态
3. 从 log 中恢复上一次可能丢失的数据（RecoverLogFile）
   1. 遍历 db 中的文件，根据已经获得的 db 元信息 LogNumber 和 PrevLogNumber,找到上一次未处理的 log 文件
   2. 遍历 log 文件中的 record（record 中的 data 即是 memtable 中的 data），重建 memtable。达到 memtable 阈值，就 dump 成 sstable。期间，用 record 中的 SequnceNumber 修正从MANIFEST 中读取的当前 SequnceNumber。
   3. 将最后的 memtable dump 成 sstable。
   4. 根据 log 文件的 FileNumber 和遍历 record 的 SequnceNumber 修正当前的 FileNumber 和SequnceNumber。
4. 生成新的 log 文件。更新 db 的元信息（VersionSet::LogAndApply()，生成最新的 MANIFEST 文件），删除无用文件（DeleteObsoleteFiles()）,尝试 compact（MaybeScheduleCompaction（））。
5. 启动完毕。

### put

> leveldb 中的写操作不是瓶颈，但可能出现过量写影响读的效率（比如 level-0 中文件过多，查找某个 key 可能会造成过量的 io），所以有一系列策略主动去限制写.

1. 将 key value 封装成 WriteBatch
2. 循环检查当前 db 状态，确定策略(DBImpl:: MakeRoomForWrite())：
   1. 如果当前 level-0 中的文件数目达到 kL0_SlowdownWritesTrigger 阈值，则 sleep 进行delay。该 delay 只会发生一次。
   2. 如果当前 memtable 的 size 未达到阈值 write_buffer_size，则允许这次写。
   3. 如果 memtable 已经达到阈值，但 immutable memtable 仍存在，则等待 compact 将其 dump完成
   4. 如果 level-0 中的文件数目达到 kL0_StopWritesTrigger 阈值，则等待 compact memtable 完成。
   5. 上述条件都不满足，则是 memtable 已经写满，并且 immutable memtable 不存在，则将当前memtable 置为 immutable memtable，生成新的 memtable 和 log file，主动触发 compact，允许该次写。
3. 设置 WriteBatch 的 SequnceNumber
4. 先将 WriteBatch 中的数据记 log(Log::AddRecord())
5. 将 WriteBatch 应用在 memtable 上。（WriteBatchInternal::InsertInto()）,即遍历 decode 出WriteBatch 中的 key/value/ValueType，根据 ValueType 对 memetable 进行 put/delete 操作。
6. 更新 SequnceNumber（last_sequnce + WriteBatch::count()）

### get

> 总体来说，get 即是找到 userkey 相同，并且 SequnceNumber 最大（最新）的数据。leveldb 支持对特定 Snapshot 的 get，只是简单的将 Snapshot 的 SequnceNumber 作为最大的 SequnceNumber 即可。

1. 如果 ReadOption 指定了 snapshot，则将指定 snapshot 的 SequnceNumber 作为最大SequnceNumber,否则，将当前最大 SequnceNumber（VersionSet::last_sequnce_number）作为最大值
2. 在 memtable 中查找(MemTable::Get())
3. 如果 memtable 中未找到，并且存在 immutable memtable，就在 immutable memtable 中查找(Memtable::Get())
4. 仍未找到，在 sstable 中查找(VersionSet::Get())。从 level-0 开始，每个 level 上依次进行查找，一旦找到，即返回。
   1. 首先找出 level 上可能包含 key 的 sstable.(key 包含在 FileMetaData 的[startest,largest]
      1. level-0 的查找只能顺序遍历 files_[0]。考虑到 level-0 中的 sstable 是 memtaledump 生成的，所以新生成的 sstable 一定比旧生成有更新的数据，同时 sstable 文件的FileNumber 是递增，所以，将从 level-0 中获得的 sstable(FileMetaData)按照FileNumber 排序（NewestFirst（） db/version_set.cc）,能够优化 level-0 中的查找。level-0 中可能会找到多个 sstable
      2. 非 level-0 中的查找，对 files_[]基于 FileMetaData::largest 做二分查找（FindFile（） db/version_set.cc）即可定位到 level 中可能包含 key 的 sstable。非 level-0上最多找到一个 sstable。
   2. 如果该 level 上没有找到可能的 sstable，跳过。否则，对要进行查找的 sstable 获得其Iterator（TableCache:: NewIterator()）,做 seek（）.
   3. seek 成功则检查有效性（GetValue() db/version_set.cc）也就是根据 ValueType 判断是否是有效的数据：
      1. kTypeValue，返回对应的 value 数据
      2. kTypeDeletion，返回 data not exist

### delete

> delete 相比于 put 操作，只在构造 WriteBatch 时，设置 ValueType 为 kTypeDeletion，其他流程和put 相同。

### snapshot

1. 取得当前的 SequnceNumber
2. 构造出 Snapshot，插入到已有链表中

### NewIterator

> 构造 DBIter，做 Seek()即可。参见 Iterator

### compact

> leveldb 中有且仅有一个后台进程（第一次 compact 触发时 create 出来）单独做 compact。当主线程主动触发 compact 时（MaybeScheduleCompaction()），做以下流程：

1. 如果 compact 已经运行或者 db 正在退出，直接返回
2. 检查当前的运行状态，确定是否需要进行 compact，如果需要，则触发后台调度 compact（Env::Schedule()），否则直接返回。
3. 做实际的 compact 逻辑（BackgroundCompaction()），完成后，再次主动触发 compact（主线程将任务入队列即返回，不会有递归栈溢出的问题）。

- 会主动触发 compact 的情况
  - db 启动时，恢复完毕，会主动触发 compact
  - 直接调用 compact 相关的函数，会把 compact 的 key-range 指定在 manual_compaction 中。
  - 每次进行写操作（put/delete）检查时（MakeRoomForWrite()），如果发现 memtable 已经写满并且没有 immutable memtable, 会将 memtable 置为 immutable memtable，生成新的memtable，同时触发 compact。
  - get 操作时，如果有超过一个 sstable 文件进行了 IO，会检查做 IO 的最后一个文件是否达到了 compact 的条件（allowed_seeks 用光），达到条件，则主动触发 compact
- 需要 compact 的运行状态
  - 如果存在 immutable memtable,将其 dump 成 sstable(DBImpl::CompactMemTable())，完成返回。
  - 如果存在外部触发的 compact，根据 manual_compaction 指定的 level/start_key/end_key,选出 Compaction(VersionSet::CompactRange())。为了避免外部指定的 key-range 过大，一次 compact 过多的 sstable 文件，manual_compaction 可能不会一次做完，所以有 done 来标识是否已经全部完成，tmp_storage 保存上一次 compact 到的 end-key，即下一次的 startkey。
    - ```cpp
        // Information for a manual compaction
        struct ManualCompaction {
          int level;
          bool done;
          const InternalKey* begin;  // null means beginning of key range
          const InternalKey* end;    // null means end of key range
          InternalKey tmp_storage;   // Used to keep track of compaction progress
        };
      ```
  - 根据 db 当前状态，选出 Compaction (VersionSet::PickCompaction())。
  - 如果不是 manual compact 并且选出的 sstable 都处于 level-n 且不会造成过多的GrandparentOverrlap（Compaction::IsTrivialMove()），简单处理，将这些 sstable 推到level-n+1，更新 db 元信息即可(VersionSet::LogAndApply())
  - 否则，根据确定出的 Compaction，做具体的 compact 处理（DoCompactionWork()），最后做异常情况的清理（CleanupCompaction()）
- 实际 compact 的流程
  - DBImpl::CompactMemTable() （db/db_impl.cc）
    - DBImpl::WriteLevel0Table()
      - 取得 memtable 的 Iterator（MemtableIterator）
      - 生成新的 sstable 文件，遍历 memtable，将每份数据写入 sstable（BuildTable()）
      - 为生成 sstable 选择合适的 level（VersionSet::PickLevelForMemTableOutput（）），记录 VersionEdit。
    - 更新当前的 lognumber，应用 VersionEdit，生效新的Version(VersionSet::LogAndApply()).
    - 删除废弃文件(DBImpl::DeleteObsoleteFiles()).
  - BuildTable() （db/builder.cc）
    - 生成新的 sstable
    - 遍历 memtable，写入 sstable（TableBuilder::add()），完成 sync
    - 记录 sstable 的 FileMetaData 信息。将新生成 sstable 加入 TableCache，作为文件正常的验证(TableCache::NewIterator()).
  - VersionSet::PickLevelForMemTableOutput（）(db/version_set.cc)
    - 对于 memtable dump 成的 sstable，考虑到 level-0 做 compact 的消耗最大（可能处理的文件最多）,所以期望尽量让 dump 出的 sstable 能够直接位于高的 level。同时，若处于过高的 level，如果对某些 rang 的 key 一直做更新，后续的 compact 又会好消耗很多，权衡考虑，设置了最大level 阈值 kMaxMemCompactLevel（当前为 2）
    - 如果新生成的 sstable 与 level-0 中的文件有 overlap，选 level-0.
    - 向上尝试不大于 kMaxMemCompactLevel 的 level，如果与 level 产生 overlap 即返回。
    - 对于不产生 overlap 的 level，同时考虑 kMaxGrandParentOverlapBytes 的阈值判断。
  - VersionSet::CompactRange() （db/version_set.cc）
    - 获得指定 level-n 中 key-range 符合[start-key, end-key]的sstable(Version::GetOverlappingInputs())
    - 避免一次 compact 过多的 sstable，控制一个 level 中参与 compact 的 sstable size 不大于MaxFileSizeForLevel(),当前是 kTargeFileSize(也就是只选取一个，略有偏小)
    - 取得需要的其他 sstable（VersionSet::SetupOtherInputs（））
      - 确定从 level-n 中获得的 sstable 的 key-range，然后获得与其有 overlap 的 level-n+1中的 sstable（Version::GetOverlappingInputs（））
      - 在不扩大已经获得的所有 sstable 的 key-range 的前提下，尝试添加 level-n 中 sstable。
      - 获得 grandparents_（参见 Compaction）。
      - 更新 level-n 中下一次要 compact 的 start-key（compact_pointer_）。
  - VersionSet::PickCompaction() （db/version_set.cc）
    - 确定需要 compact 的 level-n 以及对应的 sstable。相比由 seek 产生的不均衡（seek_compaction: file_to_compact != NULL）,更优先compact 由 sstable size/count 造成的不均衡（size_compaction: compaction_score > 1）
      - 如果 compaction_score_ > 1, 说明 compaction_level_上最不均衡，取位于compaction_level_，并且 start-key 大于该 level 的 compact_pointer_的第一个sstable。如果没有找到，就取该 level 上的第一个文件(compaction_level_ > 0,应该可以使用二分查找)。
      - 如果 compaction_score_ < 1 但 file_to_compact_存在,则取该 sstable。
      - 如果以上二者都不满足，说明 db 处于均衡状态，不需要 compact。
    - 如果 level-n 为 level-0，由于其中的 sstable 会有 overlap，取出在 level-0 中与确定compact 的 sstable 有 overlap 的文件
    - 取得需要的其他 sstable (SetupOtherInputs（）)
  - DBImpl::DoCompactionWork() (db/db_impl.cc)
    - 实际的 compact 过程就是对多个已经排序的 sstable 做一次 merge 排序，丢弃掉相同 key 以及删除的数据。
    - 将选出的 Compaction 中的 sstable，构造成MergingIterator(VersionSet::MakeInputIterator())
      - 对 level-0 的每个 sstable，构造出对应的 iterator：TwoLevelIterator（TableCache::NewIterator()）。
      - 对非 level-0 的 sstable 构造出 sstable 集合的 iterator：TwoLevelIterator(NewTwoLevelIterator())
      - 将这些 iterator 作为 children iterator 构造出 MergingIterator（NewMergingIterator()）
    - iterator->SeekToFirst()
    - 遍历 Next()
    - 检查并优先 compact 存在的 immutable memtable。
    - 如果当前与 grandparent 层产生 overlap 的 size 超过阈值（Compaction::ShouldStopBefore()），立即结束当前写入的 sstable（DBImpl::FinishCompactionOutputFile（）），停止遍历
    - 确定当前 key 的数据是否丢弃。
      - key 是与前面的 key 重复，丢弃。
      - key 是 删 除 （ 检 查 ValueType ） 并且该 key 不 位 于 指 定 的 Snapshot 内（检查SequnceNumber）并且 key 在 level-n+1 以上的的 level 中不存在（Compaction：：IsBaseLevelForKey（）），则丢弃。
    - 如 果 当 前 要 写 入 的 sstable 未生成，生成新的 sstable （ DBImpl::OpenCompactionOutputFile（））。将不丢弃的 key 数据写入（TableBulider::add()）。
    - 如果当前输出的 sstable size 达到阈值（ Compaction::MaxOutputFileSize() 即MaxFileSizeForLevel（）,当前统一为 kTargeFileSize）,结束输出的 sstable（DBImpl：：FinishCompactionOutputFile（））
    - 循环直至遍历完成或主动停止
    - 结束最后一个输出的 sstable（ DBImpl::FinishCompactionOutputFile（）
    - 更新 compact 的统计信息
    - 生效 compact 之后的状态。(DBImpl:: InstallCompactionResults())。
  - DBImpl::CleanupCompaction（） （db/db_impl.cc）
    - 如果上一次最后一个 sstable 未完成就异常结束，修复状态（TableBuilder:: Abandon()）
    - 将已经成功完成的 sstable FileNumber 从 pending_outputs_中去除。
  - DBImpl::DeleteObsoleteFiles（）
    - db 中当前 Version 的 sstable 均在 VersionSet::current_中，并发的读写造成会有多个 Version共存，VersionSet::dummy_versions_中有包含 current_所有正在服务的 Version。凡是正在服务的 Version 中的 sstable 文件都认为是 live 的。DeleteObsoleteFiles()删除非 live 的sstable 以及其他类型的废弃文件。
    - 取得 pending_output_中仍存在的以及 live 的 sstable 文件（VersinoSet::AddLiveFiles()，遍历 VersionSet::dummy_versions_即可），作为所有 live 的文件。
    - 遍历 db 目录下所有的文件名，删除非 live 的 sstable 以及废弃的其他类型文件。
      - log 文 件 保 留 大 于 VersionSet::log_number_ 以 及 辅 助 log 文 件（VersionSet::prev_log_number_）
      - Manifest 文件只保留当前的。
      - sstable 文件以及临时文件（repair 时会产生）只保留 live 的。
      - CURRENT/LOG/LOCK 文件均保留。
  - DBImpl::FinishCompactionOutputFile() (db/db_impl.cc)
    - BlockBuilder::Finish().
    - 记录统计信息
    - sync 文件，将新生的 sstable 加入 TableCache((TableCache::NewIterator())作为文件正常的验证。
  - DBImpl::InstallCompactionResults()(db/db_impl.cc)
    - 将 compact 过程中记录的操作（VersionEdit）生效。
    - 将 compact 的 input sstable 置为删除（Compaction::AddInputDeletions()），生成的output sstable 置为 leve-n+1 中要加入的文件
    - 应用 VersionEdit（VersionSet::LogAndApply()）。
    - 生效成功则释放对前一个 Version 的引用，DeleteObsoleteFiles（）；否则，删除compact 过程生成的 sstable。
  - VersionSet::LogAndApply()
    - 以当前 Version 为基准构造新的 Version，VersionSet::Builder 将 VersionEdit 应用在新Version 上，最后将新 Version 生效成 VersionSet::current_。
    - 更新 VersionSet 中的元信息（VersionSet::Builder::Apply()）
      - 根据 VersionEdit::compact_pointers_更新 VersionSet::compact_pointers_
      - 将对应 level 新生成的 sstableg 构造出 元信息数据（ FileMetadata ）， 计 算allowed_seeks
    - 更新 Version::files_[level]（VersionSet::Builder::SaveTo（））。Version::files_[level] 中保存的是每个 level 上所有的 FileMetaData，并且按照keyrange 排序。VersionSet::Builder::level_[level]中保存的是每个 level 中需要添加或者删除的 file。VersionSet::Builder::level_[level].added_files/level_[level.deleted_files 对Version::files_[level]做 merge，清除已经删除的，加入新生成的。
    - 计 算 Version 内 的 均 衡 状 态 参 数 : compaction_score_ ，compaction_level_( VersionSet::Finalize)。
    - 写 MANIFEST 文件。如果是第一次写，则将当前的 Version 状态作为 Snapshot 先写入（WriteSnapshot()），否则，将这次的操作 VersionEdit encode 写入 MANIFEST
    - 如果是新生成 MANIFEST，更新 CURRENT 文件中记录的 MANIFEST 文件名。
    - 上述步骤均成功，则生效最新的 version（VersionSet::AppendVersion（））
      - unref cunrrent_
      - current_置为新的 version
      - 插入 dummy_versions_
    - 否则，将新生成 MANIFEST 删除（原来如果是已经存在了 MANIFEST 文件，这里不会删除。但可能会出现 corrupted MANIFEST，依靠 RepairDB 来处理）。

## 总结

1. 设计/实现中的优化
   针对持久化存储避不开的磁盘随机 IO 问题，leveldb 将写操作转化成内存操作 memtable 和顺序的
   binlog IO。分 level 管理 sstable，并用 compact 操作来均衡整个 db，以达到最优的读效率。
   具体实现时，做了很多细节的优化，来达到更少的 IO，更快的查找以及更优的读写效率。

1) 可以减少数据量以及 IO 的细节：
   a． 支持数据压缩存储（snappy）
   b． 对数字类型采用变长编码
   c． 对 key 进行前缀压缩
   d． 确定每个 sstable block 的 end-key 时，并不直接使用保存的最后一个 key，而是采用
   FindShortestSeparator（）/ FindShortSuccessor（）。
   e． 内部存储的 key 仅添加 SequnceNumber(uint64)。
   f． sstable 元信息以及 block 数据都有 cache。
   g． log 文件/manifest 文件采用相同的存储格式，都以 log::block 为单位。
2) 可以加速 key 定位的细节。
   a． memtable 使用 skiplist，提供 O(logn)的查找复杂度。
   b． 分 level 管理 sstable，对于非 level-0，sstable 不存在 overlap，所以查找时最多处理一个
   sstable。
   c． 内存中有每个 level 的 sstable 元信息（VersionSet::files_[]），非 level-0 的元信息集合根
   据 sstable 的 smallest-key 排序，定位 key 时，可以做二分查找加速定位。
   d． sstable 中所有 block 的元信息(index_block)根据每个 block 的 end-key 排序，定位 key 所在
   的 block 时，可以做二分查找。
   e． block 中标识每个前缀压缩区间开始 offset（restart_point）的 restarts 集合，可以看作是
   block 中所有 restart_point 处的 key 的集合，显然它们是排序的，所以定位 key 所在的前缀压
   缩区间时，可以对 restarts 做二分查找。
   3） 均衡读写效率的细节.
   a. level-0 上 sstable 数量的阈值检查来主动限制写的速率，以避免过多 level-0 sstable 文件影响读效率。
   b. 为均衡读写效率，设计 compact 的策略,使 db 处于均衡的状态.
   a) 尽量减少 level-0 的 sstable，dump memtable 时尽可能的直接将生成的 sstable 推向
   高 level。
   b) 避免生成过多与 level-n+2 overlap 的 level-n+1 上的 sstable
   (kMaxGrandParentOverlapBytes)。
   c) 控制每个 level 上的 sstable 数量/size，设计 compact 权值（compaction_score）作
   为 compact 的选取标准。
   d) 细化 sstable 文件的 IO 控制（allowed_seeks）,主动 compact（file_to_compact）避
   免坏情况的发生。
   e) 均匀的 compact 每个 level，将这一次 compact 的 end-key 作为下一次 compact 的
   start-key(compact_pointer)。
   4） 其他的一些优雅封装
   a． SequnceNumber 解决了数据的时间点
   b． ValueType 将数据更新(put/delete)统一处理逻辑。
   c． 对 key 的查找遍历统一使用 Iterator 方式处理。复合 Iterator 简化了逻辑上的处理。
   d． Ref/Unref/RegisterCleanup，Ref/Unref 消除使用者的内存释放逻辑，而 Iterator 的
   RegisterCleanup 在 Iterator 销毁时做注册的 hook， 二者结合，简化了对结构的遍历使
   用。
   e． db 中的文件按找规则生成，FileNumber 不仅简化了如何定位文件路径，还可以表示出文件
   的创建时间先后（compact 时，找到需要 compact 的 level-0 中的文件会根据时间顺序排
   序）。
   f． sstable 格式定义中，data 与 index 使用同样的 block 格式，统一了处理逻辑。
   g． 将对当前 db 状态的修改封装成 VesrionEdit，一次 apply。
   h． log 格式，以 block 为单位，IO 友好。block 内分 record，利于解析.
   i． manifest 文件中只保存一次全量状态，后续仅保存每次的修改，减少 IO

### 可以做的优化

1. memtable/sstable 的阈值 size，level-0 中的数量阈值，每个非 level-0 上的总数据量阈值等参数，
   均会影响到 compact 的运行，从而影响到最终的读写效率，根据不同场景需要做不同的配置，以达
   到最优效果。
7) 内部策略是基于 sas 盘的 IO 性能设计，使用其他硬件存储（ssd）时，需要做相应调整。
8) 查找不存在的 key 一个最坏情况，考虑不同的场景采用写入 mock value 或者加入 bloom filter 进
   行优化。
9) db 启动后，会将当前的状态写入 manifest 文件，后面每次 compact 后，会将产生的操作
   （VersionEdit）作为追加到 manifest。如果 db 实例运行时间很长，mainifest 中会有大量的更新
   记录，当 db 重启时，replay manifest 时可能会耗费较长的时间。考虑限制单个 manifest 中的
   record 数量，达到阈值，则做一次 rotate。重启时，仅 replay 最新的 manifest 即可，做异常情况
   的 repair 时,则可以连同历史 manifest 一起 replay。
10) leveldb 中除了 memtable 使用内存有 Arena 管理外，没有其他的内存管理，内部处理中有很多小对
    象的申请释放，考虑是否可以优化内存使用，当前可以使用 tcmalloc。
11) compact 时，选取非 level-0 中符合对应 compact_pointer 的 sstable 时，可以使用二分查找定位。
