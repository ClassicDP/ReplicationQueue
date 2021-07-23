
#include "QueueFile.h"
#include <iostream>
#include "MainCluster.h"

MainCluster::MainCluster(u_int32_t size) : Cluster(mainCluster) {
    header = (MainClusterHeader *) buffer;
}

void MainCluster::write() {
    Cluster::write(0);
}
