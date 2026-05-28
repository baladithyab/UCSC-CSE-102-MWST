#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <set>
#include <iterator>
#include <unordered_map>
#include <list>
#include <queue>
#include <vector>

using namespace std;

class Edge
{
public:
    string weight;
    string dest_node;

    Edge()
    {
        weight = dest_node = "";
    }

    Edge(string Dest_node, string Weight)
    {
        weight = Weight;
        dest_node = Dest_node;
    }
    bool operator==(const Edge &ca) const { return (weight == ca.weight && dest_node == ca.dest_node); }
    bool operator!=(const Edge &ca) const { return (!operator==(ca)); }
};

class Graph
{
public:
    vector<tuple<string, string, string>> edges;

private:
    set<string> nodes;

    unordered_map<string, list<Edge>> graph;
    int actorCount;

public:
    Graph();
    void insert(vector<string>);
    list<string> shortestPath(string, string);
    bool BFS(queue<string> *, unordered_map<string, bool> *, unordered_map<string, Edge> *, string);
    string intersect(unordered_map<string, bool> *, unordered_map<string, bool> *);
    list<string> getfinalPath(unordered_map<string, Edge>, unordered_map<string, Edge>, string, string, string);
    void printMap();
    bool exists(string, string);
    vector<string> prims();
};

#endif
