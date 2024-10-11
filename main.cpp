#include<iostream>
#include<thread>
#include<mutex>
#include<atomic>
#include<chrono>

#include "skipList.h"
#include "writeAheadLog.h"
#include "utility.h"

using namespace std;

const int MAX_SIZE = 4; 
mutex writeAndSnapshotSave;
mutex compactionAndSnapshotSave;
mutex searchLock;

// process 1 - memtable
// process 2 - snapshot saving 
// --------------------------------------------
// write - insert, delete, modify
// read - search in database

// Below atomic variable can be used to make functions atomic
// atomic<int> process(0);

// long logging(WriteAheadLog& wal, int data, string operation){
//     if(process.load() == 2){
//         long key = wal.writeToLog(data, "SNAPSHOT " + operation)
//     } 
// }

// write to memtable thread
void writeToMemtableInsert(int data, SkipList& sl, WriteAheadLog& wal){
    
    lock_guard<mutex> guard(writeAndSnapshotSave); // lightweight RAII lock, automatically acquires the lock and unlocks once out of scope.
    
    long key = wal.writeToLogInsert(data, "INSERT");
    sl.insert(key, data); 
    return; 
}
void writeToMemtableDelete(long key, SkipList& sl, WriteAheadLog& wal){
    lock_guard<mutex> guard(writeAndSnapshotSave); 

    wal.writeToLogDelete(key, "DELETE");
    sl.remove(key); 
    return; 
}
void writeToMemtableUpdate(long key, int data, SkipList& sl, WriteAheadLog& wal)
{
    lock_guard<mutex> guard(writeAndSnapshotSave); 
    wal.writeToLogUpdate(key, data, "UPDATE"); 
    sl.update(key, data); 
    return;
}
// snapshotting thread
void saveSnapshotToDisk(SkipList& sl){
    // check if the size of the memTable is over the defined MAX_SIZE;
    if(sl.listSize() <= MAX_SIZE){
        cout << endl << "No need to save Snapshot\n"; 
        return; 
    }
    // if its over, acquire the lock on memTable and perform snapshotting.
    // This acquires the lock on all teh mutexes simultaneously.
    scoped_lock guard(writeAndSnapshotSave, compactionAndSnapshotSave, searchLock);
    cout << endl << "Saving memtable to disk\n"; 
    this_thread::sleep_for(chrono::seconds(2));
    // The below way is bad. The order in which the locks are acquired matters and may cause deadlocks
    // lock_guard<mutex> guard1(writeAndSnapshotSave); 
    // lock_guard<mutex> guard2(compactionAndSnapshotSave);
    // lock_guard<mutex> guard3(searchData);

    vector<pair<long, int>> data; 
    sl.display(); 
    data = sl.data();
    insertToDisk(data); 
    sl.clear();
    return; 
}
// compaction thread
void compactionOfSSTables(vector<string>& filenames){
    if(filenames.size() <= 2) return;
    // acquire the lock
    scoped_lock guard(writeAndSnapshotSave, compactionAndSnapshotSave, searchLock);
    cout << "\n" << "Compacting disk SSTables\n";
    this_thread::sleep_for(chrono::seconds(2));
    compaction(filenames); 
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
            cout << "Data : " << value << endl;
            return; 
        }
    }
    if(value == -1) cout << "Data not present\n"; 
    return;
}

int main(){
    // initializing database components 
    WriteAheadLog wal;
    SkipList skiplist;  

    bool flag = true; 
    while(flag)
    {
        vector<string> filenames = get_all_files_names_within_folder("./snapshots/");
        int option;
        cout << "\n::::::::: SELECT OPERATION :::::::::" << endl;
        cout << "INSERT - 1" << endl;
        cout << "DELETE - 2" << endl;
        cout << "UPDATE - 3" << endl;
        cout << "SEARCH - 4" << endl;
        cout << "EXIT - 0" << endl;
        cout << "Enter your choice: " << flush; // Flush output
        
        thread compactionProcess(compactionOfSSTables, ref(filenames));
        compactionProcess.detach();

        cin >> option;
        if(option == 0) break;
        if(option == 1){
            int data; 
            cout << "Input data : "; 
            cin >> data; 
            cout << endl;

            // Writing to log is independent of the mutex lock - only one process access this.
            // thread write_to_memtable(writeToMemtableInsert, data, ref(skiplist), ref(wal)); 
            writeToMemtableInsert(data, skiplist, wal); 
            thread save_snapshot(saveSnapshotToDisk, ref(skiplist));
            save_snapshot.join(); 

            // detach - async thread. The main thread doesnt wait for these to finish and moves on. Faster
            // join - execution of the thread takes place on different core and the main thread waits for it to finish.
            // calling snapshot fn async
            // write_to_memtable.join(); 
            
        } else if(option == 2){
            cout << "Currently implementing Delete method"; 
            // long key; 
            // cout << "KEY : "; 
            // cin >> key; 

            // thread write_to_memtable(writeToMemtableDelete, key, ref(skiplist), ref(wal));
            // write_to_memtable.join();

        } else if(option == 4){
            long key; 
            cout << "KEY TO SEARCH : "; 
            cin >> key;  
            searchDataInDB(skiplist, key);

        } else if(option == 3){
            long key; 
            int data; 
            cout << "Enter KEY and DATA : "; 
            cin >> key >> data; 
            writeToMemtableUpdate(key, data, skiplist, wal); 

        }else{
            cout << "Invalid selection, please select from the above options...." << endl; 
        }
    }
    return 0; 
}