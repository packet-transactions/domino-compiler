#include <graphviz/cgraph.h>
#include <assert.h>
#include <stdio.h>

int main() {
  FILE * fp = fopen("test", "r");
  assert(fp);
  Agraph_t * graph = agread(fp, NULL);
  assert(graph);
  printf("Number of nodes is %d\n", agnnodes(graph));
  printf("Number of edges is %d\n", agnedges(graph));

  /* Iterate through nodes */
  Agnode_t * node = agfstnode(graph);
  while (node != NULL) {
    assert(node);
    printf("Iterating through yet another node\n");
    char  * label = agget(node, "label");
    printf("Node label is %s\n", label);
    Agedge_t * edge   = agfstout(graph, node);
    while (edge != NULL) {
      assert(edge);
      printf("Iterating through yet another edge \n");
      assert(agtail(edge) == node);
      edge = agnxtout(graph, edge);
    }

    /* Move on to the next node */
    printf("\n");
    node  = agnxtnode(graph, node);
  }

  /* Free graph structure */
  agclose(graph);

  return 0;
}
