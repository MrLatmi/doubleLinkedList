#ifndef _LIST_
#define _LIST_

#define DUMP_HEADER_PATH "dumpSample.htm"
#define LIST_DEBUG
#define LAST_ELEM_POISON_VAL (SIZE_MAX - 100)
#define SIZE_T_CHECKER (SIZE_MAX - 1000)

#ifdef LIST_DEBUG
#define ON_DEBUG(...) __VA_ARGS__
#else
#define ON_DEBUG(...)
#endif


#define LIST_POISON_VALUE 0xDED
typedef int list_element_t;

struct dump_file
{
    FILE*   file            ;
    char    folderPath [100];
    size_t  graphCount      ;
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

//TODO: position to struct elementIdentifier

struct list_position_t
{
    size_t pos;
};
struct list_t
{
    list_element_t* data;
    size_t*            next;
    size_t*            prev;

    size_t capacity;
    size_t free_element;

#ifdef LIST_DEBUG
    size_t elementCount;
#endif
};


#ifdef LIST_DEBUG
dump_file       debugFileCreate (void);
list_error_t    listVerifier    (const list_t* list, bool firstInit, dump_file* debugFile);
#endif

list_error_t    listInit        (      list_t* list, size_t list_capacity ON_DEBUG(, dump_file* dumpFile));

list_error_t    insertElem      (      list_t* list, const list_element_t value,    const list_position_t* positionIns, list_position_t* outPosition ON_DEBUG(, dump_file* dumpFile));
list_error_t    deleteElem      (      list_t* list,                                const list_position_t* positionDel, list_position_t* outPosition ON_DEBUG(, dump_file* dumpFile));
list_element_t  getElement      (const list_t* list,                                const list_position_t* position);

void            listDump        (const list_t* list, dump_file *debugFile);


#endif
