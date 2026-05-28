#include "graph.h"
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
#include <math.h>

using namespace std;

Graph::Graph()
{
    actorCount = 0;
}

void Graph::insert(vector<string> data)
{
    string src_node = data[0];
    string dest_node = data[1];
    string weight = data[2];
    edges.push_back(tuple<string, string, string>(src_node, dest_node, weight));
    nodes.insert(src_node);
    nodes.insert(dest_node);
    list<Edge> temp = {Edge(dest_node, weight)};
    if (graph.find(src_node) == graph.end())
        graph.insert(pair<string, list<Edge>>(src_node, temp));
    else
    {
        graph[src_node].push_back(Edge(dest_node, weight));
    }
    temp = {Edge(src_node, weight)};
    if (graph.find(dest_node) == graph.end())
        graph.insert(pair<string, list<Edge>>(dest_node, temp));
    else
    {
        graph[dest_node].push_back(Edge(src_node, weight));
    }
}

bool Graph::BFS(queue<string> *queue, unordered_map<string, bool> *visited, unordered_map<string, Edge> *parent, string dest)
{
    string current = queue->front();
    queue->pop();
    for (Edge ca : graph[current])
    {
        if (visited->at(ca.dest_node) == false)
        {
            parent->insert(pair<string, Edge>(ca.dest_node, Edge(current, ca.weight)));
            visited->at(ca.dest_node) = true;
            queue->push(ca.dest_node);
            if (ca.dest_node == dest)
                return true;
        }
    }
    return false;
}

list<string> getfinalPath(unordered_map<string, Edge> s_parent, string src, string dest)
{
    list<string> path;
    string actor = dest;
    path.push_back(actor);
    while (actor != src)
    {
        path.push_back(s_parent.at(actor).dest_node);
        path.push_back(s_parent.at(actor).weight);
        actor = s_parent.at(actor).dest_node;
    }
    // cout << "path complete" << endl;
    reverse(path.begin(), path.end());
    return path;
}

list<string> Graph::shortestPath(string src, string dest)
{
    unordered_map<string, bool> s_visited;
    unordered_map<string, Edge> s_parent;
    queue<string> s_queue;
    for (pair<string, list<Edge>> pair : graph)
    {
        s_visited.insert(::pair<string, bool>(pair.first, false));
    }

    s_queue.push(src);
    s_visited[src] = true;

    s_parent.insert(::pair<string, Edge>(src, Edge("", "")));

    while (!s_queue.empty())
    {
        if (BFS(&s_queue, &s_visited, &s_parent, dest))
        {
            // cout << "finding path" << endl;
            return ::getfinalPath(s_parent, src, dest);
        }
    }
    list<string> temp = {""};
    return temp;
}

void Graph::printMap()
{
    for (pair<string, list<Edge>> pair : graph)
    {
        cout << "\033[1;32m" << pair.first << "\033[1;37m||";
        for (Edge Edge : pair.second)
        {
            cout << "\033[1;31m" << Edge.dest_node << "\033[1;37m:" << Edge.weight << "|";
        }
        cout << endl;
    }
}

bool Graph::exists(string src, string dest)
{
    return ((graph.find(src) != graph.end()) && (graph.find(dest) != graph.end()));
}

auto cmp = [](const pair<string, string> &a, const pair<string, string> &b)
{
    return stof(a.second) > stof(b.second);
};

vector<string> Graph::prims()
{
    set<string> visited;
    string start = *(nodes.begin());
    unordered_map<string, string> parent;
    unordered_map<string, float> key;

    for (string node : nodes)
    {
        key.insert(pair<string, float>(node, float(INT_MAX)));
        parent.insert(pair<string, string>(node, ""));
    }

    priority_queue<pair<string, string>, vector<pair<string, string>>, decltype(cmp)> pq(cmp);
    pq.push(pair<string, string>(start, "0"));
    key[start] = 0;

    int tree_cost = 0;

    while (!pq.empty())
    {
        pair<string, string> current = pq.top();
        pq.pop();
        string node = current.first;
        string cost = current.second;

        if (visited.find(node) == visited.end())
        {
            visited.insert(node);
            tree_cost += stoi(cost);

            for (Edge ca : graph[node])
            {
                if (visited.find(ca.dest_node) == visited.end() && key[ca.dest_node] > stof(ca.weight))
                {

                    pq.push(pair<string, string>(ca.dest_node, ca.weight));
                    parent[ca.dest_node] = node;
                    key[ca.dest_node] = stof(ca.weight);
                }
            }
        }
    }

    vector<string> path;
    for (pair<string, string> edge : parent)
    {
        path.push_back(edge.second);
        path.push_back(edge.first);
    }

    return path;
}