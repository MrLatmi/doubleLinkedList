#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <direct.h>
#include <windows.h>

#include <time.h>
#include "list.h"
#include "formatingPrint.h"

#ifdef LIST_DEBUG


//TODO: one fopen and one fclose. Destructor for debugFileCreate
//TODO: check const
dump_file debugFileCreate(void)
{
    time_t myTime;
    time(&myTime);

    tm* localTime = localtime(&myTime);

    char folderName[50] = {};


    snprintf(folderName, sizeof(folderName), "log_%02d.%02d.%d_%02d-%02d-%02d",
            localTime->tm_mday,
            localTime->tm_mon + 1,
            localTime->tm_year + 1900,
            localTime->tm_hour,
            localTime->tm_min,
            localTime->tm_sec);

    int mkDirChecker = _mkdir(folderName);

    if (mkDirChecker != 0)
    {
        ERROR_PRINTF("Problem with creating debug folder");
        dump_file errorStruct = {};
        return errorStruct;
    }

    char filePath[100];
    snprintf(filePath, sizeof(filePath), "%s/LogFile.htm", folderName);
    FILE* debugFile = fopen(filePath, "ab");

    if (debugFile == NULL)
    {
        ERROR_PRINTF("Problem with creating debug file");
        dump_file errorStruct = {};
        return errorStruct;
    }

    dump_file debugFileStruct  = {};
    debugFileStruct.file       = debugFile;

    strncpy(debugFileStruct.filePath,   filePath,   sizeof(debugFileStruct.filePath) - 1);
    strncpy(debugFileStruct.folderPath, folderName, sizeof(debugFileStruct.filePath) - 1);

    debugFileStruct.graphCount = 0;
    if (!CopyFile(DUMP_SAMPLE_PATH, filePath, FALSE))
    {
        printf("LIST DUMP: Sample copy error");
    }

    return debugFileStruct;

}

list_error_t listVerifier(const list_t *const list, bool firstInit, dump_file*const debugFile)
{
    if (list == NULL)
    {
        ERROR_PRINTF("LIST: List pointer - NULL pointer");
        listDump(list, debugFile);
        return pointerListProblem;
    }

    if (firstInit)
    {
        if (list->data != NULL || list->next != NULL || list->prev != NULL ||list->capacity != 0)
        {
            ERROR_PRINTF("LIST: List was already initialize");
            listDump(list, debugFile);
            return alreadyInitListProblem;
        }
        return noListProblem;
    }

    if(list->capacity < 0)
    {
        ERROR_PRINTF("LIST: Capacity problem");
        listDump(list, debugFile);
        return capacityListProblem;
    }

    if(list->data == NULL)
    {
        ERROR_PRINTF("LIST: massive with data - NULL pointer");
        listDump(list, debugFile);
        return dataNullPointerListProblem;
    }

    if(list->prev == NULL)
    {
        ERROR_PRINTF("LIST: massive with next pointer - NULL pointer");
        listDump(list, debugFile);
        return prevNullPointerListProblem;
    }

    if(list->next == NULL)
    {
        ERROR_PRINTF("LIST: massive with prevue pointer - NULL pointe");
        listDump(list, debugFile);
        return nextNullPointerListProblem;
    }

    int pointerH = list->next[0];
    int pointerT = list->prev[0];

    for(size_t i = 0; i < list->elementCount; i++)
    {
        if(pointerH < 0 || pointerH >= list->capacity ||
           pointerT < 0 || pointerT >= list->capacity)
        {
            ERROR_PRINTF("LIST: Index of jumping to new element out of range");
            listDump(list, debugFile);
            return connectionListProblem;
        }

        if (list->next[list->prev[pointerH]] != pointerH)
        {
            ERROR_PRINTF("LIST: Problem with list connection");
            listDump(list, debugFile);
            return connectionListProblem;
        }

        pointerH = list->next[pointerH];
        pointerT = list->prev[pointerT];
    }

    if (pointerH != 0 || pointerT != 0)
    {
        ERROR_PRINTF("LIST: Element count problem");
        listDump(list, debugFile);
        return countListProblem;
    }

    return noListProblem;
}

#endif

void listDump(const list_t *const list, dump_file*const dumpFile)
{
    assert(list);
    assert(dumpFile);
    if(dumpFile == NULL)
    {
        ERROR_PRINTF("Debug file pointer - NULL pointer");
        return;
    }

    size_t folderNameLength = strnlen(dumpFile->filePath, sizeof(dumpFile->filePath));

    if (folderNameLength == sizeof(dumpFile->filePath) || dumpFile->file == NULL)
    {
        ERROR_PRINTF("Debug file struct is corrupted");
        return;
    }

    FILE* mainLogFile = fopen(dumpFile->filePath, "a");
    if  (mainLogFile == NULL)
    {
        ERROR_PRINTF("Cannot open dump file");
        return;
    }

    char graphFileName[100] = "";
    snprintf(graphFileName, sizeof(graphFileName), "%s/graph_%d.dot", dumpFile->folderPath, dumpFile->graphCount);
    FILE* graphFile = fopen(graphFileName, "a");
    if  (graphFile == NULL)
    {
        ERROR_PRINTF("Cannot open graph_%d.dot file", dumpFile->graphCount);
        return;
    }
    dumpFile->graphCount++;

    fprintf(graphFile,
        "digraph G {\n"
        "  rankdir=LR;\n"
        "  bgcolor=\"#0a0c10\";\n"
        "  splines=ortho;\n"
        "  nodesep=0.8;\n"
        "  node[shape=Mrecord, style=filled, fillcolor=\"#b82222\", color=\"#4c0000\", fontname=Monospace, fontcolor=\"#65B7B7\"];\n"
        "  edge[color=\"#f9f9f7\", arrowhead=normal];\n"
        "  {rank = same; graph_0; main}\n"
        "  {rank = same; free_0; free}\n"
        "  main  [shape=Mrecord, label=\"Main data\", style=\"filled\", fillcolor=\"#1ca8a3\", color=\"#192832\", fontcolor =\"#131515\"];\n"
        "  free  [shape=Mrecord, label=\"Free \\<%d\\>\", style=\"filled\", fillcolor=\"#1ca8a3\", color=\"#192832\", fontcolor =\"#131515\"];\n",
        list->free_element
    );



    for(int i = 0; i < list->capacity; i++)
    {
        fprintf(mainLogFile, "[%d] = <%d>, prev = <%d>, next = <%d>\n", i, list->data[i], list->prev[i], list->next[i]);
    }

    fclose(mainLogFile);
    fclose(graphFile);
}

list_error_t listInit(list_t *const list, int list_capacity ON_DEBUG(, dump_file*const dumpFile))
{
    assert(list);

#ifdef LIST_DEBUG
    list_error_t problem1 = listVerifier(list, true, dumpFile);
    if (problem1 != noListProblem)
        return problem1;
#endif

    if (list_capacity < 0)
    {
        ERROR_PRINTF("Wrong capacity value");
        return capacityListProblem;
    }

    if (list_capacity < 2)
    {
        ERROR_PRINTF("Capacity must be more than one");
        return capacityListProblem;
    }

    list->capacity = list_capacity;

    list->data     = (list_element_t*)calloc(list_capacity, sizeof(list_element_t));
    assert(list->data);

    list->next     = (int*)           calloc(list_capacity, sizeof(int));
    assert(list->next);

    list->prev     = (int*)           calloc(list_capacity, sizeof(int));
    assert(list->prev);

    list->data[0]      = LIST_POISON_VALUE;

    list->next[0]      = 0;

    list->prev[0]      = 0;

    list->free_element = 1;

#ifdef LIST_DEBUG
    list->elementCount = 0;
#endif

    for (int i = 1; i < list_capacity; i++)
    {
        list->next[i] = (i == list_capacity - 1) ? 0 : i + 1;
        list->prev[i] = -1;
    }

#ifdef LIST_DEBUG
    list_error_t problem2 = listVerifier(list, false, dumpFile);
    if (problem2 != noListProblem)
        return problem2;
#endif

    return noListProblem;
}


list_error_t insertElem(list_t *const list, const list_element_t value, const int position ON_DEBUG(, dump_file*const dumpFile))
{
//TODO: убрать каст в проверках верификатора
#ifdef LIST_DEBUG
    int problem1 = listVerifier(list, false, dumpFile);
    if ((list_error_t) problem1 != noListProblem)
        return (list_error_t) problem1;
#endif

    assert(list);

    if (position > list->prev[0] || position < 0)
    {
        ERROR_PRINTF("LIST: Element position out of list range");
        return positionElementListProblem;
    }

    int free      = list->free_element;
    int next_free = list->next[free];
    int next      = list->next[position];

#ifdef LIST_DEBUG
    list->elementCount++;
#endif

    list->data[free]     = value;
    list->next[position] = free;
    list->next[free]     = next;
    list->prev[free]     = position;
    list->prev[next]     = free;
    list->free_element   = next_free;

#ifdef LIST_DEBUG
    int problem2 = listVerifier(list, false, dumpFile);
    if ((list_error_t) problem2 != noListProblem)
        return (list_error_t) problem2;
#endif

    return noListProblem;
}


list_error_t deleteElem(list_t *const list, const int position ON_DEBUG(, dump_file*const dumpFile))
{
#ifdef LIST_DEBUG
    int problem1 = listVerifier(list, false, dumpFile);
    if ((list_error_t) problem1 != noListProblem)
        return (list_error_t) problem1;
#endif

    assert(list);

    if (position > list->prev[0] || position < 0 || position == 0)
    {
        ERROR_PRINTF("LIST: Element position out of list range");
        return positionElementListProblem;
    }

    list->data[position] = 0;

    int next = list->next[position];
    int prev = list->prev[position];

#ifdef LIST_DEBUG
    list->elementCount--;
#endif

    list->next[position] = list->free_element;
    list->prev[position] = -1;
    list->next[prev]     = next;
    list->prev[next]     = prev;
    list->free_element   = position;

#ifdef LIST_DEBUG
    int problem2 = listVerifier(list, false, dumpFile);
    if ((list_error_t) problem2 != noListProblem)
        return (list_error_t) problem2;
#endif

    return noListProblem;
}


list_element_t getElement(const list_t *const list, const int position)
{
    assert(list);
    if (position > list->capacity || position < 1)
    {
        ERROR_PRINTF("LIST: Element position out of list range");
        return positionElementListProblem;
    }
    return list->data[position];
}
