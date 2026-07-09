#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define MAX_NODES 20
#define MAX_UNITS 5
#define INF INT_MAX
#define STATE_FILE "system_state.bin"

typedef struct Edge {
    int dest;
    int weight;
    struct Edge* next;
} Edge;

typedef struct {
    Edge* head;
} AdjList;

typedef struct {
    int id;
    int severity; 
    int location; 
    char type[20]; // Added to track required unit type
    float priorityScore;
} Emergency;

typedef struct {
    int unitId;
    char type[20]; // "Ambulance" or "Firetruck"
    int currentLocation;
    int isAvailable; 
    int currentEmergencyId;
    time_t availableAt;
    char lastPath[100]; 
} Unit;

AdjList graph[MAX_NODES];
Unit units[MAX_UNITS];
Emergency heap[100];
int heapSize = 0;

void getPathString(int parent[], int j, char* buffer) {
    if (j == -1) return;
    if (parent[j] == -1) {
        sprintf(buffer + strlen(buffer), "%d", j);
        return;
    }
    getPathString(parent, parent[j], buffer);
    sprintf(buffer + strlen(buffer), " -> %d", j);
}

void addEdge(int src, int dest, int weight, int isTwoWay) {
    Edge* node = (Edge*)malloc(sizeof(Edge));
    node->dest = dest;
    node->weight = weight;
    node->next = graph[src].head;
    graph[src].head = node;
    if (isTwoWay) {
        node = (Edge*)malloc(sizeof(Edge));
        node->dest = src;
        node->weight = weight;
        node->next = graph[dest].head;
        graph[dest].head = node;
    }
}

void buildMap() {
    for (int i = 0; i < MAX_NODES; i++) graph[i].head = NULL;
    for (int r = 0; r < 20; r += 5) {
        for (int c = 0; c < 4; c++) addEdge(r + c, r + c + 1, 2, 1);
    }
    for (int c = 0; c < 5; c++) {
        for (int r = 0; r < 15; r += 5) addEdge(r + c, r + c + 5, 3, 1);
    }
    addEdge(3, 8, 4, 0); addEdge(7, 12, 5, 0); addEdge(12, 17, 4, 0);
}

void saveState() {
    FILE *file = fopen(STATE_FILE, "wb");
    if (file) {
        fwrite(units, sizeof(Unit), MAX_UNITS, file);
        fwrite(&heapSize, sizeof(int), 1, file);
        if (heapSize > 0) fwrite(heap, sizeof(Emergency), heapSize, file);
        fclose(file);
    }
}

void loadState() {
    FILE *file = fopen(STATE_FILE, "rb");
    if (file) {
        fread(units, sizeof(Unit), MAX_UNITS, file);
        fread(&heapSize, sizeof(int), 1, file);
        if (heapSize > 0) fread(heap, sizeof(Emergency), heapSize, file);
        fclose(file);
    } else {
        units[0] = (Unit){1, "Ambulance", 0, 1, -1, 0, "Base"};
        units[1] = (Unit){2, "Firetruck", 19, 1, -1, 0, "Base"};
        units[2] = (Unit){3, "Ambulance", 10, 1, -1, 0, "Base"};
        units[3] = (Unit){4, "Ambulance", 5, 1, -1, 0, "Base"};
        units[4] = (Unit){5, "Firetruck", 14, 1, -1, 0, "Base"};
    }
}

void dijkstra(int startNode, int dist[], int parent[]) {
    int visited[MAX_NODES] = {0};
    for (int i = 0; i < MAX_NODES; i++) {
        dist[i] = INF; parent[i] = -1;
    }
    dist[startNode] = 0;
    for (int count = 0; count < MAX_NODES - 1; count++) {
        int min = INF, u = -1;
        for (int v = 0; v < MAX_NODES; v++)
            if (!visited[v] && dist[v] <= min) { min = dist[v]; u = v; }
        if (u == -1) break;
        visited[u] = 1;
        Edge* temp = graph[u].head;
        while (temp) {
            if (!visited[temp->dest] && dist[u] != INF && dist[u] + temp->weight < dist[temp->dest]) {
                dist[temp->dest] = dist[u] + temp->weight;
                parent[temp->dest] = u;
            }
            temp = temp->next;
        }
    }
}

void pushHeap(Emergency e) {
    heap[heapSize] = e;
    int i = heapSize++;
    while (i != 0 && heap[(i - 1) / 2].priorityScore < heap[i].priorityScore) {
        Emergency temp = heap[i]; heap[i] = heap[(i - 1) / 2]; heap[(i - 1) / 2] = temp;
        i = (i - 1) / 2;
    }
}

Emergency popHeap() {
    Emergency root = heap[0];
    heap[0] = heap[--heapSize];
    int i = 0;
    while (2 * i + 1 < heapSize) {
        int j = 2 * i + 1;
        if (j + 1 < heapSize && heap[j + 1].priorityScore > heap[j].priorityScore) j++;
        if (heap[i].priorityScore >= heap[j].priorityScore) break;
        Emergency temp = heap[i]; heap[i] = heap[j]; heap[j] = temp;
        i = j;
    }
    return root;
}

void processEmergency() {
    if (heapSize == 0) return;
    Emergency current = popHeap();
    int dists[MAX_NODES], parent[MAX_NODES];
    dijkstra(current.location, dists, parent);

    int bestUnit = -1, minDist = INF;
    for (int i = 0; i < MAX_UNITS; i++) {
        // Updated logic: Match unit type with incident type
        if (units[i].isAvailable && strcmp(units[i].type, current.type) == 0) {
            if (dists[units[i].currentLocation] < minDist) {
                minDist = dists[units[i].currentLocation];
                bestUnit = i;
            }
        }
    }

    if (bestUnit != -1) {
        int p_dist[MAX_NODES], p_parent[MAX_NODES];
        dijkstra(units[bestUnit].currentLocation, p_dist, p_parent);
        memset(units[bestUnit].lastPath, 0, 100);
        getPathString(p_parent, current.location, units[bestUnit].lastPath);

        units[bestUnit].isAvailable = 0;
        units[bestUnit].currentEmergencyId = current.id;
        units[bestUnit].availableAt = time(NULL) + (2 * current.severity);
        units[bestUnit].currentLocation = current.location;
    } else {
        current.priorityScore += 2; pushHeap(current);
    }
}

int main(int argc, char *argv[]) {
    buildMap(); loadState();
    time_t now = time(NULL);
    for (int i = 0; i < MAX_UNITS; i++) {
        if (!units[i].isAvailable && now >= units[i].availableAt) {
            units[i].isAvailable = 1; units[i].currentEmergencyId = -1;
        }
    }
    if (argc > 1) {
        if (strcmp(argv[1], "add") == 0) {
            Emergency e;
            e.id = atoi(argv[2]);
            e.severity = atoi(argv[3]);
            e.location = atoi(argv[4]);
            strcpy(e.type, argv[5]); // Store the specific unit type required
            e.priorityScore = e.severity * 10.0;
            pushHeap(e);
        } else if (strcmp(argv[1], "dispatch") == 0) processEmergency();
        else if (strcmp(argv[1], "status") == 0) {
            printf("{ \"units\": [");
            for (int i = 0; i < MAX_UNITS; i++) {
                printf("{\"id\":%d, \"type\":\"%s\", \"loc\":%d, \"status\":\"%s\", \"emergency\":%d, \"path\":\"%s\"}", 
                       units[i].unitId, units[i].type, units[i].currentLocation, 
                       units[i].isAvailable ? "Available" : "BUSY", 
                       units[i].currentEmergencyId, units[i].lastPath);
                if (i < MAX_UNITS - 1) printf(",");
            }
            printf("], \"heapSize\": %d }", heapSize);
        }
        else if (strcmp(argv[1], "complete") == 0) {
            int uid = atoi(argv[2]);
            for (int i = 0; i < MAX_UNITS; i++) {
                if (units[i].unitId == uid) {
                    units[i].isAvailable = 1;
                    units[i].currentEmergencyId = -1;
                    strcpy(units[i].lastPath, "Base");
                }
            }
        }

         else if (strcmp(argv[1], "reset") == 0) remove(STATE_FILE);
    }
    saveState();
    return 0;
}