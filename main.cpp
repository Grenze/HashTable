

#include <iostream>
#include "timing.h"
#include "HashTable.h"

using CuckooHash::HashTable;
using std::cout;
using std::endl;

int main() {


    // 1024*16*0.96 = 15728 (15700)
    // 1024*16*0.96*16 = 125829
    //size_t total_items = 1000000;
    size_t total_items = 125829;
    // TODO: the faster hash insert proceeds, the faster to form nvm_imm_. Maybe a better hash function matters.
    HashTable<size_t, 32, 64> table(total_items);
    // IF different keys have same hash value,
    // the key inserted before will be invisible,
    // only the latest key is accessible.
    size_t i = 1;
    table.Add(i, i+10);
    table.Add(i, i+1);
    table.Delete(i);
    uint32_t loc1 = 0;
    table.Find(i, &loc1);
    std::cout<<loc1<<std::endl;
    table.Delete(i);

    auto start_time = NowNanos();

    size_t num_inserted = 0;
    for (size_t i = 0; i < total_items; i++, num_inserted++) {
        if (table.Add(i, i) != CuckooHash::Ok) {
            cout<<"error: full"<<endl;
            break;
        }
    }
    cout<<"total inserted items' count:"<<num_inserted<<endl;

    cout<<table.Info();

    uint32_t loc = 0;
    for (size_t i = 0; i < total_items; i++) {
        if (table.Find(i, &loc) == CuckooHash::Ok && loc == i) {
            //cout<<loc<<endl;
            continue;
        } else {
            cout<<"find error"<<endl;
            break;
        }
    }

    for (size_t i = 0; i < total_items; i++) {
        if (table.Delete(i) == CuckooHash::Ok) {
            continue;
        } else {
            cout<<"delete error"<<endl;
            break;
        }
    }

    for (size_t i = 0; i < total_items; i++) {
        if (table.Find(i, &loc) == CuckooHash::NotFound) {
            continue;
        } else {
            cout<<"delete not completed"<<endl;
            break;
        }
    }

    auto end_time = NowNanos();
    cout<< end_time - start_time <<endl;
    return 0;
}