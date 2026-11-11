#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <direct.h>
#include <windows.h>

#include <time.h>
#include "list.h"
#include "formatingPrint.h"

#include "dumpList.cpp"

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
//TODO: delete magic numbers, create FILE_PATH_LENGTH
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

    strncpy(debugFileStruct.folderPath, folderName, sizeof(filePath) - 1);

    debugFileStruct.graphCount = 0;
    if (!CopyFile(DUMP_HEADER_PATH, filePath, FALSE))
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

    if(list->capacity > SIZE_T_CHECKER)
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

    size_t pointerH = list->next[0];
    size_t pointerT = list->prev[0];

    for(size_t i = 0; i < list->elementCount; i++)
    {
        if(pointerH > SIZE_T_CHECKER || pointerH >= list->capacity ||
           pointerT > SIZE_T_CHECKER || pointerT >= list->capacity)
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




list_error_t listInit(list_t *const list, size_t list_capacity ON_DEBUG(, dump_file*const dumpFile))
{
    assert(list);

#ifdef LIST_DEBUG
    list_error_t problem1 = listVerifier(list, true, dumpFile);
    if (problem1 != noListProblem)
        return problem1;
#endif

    if (list_capacity > SIZE_MAX)
    {
        ERROR_PRINTF("Wrong capacity value");
        return capacityListProblem;
    }

    if (list_capacity < 2)
    {
        ERROR_PRINTF("Capacity must be more than one");
        return capacityListProblem;
    }

    list->capacity      = list_capacity;

    list->data          = (list_element_t*)calloc(list_capacity, sizeof(list_element_t));
    assert(list->data);

    list->next          = (size_t*)           calloc(list_capacity, sizeof(size_t));
    assert(list->next);

    list->prev          = (size_t*)           calloc(list_capacity, sizeof(size_t));
    assert(list->prev);

    list->data[0]       = LIST_POISON_VALUE;

    list->next[0]       = 0;

    list->prev[0]       = 0;

    list->free_element  = 1;

#ifdef LIST_DEBUG
    list->elementCount  = 0;
#endif

    for (size_t i = 1; i < list_capacity; i++)
    {
        list->next[i] = (i == list_capacity - 1) ? LAST_ELEM_POISON_VAL : i + 1;
        list->prev[i] = SIZE_MAX;
    }

#ifdef LIST_DEBUG
    list_error_t problem2 = listVerifier(list, false, dumpFile);
    if (problem2 != noListProblem)
        return problem2;
#endif

    return noListProblem;
}


list_error_t insertElem(list_t *const list, const list_element_t value, const list_position_t* positionIns, list_position_t* outPosition ON_DEBUG(, dump_file*const dumpFile))
{
#ifdef LIST_DEBUG
    list_error_t problem1 = listVerifier(list, false, dumpFile);
    if (problem1 != noListProblem)
        return problem1;
#endif
    size_t position = positionIns->pos;
    assert(list);

    if (position > list->prev[0] || position > SIZE_T_CHECKER)
    {
        ERROR_PRINTF("LIST: Element position out of list range");
        return positionElementListProblem;
    }

    size_t free      = list->free_element;
    size_t next_free = list->next[free];
    size_t next      = list->next[position];

#ifdef LIST_DEBUG
    list->elementCount++;
#endif
    outPosition->pos    = free;
    list->data[free]     = value;
    list->next[position] = free;
    list->next[free]     = next;
    list->prev[free]     = position;
    list->prev[next]     = free;
    list->free_element   = next_free;

#ifdef LIST_DEBUG
    list_error_t problem2 = listVerifier(list, false, dumpFile);
    if ( problem2 != noListProblem)
        return problem2;
#endif

    return noListProblem;
}


list_error_t deleteElem(list_t *const list, const list_position_t* positionDel, list_position_t* outPosition ON_DEBUG(, dump_file*const dumpFile))
{
#ifdef LIST_DEBUG
    list_error_t problem1 = listVerifier(list, false, dumpFile);
    if ( problem1 != noListProblem)
        return problem1;
#endif

    size_t position = positionDel->pos;
    assert(list);

    if (position > list->prev[0] || position > SIZE_T_CHECKER || position == 0)
    {
        ERROR_PRINTF("LIST: Element position out of list range");
        return positionElementListProblem;
    }

    list->data[position] = 0;

    size_t next         = list->next[position];
    size_t prev         = list->prev[position];
    outPosition->pos    = prev;
#ifdef LIST_DEBUG
    list->elementCount--;
#endif

    list->next[position] = list->free_element;
    list->prev[position] = SIZE_MAX;
    list->next[prev]     = next;
    list->prev[next]     = prev;
    list->free_element   = position;

#ifdef LIST_DEBUG
    list_error_t problem2 = listVerifier(list, false, dumpFile);
    if (problem2 != noListProblem)
        return problem2;
#endif

    return noListProblem;
}


list_element_t getElement(const list_t *const list, const list_position_t* position)
{
    assert(list);
    if (position->pos > list->capacity || position->pos < 1)
    {
        ERROR_PRINTF("LIST: Element position out of list range");
        return positionElementListProblem;
    }
    return list->data[position->pos];
}
