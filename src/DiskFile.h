#ifndef DISKFILE_H
#define DISKFILE_H

#include <stdint.h>
#include <stdbool.h>
#include "pff.h"

class DiskFile
{
    private:    
    FATFS fs; //petitfs
    FILINFO fno;
    DIR dir;
    bool sdInitialized = false;
    bool initSD();

    public:
    FRESULT res;
    int16_t nFiles;
    DiskFile();   
    void printFileName();
    int16_t scanFiles(char *path);
    bool openDir(char *path);
    bool getNextFile();
    bool getFileInfo(char *path, char *filename);
    uint32_t getStartSector();
    bool getReadOnly() {return (fno.fattrib & AM_RDO);}
    char *getFileName() {return fno.fname;}
    uint32_t getFileSize() {return fno.fsize;}
};

extern DiskFile sdfile;

#endif //DISKFILE_H