
#include "QueueFile.h"
#include <iostream>
#include "MainCluster.h"

MainCluster::MainCluster(u_int32_t size) : Cluster(size, mainCluster) {
    header = (MainClusterHeader *) buffer;

}
