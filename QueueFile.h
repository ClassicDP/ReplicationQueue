
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
    MainCluster *mainCluster;


    QueueFile(char *fileName, uint32_t clusterSize);

    ~QueueFile();

    void putMsg(char *msg);

    uint16_t msgCount(const char *msg) const;
};


#endif //UNTITLED_QUEUEFILE_H
