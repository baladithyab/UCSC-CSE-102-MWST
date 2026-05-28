package graph

type Graph interface {
	Nodes() []string
	Edges() []*Edge
	Add_Node(node string)
	Add_Edge(node1 string, node2 string, weight int)
	Print()
	Get_Neighboring_Edges(node string) []*Edge
	Sort()
}
