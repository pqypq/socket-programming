/**
**
** edge.hpp
** Author: Qingyuan Peng
** USC-ID#: 8544146131
**
** The code for establishing the adjacency list and dijkstra algorithm refers to a blog on CSDN
** https://blog.csdn.net/weixin_42375679/article/details/112466514
** The code is modified and expanded according to project requirements
**
*/

template <typename T>
class Edge {
public:
	T vertex;
	double weight;

	Edge(T neighbour_vertex) {
		this->vertex = neighbour_vertex;
		this->weight = 0;
	}

	Edge(T neighbour_vertex, double weight) {
		this->vertex = neighbour_vertex;
		this->weight = weight;
	}

	bool operator<(const Edge& obj) const {
		return obj.vertex > vertex;
	}

	bool operator==(const Edge& obj) const {
		return obj.vertex == vertex;
	}
};