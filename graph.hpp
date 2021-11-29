/**
**
** graph.hpp
** Author: Qingyuan Peng
** USC-ID#: 8544146131
**
** The code for establishing the adjacency list and dijkstra algorithm refers to a blog on CSDN
** https://blog.csdn.net/weixin_42375679/article/details/112466514
** The code is modified and expanded according to project requirements
**
*/

#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<set>
#include<queue>
#include<stack>
#include<limits.h>
#include <float.h>
#include "edge.hpp"
using namespace std;

// this structure used in dijkstra's map's value
struct shortestPath
{
    double distance;
    string lastNode;
};

// it used to compare the distance in the structure
bool operator<(shortestPath a, shortestPath b) {
    return a.distance < b.distance;
}


template <typename T>
class Graph {
public:
	map<T, set<Edge<T>>> adj;  /* Adjacency list */

	bool contains(const T& u); /* Determine whether vertex u is in the graph */
	bool adjacent(const T& u, const T& v); /* Determine whether vertices u and v are adjacent */

	void add_vertex(const T& u); /* Add vertices */
	void add_edge(const T& u, const T& v, double weight); /* Add edges and weights */

	vector<T> get_vertices(); /* Get all vertices in the graph */

	void show();

	void dft_recursion(const T& u, set<T>& visited, vector<T>& result); /* Depth-first traversal recursive helper function */
	vector<T> depth_first_rec(const T& u); /* Depth-first traversal recursion */
	vector<T> depth_first_itr(const T& u); /* Depth-first traversal iteration*/
	vector<T> breadth_first(const T& u); /* Breadth-first traversal iteration */

	map<T, shortestPath> dijkstra(T start); /*  dijkstra shortest path algorithm */
};

template <typename T> void Graph<T>::show() {
	for (const auto& u : adj) {
		cout << "point-" << u.first << ": ";
		for (const auto& v : adj[u.first])
			cout << "(ajacent points: " << v.vertex << ", weight of edge: " << v.weight << ") ";
		cout << endl;
	}
}

template <typename T> bool Graph<T>::contains(const T& u) {
	return adj.find(u) != adj.end();
}

template <typename T> bool Graph<T>::
adjacent(const T& u, const T& v) {
	if (contains(u) && contains(v) && u != v) {
		for (auto edge : adj[u])
			if (edge.vertex == v)
				return true;
	}
	return false;
}

template <typename T> void Graph<T>::add_vertex(const T& u) {
	if (!contains(u)) {
		set<Edge<T>> edge_list;
		adj[u] = edge_list;
	}
}

template <typename T> void Graph<T>::add_edge(const T& u, const T& v, double weight) {
	if (!adjacent(u, v)) {
		adj[u].insert(Edge<T>(v, weight));
		adj[v].insert(Edge<T>(u, weight));
	}
}

template <typename T> vector<T> Graph<T>::get_vertices() {
	vector<T> vertices;
	for (auto vertex : adj)
		vertices.push_back(vertex.first);

	return vertices;
}

template <typename T> void Graph<T>::dft_recursion(const T& u, set<T>& visited, vector<T>& result) {
	result.push_back(u);
	visited.insert(u);

	for (Edge<T> edge : adj[u])
		if (visited.find(edge.vertex) == visited.end())
			dft_recursion(edge.vertex, visited, result);
}

template <typename T> vector<T> Graph<T>::depth_first_rec(const T& u) {
	vector<T> result;
	set<T> visited;
	if (contains(u))  dft_recursion(u, visited, result);
	return  result;
}

template <typename T> vector<T> Graph<T>::depth_first_itr(const T& u) {
	vector<T> result;
	set<T> visited;
	stack<T> s;

	s.push(u);
	while (!s.empty()) {
		T v = s.top();
		s.pop();

		if (visited.find(v) != visited.end()) {
			continue;
		}
		visited.insert(v);
		result.push_back(v);

		for (auto w : adj[v]) {
			if (visited.find(w.vertex) == visited.end()) {
				s.push(w.vertex);
			}
		}
	}
	return  result;
}

template <typename T> vector<T> Graph<T>::breadth_first(const T& u) {
	vector<T>result;
	set<T> visited;
	queue<T> q;

	q.push(u);
	while (!q.empty()) {
		T v = q.front();
		q.pop();

		if (visited.find(v) != visited.end()) {
			continue;
		}

		visited.insert(v);
		result.push_back(v);

		for (Edge<T> edge : adj[v]) {
			if (visited.find(edge.vertex) == visited.end()) {
				q.push(edge.vertex);
			}
		}
	}
	return result;
}

template <typename T> map<T, shortestPath> Graph<T>::dijkstra(T start) {
	// Set dis to store the distance from the initial point to any vertex in the graph
	map<T, shortestPath> dis;

	// Set up a weighted queue, sort from small to large according to the first element of each pair
	priority_queue<pair<shortestPath, T>, vector<pair<shortestPath, T>>, greater<pair<shortestPath, T>>> q;
	

	for (T vertex : get_vertices()) {
		// Set the distance from the initial vertex to itself to 0
		if (vertex == start) {
			dis[start].distance = 0.0;
			dis[start].lastNode = start;
		}
		// Set the distance from the initial vertex to other vertices to infinity
		else {
			dis[vertex].distance = DBL_MAX;
			dis[start].lastNode = "unreachable";
		}
	}

	// Set the set visited to store the vertices that have been visited
	set<T> visited;

	// Enqueue: The element enlisted is a pair type, the first value is the weight, and the second value is the vertex
	q.push(make_pair(shortestPath{0.0,start}, start));

	while (!q.empty()) {
		// Get leading element from the queue
		auto front = q.top();
		q.pop();

		// Get the current vertex
		T u = front.second;

		// If the vertex has been visited, skip this cycle, 
		// otherwise it is stored in the visited set to indicate that it has been visited
		if (visited.find(u) != visited.end()) continue;
		else visited.insert(u);

		// Obtain the shortest path "shortest_distance_to_u" to vertex u, and store this path in the result of dis
		double shortest_distance_to_u = front.first.distance;
		dis[u].distance = shortest_distance_to_u;
		dis[u].lastNode = front.first.lastNode;

		// Visit the neighbors that vertex u has not visited in turn
		for (auto v : adj[u]) {
			if (visited.find(v.vertex) == visited.end()) {
				// The path from vertex u to neighbor v is denoted as "distance_to_v"
				double distance_to_v = v.weight;
				q.push(make_pair(shortestPath{shortest_distance_to_u + distance_to_v, u}, v.vertex));
			}
		}
	}
	return dis;
}