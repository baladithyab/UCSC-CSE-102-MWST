package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"

	graph "github.com/baladithyab/UCSC-CSE-102-MWST/kruskal-go/graph"
	"github.com/baladithyab/UCSC-CSE-102-MWST/kruskal-go/graph/mwst"
)

func graphInit(graphtype, filename string) {
	g := graph.New_Edge_List_Graph()

	readFile, err := os.Open(filename)
	if err != nil {
		fmt.Println("Error:", err)
		os.Exit(1)
	}
	fileScanner := bufio.NewScanner(readFile)
	fileScanner.Split(bufio.ScanLines)
	fileScanner.Scan()
	fmt.Println("Number of Nodes: ", fileScanner.Text())
	fileScanner.Scan()
	fmt.Println("Number of Edges: ", fileScanner.Text())
	fmt.Println("Edges:")
	for fileScanner.Scan() {
		edgeRaw := fileScanner.Text()
		// fmt.Println("\t", edgeRaw)
		edge := strings.Split(edgeRaw, " ")
		// fmt.Println("\t", edge)
		weight, _ := strconv.Atoi(edge[2])
		g.Add_Edge(edge[0], edge[1], weight)
	}
	g.Print()
	fmt.Println("-----------------------")
	g.Sort()
	g.Print()
	fmt.Println("-----------------------")
	k := mwst.New_Kruskal(g)
	for _, edge := range k.Run() {
		fmt.Println(edge.String())
	}
}

func NewEdgeList() {
	panic("unimplemented")
}

func argsParser(args []string) (string, string) {
	graphType := "map"
	filename := ""
	for idx := 0; idx < len(args); idx++ {
		switch args[idx] {
		case "-h", "--help":
			fmt.Println("Usage:")
			fmt.Println("  go run main.go  [OPTIONS]")
			fmt.Println("  ./cse102progasg [OPTIONS]")
			fmt.Println("Options:")
			fmt.Println("  -h, --help                    Print this help")
			fmt.Println("  -f, --file       (mandatory)  Path to file input")
			fmt.Println("  -g, --graphType  (optional)   Type of graph: [adj_mat, edge_list, map]")
			os.Exit(0)
		case "-f", "--file":
			idx++
			fmt.Println("File:", args[idx])
			filename = args[idx]
		case "-g", "--graphType":
			idx++
			fmt.Println("Graph Type:", args[idx])
			graphType = args[idx]
		default:
			fmt.Println("Unknown option:", args[idx])
			fmt.Println("Usage:")
			fmt.Println("  go run main.go  -h")
			fmt.Println("  ./cse102progasg -h")
			os.Exit(1)
		}
	}
	if filename == "" {
		fmt.Println("Error: No file specified")
		os.Exit(1)
	}
	return graphType, filename
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage:")
		fmt.Println("  go run main.go  -h")
		fmt.Println("  ./cse102progasg -h")
		os.Exit(1)
	}
	graphType, filename := argsParser(os.Args[1:])
	fmt.Println(graphType, filename)
	graphInit(graphType, filename)
}
