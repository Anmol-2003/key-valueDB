#include<iostream>
#include<vector>
#include<string>
#include<algorithm>
#include<fstream>
#include<sstream>

using namespace std;

void searchData(vector<string>& filenames, long& k)
{
    sort(filenames.rbegin(), filenames.rend());
    const string dir = "./snapshots/";
    bool found = false; 
    int i = 0; 
    while(!found){
        ifstream file(dir + filenames[i]); 
        if(file.is_open()){
            string line; 
            while(getline(file, line)){
                stringstream ss(line); 
                long key; 
                int value; 
                char delim; 
                if(ss >> key >> delim >> value){
                    if(key == k){
                        cout << "Data: " << value << endl;
                        return; 
                    }
                }
            }
            file.close();
        }
    }
    return; 
}