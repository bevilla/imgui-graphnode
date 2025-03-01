#pragma once

#ifndef IMGUI_GRAPHNODE_INTERNAL_H_
#define IMGUI_GRAPHNODE_INTERNAL_H_

#include <vector>
#include <string>
#include <map>

extern "C"
{
#include <gvc.h>
}

#include "imgui_graphnode.h"
#include "imgui_internal.h"

#define IMGUI_GRAPHNODE_CREATE_LABEL_ALLOCA(_name, _label) \
    char * _name; \
    do { \
        char const * const end = ImGui::FindRenderedTextEnd(_label); \
        size_t const size = end - _label; \
        _name = (char *)alloca(size + 1); \
        memcpy(_name, _label, size); \
        _name[size] = '\0'; \
    } while(0)

#define IMGUI_GRAPHNODE_DRAW_NODE_PATH_COUNT 32
#define IMGUI_GRAPHNODE_DRAW_EDGE_PATH_COUNT 64

struct ImGuiGraphNode_Node
{
    std::string name;
    std::string label;
    ImVec2 pos;
    ImVec2 size;
    ImU32 color;
    ImU32 fillcolor;
};

struct ImGuiGraphNode_Edge
{
    std::vector<ImVec2> points;
    std::string tail;
    std::string head;
    std::string label;
    ImVec2 labelPos;
    ImU32 color;
    ImGuiID id;
};

struct ImGuiGraphNode_EdgeInfo
{
    ImU32 color;
};

struct ImGuiGraphNode_EdgeRectangle
{
    ImVec2 a, b, c, d;
};

struct ImGuiGraphNode_Graph
{
    std::map<ImGuiID, ImRect> nodesBB;
    std::map<ImGuiID, std::vector<ImGuiGraphNode_EdgeRectangle>> edgesRectangle;
    std::vector<ImGuiGraphNode_Node> nodes;
    std::vector<ImGuiGraphNode_Edge> edges;
    ImVec2 size;
    float scale;
};

struct ImGuiGraphNode_DrawNode
{
    ImVec2 path[IMGUI_GRAPHNODE_DRAW_NODE_PATH_COUNT];
    ImVec2 textpos;
    char const * text;
    ImU32 color;
    ImU32 fillcolor;
};

struct ImGuiGraphNode_DrawEdge
{
    ImVec2 path[IMGUI_GRAPHNODE_DRAW_EDGE_PATH_COUNT];
    ImVec2 arrow1;
    ImVec2 arrow2;
    ImVec2 arrow3;
    ImVec2 textpos;
    char const * text;
    ImU32 color;
};

struct ImGuiGraphNodeContextCache
{
    std::map<ImGuiID, ImGuiGraphNode_EdgeInfo> edgeIdToInfo;
    ImGuiGraphNode_Graph graph;
    ImGuiGraphNodeLayout layout = ImGuiGraphNodeLayout_Dot;
    float pixel_per_unit = 100.f;
    std::vector<ImGuiGraphNode_DrawNode> drawnodes;
    std::vector<ImGuiGraphNode_DrawEdge> drawedges;
    ImVec2 cursor_previous;
    ImVec2 cursor_current;
    std::string graphid_previous;
    std::string graphid_current;
};

struct ImGuiGraphNodeContext
{
    GVC_t * gvcontext = nullptr;
    graph_t * gvgraph = nullptr;
    ImGuiID lastid = 0;
    std::map<ImGuiID, ImGuiGraphNodeContextCache> graph_caches;
};

extern ImGuiGraphNodeContext g_ctx;

template <size_t N>
class ImGuiGraphNode_ShortString
{
public:
    friend ImGuiGraphNode_ShortString<32> ImGuiIDToString(char const * id);
    friend ImGuiGraphNode_ShortString<16> ImVec4ColorToString(ImVec4 const & color);

    operator char *() { return buf; }
    operator char const *() const { return buf; }

private:
    char buf[N];
};

IMGUI_API ImGuiGraphNode_ShortString<32> ImGuiIDToString(char const * id);
IMGUI_API ImGuiGraphNode_ShortString<16> ImVec4ColorToString(ImVec4 const & color);
IMGUI_API ImU32 ImGuiGraphNode_StringToU32Color(char const * color);
IMGUI_API ImVec4 ImGuiGraphNode_StringToImVec4Color(char const * color);
IMGUI_API char * ImGuiGraphNode_ReadToken(char ** stringp);
IMGUI_API char * ImGuiGraphNode_ReadLine(char ** stringp);
IMGUI_API bool ImGuiGraphNode_ReadGraphFromMemory(ImGuiGraphNodeContextCache & cache, char const * data, size_t size);
IMGUI_API char const * ImGuiGraphNode_GetEngineNameFromLayoutEnum(ImGuiGraphNodeLayout layout);
IMGUI_API ImVec2 ImGuiGraphNode_BezierVec2(ImVec2 const * points, int count, float x);
IMGUI_API ImVec2 ImGuiGraphNode_BSplineVec2(ImVec2 const * points, int count, float x);
IMGUI_API void ImGuiGraphNodeRenderGraphLayout(ImGuiGraphNodeContextCache & cache);

#endif /* !IMGUI_GRAPHNODE_INTERNAL_H_ */
