#include "config.h"

/*
In the future this will probably not work like this. Each node will calculate it's neighbours automatically.
But for now it's enough to properly validate the mpi implementation.
The config format:
The first line contains the number of units we need to communicate with.
The following lines specifies the face and the neighbour rank for each unit we communicate with according to the following enum. 
{
    Top = 0, 
    Bottom = 1, 
    Left = 2, 
    Right = 3, 
    Front = 4, 
    Back = 6
}
Then the room file name.
Then the number of source files.
Then the source file names.

Example config:
2
3 1
4 2
room_0.dwm
1
source.pcm
*/

Config readConfigFile(int rank)
{
    int size = snprintf(NULL, 0, "cfg%d.txt", rank) + 1;

    char* configFile = (char*)malloc(size);
	if (configFile == NULL)
	{
		fprintf(stderr, "Out of memory");
		exit(EXIT_FAILURE);
	}
    snprintf(configFile, size, "cfg%d.txt", rank);
    
    FILE* config = fopen(configFile, "r");

    if (config == NULL)
    {
        fprintf(stderr, "Failed to open configuration file");
		exit(EXIT_FAILURE);
    }

    Config cfg = { 0 };
    fscanf(config, "%d", &(cfg.faceCount));

    if (cfg.faceCount > 0) 
    {
        cfg.faces = (FaceBuffer*)malloc(sizeof(FaceBuffer) * cfg.faceCount);
        if (cfg.faces == NULL)
        {
            fprintf(stderr, "Out of memory");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < cfg.faceCount; i++)
        {
            int face;
            int port;

            fscanf(config, "%d %d", &face, &port);

            FaceBuffer f = { 0 };
            f.face = face;
            f.devPort = port;

            cfg.faces[i] = f;
        }
    }

    fscanf(config, "\n");

    size_t len = 0;
    ssize_t read = getline(&(cfg.roomFileName), &len, config);
    if (read == -1)
    {
        fprintf(stderr, "Failed to read room file name from configuration file");
        exit(EXIT_FAILURE);
    }
    removeNewLine(cfg.roomFileName);

    fscanf(config, "%d\n", &(cfg.sourceCnt));
    
    if(cfg.sourceCnt <= 0) return cfg;

    cfg.sourceFileNames = (char**)malloc(sizeof(char*) * cfg.sourceCnt);
    if (cfg.sourceFileNames == NULL)
    {
        fprintf(stderr, "Out of memory");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < cfg.sourceCnt; i++)
    {
        len = 0; // this line owes me 3 hours of debugging time
        read = getline(&(cfg.sourceFileNames[i]), &len, config);
        if (read == -1)
        {
            fprintf(stderr, "Failed to read source file");
            exit(EXIT_FAILURE);
        }
        removeNewLine(cfg.sourceFileNames[i]);
    }

    free(configFile);
    fclose(config);

    return cfg;
}

void freeConfig(Config* cfg)
{
    if (cfg == NULL) return;
    free(cfg->roomFileName);
    for (int i = 0; i < cfg->sourceCnt; i++)
    {
        free(cfg->sourceFileNames[i]);
    }
    free(cfg->sourceFileNames);
}

void removeNewLine(char* str)
{
    if (str == NULL) return;
    int len = strlen(str);
    if (len > 0 && str[len-1] == '\n')
    {
        str[len-1] = '\0';
    } 
}