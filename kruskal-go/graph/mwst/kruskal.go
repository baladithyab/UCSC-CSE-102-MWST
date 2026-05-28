package mwst

import (
	graph "github.com/baladithyab/UCSC-CSE-102-MWST/kruskal-go/graph"
)

type Kruskal struct {
	graph graph.Graph
	mst   graph.Edge_List_Graph
}

func New_Kruskal(g graph.Graph) *Kruskal {
	return &Kruskal{g, *graph.New_Edge_List_Graph()}
}

type subset struct {
	parent string
	rank   int
}

func find(subsets map[string]*subset, node string) string {
	if subsets[node].parent != node {
		subsets[node].parent = find(subsets, subsets[node].parent)
	}
	return subsets[node].parent
}

func Check_Cycles(g *graph.Edge_List_Graph) bool {
	subsets := make(map[string]*subset)
	for _, node := range g.Nodes() {
		subsets[node] = &subset{node, 0}
	}
	for _, edge := range g.Edges() {
		nodes := edge.Nodes()

		node0 := find(subsets, nodes[0])

		node1 := find(subsets, nodes[1])

		if node0 == node1 {
			return true
		}
		if subsets[node0].rank < subsets[node1].rank {
			subsets[node0].parent = node1
		} else if subsets[node0].rank > subsets[node1].rank {
			subsets[node1].parent = node0
		} else {
			subsets[node1].parent = node0
			subsets[node1].rank++
		}
	}
	return false
}

func (k *Kruskal) Run() []*graph.Edge {
	edges := k.graph.Edges()
	for _, edge := range edges {
		if len(k.mst.Nodes()) == 0 {
			k.mst.Add_Edge(edge.Nodes()[0], edge.Nodes()[1], edge.Weight())
		} else {
			mstcopy := k.mst.Add_Edge_Copy(edge.Nodes()[0], edge.Nodes()[1], edge.Weight())
			if !Check_Cycles(mstcopy) {
				k.mst.Add_Edge(edge.Nodes()[0], edge.Nodes()[1], edge.Weight())
			}
		}
	}

	return k.mst.Edges()
}
