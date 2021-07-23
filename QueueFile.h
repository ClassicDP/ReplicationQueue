
#ifndef UNTITLED_QUEUEFILE_H
#define UNTITLED_QUEUEFILE_H


#include "fcntl.h"
#include <cstdio>
#include "unistd.h"
#include "Cluster.h"
#include "MainCluster.h"


class QueueFile {
public:
    static int fileDescriptor;
    static uint32_t clusterSize;
    MainCluster *mainCluster;



    QueueFile(char *fileName, uint32_t fileClusterSize);

    ~QueueFile();

    void putMsg(char *msg);
    void takeMsg(uint32_t ptr);

    uint16_t clustersPerMessage(const char *msg) const;
};


#endif //UNTITLED_QUEUEFILE_H
