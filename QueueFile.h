
#ifndef UNTITLED_QUEUEFILE_H
#define UNTITLED_QUEUEFILE_H


#include "fcntl.h"
#include <cstdio>
#include <string>
#include "unistd.h"
#include "Cluster.h"
#include "MainCluster.h"
#include "StackList.h"
#include "DynamicArray.h"

struct SaveClusterHeader {
    // DiskBuf saveClasterSize
    u_int32_t saveClasterSize;
    // cluster saveClasterSize
    u_int32_t saveDataSize;
    // position of cluster in file
    u_int32_t dataPtr;
};
class SaveCluster {
public:
    SaveClusterHeader * header;
    char * buf;
    uint32_t write(uint32_t ptr);

    SaveCluster(uint32_t ptr, uint32_t size);
    SaveCluster(uint32_t ptr);
    void restore();
    ~SaveCluster();
};


class QueueFile {
    uint32_t newClustersPtr;
    uint32_t freeClustersPtr;
public:
    static int fileDescriptor;
    static uint32_t clusterSize;
    static QueueFile *queueFile;
    char const * filName;
    MainCluster *mainCluster;
    StackList<u_int8_t *> stackList;

    QueueFile(char const *fileName, uint32_t fileClusterSize);

    ~QueueFile();

    void putMsg(DynamicArray<char> &msg);

    DynamicArray<char> takeMsg();

    void takeMsg(uint32_t ptr);

    void safeWrite(DynamicArray<Cluster *> &chain);

    uint32_t clustersPerMessage(DynamicArray<char> &msg) const;
};


#endif //UNTITLED_QUEUEFILE_H
