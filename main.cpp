#include<iostream>
#include<thread>
#include<mutex>
#include<atomic>

#include "skipList.h"
#include "writeAheadLog.h"
#include "utility.h"

using namespace std;

const int MAX_SIZE = 10; 
mutex memtableAndSnapshotSave;
mutex compactionAndSnapshotSave;
mutex searchLock;

// process 1 - memtable
// process 2 - snapshot saving 
// --------------------------------------------
// write - insert, delete, modify
// read - search in database

atomic<int> process(0);

// long logging(WriteAheadLog& wal, int data, string operation){
//     if(process.load() == 2){
//         long key = wal.writeToLog(data, "SNAPSHOT " + operation)
//     } 
// }

// write to memtable thread
void writeToMemTable(string operation, int data, SkipList& sl, WriteAheadLog& wal){
    lock_guard<mutex> guard(memtableAndSnapshotSave); // lightweight RAII lock, automatically acquires the lock and unlocks once out of scope.
    process.store(1);
    long key = wal.writeToLog(data, operation);
    sl.insert(key, data); 
    return; 
}

// snapshotting thread
void saveSnapshotToDisk(SkipList& sl){
    // check if the size of the memTable is over the defined MAX_SIZE;
    if(sl.listSize() <= MAX_SIZE){
        cout << "No need to save Snapshot"; 
        return; 
    }
    // if its over, acquire the lock on memTable and perform snapshotting.
    // This acquires the lock on all teh mutexes simultaneously.
    lock(memtableAndSnapshotSave, compactionAndSnapshotSave, searchData);
    // Below here is for automatic unlocking of the locks after process goeas OOS.
    lock_guard<mutex> guard1(memtableAndSnapshotSave);
    lock_guard<mutex> guard2(compactionAndSnapshotSave);
    lock_guard<mutex> guard3(searchLock);
    
    // The below way is bad. The order in which the locks are acquired matters and may cause deadlocks
    // lock_guard<mutex> guard1(memtableAndSnapshotSave); 
    // lock_guard<mutex> guard2(compactionAndSnapshotSave);
    // lock_guard<mutex> guard3(searchData);
    process.store(2); 
    vector<pair<long, int>> data; 
    data = sl.data(); 
    insertToDisk(data); 
    return; 
}

void searchDataInDB(SkipList& sl, long key){
    lock_guard<mutex> search(searchLock); 

    int value = sl.search(key); 
    if(value == -1){
        vector<string> filenames = get_all_files_names_within_folder("./snapshots");
        value = searchData(filenames, key);
        if(value == -1){
            cout << "Data not present" << endl;
        }else{
            cout << "Data : " << value;
            return; 
        }
    }
    cout << "Data : " << value << endl;
    return;
}

int main(){
    // initializing database components 
    WriteAheadLog wal;
    SkipList skiplist;  

    bool flag = true; 
    while(flag)
    {
        int option;
        cout << "::::::::: SELECT OPERATION :::::::::" << endl;
        cout << "INSERT - 1" << endl;
        cout << "DELETE - 2" << endl;
        cout << "SEARCH - 3" << endl;
        cout << "EXIT - 0" << endl;
        cout << "Enter your choice: " << flush; // Flush output

        cin >> option;
        if(option == 0) break;
        
        if(option == 1){
            int data; 
            cout << "Input data : "; 
            cin >> data; 
            cout << endl;
            // Writing to log is independent of the mutex lock - only one process access this.
            thread write_to_memtable(writeToMemTable, "INSERT", data, ref(skiplist), ref(wal)); 
            thread save_snapshot(saveSnapshotToDisk, ref(skiplist));

            write_to_memtable.join();
            save_snapshot.join(); 

        } else if(option == 2){
            cout << "Deleting data" << endl;

        } else if(option == 3){
            cout << "IDK" << endl;

        } else{
            cout << "Invalid selection, please select from the above options...." << endl; 
        }
    }
    return 0; 
}


