//
// Created by Peng,Wei(BAMRD) on 2021/5/11.
//

#ifndef V8PPDEMO_SNAPSHOTUTIL_H
#define V8PPDEMO_SNAPSHOTUTIL_H


#include <v8.h>

class SnapshotUtil {
public:
    static void writeFile(v8::StartupData data);
    static void readFile(v8::StartupData &data);
};


#endif //V8PPDEMO_SNAPSHOTUTIL_H
