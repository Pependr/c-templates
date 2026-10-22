#include <ctype.h>
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>


typedef unsigned long node_t;

#define T node_t
#define SUFFIX nodes
#define DEFAULT_CAP 8
#include "vector.h"

#define K node_t
#define V vector_nodes_s
#define HASH_FN
#define CMP_FN hashmap_intcmp
#define SUFFIX graph
#define DEFAULT_CAP 32
#include "hashmap.h"

#define K node_t
#define V char
#define HASH_FN
#define CMP_FN hashmap_intcmp
#define SUFFIX visited
#define DEFAULT_CAP 32
#include "hashmap.h"

#define T node_t
#define SUFFIX nodes
#define DEFAULT_CAP 16
#include "deque.h"


bool parse_file(const char *path, hashmap_graph_s *graph) {
    struct stat stats = {0};
    if (stat(path, &stats) < 0) return false;

    int filedes = open(path, O_RDONLY);
    if (filedes < 0) return false;

    char *text = mmap(
        NULL, stats.st_size,
        PROT_READ, MAP_PRIVATE,
        filedes, 0
    );
    if (text == MAP_FAILED) return false;

    char *cur = text;
    char *end = text + stats.st_size;

    while (cur < end) {
        node_t parent = strtoul(cur, &cur, 10);
        
        while (isspace(*cur)) cur += 1;

        node_t child = strtoul(cur, &cur, 10);
        
        if (!hashmap_graph_has(graph, parent)) {
            vector_nodes_s children = {0};
            if (!vector_nodes_init(&children))
                return false;

            if (!hashmap_graph_set(graph, parent, children))
                return false;
        }

        vector_nodes_s *children;
        hashmap_graph_get_ptr(graph, parent, &children);

        if (!vector_nodes_append(children, child))
            return false;

        while (*cur != '\n') cur += 1;

        cur += 1;
    }

    munmap(text, stats.st_size);
    close(filedes);

    return true;
}


void visit_node(node_t node) {
    printf("Visited node %ld\n", node);
}

typedef typeof(visit_node) callback_t;

bool bfs(const hashmap_graph_s *graph, node_t src, callback_t callback) {
    deque_nodes_s queued = {0};
    if (!deque_nodes_init(&queued)) return false;

    hashmap_visited_s visited = {0}; // I'm too lazy to implement hashset just for this
    if (!hashmap_visited_init(&visited)) return false;

    deque_nodes_push_right(&queued, src);

    while (queued.len > 0) {
        node_t next;
        if (!deque_nodes_pop_left(&queued, &next))
            return false;

        if (hashmap_visited_has(&visited, next))
            continue;

        callback(next);

        if (!hashmap_visited_set(&visited, next, 0))
            return false;

        vector_nodes_s *children;
        if (!hashmap_graph_get_ptr(graph, next, &children))
            continue;

        for (size_t i = 0; i < children->len; ++i)
            deque_nodes_push_right(&queued, children->items[i]);
    }

    return true;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s {INPUT FILE}\n", argv[0]);
        return 1;
    }

    hashmap_graph_s graph = {0};
    if (!hashmap_graph_init(&graph)) return 1;

    if (!parse_file(argv[1], &graph)) return 1;

    if (!bfs(&graph, 1, visit_node)) return 1;

    return 0;
}

