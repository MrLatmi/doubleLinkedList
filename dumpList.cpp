#define STYLE_BAD  "fillcolor=\"#b82222\", color=\"#4c0000\", fontcolor =\"#cf8bb9\""
#define STYLE_FREE "fillcolor=\"#ddaf0b\", color=\"#8e6a06\", fontcolor =\"#302214\""
#define STYLE_DATA "fillcolor=\"#27512e\", color=\"#092604\", fontcolor =\"#65B7B7\""


//TODO: вершины в глобальном порядке
static bool isDataNodeBad(const list_t* list, const size_t* mark, size_t i)
{
    if (mark[i] != 2)
        return true;

    size_t next = list->next[i];
    size_t prev = list->prev[i];

    return next >= SIZE_T_CHECKER || next >= list->capacity ||
           prev >= SIZE_T_CHECKER || prev >= list->capacity ||
           list->prev[next] != i || list->next[prev] != i;
}

static bool isFreeNodeBad(const list_t* list, size_t i)
{
    return list->prev[i] < SIZE_T_CHECKER;
}


static void dumpLink(FILE* file, const size_t* mark, const char* fromType, size_t from, size_t to, bool bad)
{
    fprintf(file, "    %s_%zu -> %s_%zu [constraint=false%s];\n",
            fromType, from, mark[to] == 1 ? "free" : "graph", to,
            bad ? ", color=\"#e01b1b\", penwidth=2" : "");
}


void listDump(const list_t *const list, dump_file *const dumpFile)
{
    if (list == NULL || list->data == NULL || list->next == NULL || list->prev == NULL ||
        list->capacity >= SIZE_T_CHECKER || dumpFile == NULL || dumpFile->file == NULL)
    {
        ERROR_PRINTF("Dump: bad list or debug file");
        return;
    }

    const size_t cap = list->capacity;

    char graphFileName[200] = "";
    snprintf(graphFileName, sizeof(graphFileName), "%s/graph_%zu.dot",
             dumpFile->folderPath, dumpFile->graphCount);

    FILE* graphFile = fopen(graphFileName, "w");

    size_t* dataIndexOrder = (size_t*) calloc(cap, sizeof(size_t));
    size_t* freeIndexOrder = (size_t*) calloc(cap, sizeof(size_t));
    size_t* mark           = (size_t*) calloc(cap, sizeof(size_t));

    if (graphFile == NULL || dataIndexOrder == NULL || freeIndexOrder == NULL || mark == NULL)
    {
        ERROR_PRINTF("Dump: cannot open graph file or allocate memory");
        if (graphFile)
            fclose(graphFile);

        free(dataIndexOrder);
        free(freeIndexOrder);
        free(mark);
        return;
    }


    size_t freeCount = 0;
    for (size_t i = list->free_element; i < SIZE_T_CHECKER && i < cap && mark[i] == 0; i = list->next[i])
    {
        mark[i] = 1;
        freeIndexOrder[freeCount++] = i;
    }

    size_t dataCount = 0;
    for (size_t i = 0; i < SIZE_T_CHECKER && i < cap && mark[i] == 0; i = list->next[i])
    {
        mark[i] = 2;
        dataIndexOrder[dataCount++] = i;
    }

    for (size_t i = 0; i < cap; i++)
        if (mark[i] == 0)
            dataIndexOrder[dataCount++] = i;

    fprintf(graphFile,
        "digraph G {\n"
        "  rankdir=LR;\n"
        "  bgcolor=\"#0a0c10\";\n"
        "  splines=ortho;\n"
        "  nodesep=0.8;\n"
        "  node[shape=Mrecord, style=filled, fillcolor=\"#b82222\", color=\"#4c0000\", fontname=Monospace, fontcolor=\"#65B7B7\"];\n"
        "  edge[color=\"#f9f9f7\", arrowhead=normal];\n"
        "  {rank = same; graph_0; main}\n"
        "  main  [shape=Mrecord, label=\"Main data\", style=\"filled\", fillcolor=\"#1ca8a3\", color=\"#192832\", fontcolor =\"#131515\"];\n");

    if (freeCount > 0)
        fprintf(graphFile,
            "  free  [shape=Mrecord, label=\"Free \\<%zu\\>\", style=\"filled\", fillcolor=\"#1ca8a3\", color=\"#192832\", fontcolor =\"#131515\"];\n"
            "  {rank = same; free_%zu; free}\n", list->free_element, freeIndexOrder[0]);


    for (size_t i = 0; i < dataCount; i++)
    {
        size_t elementIdx      = dataIndexOrder[i];
        const char* style   = isDataNodeBad(list, mark, elementIdx) ? STYLE_BAD : STYLE_DATA;

        fprintf(graphFile, "    graph_%zu [shape=Mrecord, label=\"position \\<%zu\\>| data \\<%d\\>| {next \\<%zu\\>| prev \\<%zu\\>}\", style=\"filled\", %s];\n",
                elementIdx, elementIdx, list->data[elementIdx], list->next[elementIdx], list->prev[elementIdx], style);

        if (i + 1 < dataCount)
            fprintf(graphFile, "    graph_%zu -> graph_%zu [style=invis, weight=100];\n",
                    elementIdx, dataIndexOrder[i + 1]);
    }

    for (size_t i = 0; i < freeCount; i++)
    {
        size_t elementIdx = freeIndexOrder[i];
        const char* style = isFreeNodeBad(list, elementIdx) ? STYLE_BAD : STYLE_FREE;

        fprintf(graphFile, "    free_%zu [shape=Mrecord, label=\"position \\<%zu\\>| next \\<%zu\\>\", style=\"filled\", %s];\n",
                elementIdx, elementIdx, list->next[elementIdx], style);
        if (i + 1 < freeCount)
            fprintf(graphFile, "    free_%zu -> free_%zu [style=invis, weight=100];\n",
                    elementIdx, freeIndexOrder[i + 1]);
    }

    for (size_t i = 0; i < cap; i++)
    {
        if (mark[i] == 1)
            continue;

        size_t next = list->next[i];
        size_t prev = list->prev[i];

        if (next < SIZE_T_CHECKER  && next < cap)
            dumpLink(graphFile, mark, "graph", i, next, list->prev[next] != i);

        if (prev < SIZE_T_CHECKER && prev < cap)
            dumpLink(graphFile, mark, "graph", i, prev, list->next[prev] != i);
    }

    for (size_t i = 0; i < cap; i++)
    {
        if (mark[i] != 1)
            continue;

        size_t next = list->next[i];
        if (next < SIZE_T_CHECKER && next < cap)
            dumpLink(graphFile, mark, "free", i, next, mark[next] != 1);
    }

    fprintf(graphFile, "}\n");
    fclose(graphFile);
    free(dataIndexOrder);
    free(freeIndexOrder);
    free(mark);

    char command[512] = "";
    snprintf(command, sizeof(command), "dot -Tsvg %s/graph_%zu.dot -o %s/graph_%zu.svg",
             dumpFile->folderPath, dumpFile->graphCount, dumpFile->folderPath, dumpFile->graphCount);
    system(command);

    time_t nowTime = time(NULL);
    char   timeStr[32] = "";
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&nowTime));

    fprintf(dumpFile->file, "<div class=\"section\">"
                            "<div class=\"info\">"
                            "<div><b>Time:</b> %s</div>"
                            "<div><b>Info:</b> head &lt;%zu&gt;, tail &lt;%zu&gt;, free &lt;%zu&gt;</div>"
                            "<div><b>Capacity:</b> %zu</div>"
                            "</div>"
                            "<div class=\"graph-box\"><img src=\"graph_%zu.svg\"></div>"
                            "</div>\n",
                            timeStr, list->next[0], list->prev[0], list->free_element, cap, dumpFile->graphCount);

    dumpFile->graphCount++;
}
