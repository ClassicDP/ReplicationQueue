#include <iostream>
#include "QueueFile.h"
#include "MainCluster.h"

u_int8_t bit_per_vector = 4;

uint_fast32_t clusterSize = 50;


int main() {
    char fileName[] = "_123.db";
    char msg[] = "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_";

    QueueFile dbFile = QueueFile(fileName, clusterSize);
    dbFile.putMsg(msg);
    std::cout << dbFile.takeMsg();
    std::cout << sizeof(mainCluster) << "\n"
              << sizeof(FirstClusterHeader) << "\n";


    return 0;
}
