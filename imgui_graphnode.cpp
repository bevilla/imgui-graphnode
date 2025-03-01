#include "imgui_graphnode.h"
#include "imgui_graphnode_internal.h"
#include "imgui_internal.h"

static float IsPointInRectangle_IsLeft(ImVec2 const & p0, ImVec2 const & p1, ImVec2 const & p2)
{
    return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
}

static bool IsPointInRectangle(ImVec2 const & a, ImVec2 const & b, ImVec2 const & c, ImVec2 const & d, ImVec2 const & p)
{
    return IsPointInRectangle_IsLeft(a, b, p) > 0
        && IsPointInRectangle_IsLeft(b, c, p) > 0
        && IsPointInRectangle_IsLeft(c, d, p) > 0
        && IsPointInRectangle_IsLeft(d, a, p) > 0;
}

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

bool IMGUI_GRAPHNODE_NAMESPACE::BeginNodeGraph(char const * id, ImGuiGraphNodeLayout layout, float pixel_per_unit)
{
    g_ctx.lastid = ImGui::GetID(id);
    auto & cache = g_ctx.graph_caches[g_ctx.lastid];
    IM_ASSERT(g_ctx.gvgraph == nullptr);
    IM_ASSERT(cache.graphid_current.empty());
    g_ctx.gvgraph = agopen(const_cast<char *>("g"), Agdirected, 0);
    cache.layout = layout;
    cache.pixel_per_unit = pixel_per_unit;

    char graphid_buf[16] = { 0 };
    snprintf(graphid_buf, sizeof(graphid_buf) - 1, "%d", (int)layout);
    cache.graphid_current += graphid_buf;

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
    auto & cache = g_ctx.graph_caches[g_ctx.lastid];
    IM_ASSERT(g_ctx.gvgraph != nullptr);
    Agnode_t * const n = agnode(g_ctx.gvgraph, ImGuiIDToString(id), 1);
    IM_ASSERT(n != nullptr);
    IMGUI_GRAPHNODE_CREATE_LABEL_ALLOCA(text, id);
    auto const color_str = ImVec4ColorToString(color);
    auto const fillcolor_str = ImVec4ColorToString(fillcolor);
    agsafeset(n, (char *)"label", text, "");
    agsafeset(n, (char *)"color", color_str, "");
    agsafeset(n, (char *)"fillcolor", fillcolor_str, "");

    cache.graphid_current += id;
    cache.graphid_current += color_str;
    cache.graphid_current += fillcolor_str;

    ImGuiID const imid = ImGui::GetID(id);
    auto const it = cache.graph.nodesBB.find(imid);
    ImRect const bb = it != cache.graph.nodesBB.end() ? it->second : ImRect();
    ImGui::ItemAdd(bb, imid);
}

void IMGUI_GRAPHNODE_NAMESPACE::NodeGraphAddEdge(char const * id, char const * node_id_a, char const * node_id_b)
{
    ImVec4 const color = ImGui::GetStyle().Colors[ImGuiCol_Text];
    NodeGraphAddEdge(id, node_id_a, node_id_b, color);
}

void IMGUI_GRAPHNODE_NAMESPACE::NodeGraphAddEdge(char const * id, char const * node_id_a, char const * node_id_b, ImVec4 const & color)
{
    auto & cache = g_ctx.graph_caches[g_ctx.lastid];
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
    ImGuiID const imid = ImGui::GetID(id, ImGui::FindRenderedTextEnd(id));
    char identifier[16];
    sprintf(identifier, "#%x", imid);
    // graphviz library doesn't serialize the edge's identifier, so we use the
    // color field to store the ImGuiID, which will later be used to retrieve
    // the edge's properties.
    agsafeset(e, (char *)"color", identifier, "");
    cache.edgeIdToInfo[imid] = ImGuiGraphNode_EdgeInfo { ImGui::GetColorU32(color) };

    cache.graphid_current += id;
    cache.graphid_current += node_id_a;
    cache.graphid_current += node_id_b;
    cache.graphid_current += color_str;

    ImGui::ItemAdd(ImRect(), imid);
    auto const it = cache.graph.edgesRectangle.find(imid);
    if (it != cache.graph.edgesRectangle.end())
    {
        for (auto const & rect : it->second)
        {
            // Uncomment to draw edge bouding boxes
            //ImVec2 lines[] { rect.a, rect.b, rect.c, rect.d };
            //ImGui::GetWindowDrawList()->AddPolyline(lines, 4, IM_COL32(255, 0, 0, 255), ImDrawFlags_Closed, 2.0f);

            if (IsPointInRectangle(rect.a, rect.b, rect.c, rect.d, ImGui::GetIO().MousePos))
            {
                GImGui->LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredRect;
                break;
            }
        }
    }
}

int ImGuiGraphNodeFillDrawNodeBuffer(ImGuiGraphNode_Graph & graph, ImGuiGraphNode_DrawNode * drawnodes, ImVec2 cursor_pos, float ppu)
{
    int const count = (int)graph.nodes.size();

    if (drawnodes)
    {
        constexpr int num_segments = IMGUI_GRAPHNODE_DRAW_NODE_PATH_COUNT - 1;
        static_assert(num_segments > 0, "");
        float a_min = 0.f;
        float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;

        for (int i = 0; i < count; ++i)
        {
            ImGuiGraphNode_Node const & node = graph.nodes[i];
            ImVec2 const textsize = ImGui::CalcTextSize(node.label.c_str());

            for (int j = 0; j <= num_segments; j++)
            {
                const float a = a_min + ((float)j / (float)num_segments) * (a_max - a_min);
                drawnodes[i].path[j].x = cursor_pos.x + (node.pos.x + ImCos(a) * node.size.x / 2.f) * ppu;
                drawnodes[i].path[j].y = cursor_pos.y + ((graph.size.y - node.pos.y) + ImSin(a) * node.size.y / 2.f) * ppu;
            }
            drawnodes[i].textpos.x = cursor_pos.x + node.pos.x * ppu - textsize.x / 2.f;
            drawnodes[i].textpos.y = cursor_pos.y + (graph.size.y - node.pos.y) * ppu - textsize.y / 2.f;
            drawnodes[i].text = node.label.c_str();
            drawnodes[i].color = node.color;
            drawnodes[i].fillcolor = node.fillcolor;

            ImRect const bb(
                cursor_pos.x + (node.pos.x - node.size.x / 2.f) * ppu,
                cursor_pos.y + ((graph.size.y - node.pos.y) - node.size.y / 2.f) * ppu,
                cursor_pos.x + (node.pos.x + node.size.x / 2.f) * ppu,
                cursor_pos.y + ((graph.size.y - node.pos.y) + node.size.y / 2.f) * ppu
            );
            ImGuiID const imid = atol(node.name.c_str());
            graph.nodesBB[imid] = bb;
        }
    }
    return count;
}

int ImGuiGraphNodeFillDrawEdgeBuffer(ImGuiGraphNode_Graph & graph, ImGuiGraphNode_DrawEdge * drawedges, ImVec2 cursor_pos, float ppu)
{
    int const count = (int)graph.edges.size();

    if (drawedges)
    {
        constexpr int points_count = IMGUI_GRAPHNODE_DRAW_EDGE_PATH_COUNT;
        static_assert(points_count > 1, "");

        for (int i = 0; i < count; ++i)
        {
            ImGuiGraphNode_Edge const & edge = graph.edges[i];
            ImVec2 const textsize = ImGui::CalcTextSize(edge.label.c_str());

            for (size_t j = 0; j < (edge.points.size() - 1); ++j)
            {
                ImVec2 const p1(
                    cursor_pos.x + edge.points[j].x * ppu,
                    cursor_pos.y + (graph.size.y - edge.points[j].y) * ppu
                );
                ImVec2 const p2(
                    cursor_pos.x + edge.points[j + 1].x * ppu,
                    cursor_pos.y + (graph.size.y - edge.points[j + 1].y) * ppu
                );
                ImVec2 const dir(p2.x - p1.x, p2.y - p1.y);
                ImVec2 left(-dir.y, dir.x);
                ImVec2 right(dir.y, -dir.x);
                float const magLeft = ImSqrt(left.x * left.x + left.y * left.y);
                float const magRight = ImSqrt(right.x * right.x + right.y * right.y);

                left.x /= magLeft;
                left.y /= magLeft;
                right.x /= magRight;
                right.y /= magRight;

                constexpr float k = 3.f;
                ImVec2 const a(p1.x + left.x * k, p1.y + left.y * k);
                ImVec2 const b(p1.x + right.x * k, p1.y + right.y * k);
                ImVec2 const c(p2.x + right.x * k, p2.y + right.y * k);
                ImVec2 const d(p2.x + left.x * k, p2.y + left.y * k);

                graph.edgesRectangle[edge.id].push_back({ a, b, c, d });
            }
            for (int x = 0; x < points_count; ++x)
            {
                drawedges[i].path[x] = ImGuiGraphNode_BezierVec2(edge.points.data(), (int)edge.points.size(), x / float(points_count - 1));
                drawedges[i].path[x].y = graph.size.y - drawedges[i].path[x].y;
                drawedges[i].path[x].x *= ppu;
                drawedges[i].path[x].y *= ppu;
                drawedges[i].path[x].x += cursor_pos.x;
                drawedges[i].path[x].y += cursor_pos.y;
            }
            drawedges[i].textpos.x = cursor_pos.x + edge.labelPos.x * ppu - textsize.x / 2.f;
            drawedges[i].textpos.y = cursor_pos.y + (graph.size.y - edge.labelPos.y) * ppu - textsize.y / 2.f;
            drawedges[i].text = edge.label.c_str();
            drawedges[i].color = edge.color;

            ImVec2 const lastpoint = drawedges[i].path[points_count - 1];
            float dirx = lastpoint.x - drawedges[i].path[points_count - 2].x;
            float diry = lastpoint.y - drawedges[i].path[points_count - 2].y;
            float const mag = ImSqrt(dirx * dirx + diry * diry);
            float const mul1 = ppu * 0.1f;
            float const mul2 = ppu * 0.0437f;

            dirx /= mag;
            diry /= mag;
            drawedges[i].arrow1.x = lastpoint.x - dirx * mul1 - diry * mul2;
            drawedges[i].arrow1.y = lastpoint.y - diry * mul1 + dirx * mul2;
            drawedges[i].arrow2.x = lastpoint.x - dirx * mul1 + diry * mul2;
            drawedges[i].arrow2.y = lastpoint.y - diry * mul1 - dirx * mul2;
            drawedges[i].arrow3 = lastpoint;
        }
    }
    return count;
}

void IMGUI_GRAPHNODE_NAMESPACE::EndNodeGraph()
{
    auto & cache = g_ctx.graph_caches[g_ctx.lastid];
    float const ppu = cache.pixel_per_unit;
    ImVec2 const cursor_pos = ImGui::GetCursorScreenPos();
    ImDrawList * const drawlist = ImGui::GetWindowDrawList();

    if (cache.graphid_current != cache.graphid_previous)
    {
        ImGuiGraphNodeRenderGraphLayout(cache);
        cache.graphid_previous = cache.graphid_current;
        cache.cursor_previous.x = cursor_pos.x - 1; // force recompute draw buffers
    }
    cache.graphid_current.clear();
    agclose(g_ctx.gvgraph);
    g_ctx.gvgraph = nullptr;

    cache.cursor_current = cursor_pos;
    if (cache.cursor_current.x != cache.cursor_previous.x || cache.cursor_current.y != cache.cursor_previous.y)
    {
        cache.drawnodes.resize(ImGuiGraphNodeFillDrawNodeBuffer(cache.graph, nullptr, cursor_pos, ppu));
        ImGuiGraphNodeFillDrawNodeBuffer(cache.graph, cache.drawnodes.data(), cursor_pos, ppu);
        cache.drawedges.resize(ImGuiGraphNodeFillDrawEdgeBuffer(cache.graph, nullptr, cursor_pos, ppu));
        ImGuiGraphNodeFillDrawEdgeBuffer(cache.graph, cache.drawedges.data(), cursor_pos, ppu);
        cache.cursor_previous = cache.cursor_current;
    }

    for (auto const & node : cache.drawnodes)
    {
        drawlist->AddConvexPolyFilled(node.path, IMGUI_GRAPHNODE_DRAW_NODE_PATH_COUNT, node.fillcolor);
        drawlist->AddPolyline(node.path, IMGUI_GRAPHNODE_DRAW_NODE_PATH_COUNT, node.color, ImDrawFlags_Closed, 1.f);
        drawlist->AddText(node.textpos, node.color, node.text);
    }
    for (auto const & edge : cache.drawedges)
    {
        drawlist->AddText(edge.textpos, edge.color, edge.text);
        drawlist->AddPolyline(edge.path, IMGUI_GRAPHNODE_DRAW_EDGE_PATH_COUNT, edge.color, ImDrawFlags_None, 1.f);
        drawlist->AddTriangleFilled(edge.arrow1, edge.arrow2, edge.arrow3, edge.color);
    }
    ImGui::Dummy(ImVec2(cache.graph.size.x * ppu, cache.graph.size.y * ppu));
}
