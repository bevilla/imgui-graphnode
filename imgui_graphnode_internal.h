#pragma once

#ifndef IMGUI_GRAPHNODE_INTERNAL_H_
#define IMGUI_GRAPHNODE_INTERNAL_H_

#include <vector>
#include <string>

extern "C"
{
#include <gvc.h>
}

#include "imgui_graphnode.h"

#define IMGUI_GRAPHNODE_CREATE_LABEL_ALLOCA(_name, _label) \
    char * _name; \
    do { \
        char const * const end = ImGui::FindRenderedTextEnd(_label); \
        size_t const size = end - _label; \
        _name = (char *)alloca(size + 1); \
        memcpy(_name, _label, size); \
        _name[size] = '\0'; \
    } while(0)

struct ImGuiGraphNodeContext
{
    GVC_t * gvcontext = nullptr;
    graph_t * gvgraph = nullptr;
    ImGuiGraphNodeLayout layout = ImGuiGraphNodeLayout_Dot;
    float pixel_per_unit = 100.f;
};

extern ImGuiGraphNodeContext g_ctx;

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
};

struct ImGuiGraphNode_Graph
{
    std::vector<ImGuiGraphNode_Node> nodes;
    std::vector<ImGuiGraphNode_Edge> edges;
    ImVec2 size;
    float scale;
};

template <size_t N>
class ImGuiGraphNode_ShortString
{
public:
    friend ImGuiGraphNode_ShortString<32> ImGuiIDToString(char const * id);
    friend ImGuiGraphNode_ShortString<16> ImVec4ColorToString(ImVec4 const & color);

    operator char *() { return buf; }

private:
    char buf[N];
};

IMGUI_API ImGuiGraphNode_ShortString<32> ImGuiIDToString(char const * id);
IMGUI_API ImGuiGraphNode_ShortString<16> ImVec4ColorToString(ImVec4 const & color);
IMGUI_API ImU32 ImGuiGraphNode_StringToU32Color(char const * color);
IMGUI_API ImVec4 ImGuiGraphNode_StringToImVec4Color(char const * color);
IMGUI_API char * ImGuiGraphNode_ReadToken(char ** stringp);
IMGUI_API char * ImGuiGraphNode_ReadLine(char ** stringp);
IMGUI_API bool ImGuiGraphNode_ReadGraphFromMemory(ImGuiGraphNode_Graph & graph, char const * data, size_t size);
IMGUI_API char const * ImGuiGraphNode_GetEngineNameFromLayoutEnum(ImGuiGraphNodeLayout layout);

#endif /* !IMGUI_GRAPHNODE_INTERNAL_H_ */
