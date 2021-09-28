#include "imgui_graphnode.h"
#include "imgui_graphnode_internal.h"

ImGuiGraphNodeContext g_ctx;

ImGuiGraphNode_ShortString<32> ImGuiIDToString(char const * id)
{
    ImGuiGraphNode_ShortString<32> str;

    sprintf(str.buf, "%u", ImGui::GetID(id));
    return str;
}

ImGuiGraphNode_ShortString<16> ImVec4ColorToString(ImVec4 const & color)
{
    ImGuiGraphNode_ShortString<16> str;
    ImU32 const rgba = ImGui::ColorConvertFloat4ToU32(color);

    sprintf(str.buf, "#%x", rgba);
    return str;
}

ImU32 ImGuiGraphNode_StringToU32Color(char const * color)
{
    ImU32 rgba;

    sscanf(color, "#%x", &rgba);
    return rgba;
}

ImVec4 ImGuiGraphNode_StringToImVec4Color(char const * color)
{
    return ImGui::ColorConvertU32ToFloat4(ImGuiGraphNode_StringToU32Color(color));
}

char * ImGuiGraphNode_ReadToken(char ** stringp)
{
    if (*stringp && **stringp == '"')
    {
        *stringp += 1;
        char * token = strsep(stringp, "\"");
        strsep(stringp, " ");
        return token;
    }
    return strsep(stringp, " ");
}

char * ImGuiGraphNode_ReadLine(char ** stringp)
{
    return strsep(stringp, "\n");
}

bool ImGuiGraphNode_ReadGraphFromMemory(ImGuiGraphNode_Graph & graph, char const * data, size_t size)
{
    char * copy = static_cast<char *>(alloca(sizeof(*copy) * size + 1));
    char * line = nullptr;

    memcpy(copy, data, size);
    copy[size] = '\0';
    while ((line = ImGuiGraphNode_ReadLine(&copy)) != nullptr)
    {
        char * token = ImGuiGraphNode_ReadToken(&line);

        if (strcmp(token, "graph") == 0)
        {
            graph.scale = atof(ImGuiGraphNode_ReadToken(&line));
            graph.size.x = atof(ImGuiGraphNode_ReadToken(&line));
            graph.size.y = atof(ImGuiGraphNode_ReadToken(&line));
        }
        else if (strcmp(token, "node") == 0)
        {
            ImGuiGraphNode_Node node;

            node.name = ImGuiGraphNode_ReadToken(&line);
            node.pos.x = atof(ImGuiGraphNode_ReadToken(&line));
            node.pos.y = atof(ImGuiGraphNode_ReadToken(&line));
            node.size.x = atof(ImGuiGraphNode_ReadToken(&line));
            node.size.y = atof(ImGuiGraphNode_ReadToken(&line));
            node.label = ImGuiGraphNode_ReadToken(&line);
            ImGuiGraphNode_ReadToken(&line); // style
            ImGuiGraphNode_ReadToken(&line); // shape
            node.color = ImGuiGraphNode_StringToU32Color(ImGuiGraphNode_ReadToken(&line));
            node.fillcolor = ImGuiGraphNode_StringToU32Color(ImGuiGraphNode_ReadToken(&line));
            graph.nodes.push_back(node);
        }
        else if (strcmp(token, "edge") == 0)
        {
            ImGuiGraphNode_Edge edge;

            edge.tail = ImGuiGraphNode_ReadToken(&line);
            edge.head = ImGuiGraphNode_ReadToken(&line);
            int const n = atoi(ImGuiGraphNode_ReadToken(&line));
            edge.points.resize(n);
            for (int i = 0; i < n; ++i)
            {
                edge.points[i].x = atof(ImGuiGraphNode_ReadToken(&line));
                edge.points[i].y = atof(ImGuiGraphNode_ReadToken(&line));
            }

            char const * s1 = ImGuiGraphNode_ReadToken(&line);
            char const * s2 = ImGuiGraphNode_ReadToken(&line);
            char const * s3 = ImGuiGraphNode_ReadToken(&line);
            char const * s4 = ImGuiGraphNode_ReadToken(&line); (void)s4; // style
            char const * s5 = ImGuiGraphNode_ReadToken(&line);

            if (s3)
            {
                edge.label = s1;
                edge.labelPos.x = atof(s2);
                edge.labelPos.y = atof(s3);
                edge.color = ImGuiGraphNode_StringToU32Color(s5);
            }
            else
            {
                edge.color = ImGuiGraphNode_StringToU32Color(s2);
            }
            graph.edges.push_back(edge);
        }
        else if (strcmp(token, "stop") == 0)
        {
            return true;
        }
        else
        {
            IM_ASSERT(false);
        }
    }
    return true;
}

char const * ImGuiGraphNode_GetEngineNameFromLayoutEnum(ImGuiGraphNodeLayout layout)
{
    switch (layout)
    {
        case ImGuiGraphNodeLayout_Circo: return "circo";
        case ImGuiGraphNodeLayout_Dot: return "dot";
        case ImGuiGraphNodeLayout_Fdp: return "fdp";
        case ImGuiGraphNodeLayout_Neato: return "neato";
        case ImGuiGraphNodeLayout_Osage: return "osage";
        case ImGuiGraphNodeLayout_Sfdp: return "sfdp";
        case ImGuiGraphNodeLayout_Twopi: return "twopi";
        default:
            IM_ASSERT(false);
            return "";
    }
}
