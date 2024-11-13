#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <bits/stdc++.h>

using namespace std;

// Graph structure
struct Graph
{
    int numAgents, numHouses;
    vector<vector<int>> adj;                                        // adj[a] contains houses that agent a finds acceptable
    Graph(int a, int h) : numAgents(a), numHouses(h), adj(a + 1) {} // One-based indexing
};

// Phase 1: Hopcroft-Karp Algorithm to find maximal matching
bool bfs(vector<int> &matchA, vector<int> &matchH, vector<int> &dist, const Graph &graph)
{
    queue<int> Q;
    for (int a = 1; a <= graph.numAgents; a++)
    {
        if (matchA[a] == 0)
        { // Use 0 to represent unmatched
            dist[a] = 0;
            Q.push(a);
        }
        else
        {
            dist[a] = INT_MAX;
        }
    }
    dist[0] = INT_MAX; // Use 0 for the NIL node

    while (!Q.empty())
    {
        int a = Q.front();
        Q.pop();
        if (dist[a] < dist[0])
        {
            for (int h : graph.adj[a])
            {
                if (dist[matchH[h]] == INT_MAX)
                {
                    dist[matchH[h]] = dist[a] + 1;
                    Q.push(matchH[h]);
                }
            }
        }
    }
    return dist[0] != INT_MAX;
}

bool dfs(int a, vector<int> &matchA, vector<int> &matchH, vector<int> &dist, const Graph &graph)
{
    if (a != 0)
    {
        for (int h : graph.adj[a])
        {
            if (dist[matchH[h]] == dist[a] + 1)
            {
                if (dfs(matchH[h], matchA, matchH, dist, graph))
                {
                    matchH[h] = a;
                    matchA[a] = h;
                    return true;
                }
            }
        }
        dist[a] = INT_MAX;
        return false;
    }
    return true;
}

int hopcroftKarp(const Graph &graph, vector<int> &matchA, vector<int> &matchH)
{
    matchA.assign(graph.numAgents + 1, 0); // One-based indexing
    matchH.assign(graph.numHouses + 1, 0); // One-based indexing
    vector<int> dist(graph.numAgents + 1);

    int matchingSize = 0;
    while (bfs(matchA, matchH, dist, graph))
    {
        for (int a = 1; a <= graph.numAgents; a++)
        {
            if (matchA[a] == 0 && dfs(a, matchA, matchH, dist, graph))
            {
                matchingSize++;
            }
        }
    }
    return matchingSize;
}

// Phase 2: Make the matching trade-in-free
void makeTradeInFree(vector<int> &matchA, vector<int> &matchH, const Graph &graph)
{
    vector<list<pair<int, int>>> prefLists(graph.numHouses + 1);
    vector<int> curRank(graph.numAgents + 1, -1);
    queue<int> unmatchedHouses;

    for (int a = 1; a <= graph.numAgents; ++a)
    {
        if (matchA[a] != 0)
        {
            int h = matchA[a];
            prefLists[h].push_back({a, curRank[a]});
            curRank[a] = find(graph.adj[a].begin(), graph.adj[a].end(), h) - graph.adj[a].begin();
        }
    }

    for (int h = 1; h <= graph.numHouses; ++h)
    {
        if (matchH[h] == 0 && !prefLists[h].empty())
        {
            unmatchedHouses.push(h);
        }
    }

    while (!unmatchedHouses.empty())
    {
        int h = unmatchedHouses.front();
        unmatchedHouses.pop();

        while (!prefLists[h].empty())
        {
            auto [a, rank] = prefLists[h].front();
            prefLists[h].pop_front();
            if (rank < curRank[a])
            {
                int oldH = matchA[a];
                matchA[a] = h;
                matchH[h] = a;
                if (!prefLists[oldH].empty())
                {
                    unmatchedHouses.push(oldH);
                }
                break;
            }
        }
    }
}

// Phase 3: Remove coalitions using Top Trading Cycles Method
void makeCoalitionFree(vector<int> &matchA, vector<int> &matchH, const Graph &graph)
{
    vector<bool> labeled(graph.numHouses + 1, false);
    vector<int> pointers(graph.numAgents + 1, 0);
    queue<int> cycleQueue;

    for (int a = 1; a <= graph.numAgents; ++a)
    {
        if (matchA[a] != 0)
        {
            cycleQueue.push(a);
        }
    }

    while (!cycleQueue.empty())
    {
        int a = cycleQueue.front();
        cycleQueue.pop();

        int h = graph.adj[a][pointers[a]];
        while (labeled[h] && pointers[a] < graph.adj[a].size())
        {
            h = graph.adj[a][++pointers[a]];
        }

        if (!labeled[h])
        {
            matchA[a] = h;
            matchH[h] = a;
            labeled[h] = true;
        }
        else
        {
            int b = matchH[h];
            matchA[b] = 0;
            cycleQueue.push(b);
        }
    }
}

// Main function to execute the algorithm
int main()
{
    int numAgents, numHouses, numPref;
    cin >> numAgents >> numHouses >> numPref;
    Graph graph(numAgents, numHouses);
    for (int i=0; i<numAgents; i++)
    {
        for (int j=0; j<numPref; j++)
        {
            int h;
            cin>>h;
            graph.adj[i+1].push_back(h);
            graph.adj[h].push_back(i+1);
        }
    }
    

    vector<int> matchA, matchH;

    // Phase 1: Find maximal matching
    int maxMatchingSize = hopcroftKarp(graph, matchA, matchH);
    for (int a = 1; a <= numAgents; ++a)
    {
        if (matchA[a] != 0)
        {
            cout << "Agent " << a << " is assigned to House " << matchA[a] << "\n";
        }
    }

    // Phase 2: Make the matching trade-in-free
    makeTradeInFree(matchA, matchH, graph);
    for (int a = 1; a <= numAgents; ++a)
    {
        if (matchA[a] != 0)
        {
            cout << "Agent " << a << " is assigned to House " << matchA[a] << "\n";
        }
    }

    // // Phase 3: Make the matching coalition-free
    makeCoalitionFree(matchA, matchH, graph);

    // Output the final Pareto optimal matching
    cout << "Pareto Optimal Matching:\n";
    for (int a = 1; a <= numAgents; ++a)
    {
        if (matchA[a] != 0)
        {
            cout << "Agent " << a << " is assigned to House " << matchA[a] << "\n";
        }
    }

    return 0;
}
