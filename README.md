# imgui-graphnode
[Dear ImGui](https://github.com/ocornut/imgui) helper functions to display nodes using [graphviz](https://graphviz.org/) library

## Dependencies

- [Dear ImGui](https://github.com/ocornut/imgui)
- [graphviz](https://graphviz.org/)

## Compilation

Add the following files to your project:
- imgui_graphnode.cpp
- imgui_graphnode.h
- imgui_graphnode_internal.cpp
- imgui_graphnode_internal.h
- imgui_graphnode_demo.cpp (optional)
- imgui_graphnode_demo.h (optional)

## Usage

A small set of functions are provided, allowing to draw a graph by declaring nodes and edges.
```c++
bool ImGuiNodeGraph::BeginNodeGraph(ImGuiGraphNodeLayout layout = ImGuiGraphNodeLayout_Dot, float pixel_per_unit = 100.f);
void ImGuiNodeGraph::NodeGraphAddNode(char const * id);
void ImGuiNodeGraph::NodeGraphAddNode(char const * id, ImVec4 const & color, ImVec4 const & fillcolor);
void ImGuiNodeGraph::NodeGraphAddEdge(char const * id, char const * node_id_a, char const * node_id_b);
void ImGuiNodeGraph::NodeGraphAddEdge(char const * id, char const * node_id_a, char const * node_id_b, ImVec4 const & color);
void ImGuiNodeGraph::EndNodeGraph();
```

## Examples

Examples source code can be found in imgui_graphnode_demo.cpp

```c++
if (ImGuiGraphNode::BeginNodeGraph(ImGuiGraphNodeLayout_Circo))
{
    ImGuiGraphNode::NodeGraphAddNode("A");
    ImGuiGraphNode::NodeGraphAddNode("B");
    ImGuiGraphNode::NodeGraphAddNode("C");
    ImGuiGraphNode::NodeGraphAddNode("D");
    ImGuiGraphNode::NodeGraphAddEdge("a->b", "A", "B");
    ImGuiGraphNode::NodeGraphAddEdge("b->c", "B", "C");
    ImGuiGraphNode::NodeGraphAddEdge("c->d", "C", "D");
    ImGuiGraphNode::NodeGraphAddEdge("d->a", "D", "A");
    ImGuiGraphNode::EndNodeGraph();
}
```
![alt text](https://github.com/bevilla/imgui-graphnode/raw/master/docs/example1.png "Example 1")

```c++
if (ImGuiGraphNode::BeginNodeGraph(ImGuiGraphNodeLayout_Dot))
{
    ImGuiGraphNode::NodeGraphAddNode("LR_0");
    ImGuiGraphNode::NodeGraphAddNode("LR_1");
    ImGuiGraphNode::NodeGraphAddNode("LR_2");
    ImGuiGraphNode::NodeGraphAddNode("LR_3");
    ImGuiGraphNode::NodeGraphAddNode("LR_4");
    ImGuiGraphNode::NodeGraphAddNode("LR_5");
    ImGuiGraphNode::NodeGraphAddNode("LR_6");
    ImGuiGraphNode::NodeGraphAddNode("LR_7");
    ImGuiGraphNode::NodeGraphAddNode("LR_8");
    ImGuiGraphNode::NodeGraphAddEdge("SS(B)", "LR_0", "LR_2");
    ImGuiGraphNode::NodeGraphAddEdge("SS(S)", "LR_0", "LR_1");
    ImGuiGraphNode::NodeGraphAddEdge("S($end)", "LR_1", "LR_3");
    ImGuiGraphNode::NodeGraphAddEdge("SS(b)", "LR_2", "LR_6");
    ImGuiGraphNode::NodeGraphAddEdge("SS(a)", "LR_2", "LR_5");
    ImGuiGraphNode::NodeGraphAddEdge("S(A)", "LR_2", "LR_4");
    ImGuiGraphNode::NodeGraphAddEdge("S(b)", "LR_5", "LR_7");
    ImGuiGraphNode::NodeGraphAddEdge("S(a)", "LR_5", "LR_5");
    ImGuiGraphNode::NodeGraphAddEdge("S(b)", "LR_6", "LR_6");
    ImGuiGraphNode::NodeGraphAddEdge("S(a)", "LR_6", "LR_5");
    ImGuiGraphNode::NodeGraphAddEdge("S(b)", "LR_7", "LR_8");
    ImGuiGraphNode::NodeGraphAddEdge("S(a)", "LR_7", "LR_5");
    ImGuiGraphNode::NodeGraphAddEdge("S(b)", "LR_8", "LR_6");
    ImGuiGraphNode::NodeGraphAddEdge("S(a)", "LR_8", "LR_5");
    ImGuiGraphNode::EndNodeGraph();
}
```
![alt text](https://github.com/bevilla/imgui-graphnode/raw/master/docs/example2.png "Example 2")

![alt text](https://github.com/bevilla/imgui-graphnode/raw/master/docs/rbtree.png "Red-black tree")
