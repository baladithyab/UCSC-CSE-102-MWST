package graph

import "fmt"

type Edge struct {
	node1  string
	node2  string
	weight int
}

func New_Edge_List_Empty() []*Edge {
	return make([]*Edge, 0)
}

func (e *Edge) Nodes() []string {
	return []string{e.node1, e.node2}
}

func (e *Edge) Weight() int {
	return e.weight
}

func (e *Edge) String() string {
	return fmt.Sprintf("|%s --%d-> %s|", e.node1, e.weight, e.node2)
}
