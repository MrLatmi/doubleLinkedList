#ifndef _LIST_
#define _LIST_

#define DUMP_SAMPLE_PATH "dumpSample.htm"
#define LIST_DEBUG


#ifdef LIST_DEBUG
#define ON_DEBUG(...) __VA_ARGS__
#else
#define ON_DEBUG(...)
#endif


#define LIST_POISON_VALUE 0xDED
typedef int list_element_t;

struct dump_file
{
    FILE*   file;
    char    filePath   [100];
    char    folderPath [100];
    int     graphCount      ;
};

enum list_error_t
{
    noListProblem,
    capacityListProblem,
    connectionListProblem,
    elementCountListProblem,
    pointerListProblem,
    alreadyInitListProblem,
    positionElementListProblem,
    dataNullPointerListProblem,
    prevNullPointerListProblem,
    nextNullPointerListProblem,
    countListProblem,
    verifierListProblem,
};

//TODO: prev, next, capacity to size_t
//TODO: position to struct elementIdentifier
struct list_t
{
    list_element_t* data;
    int*            next;
    int*            prev;

    int capacity;
    int free_element;

#ifdef LIST_DEBUG
    size_t elementCount;
#endif
};


#ifdef LIST_DEBUG
dump_file       debugFileCreate (void);
list_error_t    listVerifier    (const list_t* list, bool firstInit, dump_file* debugFile);
#endif

list_error_t    listInit        (      list_t* list, int list_capacity ON_DEBUG(, dump_file* dumpFile));

list_error_t    insertElem      (      list_t* list, const list_element_t value,    int position ON_DEBUG(, dump_file* dumpFile));
list_error_t    deleteElem      (      list_t* list,                                int position ON_DEBUG(, dump_file* dumpFile));
list_element_t  getElement      (const list_t* list, int position);

void            listDump        (const list_t* list, dump_file *debugFile);


#endif
