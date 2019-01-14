

#include <iostream>
#include "timing.h"
#include "HashTable.h"

using CuckooHash::HashTable;
using std::cout;
using std::endl;

int main() {

    size_t total_items = 1000000;
    HashTable<size_t, 32, 64> table(total_items);

    auto start_time = NowNanos();

    size_t num_inserted = 0;
    for (size_t i = 0; i < total_items; i++, num_inserted++) {
        if (table.Add(i, i) != CuckooHash::Ok) {
            cout<<"error: full"<<endl;
            break;
        }
    }
    uint32_t loc = 0;
    for (size_t i = 0; i < total_items; i++, num_inserted++) {
        if (table.Find(i, &loc) == CuckooHash::Ok && loc == i) {
            continue;
        } else {
            cout<<"find error"<<endl;
            break;
        }
    }

    for (size_t i = 0; i < total_items; i++, num_inserted++) {
        if (table.Delete(i) == CuckooHash::Ok) {
            continue;
        } else {
            cout<<"delete error"<<endl;
            break;
        }
    }

    for (size_t i = 0; i < total_items; i++, num_inserted++) {
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