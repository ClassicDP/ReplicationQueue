

#ifndef UNTITLED_MAINCLUSTER_H
#define UNTITLED_MAINCLUSTER_H

#include "types.h"
#include "Cluster.h"

class MainCluster : public Cluster {
public:
    MainClusterHeader *header;

    MainCluster(u_int32_t size);
};

#endif //UNTITLED_MAINCLUSTER_H
