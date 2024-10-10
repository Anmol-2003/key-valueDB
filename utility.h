#include<iostream>
#include<windows.h>
#include<string>
#include<vector>
#include<fstream>
#include<ctime>
#include<map>
#include<algorithm>
#include<sstream>

using namespace std; 

// Utility function used in get_all_files_names_within_folder();
wstring string_to_wstring(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

using namespace std;

// Function analogous to os.listdir() in python;
vector<string> get_all_files_names_within_folder(const string &folder)
{
    vector<string> names;
    wstring search_path = string_to_wstring(folder + "/*.*");
    WIN32_FIND_DATAW fd; // Use the wide version
    HANDLE hFind = FindFirstFileW(search_path.c_str(), &fd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {

            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                wstring file_name_w = fd.cFileName;
                string file_name(file_name_w.begin(), file_name_w.end());
                names.push_back(file_name);
            }
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }

    return names;
}

// Inserting to disk - SSTable
void insertToDisk(vector<pair<long, int>> data){
    time_t timestamp; 
    string fileName = "./snapshots/" + to_string(time(&timestamp)) + ".txt"; 
    ofstream file(fileName, ios::app);
    if(file.is_open()){
        for(auto d : data){
            file << to_string(d.first) + " : " + to_string(d.second) + "\n";
        }
        file.close(); 
        cout << "Data snapshotted to disk" << endl;
    } else{
        cout << "Error opening the file" <<endl;
    }
}

// Searching for a key
int searchData(vector<string>& filenames, long& k)
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
                        // cout << "Data: " << value << endl;
                        return value; 
                    }
                }
            }
            file.close();
        }
    }
    return -1; 
}

// Compaction - Performs compaction of the SSTables.
void compaction(vector<string> filenames)
{
    sort(filenames.begin(), filenames.end());
    vector<pair<long, pair<int, int>>> data;

    for (string name : filenames)
    {

        vector<pair<long, pair<int, int>>> temp;
        ifstream file("./snapshots/" + name);
        string line;

        while (getline(file, line))
        {
            stringstream ss(line);
            long key;
            int value;
            char delimiter;
            if (ss >> key >> delimiter >> value)
            { // reading from the sstream object. stringstream associates the str as a stream object.
                temp.push_back({key, {stoi(name), value}});
            }
        }
        file.close();

        // compaction algorithm - merging of N sorted arrays
        if (data.empty())
        {
            data = temp;
        }else{
            vector<pair<long, pair<int, int>>> d; 
            sort(data.begin(), data.end());       
            sort(temp.begin(), temp.end());   

            int n = data.size();
            int m = temp.size();
            int i = 0, j = 0;

            while (i < n && j < m)
            {
                if (data[i].first < temp[j].first)
                {
                    d.push_back(data[i]); 
                    i++;
                }
                else if (data[i].first > temp[j].first)
                {
                    d.push_back(temp[j]); 
                    j++;
                }
                else
                {
                    if (data[i].second.first > temp[j].second.first)
                    {
                        d.push_back(data[i]); 
                    }
                    else
                    {
                        d.push_back(temp[j]); 
                    }
                    i++;
                    j++;
                }
            }
            while (i < n)
            {
                d.push_back(data[i]);
                i++;
            }
            while (j < m)
            {
                d.push_back(temp[j]);
                j++;
            }
            data = d;
        }
    }
    vector<pair<long, int>> d;
    for(auto ele : data){
        d.push_back({ele.first, ele.second.second});
    }
    insertToDisk(d);
    return;    
}