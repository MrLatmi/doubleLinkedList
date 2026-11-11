#include <TXLib.h>
#include "list.cpp"
#include "formatingPrint.h"



int main(void)
{
    dump_file dumpFile  = debugFileCreate();
    list_t myList       = {};
    size_t capacity     = 20;

    listInit(&myList, capacity, &dumpFile);
    list_position_t pos= {};
    pos.pos = 0;
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);
    insertElem(&myList, 10, &pos, &pos, &dumpFile);

    listDump(&myList, &dumpFile);

}

