#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<ctime>
#include<map>
#include<algorithm>
#include<sstream>
#include<filesystem>

using namespace std; 

namespace fs = std::filesystem;

// Function analogous to os.listdir() in Python;
vector<string> get_all_files_names_within_folder(const string &folder)
{
    vector<string> names;
    for (const auto &entry : fs::directory_iterator(folder)) 
    {
        if (fs::is_regular_file(entry.path())) 
        {
            names.push_back(entry.path().filename().string());  
        }
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
int searchData(vector<string>& filenames, long& key) {
    const string dir = "./snapshots/";
    bool found = false;
    int result = -1;

    // Iterate through filenames in reverse order (if that's required, otherwise remove rbegin/rend)
    for (const string& filename : filenames) {
        ifstream file(dir + filename); 
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                long fileKey;
                int value;
                char delim;
                if (ss >> fileKey >> delim >> value) {
                    if (fileKey == key) {
                        result = value;
                        found = true;
                        break;  // Break out of the loop once key is found
                    }
                }
            }
            file.close(); // Close the file after reading
        }
        if (found) {
            break; // Exit the outer loop if the key was found
        }
    }

    return result;  // Return -1 if key wasn't found, or the corresponding value if found
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