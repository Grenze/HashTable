

#include <iostream>
#include "timing.h"
#include "HashTable.h"

using CuckooHash::HashTable;
using std::cout;
using std::endl;

int main() {


    // 1024*16*0.96 = 15728 (15700)
    // 1024*16*0.96*2 = 31457 (31400)
    //size_t total_items = 1000000;
    size_t total_items = 15700;
    HashTable<size_t, 32, 64> table(total_items);

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