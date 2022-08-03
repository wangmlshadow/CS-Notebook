#include <cassert>
#include <string>
#include <iostream>
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "leveldb/comparator.h"
#include "leveldb/export.h"
#include "leveldb/cache.h"

int main1(int argc, char **argv) {
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

int main2(int argc, char** argv) {
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


int main3(int argc, char** argv) {
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


int main4(int argc, char** argv) {
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status s = leveldb::DB::Open(options, "./testdb", &db);
    if (!s.ok()) { 
        std::cout << "open failed" << std::endl;
    } else {
        std::cout << "opened successfully" << std::endl;
    }

    leveldb::WriteOptions write_options;
    write_options.sync = true;

    //std::string key1 = "key1", value1 = "value1";
    std::string key2 = "key2", value2 = "value2";
    std::string key3 = "key3", value3 = "value3";
    s = db->Put(write_options, key2, value2);
    if (!s.ok()) {
        std::cout << "write error: " << s.ToString() << std::endl;
    }
    s = db->Put(write_options, key3, value3);
    if (!s.ok()) {
        std::cout << "write error: " << s.ToString() << std::endl;
    }
}


int main5(int argc, char** argv) {
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



int main6(int argc, char** argv) {
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


int main7(int argc, char** argv) {
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


// Comparator
// define a proper subclass of leveldb::Comparator that expresses your rule.
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




// Performance
// Performance can be tuned by changing the default values of the types defined in include/options.h

// Block size
// leveldb groups adjacent keys together into the same block and such a block is the unit of transfer to and from persistent storage. The default block size is approximately 4096 uncompressed bytes. Applications that mostly do bulk scans over the contents of the database may wish to increase this size. Applications that do a lot of point reads of small values may wish to switch to a smaller block size if performance measurements indicate an improvement. There isn't much benefit in using blocks smaller than one kilobyte, or larger than a few megabytes. Also note that compression will be more effective with larger block sizes.

// Cache
// The contents of the database are stored in a set of files in the filesystem and each file
// stores a sequence of compressed blocks. If options.block_cache is non-NULL, it is used to
// cache frequently used uncompressed block contents.
int main8(int argc, char** argv) { 
    leveldb::Options options;
    options.block_cache = leveldb::NewLRUCache(100 * 1024 * 1024);  // 100 MB   cache sizes
    leveldb::DB* db;
    leveldb::DB::Open(options, "./testdb", &db);
    // ...
    delete db;
    delete options.block_cache;
}





