#include<iostream>
#include<vector>
#include<algorithm>

using namespace std; 
const int maxNumLevels = 5; 

struct Data{
    long key; 
    int value; 
};

class Node{
    public:
        Data* data; 
        vector<Node*> next; 
        Node(int key, int value, int level) : data(new Data{key, value}), next(level+1, nullptr) {}
};

class SkipList{
    Node* head; 
    int level; 

    public: 
    SkipList(); 

    long insert(long key, int data); 
    void remove(long key); 
    int search(long key); 
    vector<pair<long, int>> data();
    int listSize();
}; 

// constructor func
SkipList::SkipList(){
    head = new Node(0, 0, maxNumLevels); 
    level = 0; // current level of insertion is 0; 
}


// INSERT OPERATION
long SkipList::insert(long key, int data){
    // updating the key to keep it unique

    int newLevel = 0; //Will be inserting by default from level 0;
    while(newLevel < maxNumLevels && (rand() % 2) == 1){
            newLevel ++; // increasing the levels based on a random event - coin toss; 
    }
    if(level < newLevel){
        level = newLevel;
        head->next.resize(newLevel + 1, nullptr); 
    }

    Node* curr = head;
    vector<Node*> prev(level + 1, nullptr); // stores the nodes immediately before the pos of insertion at level i; 
    
    // finding the right place for insertion                    
    for(int i = level; i >=0; i--){
        while (curr->next[i] && curr->next[i]->data->key < key){ // inserting on the basis of sorted keys
            curr = curr->next[i]; 
        }
        prev[i] = curr; 
    }

    curr = curr->next[0]; 
    if(curr == nullptr || curr->data->key != key){
        Node* newNode = new Node(key, data, level); 
        for(int i =0; i <= newLevel; i++){
            newNode->next[i] = prev[i]->next[i]; 
            prev[i]->next[i] = newNode; 
        }
        cout << "Successful insertion of node" << endl; 
        return key;
    }
   
    cout << "Node already present" << endl; 
    return 0; 
}

// DELETE OPERATION
void SkipList::remove(long key){
    Node* curr = head; 
    vector<Node*> prev(level+1, nullptr); 

    for(int i = level; i>=0; i--){
        while(curr->next[i] && curr->next[i]->data->key < key){
            curr = curr->next[i]; 
        }
        prev[i] = curr; 
    }
    curr = curr->next[0]; 

    if(curr != nullptr && curr->data->key == key){
        for(int i = 0; i <= level; i++){
            if(prev[i]->next[i] != curr){
                break; 
            }
            else{
                prev[i]->next[i] = curr->next[i];
            }
        }
        delete curr; 
        while (level > 0 && head->next[level] == nullptr){
            level --; 
        }
    }
}

// SEARCH OPERATION
int SkipList::search(long key){
    Node* curr = head;                  
    for(int i = level; i >= 0; i--){
        while(curr->next[i] && curr->next[i]->data->key < key){
            curr = curr->next[i];
        }
    }                                         
    if( curr->next[0] != nullptr && curr->next[0]->data->key == key){
        // cout << "Data: " << curr->next[0]->data->value << endl; 
        return curr->next[0]->data->value;
    }
    return -1;     
}                           

int SkipList::listSize(){
    Node* curr = head;
    int ans = 0; 
    while(curr->next[0] != nullptr){
        ans++; 
        curr = curr->next[0];
    }
    return ans; 
}

vector<pair<long, int>> SkipList::data()                                                                                                                                                                
{
    Node* curr = head;
    vector<pair<long, int>> t;
    while(curr->next[0] != nullptr){
        curr = curr->next[0]; 
        t.push_back({curr->data->key, curr->data->value});
    }    
    return t; 
}

