#include <bits/stdc++.h>

using namespace std;

// Graph structure
struct Graph
{
    int numAgents, numHouses;
    vector<vector<int>> adj;                                        // adj[a] contains houses that agent a finds acceptable
    Graph(int a, int h) : numAgents(a), numHouses(h), adj(a + 1) {} // +1 for one-based indexing
};
vector<int> matchH2; // Global variable to store the matching result for houses
pair<int, vector<int>> hopcroftKarp(const Graph &graph, vector<int> &matchA, vector<int> &matchH);
bool bfs(vector<int> &matchA, vector<int> &matchH, vector<int> &dist, const Graph &graph);


void makeCoalitionFree(vector<int>& matchA, vector<int>& matchH, const Graph& graph) {
    vector<int> ptr(graph.numAgents + 1, 0); // Tracks the next preference for each agent
    bool improved;

    do {
        improved = false;
        vector<bool> visitedAgent(graph.numAgents + 1, false);
        vector<bool> visitedHouse(graph.numHouses + 1, false);

        for (int a = 1; a <= graph.numAgents; ++a) {
            // Skip if agent is unmatched or already visited
            if (matchA[a] == 0 || visitedAgent[a]) continue;

            vector<int> cycleAgents, cycleHouses;
            int currentAgent = a;

            // Detect a cycle among matched agents
            while (true) {
                if (visitedAgent[currentAgent]) break;
                visitedAgent[currentAgent] = true;

                // Find the next preferred house not owned by the current agent
                int nextHouse = -1;
                while (ptr[currentAgent] > graph.adj[currentAgent].size()) {
                    nextHouse = graph.adj[currentAgent][ptr[currentAgent]++];
                    if (nextHouse != matchA[currentAgent] && !visitedHouse[nextHouse]) {
                        break;
                    }
                }
                if (nextHouse == -1) break;

                cycleAgents.push_back(currentAgent);
                cycleHouses.push_back(nextHouse);
                visitedHouse[nextHouse] = true;

                // Move to the owner of the next house (if matched)
                currentAgent = matchH[nextHouse];
                if (currentAgent == 0) break; // Skip if house is unassigned
            }

            // Execute the cycle if it's valid (size > 1)
            if (cycleAgents.size() > 1) {
                improved = true;
                // Unassign all agents in the cycle from their current houses
                for (int agent : cycleAgents) {
                    if (matchA[agent] != 0) {
                        matchH[matchA[agent]] = 0;
                    }
                }
                // Assign new houses in the cycle
                for (size_t i = 0; i < cycleAgents.size(); ++i) {
                    int agent = cycleAgents[i];
                    int house = cycleHouses[i];
                    matchA[agent] = house;
                    matchH[house] = agent;
                }
            }
        }
    } while (improved); // Repeat until no more improvements
}


// Hopcroft-Karp Algorithm to find maximal matching
bool bfs(vector<int> &matchA, vector<int> &matchH, vector<int> &dist, const Graph &graph)
{
    queue<int> Q;
    for (int a = 1; a <= graph.numAgents; a++)
    {
        if (matchA[a] == 0)
        { // Unmatched agents have matchA[a] = 0 in one-based indexing
            dist[a] = 0;
            Q.push(a);
        }
        else
        {
            dist[a] = INT_MAX;
        }
    }
    dist[0] = INT_MAX; // Placeholder for unmatched state in one-based indexing

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

pair<int, vector<int>> hopcroftKarp(const Graph &graph, vector<int> &matchA, vector<int> &matchH)
{
    matchA.assign(graph.numAgents + 1, 0); // One-based, 0 means unmatched
    matchH.assign(graph.numHouses + 1, 0); // One-based, 0 means unmatched
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
    return {matchingSize, matchA};
}
pair<int, vector<int>> leastDissatisfaction(const Graph &graph, int maxMatchingSize)
{
    int left = 1, right = graph.numHouses, result = right;
    pair<int, vector<int>> currentMaxMatchingSize;
    pair<int, vector<int>> ans;
    while (left <= right)
    {
        int mid = (left + right) / 2;

        // Create a restricted graph with only top 'mid' preferences for each agent
        Graph restrictedGraph(graph.numAgents, graph.numHouses);
        for (int a = 1; a <= graph.numAgents; ++a)
        {
            for (int i = 0; i < min(mid, (int)graph.adj[a].size()); ++i)
            {
                restrictedGraph.adj[a].push_back(graph.adj[a][i]);
            }
        }

        // Run the maximal matching algorithm on this restricted graph
        vector<int> matchA, matchH;
        currentMaxMatchingSize = hopcroftKarp(restrictedGraph, matchA, matchH);

        // Check if we have a maximal matchingut

        if (maxMatchingSize == currentMaxMatchingSize.first)
        {
            result = mid;        // Store the minimum value of k that works
            ans = {mid, matchA}; // Store the matching result
            matchH2 = matchH;    // Store the matching result for houses
            right = mid - 1;     // Try for a smaller k
        }
        else
        {
            left = mid + 1; // Increase k if matching is not maximal
        }
    }

    return ans;
}
// Main function to execute the algorithm
int main()
{
    int numAgents, numHouses, numPref;
    cin >> numAgents >> numHouses >> numPref;
    Graph graph(numAgents, numHouses);
    for (int i = 0; i < numAgents; i++)
    {
        for (int j = 0; j < numPref; j++)
        {
            int h;
            cin >> h;
            graph.adj[i + 1].push_back(h); // add the agent-house edge
        }
    }

    vector<int> matchA, matchH;
    pair<int, vector<int>> maxMatchingSize = hopcroftKarp(graph, matchA, matchH);
    pair<int, vector<int>> res = leastDissatisfaction(graph, maxMatchingSize.first);
    vector<int> matchA2 = res.second;

    cout << "Maximal Matching Size: " << maxMatchingSize.first << endl;
    cout << "Least Dissatisfaction Matching Size: " << res.first << endl;

    makeCoalitionFree(matchA2, matchH2, graph);
    for (int a = 1; a <= numAgents; ++a)
    {
        if (matchA2[a] != 0)
        {
            cout << "Agent " << a << " is assigned to House " << matchA2[a] << "\n";
        }
    }

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
