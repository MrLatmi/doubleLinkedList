#include <TXLib.h>
#include "list.cpp"
#include "formatingPrint.h"



//TODO: connect graphviz


int  main(void)
{
    int capacity = 8;
    list_t list = {};
    dump_file dumpFile = debugFileCreate ();

    listInit(&list, capacity, &dumpFile);
    listDump(&list, &dumpFile);
    insertElem(&list, 10, 0, &dumpFile);
    listDump(&list, &dumpFile);
    insertElem(&list, 20, 1, &dumpFile);
    listDump(&list, &dumpFile);
    insertElem(&list, 30, 2, &dumpFile);

    listDump(&list, &dumpFile);
}
