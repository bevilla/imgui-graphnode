#include "imgui_graphnode.h"
#include "imgui_graphnode_internal.h"
#include "imgui_internal.h"

void IMGUI_GRAPHNODE_NAMESPACE::CreateContext()
{
    IM_ASSERT(g_ctx.gvcontext == nullptr);
    g_ctx.gvcontext = gvContext();
}

void IMGUI_GRAPHNODE_NAMESPACE::DestroyContext()
{
    IM_ASSERT(g_ctx.gvcontext != nullptr);
    gvFreeContext(g_ctx.gvcontext);
    g_ctx.gvcontext = nullptr;
}

bool IMGUI_GRAPHNODE_NAMESPACE::BeginNodeGraph(ImGuiGraphNodeLayout layout, float pixel_per_unit)
{
    IM_ASSERT(g_ctx.gvgraph == nullptr);
    IM_ASSERT(g_ctx.graphid_current.empty());
    g_ctx.gvgraph = agopen(const_cast<char *>("g"), Agdirected, 0);
    g_ctx.layout = layout;
    g_ctx.pixel_per_unit = pixel_per_unit;

    char graphid_buf[16] = { 0 };
    snprintf(graphid_buf, sizeof(graphid_buf) - 1, "%d", (int)layout);
    g_ctx.graphid_current += graphid_buf;

    return true;
}

void IMGUI_GRAPHNODE_NAMESPACE::NodeGraphAddNode(char const * id)
{
    ImVec4 const color = ImGui::GetStyle().Colors[ImGuiCol_Text];
    ImVec4 const fillcolor = ImVec4(0.f, 0.f, 0.f, 0.f);
    NodeGraphAddNode(id, color, fillcolor);
}

void IMGUI_GRAPHNODE_NAMESPACE::NodeGraphAddNode(char const * id, ImVec4 const & color, ImVec4 const & fillcolor)
{
    IM_ASSERT(g_ctx.gvgraph != nullptr);
    Agnode_t * const n = agnode(g_ctx.gvgraph, ImGuiIDToString(id), 1);
    IM_ASSERT(n != nullptr);
    IMGUI_GRAPHNODE_CREATE_LABEL_ALLOCA(text, id);
    auto const color_str = ImVec4ColorToString(color);
    auto const fillcolor_str = ImVec4ColorToString(fillcolor);
    agsafeset(n, (char *)"label", text, "");
    agsafeset(n, (char *)"color", color_str, "");
    agsafeset(n, (char *)"fillcolor", fillcolor_str, "");

    g_ctx.graphid_current += id;
    g_ctx.graphid_current += color_str;
    g_ctx.graphid_current += fillcolor_str;
}

void IMGUI_GRAPHNODE_NAMESPACE::NodeGraphAddEdge(char const * id, char const * node_id_a, char const * node_id_b)
{
    ImVec4 const color = ImGui::GetStyle().Colors[ImGuiCol_Text];
    NodeGraphAddEdge(id, node_id_a, node_id_b, color);
}

void IMGUI_GRAPHNODE_NAMESPACE::NodeGraphAddEdge(char const * id, char const * node_id_a, char const * node_id_b, ImVec4 const & color)
{
    IM_ASSERT(g_ctx.gvgraph != nullptr);
    Agnode_t * const a = agnode(g_ctx.gvgraph, ImGuiIDToString(node_id_a), 0);
    Agnode_t * const b = agnode(g_ctx.gvgraph, ImGuiIDToString(node_id_b), 0);
    IM_ASSERT(a != nullptr);
    IM_ASSERT(b != nullptr);
    Agedge_t * const e = agedge(g_ctx.gvgraph, a, b, ImGuiIDToString(id), 1);
    IM_ASSERT(e != nullptr);
    IMGUI_GRAPHNODE_CREATE_LABEL_ALLOCA(text, id);
    auto const color_str = ImVec4ColorToString(color);
    agsafeset(e, (char *)"label", text, "");
    agsafeset(e, (char *)"color", color_str, "");

    g_ctx.graphid_current += id;
    g_ctx.graphid_current += node_id_a;
    g_ctx.graphid_current += node_id_b;
    g_ctx.graphid_current += color_str;
}

void IMGUI_GRAPHNODE_NAMESPACE::EndNodeGraph()
{
    auto & graph = g_ctx.graph;
    float const ppu = g_ctx.pixel_per_unit;

    if (g_ctx.graphid_current != g_ctx.graphid_previous)
    {
        ImGuiGraphNodeRenderGraphLayout(graph);
        g_ctx.graphid_previous = g_ctx.graphid_current;
    }
    g_ctx.graphid_current.clear();
    agclose(g_ctx.gvgraph);
    g_ctx.gvgraph = nullptr;

    {
        ImDrawList * drawlist = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();

        for (ImGuiGraphNode_Node const & node : graph.nodes)
        {
            constexpr int num_segments = 31;
            float a_min = 0.f;
            float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
            ImVec2 path[num_segments + 1];

            for (int i = 0; i <= num_segments; i++)
            {
                const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
                path[i].x = p.x + (node.pos.x + ImCos(a) * node.size.x / 2.f) * ppu;
                path[i].y = p.y + ((graph.size.y - node.pos.y) + ImSin(a) * node.size.y / 2.f) * ppu;
            }
            ImVec2 const textsize = ImGui::CalcTextSize(node.label.c_str());
            drawlist->AddConvexPolyFilled(path, num_segments + 1, node.fillcolor);
            drawlist->AddPolyline(path, num_segments + 1, node.color, ImDrawFlags_Closed, 1.f);
            drawlist->AddText(ImVec2(p.x + node.pos.x * ppu - textsize.x / 2.f, p.y + (graph.size.y - node.pos.y) * ppu - textsize.y / 2.f), node.color, node.label.c_str());
        }
        for (ImGuiGraphNode_Edge const & edge : graph.edges)
        {
            ImVec2 const textsize = ImGui::CalcTextSize(edge.label.c_str());
            drawlist->AddText(ImVec2(p.x + edge.labelPos.x * ppu - textsize.x / 2.f, p.y + (graph.size.y - edge.labelPos.y) * ppu - textsize.y / 2.f), edge.color, edge.label.c_str());
            constexpr int pointscount = 64;
            ImVec2 points[pointscount];
            for (int x = 0; x < pointscount; ++x)
            {
                points[x] = ImGuiGraphNode_BezierVec2(edge.points.data(), (int)edge.points.size(), x / float(pointscount - 1));
                points[x].y = graph.size.y - points[x].y;
                points[x].x *= ppu;
                points[x].y *= ppu;
                points[x].x += p.x;
                points[x].y += p.y;
            }
            drawlist->AddPolyline(points, pointscount, edge.color, ImDrawFlags_None, 1.f);
            ImVec2 const lastpoint = points[pointscount - 1];
            float dirx = lastpoint.x - points[pointscount - 2].x;
            float diry = lastpoint.y - points[pointscount - 2].y;
            float const mag = ImSqrt(dirx * dirx + diry * diry);
            float const mul1 = ppu * 0.1f;
            float const mul2 = ppu * 0.0437f;
            dirx /= mag;
            diry /= mag;
            ImVec2 const p1(lastpoint.x - dirx * mul1 - diry * mul2, lastpoint.y - diry * mul1 + dirx * mul2);
            ImVec2 const p2(lastpoint.x - dirx * mul1 + diry * mul2, lastpoint.y - diry * mul1 - dirx * mul2);
            ImVec2 const p3 = lastpoint;
            drawlist->AddTriangleFilled(p1, p2, p3, edge.color);
        }
        ImGui::Dummy(ImVec2(graph.size.x * ppu, graph.size.y * ppu));
    }
}
