package graph

import (
	"fmt"
	"sort"

	"github.com/baladithyab/UCSC-CSE-102-MWST/kruskal-go/utils"
)

type Edge_List_Graph struct {
	nodes *utils.Set
	edges []*Edge
}

func New_Edge_List_Graph() *Edge_List_Graph {
	return &Edge_List_Graph{utils.NewSet(), make([]*Edge, 0)}
}

func (g *Edge_List_Graph) Nodes() []string {
	return g.nodes.Keys()
}

func (g *Edge_List_Graph) Edges() []*Edge {
	return g.edges
}

func (g *Edge_List_Graph) Add_Node(node string) {
	g.nodes.Add(node)
}

func (g *Edge_List_Graph) Add_Edge(node1 string, node2 string, weight int) {
	g.Add_Node(node2)
	g.Add_Node(node1)
	g.edges = append(g.edges, &Edge{node1, node2, weight})
}

func (g *Edge_List_Graph) Add_Edge_Copy(node1 string, node2 string, weight int) *Edge_List_Graph {
	newgraph := &Edge_List_Graph{g.nodes, g.edges}
	newgraph.Add_Node(node2)
	newgraph.Add_Node(node1)
	newgraph.edges = append(newgraph.edges, &Edge{node1, node2, weight})
	return newgraph
}

func (g *Edge_List_Graph) Print() {
	fmt.Println(g.Nodes())
	for _, edge := range g.Edges() {
		fmt.Println(edge.node1, edge.node2, edge.weight)
	}
}

func (g *Edge_List_Graph) Sort() {
	sort.Slice(g.edges, func(i, j int) bool {
		return g.edges[i].weight < g.edges[j].weight
	})
}

func (g *Edge_List_Graph) Get_Neighboring_Edges(node string) []*Edge {
	var edges []*Edge
	for _, edge := range g.Edges() {
		if (edge.node1 == node) || (edge.node2 == node) {
			edges = append(edges, edge)
		}
	}
	return edges
}
