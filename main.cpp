#include <iostream>
#include "QueueFile.h"
#include "MainCluster.h"

u_int8_t bit_per_vector = 4;

uint_fast32_t clusterSize = 256;


int main() {
    char fileName[] = "_123.db";
//    Cluster*  cl[100];
//    std::cout << sizeof (cl)<< "\n";




    std::cout << sizeof (fileName) << std::endl;
    QueueFile dbFile = QueueFile(fileName, clusterSize);
    dbFile.putMsg(fileName);
    std::cout << sizeof(mainCluster) << "\n"
              << sizeof(FirstClusterHeader) << "\n";


    return 0;
}
