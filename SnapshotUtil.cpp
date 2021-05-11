//
// Created by Peng,Wei(BAMRD) on 2021/5/11.
//

#include <unistd.h>
#include <string>
#include "SnapshotUtil.h"

#define MAX_PATH_LEN 256
using namespace v8;

void SnapshotUtil::writeFile(v8::StartupData data) {
    char currentPath[MAX_PATH_LEN];
    getcwd(currentPath, MAX_PATH_LEN);
    printf("current path =%s\n", currentPath);
    std::string path = currentPath;
    FILE *file = fopen((path + "/a.blob").c_str(), "w");
    fseek(file, 0, SEEK_END);
    rewind(file);
    int writeSize = fwrite(data.data, data.raw_size, 1, file);
    printf("write size=%d\n", writeSize);
    fclose(file);
}

void SnapshotUtil::readFile(v8::StartupData &data) {
    char currentPath[MAX_PATH_LEN];
    getcwd(currentPath, MAX_PATH_LEN);
    printf("current path =%s\n", currentPath);
    std::string path = currentPath;
    FILE *file = fopen((path + "/a.blob").c_str(), "rb");
    fseek(file, 0, SEEK_END);
    data.raw_size = static_cast<int>(ftell(file));
    rewind(file);
    data.data = new char[data.raw_size];
    int read_size = static_cast<int>(fread(const_cast<char *>(data.data),
                                           1, data.raw_size, file));
    fclose(file);
    printf("readFile ## raw_size =%d, CanBeRehashed=%d, read_size=%d\n",
           data.raw_size, data.CanBeRehashed(), read_size);
}
