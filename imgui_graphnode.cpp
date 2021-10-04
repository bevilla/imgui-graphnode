#include "imgui_graphnode.h"
#include "imgui_graphnode_internal.h"
#include "imgui_internal.h"

bool IMGUI_GRAPHNODE_NAMESPACE::BeginNodeGraph(ImGuiGraphNodeLayout layout, float pixel_per_unit)
{
    IM_ASSERT(g_ctx.gvcontext == nullptr);
    g_ctx.gvcontext = gvContext();
    g_ctx.gvgraph = agopen(const_cast<char *>("g"), Agdirected, 0);
    g_ctx.layout = layout;
    g_ctx.pixel_per_unit = pixel_per_unit;
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
    agsafeset(n, (char *)"label", text, "");
    agsafeset(n, (char *)"color", ImVec4ColorToString(color), "");
    agsafeset(n, (char *)"fillcolor", ImVec4ColorToString(fillcolor), "");
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
    agsafeset(e, (char *)"label", text, "");
    agsafeset(e, (char *)"color", ImVec4ColorToString(color), "");
}

void IMGUI_GRAPHNODE_NAMESPACE::EndNodeGraph()
{
    ImGuiGraphNode_Graph graph;
    char * data = nullptr;
    unsigned int size = 0;
    char const * const engine = ImGuiGraphNode_GetEngineNameFromLayoutEnum(g_ctx.layout);
    float const ppu = g_ctx.pixel_per_unit;
    int ok = 0;

    IM_ASSERT(g_ctx.gvcontext != nullptr);
    IM_ASSERT(g_ctx.gvgraph != nullptr);
    agattr(g_ctx.gvgraph, AGEDGE, (char *)"dir", "none");
    ok = gvLayout(g_ctx.gvcontext, g_ctx.gvgraph, engine);
    IM_ASSERT(ok == 0);
    ok = gvRenderData(g_ctx.gvcontext, g_ctx.gvgraph, "plain", &data, &size);
    IM_ASSERT(ok == 0);
    ImGuiGraphNode_ReadGraphFromMemory(graph, data, size);
    gvFreeRenderData(data);
    gvFreeLayout(g_ctx.gvcontext, g_ctx.gvgraph);
    agclose(g_ctx.gvgraph);
    gvFreeContext(g_ctx.gvcontext);
    g_ctx.gvgraph = nullptr;
    g_ctx.gvcontext = nullptr;

    {
        ImDrawList * drawlist = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();

        for (ImGuiGraphNode_Node const & node : graph.nodes)
        {
            std::vector<ImVec2> path;
            int num_segments = 31;
            float a_min = 0.f;
            float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;

            for (int i = 0; i <= num_segments; i++)
            {
                const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
                const float x = p.x + (node.pos.x + ImCos(a) * node.size.x / 2.f) * ppu;
                const float y = p.y + ((graph.size.y - node.pos.y) + ImSin(a) * node.size.y / 2.f) * ppu;
                path.push_back(ImVec2(x, y));
            }
            ImVec2 const textsize = ImGui::CalcTextSize(node.label.c_str());
            drawlist->AddConvexPolyFilled(path.data(), path.size(), node.fillcolor);
            drawlist->AddPolyline(path.data(), path.size(), node.color, ImDrawFlags_Closed, 1.f);
            drawlist->AddText(ImVec2(p.x + node.pos.x * ppu - textsize.x / 2.f, p.y + (graph.size.y - node.pos.y) * ppu - textsize.y / 2.f), node.color, node.label.c_str());
        }
        for (ImGuiGraphNode_Edge const & edge : graph.edges)
        {
            ImVec2 const textsize = ImGui::CalcTextSize(edge.label.c_str());
            drawlist->AddText(ImVec2(p.x + edge.labelPos.x * ppu - textsize.x / 2.f, p.y + (graph.size.y - edge.labelPos.y) * ppu - textsize.y / 2.f), edge.color, edge.label.c_str());
            constexpr int pointscount = 512;
            ImVec2 points[pointscount];
            for (int x = 0; x < pointscount; ++x)
            {
                points[x] = ImGuiGraphNode_BezierVec2(edge.points.data(), (int)edge.points.size(), x / float(pointscount - 1));
                //points[x] = ImGuiGraphNode_BSplineVec2(edge.points.data(), (int)edge.points.size(), x / float(pointscount - 1));
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
