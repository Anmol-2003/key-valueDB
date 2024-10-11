#include<iostream>
#include<fstream>
#include<string> 
#include<ctime>

using namespace std;

long KEY = 1;

class WriteAheadLog {
    ofstream logFile;
    ofstream keyFile; 
    string fileName = "logFile.txt";
    string keyFileName = "key.txt";

public:
    WriteAheadLog();
    ~WriteAheadLog();
    long writeToLogInsert(int data, const string operation);
    void readFromLog();
    void writeToLogDelete(long key, const string operation);
    void writeToLogUpdate(long key, int data, const string operation);
};

// Constructor
WriteAheadLog::WriteAheadLog() {
    // ifstream readKey(keyFileName);
    // if(readKey.is_open()){
    //     string key;
    //     getline(readKey, key);
    //     KEY = stol(key); 
    //     cout << "Previous key : " << KEY << endl;
    //     readKey.close();
    // }
    // else{
    //     cout << "Error opening the key file !!!" << endl;
    // }
    cout << "WAL file instantiated.\n";
}

//Destructor
WriteAheadLog::~WriteAheadLog(){
    // keyFile.open(keyFileName, ios::app);
    // if(keyFile.is_open()){
    //     keyFile << to_string(KEY);
    //     keyFile.close();
    // }else{
    //     cout << "Error in opening key file" << endl;
    // }
}

// Insert Log
long WriteAheadLog::writeToLogInsert(int data, const string operation) {
    logFile.open(fileName, ios::app);
    time_t timestamp; 
    long key = KEY++;

    if(logFile.is_open()){
        logFile << to_string(time(&timestamp)) + " -- " + operation +  " : " + to_string(key) + " : " + to_string(data) + "\n"; 
    }
    logFile.close();
    return key;
}
// Delete log 
void WriteAheadLog::writeToLogDelete(long key, const string operation){
    logFile.open(fileName, ios::app);
    time_t timestamp; 
    if(logFile.is_open()){
        logFile << to_string(time(&timestamp)) + " -- " + operation + " : " + to_string(key); 
    }
    logFile.close();
    return; 
}

// Read Log
void WriteAheadLog::readFromLog() {
    ifstream read(fileName); 
    if (read.is_open()) {
        string line;
        while (getline(read, line)) {
            cout << line << endl; 
        }
        read.close(); 
    } else {
        cout << "Failed to open log file for reading!" << endl;
    }
}


void WriteAheadLog::writeToLogUpdate(long key, int data, const string operation){
    logFile.open(fileName, ios::app);
    time_t timestamp; 
    long key = KEY++;

    if(logFile.is_open()){
        logFile << to_string(time(&timestamp)) + " -- " + operation +  " : " + to_string(key) + " : " + to_string(data) + "\n"; 
    }
    logFile.close();
    return;
}