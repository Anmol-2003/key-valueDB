#include<iostream>
#include<thread> 
#include<mutex>
#include<chrono>

using namespace std; 

mutex mtx;
void func1(int& a){
    lock_guard<mutex> guard(mtx); 
    cout << "func1 is executing" << endl; 
    a += 10; 
    cout << a << endl;
}   

void func2(int& a){
    lock_guard<mutex> guard(mtx); 
    cout << "func2 is executing" << endl;
    this_thread::sleep_for(chrono::seconds(3));
    a -= 20; 
    cout << a << endl; 
}

int main(){
    int a = 30; 
    thread f2(func2, ref(a));
    thread f1(func1, ref(a));
    
    f2.join();
    f1.join();
    
    return 0; 
}