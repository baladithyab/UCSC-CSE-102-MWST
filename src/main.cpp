#include "graph.h"
#include <iostream>
#include <stack>
#include <stdexcept>
#include <fstream>
#include <array>
#include <tuple>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <string>
#include <chrono>
using namespace std;
using namespace std::chrono;

auto cmpy = [](const tuple<string, string, string, int> &a, const tuple<string, string, string, int> &b)
{
    return stof(get<2>(a)) > stof(get<2>(b));
};

int main(int argc, char const **argv)
{
    if (argc < 3)
    {
        cout << "Usage: ./MWST <INPUT FILE> <OUTPUTFILE>";
        exit(EXIT_FAILURE);
    }

    ifstream in;
    ofstream out;
    ofstream log;
    in.open(argv[1]);
    out.open(argv[2]);
    log.open("OutputLog.txt");
    if (!in.is_open())
    {
        printf("Unable to read file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    Graph mygraph;
    string MLBuffer;
    auto starttotal = high_resolution_clock::now();
    getline(in, MLBuffer);
    int numVerts = stoi(MLBuffer);
    getline(in, MLBuffer);
    int numEdges = stoi(MLBuffer);
    while (getline(in, MLBuffer))
    {
        if (MLBuffer.length() == 0)
            continue;

        stringstream inputstream(MLBuffer);
        string val;
        vector<string> vals;
        while (inputstream >> val)
            vals.push_back(val);
        mygraph.insert(vals);
    }
    auto stoptotal = high_resolution_clock::now();
    auto durationstotal = duration_cast<seconds>(stoptotal - starttotal);
    auto durationmstotal = duration_cast<milliseconds>(stoptotal - starttotal);
    auto durationustotal = duration_cast<microseconds>(stoptotal - starttotal);
    log << "Total Time to Insert Data: " << durationstotal.count() << " seconds or "
        << durationmstotal.count() << " milliseconds or " << durationustotal.count() << " microseconds" << endl;
    log << "***********************************************************" << endl;
    string temp;
    starttotal = high_resolution_clock::now();
    auto prims = mygraph.prims();
    // cout << "found prims" << endl;
    priority_queue<tuple<string, string, string, int>, vector<tuple<string, string, string, int>>, decltype(cmpy)> pq(cmpy);
    for (int i = 0; i < prims.size(); i++)
    {
        string node1 = prims[i];
        string node2 = prims[++i];
        for (int j = 0; j < mygraph.edges.size(); j++)
        {
            if (get<0>(mygraph.edges[j]) == node1 && get<1>(mygraph.edges[j]) == node2 || get<0>(mygraph.edges[j]) == node2 && get<1>(mygraph.edges[j]) == node1)
            {
                pq.push(tuple<string, string, string, int>(get<0>(mygraph.edges[j]), get<1>(mygraph.edges[j]), get<2>(mygraph.edges[j]), j + 1));
            }
        }
    }
    // cout << "order edges" << endl;
    float cost = 0;
    while (!pq.empty())
    {
        tuple<string, string, string, int> temp = pq.top();
        // cout << tuple_size<decltype(temp)>::value << endl;
        // cout << " " << get<3>(temp) << ": "
        //      << "(" << get<0>(temp) << ", " << get<1>(temp) << ") " << stof(get<2>(temp)) << endl;
        char *tempy = (char *)malloc(sizeof(char) * 100);
        sprintf(tempy, "%*d: (%s, %s) %.1f\n", 4, get<3>(temp), get<0>(temp).c_str(), get<1>(temp).c_str(), stof(get<2>(temp)));
        out << tempy;
        // printf("%*d: (%s, %s) %.1f\n", 4, get<3>(temp), get<0>(temp).c_str(), get<1>(temp).c_str(), stof(get<2>(temp)));
        cost += stof(get<2>(temp));
        pq.pop();
    }
    // cout << "print edges" << endl;
    char *tempy2 = (char *)malloc(sizeof(char) * 100);
    sprintf(tempy2, "Total Weight = %.2f\n", cost);
    out << tempy2;
    // printf("Total Cost: %.2f\n", cost);

    stoptotal = high_resolution_clock::now();

    durationstotal = duration_cast<seconds>(stoptotal - starttotal);
    durationmstotal = duration_cast<milliseconds>(stoptotal - starttotal);
    durationustotal = duration_cast<microseconds>(stoptotal - starttotal);
    log << "Total Time to Find MWST: " << durationstotal.count() << " seconds or "
        << durationmstotal.count() << " milliseconds or " << durationustotal.count() << " microseconds" << endl;
    log << "***********************************************************" << endl;
    in.close();
    out.close();
    log.close();
}
