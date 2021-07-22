

#include "QueueFile.h"

int QueueFile::fileDescriptor;

QueueFile::QueueFile(char *fileName, uint32_t clusterSize) {
    remove(fileName);
    fileDescriptor = open(fileName, O_RDWR | O_CREAT, 0777);
    mainCluster = new MainCluster(clusterSize);
    mainCluster->header->clusterSize = clusterSize;
    mainCluster->header->fileSize = clusterSize;
    mainCluster->header->firstFreePtr = clusterSize;
    mainCluster->header->firstDataPtr = clusterSize;
    mainCluster->write(0);
}

QueueFile::~QueueFile() {
    close(fileDescriptor);
    delete mainCluster;
}

void QueueFile::putMsg(char *msg) {
    auto cnt = msgCount(msg);
    Cluster *chain[cnt];
    chain[0] = new Cluster(mainCluster->header->clusterSize, ClusterType::firstCluster);
    for (int i = 1; i < cnt; i++) {
        chain[i] = new Cluster(mainCluster->header->clusterSize, ClusterType::nextCluster);
    }
    uint16_t nextPtr = mainCluster->header->firstFreePtr;
    uint32_t msgPtr=0;
    for (int i=0; i<cnt; i++) {
        uint16_t ptr = nextPtr;
        if (nextPtr!=mainCluster->header->fileSize) {
            nextPtr = *Cluster(ptr).nextClusterPtr();
        } else {
            nextPtr += mainCluster->header->clusterSize;
        }
        chain[i]->setData(msg, msgPtr);
        *chain[i]->nextClusterPtr() = nextPtr;
        // need to backup algorithm
        chain[i]->write(ptr);
        ptr+=chain[i]->dataSize();
    }
    for (int i = 0; i < cnt; i++) {
        delete chain[i];
    }
}

uint16_t QueueFile::msgCount(const char *msg) const {
    uint16_t size = mainCluster->header->clusterSize;
    uint16_t firstHLen = sizeof(FirstClusterHeader);
    uint16_t nextHLen = sizeof(NextClusterHeader);
    uint16_t msgLen = sizeof(msg);
    uint16_t a = (msgLen - (size - firstHLen));
    uint16_t b = size - nextHLen;
    return 1 + a / b + a % b ? 1 : 0;
}
