#include <iostream>
#include <list>

using namespace std;

class Edge;

class Vertex {
    string name;
    int distance;
    //Vertex path;
    int weight;
    bool known;
    list<shared_ptr<Edge>> edgeList;
    list<shared_ptr<Vertex>> adjVertexList;

public:
    Vertex();
    Vertex(const string & nm);
    virtual ~Vertex();
};

class Edge {
    Vertex target;
    int weight;

public:
    Edge();
    Edge(const Vertex & v, int w);
    virtual ~Edge();

    Vertex getTarget();
    void setTarget(const Vertex & target);
    int getWeight();
    void setWeight(int weight);
};

int main()
{
    cout << "Hello World!" << endl;
    return 0;
}

