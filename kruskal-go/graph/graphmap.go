package graph

import (
	"fmt"
	"sort"
)

type GraphMap struct {
	graph map[string][]*Edge
}

func NewGraphMap() *GraphMap {
	return &GraphMap{
		graph: make(map[string][]*Edge),
	}
}

func (g *GraphMap) Add_Edge(from, to string, weight int) {
	g.graph[from] = append(g.graph[from], &Edge{from, to, weight})
	g.graph[to] = append(g.graph[to], &Edge{to, from, weight})
}

func (g *GraphMap) Edges(from string) []*Edge {
	return g.graph[from]
}

func (g *GraphMap) Sort() {
	for _, edges := range g.graph {
		sort.Slice(edges, func(i, j int) bool {
			return edges[i].weight < edges[j].weight
		})
	}
}

func (g *GraphMap) Print() {
	for node, edges := range g.graph {
		fmt.Println(node, ":")
		for _, edge := range edges {
			fmt.Printf("%s", edge.String())
		}
		fmt.Println()
	}
}
