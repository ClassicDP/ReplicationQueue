
#ifndef UNTITLED_QUEUEFILE_H
#define UNTITLED_QUEUEFILE_H


#include "fcntl.h"
#include <cstdio>
#include <string>
#include "unistd.h"
#include "Cluster.h"
#include "MainCluster.h"
#include "StackList.h"


class QueueFile {
public:
    static int fileDescriptor;
    static uint32_t clusterSize;
    static QueueFile *queueFile;
    char *filName;
    MainCluster *mainCluster;
    StackList<u_int8_t *> stackList;

    void safeWrite(uint32_t ptr, u_int32_t dataSize, char *buf = nullptr);

    void safeTruncate(u_int32_t fileSize);

    void safeWriteComplete();


    QueueFile(char *fileName, uint32_t fileClusterSize);

    ~QueueFile();

    void putMsg(std::string &msg);
    std::string takeMsg();

    void takeMsg(uint32_t ptr);

    uint16_t clustersPerMessage(std::string &msg) const;
};


#endif //UNTITLED_QUEUEFILE_H
