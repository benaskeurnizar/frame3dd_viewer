// ui.cpp
// Nothing is added to World. It uses only render_flags, load_case, view_state and model.

#include "ui.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <system_error>

#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
namespace fs = std::filesystem;

#ifdef __GNUC__
#define UI_PRINTF(a, b) __attribute__((format(printf, a, b)))
#else
#define UI_PRINTF(a, b)
#endif

#if IMGUI_VERSION_NUM >= 19090
#define UI_COL_TAB_ACTIVE ImGuiCol_TabSelected
#else
#define UI_COL_TAB_ACTIVE ImGuiCol_TabActive
#endif

// Rename these if your structureModel uses other names.
#define UI_NODE_COUNT(w) ((w)->model.nodeCount)
#define UI_ELEM_COUNT(w) ((w)->model.elementCount)
#define UI_LC_COUNT(w)   ((w)->model.caseCount)
#define UI_NODES(w)      ((w)->model.nodes)
#define UI_ELEMS(w)      ((w)->model.elements)
#define UI_MODE_COUNT(w) ((w)->model.mode_count)

// ----------------------------------------------------------------------------
// UI-side state
// ----------------------------------------------------------------------------
static GLFWwindow* g_window = nullptr;
static float g_scale = 1.0f;
static float g_menu_h = 0.0f, g_tool_h = 0.0f, g_status_h = 0.0f;
static float g_left_w = 270.0f, g_right_w = 300.0f;      // scaled in ui_init, kept while running
static bool  g_show_left = true, g_left_vis = true, g_right_vis = false;
static char  g_status[192] = "Ready";
static int   g_nodes = 0, g_elems = 0, g_lcs = 0, g_lc = 0;   // cached for the status bar

static unsigned int g_req = 0;      // UI_REQ_* bits, cleared by ui_take_requests()
static int   g_cam = 0;             // UiCameraRequest, cleared by ui_take_camera()
static bool  g_solved = false, g_prev_solved = false;
static int   g_sel_kind = 0, g_sel_id = 0;

static float g_line_w = 2.0f, g_arrow = 1.0f, g_deform = 100.0f, g_diagram = 1.0f;
static int   g_mode = 0;
static bool  g_anim = true;
static float g_anim_speed = 1.0f;

static int           g_freq_n = 0;
static const double* g_freq = nullptr;

static int  g_drag = 0;             // splitter being dragged: 0 none, 1 left, 2 right
static bool g_grip_hot = false;     // mouse is over a splitter

static const unsigned int k_colormaps = COLOR_BY_AXIAL | COLOR_BY_MOMENT | COLOR_BY_DISPLACEMENT;

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------
static ImVec4 rgba(int r, int g, int b, int a = 255) { return ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f); }
static ImVec4 accent(float a = 1.0f) { return ImVec4(0.22f, 0.55f, 0.92f, a); }

static void tip(const char* t)
{
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", t);
}

static bool any_popup_open(void)
{
    return ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
}

static void kv_row(const char* k, const char* fmt, ...) UI_PRINTF(2, 3);
static void kv_row(const char* k, const char* fmt, ...)
{
    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s", k);
    ImGui::TableNextColumn();
    va_list ap;
    va_start(ap, fmt);
    ImGui::TextV(fmt, ap);
    va_end(ap);
}

static bool kv_begin(const char* id)
{
    if (!ImGui::BeginTable(id, 2)) return false;
    ImGui::TableSetupColumn("k", ImGuiTableColumnFlags_WidthFixed, 90.0f * g_scale);
    ImGui::TableSetupColumn("v", ImGuiTableColumnFlags_WidthStretch);
    return true;
}

// col: optional tint of the check mark (0 = default accent). Match it to your renderer's colors.
struct FlagItem { const char* label; unsigned int bit; ImU32 col; };

static const FlagItem k_structure[] = {
    {"Nodes", SHOW_NODES},             {"Elements", SHOW_ELEMENTS},
    {"Supports", SHOW_SUPPORTS},       {"Releases", SHOW_RELEASES},
    {"Local axes", SHOW_LOCAL_AXES},   {"Global axes", SHOW_GLOBAL_AXES},
    {"Node labels", SHOW_NODE_LABELS}, {"Elem. labels", SHOW_ELEMENT_LABELS},
};
static const FlagItem k_loads[] = {
    {"Point", SHOW_POINT_LOADS},        {"Uniform", SHOW_UNIFORM_LOADS},
    {"Trapezoidal", SHOW_TRAP_LOADS},   {"Internal", SHOW_INTERNAL_LOADS},
    {"Temperature", SHOW_TEMP_LOADS},   {"Presc. disp.", SHOW_PRESCRIBED_DISP},
    {"Gravity", SHOW_GRAVITY},
};
static const FlagItem k_shape[] = {
    {"Deformed", SHOW_DEFORMED}, {"Undeformed", SHOW_UNDEFORMED}, {"Reactions", SHOW_REACTIONS},
};
static const FlagItem k_diagrams[] = {
    {"Axial Fx",  SHOW_AXIAL_FORCE, IM_COL32( 86, 156, 255, 255)},
    {"Torque Mx", SHOW_TORQUE_MX,   IM_COL32(180, 120, 255, 255)},
    {"Shear Vy",  SHOW_SHEAR_VY,    IM_COL32( 90, 200, 120, 255)},
    {"Moment My", SHOW_MOMENT_MY,   IM_COL32(255, 140,  70, 255)},
    {"Shear Vz",  SHOW_SHEAR_VZ,    IM_COL32( 70, 200, 200, 255)},
    {"Moment Mz", SHOW_MOMENT_MZ,   IM_COL32(240, 200,  80, 255)},
};

// Two-column grid of CheckboxFlags, optional All/None built from the items' own bits.
template <int N>
static void flag_grid(unsigned int* flags, const char* id, const FlagItem (&items)[N], bool all_none = false)
{
    if (all_none) {
        unsigned int mask = 0;
        for (int i = 0; i < N; i++) mask |= items[i].bit;
        ImGui::PushID(id);
        if (ImGui::SmallButton("All"))  *flags |= mask;
        ImGui::SameLine();
        if (ImGui::SmallButton("None")) *flags &= ~mask;
        ImGui::PopID();
    }
    if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchSame)) return;
    for (int i = 0; i < N; i++) {
        ImGui::TableNextColumn();
        if (items[i].col) ImGui::PushStyleColor(ImGuiCol_CheckMark, items[i].col);
        ImGui::CheckboxFlags(items[i].label, flags, items[i].bit);
        if (items[i].col) ImGui::PopStyleColor();
    }
    ImGui::EndTable();
}

static fs::path home_dir(void)
{
#ifdef _WIN32
    const char* h = getenv("USERPROFILE");
#else
    const char* h = getenv("HOME");
#endif
    std::error_code ec;
    return h ? fs::path(h) : fs::current_path(ec);
}

static bool is_3dd(const fs::path& p)
{
    std::string e = p.extension().string();
    std::transform(e.begin(), e.end(), e.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return e == ".3dd";
}

static void open_model_dialog(void);
static void open_solver_dialog(void);

static void request_solve(World* w)
{
    if (w->solver_found) g_req |= UI_REQ_SOLVE;
    else                 open_solver_dialog();
}

// ----------------------------------------------------------------------------
// Font: a real UI typeface instead of ImGui's bitmap font
// ----------------------------------------------------------------------------
static void load_ui_font(ImGuiIO& io, float px)
{
    static const char* k_fonts[] = {
        "fonts/ui.ttf",                                   // drop Inter / Roboto here to override
#ifdef _WIN32
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
#elif defined(__APPLE__)
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
#endif
    };
    for (const char* p : k_fonts) {
        std::error_code ec;
        if (fs::is_regular_file(fs::path(p), ec) && io.Fonts->AddFontFromFileTTF(p, px)) return;
    }
    ImFontConfig cfg;
    cfg.SizePixels = px;
    io.Fonts->AddFontDefault(&cfg);
}

// ----------------------------------------------------------------------------
// Style
// ----------------------------------------------------------------------------
static void apply_style(void)
{
    ImGuiStyle& s = ImGui::GetStyle();
    ImGui::StyleColorsDark(&s);

    s.WindowPadding = ImVec2(10, 8);
    s.FramePadding = ImVec2(8, 4);
    s.ItemSpacing = ImVec2(8, 6);
    s.ItemInnerSpacing = ImVec2(6, 4);
    s.CellPadding = ImVec2(6, 3);
    s.IndentSpacing = 16;
    s.ScrollbarSize = 12;
    s.GrabMinSize = 10;
    s.WindowBorderSize = 1;
    s.FrameBorderSize = 0;
    s.PopupBorderSize = 1;
    s.SeparatorTextBorderSize = 1;
    s.SeparatorTextPadding = ImVec2(8, 3);
    s.WindowRounding = 0;
    s.ChildRounding = 4;
    s.FrameRounding = 3;
    s.PopupRounding = 4;
    s.ScrollbarRounding = 4;
    s.GrabRounding = 3;
    s.TabRounding = 3;
    s.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    s.DisabledAlpha = 0.45f;
    s.ScaleAllSizes(g_scale);

    ImVec4* c = s.Colors;
    c[ImGuiCol_Text] = rgba(225, 228, 233);
    c[ImGuiCol_TextDisabled] = rgba(125, 131, 143);
    c[ImGuiCol_WindowBg] = rgba(24, 26, 30);
    c[ImGuiCol_ChildBg] = rgba(24, 26, 30, 0);
    c[ImGuiCol_PopupBg] = rgba(30, 33, 38, 250);
    c[ImGuiCol_Border] = rgba(52, 56, 64);
    c[ImGuiCol_FrameBg] = rgba(36, 39, 46);
    c[ImGuiCol_FrameBgHovered] = rgba(44, 48, 57);
    c[ImGuiCol_FrameBgActive] = rgba(50, 55, 66);
    c[ImGuiCol_TitleBg] = rgba(28, 31, 37);
    c[ImGuiCol_TitleBgActive] = rgba(28, 31, 37);
    c[ImGuiCol_TitleBgCollapsed] = rgba(28, 31, 37);
    c[ImGuiCol_MenuBarBg] = rgba(28, 31, 37);
    c[ImGuiCol_ScrollbarBg] = rgba(24, 26, 30, 150);
    c[ImGuiCol_ScrollbarGrab] = rgba(58, 63, 74);
    c[ImGuiCol_ScrollbarGrabHovered] = rgba(74, 80, 93);
    c[ImGuiCol_ScrollbarGrabActive] = rgba(90, 97, 112);
    c[ImGuiCol_CheckMark] = accent();
    c[ImGuiCol_SliderGrab] = accent();
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.65f, 1.0f, 1.0f);
    c[ImGuiCol_Button] = rgba(45, 49, 58);
    c[ImGuiCol_ButtonHovered] = rgba(58, 63, 75);
    c[ImGuiCol_ButtonActive] = rgba(70, 76, 90);
    c[ImGuiCol_Header] = rgba(38, 42, 50);
    c[ImGuiCol_HeaderHovered] = accent(0.30f);
    c[ImGuiCol_HeaderActive] = accent(0.45f);
    c[ImGuiCol_Separator] = rgba(52, 56, 64);
    c[ImGuiCol_ResizeGrip] = accent(0.0f);
    c[ImGuiCol_ResizeGripHovered] = accent(0.50f);
    c[ImGuiCol_ResizeGripActive] = accent(0.80f);
    c[ImGuiCol_Tab] = rgba(24, 26, 30, 0);
    c[ImGuiCol_TabHovered] = accent(0.35f);
    c[UI_COL_TAB_ACTIVE] = rgba(44, 48, 57);
    c[ImGuiCol_TableHeaderBg] = rgba(32, 35, 42);
    c[ImGuiCol_TableBorderStrong] = rgba(52, 56, 64);
    c[ImGuiCol_TableBorderLight] = rgba(42, 45, 52);
    c[ImGuiCol_TextSelectedBg] = accent(0.35f);
}

// ----------------------------------------------------------------------------
// Panel width handling (user-resizable, remembered in the statics above)
// ----------------------------------------------------------------------------
static float clamp_w(float w)
{
    const float W = ImGui::GetIO().DisplaySize.x;
    const float lo = 200.0f * g_scale;
    const float hi = std::max(lo, 0.40f * W);
    return w < lo ? lo : (w > hi ? hi : w);
}

// Runs before the panels are drawn, so panels and 3D viewport use the same width this frame.
static void update_splitters(float top, float h)
{
    ImGuiIO& io = ImGui::GetIO();
    const float W = io.DisplaySize.x;
    const float half = 4.0f * g_scale;
    const bool right_on = (g_sel_kind != 0);

    g_left_w = clamp_w(g_left_w);
    g_right_w = clamp_w(g_right_w);

    const bool blocked = any_popup_open();
    const float mx = io.MousePos.x, my = io.MousePos.y;
    const bool valid = ImGui::IsMousePosValid(&io.MousePos);
    const bool in_y = valid && my >= top && my < top + h;
    const bool on_left  = g_show_left && in_y && std::fabs(mx - g_left_w) <= half;
    const bool on_right = right_on && in_y && std::fabs(mx - (W - g_right_w)) <= half;

    g_grip_hot = !blocked && (on_left || on_right);
    if (g_drag == 0 && g_grip_hot && ImGui::IsMouseClicked(0)) g_drag = on_left ? 1 : 2;
    if (g_drag != 0 && !ImGui::IsMouseDown(0)) g_drag = 0;

    if (g_drag == 1)      g_left_w  = clamp_w(mx);
    else if (g_drag == 2) g_right_w = clamp_w(W - mx);

    if (g_drag || g_grip_hot) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        const bool left = (g_drag != 0) ? (g_drag == 1) : on_left;
        const float x = left ? g_left_w : (W - g_right_w);
        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(x - 1.0f, top), ImVec2(x + 1.0f, top + h),
                                                      ImGui::GetColorU32(accent(0.9f)));
    }
}

// ----------------------------------------------------------------------------
// Menu bar + toolbar
// ----------------------------------------------------------------------------
static void draw_menu_bar(World* w)
{
    if (!ImGui::BeginMainMenuBar()) return;
    const bool has_file = UI_NODE_COUNT(w) > 0;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open .3dd...", "Ctrl+O"))            open_model_dialog();
        if (ImGui::MenuItem("Reload", nullptr, false, has_file))  g_req |= UI_REQ_RELOAD;
        ImGui::Separator();
        if (ImGui::MenuItem("Quit")) glfwSetWindowShouldClose(g_window, GLFW_TRUE);
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

static void tb_sep(void)
{
    const float sp = 8.0f * g_scale;
    ImGui::SameLine(0.0f, sp);
    ImGui::Dummy(ImVec2(1.0f, ImGui::GetFrameHeight()));
    const ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddLine(ImVec2(a.x, a.y + 3.0f * g_scale), ImVec2(a.x, b.y - 3.0f * g_scale),
                                        ImGui::GetColorU32(ImGuiCol_Border));
    ImGui::SameLine(0.0f, sp);
}

static bool tb_button(const char* label, const char* tooltip, bool enabled = true)
{
    ImGui::BeginDisabled(!enabled);
    const bool hit = ImGui::Button(label);
    ImGui::EndDisabled();
    tip(tooltip);
    return hit && enabled;
}

// Toolbar toggle: highlighted while its bit is set in *state, flips the bit on click.
static bool tb_toggle(const char* label, const char* tooltip, unsigned int* state,
                      unsigned int bit, unsigned int group)
{
    const bool on = (*state & bit) != 0;
    if (on) {
        ImGui::PushStyleColor(ImGuiCol_Button, accent(0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.62f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.16f, 0.45f, 0.80f, 1.0f));
    }
    const bool hit = ImGui::Button(label);
    if (on) ImGui::PopStyleColor(3);
    tip(tooltip);
    if (hit && !on) *state = (*state & ~group) | bit;
    return hit;
}

static void draw_toolbar(World* w, float top)
{
    const ImGuiIO& io = ImGui::GetIO();
    const bool has_file = UI_NODE_COUNT(w) > 0;

    ImGui::SetNextWindowPos(ImVec2(0.0f, top));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, g_tool_h));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * g_scale, 5.0f * g_scale));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, rgba(30, 33, 39));
    const ImGuiWindowFlags f = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
                               ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                               ImGuiWindowFlags_NoScrollbar;
    if (ImGui::Begin("##toolbar", nullptr, f)) {
        const ImVec2 wp = ImGui::GetWindowPos();
        ImGui::GetWindowDrawList()->AddLine(ImVec2(wp.x, wp.y + g_tool_h - 1.0f),
                                            ImVec2(wp.x + io.DisplaySize.x, wp.y + g_tool_h - 1.0f),
                                            ImGui::GetColorU32(ImGuiCol_Border));

        // flat buttons
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, rgba(58, 63, 75));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, rgba(70, 76, 90));

        if (tb_button("Open", "Open a .3dd model  (Ctrl+O)")) open_model_dialog();
        ImGui::SameLine();
        if (tb_button("Reload", "Reload the current model", has_file)) g_req |= UI_REQ_RELOAD;
        tb_sep();
        tb_toggle("WASD", "Move the camera with W A S D", &w->view_state, WASD, WASD | ROTATE_OBJECT);
        ImGui::SameLine();
        tb_toggle("Rotate object", "Rotate the object with left-drag", &w->view_state, ROTATE_OBJECT, WASD | ROTATE_OBJECT);
        

        ImGui::PopStyleColor(3);
        tb_sep();

        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Load case");
        ImGui::SameLine();
        ImGui::BeginDisabled(UI_LC_COUNT(w) <= 0);
        ImGui::SetNextItemWidth(84.0f * g_scale);
        char prev[24];
        snprintf(prev, sizeof prev, "LC %d", w->load_case + 1);
        if (ImGui::BeginCombo("##lc", prev)) {
            for (int i = 0; i < UI_LC_COUNT(w); i++) {
                char b[24];
                snprintf(b, sizeof b, "LC %d", i + 1);
                const bool sel = (i == w->load_case);
                if (ImGui::Selectable(b, sel) && !sel) {
                    w->load_case = i;
                    g_req |= UI_REQ_LC_CHANGED;
                }
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::EndDisabled();

        ImGui::SameLine(0.0f, 14.0f * g_scale);
        ImGui::BeginDisabled(!has_file);
        ImGui::PushStyleColor(ImGuiCol_Button, accent(0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.62f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.16f, 0.45f, 0.80f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
        if (ImGui::Button("Solve", ImVec2(72.0f * g_scale, 0.0f))) request_solve(w);
        ImGui::PopStyleColor(4);
        ImGui::EndDisabled();
        tip(has_file ? "Run frame3dd on the current model  (F5)" : "Open a .3dd file first");
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

// ----------------------------------------------------------------------------
// Left panel (tabs)
// ----------------------------------------------------------------------------
static void draw_left_body(World* w)
{
    unsigned int* fl = &w->render_flags;
    const bool just_solved = g_solved && !g_prev_solved;
    g_prev_solved = g_solved;
    const float iw = -(ImGui::CalcTextSize("Diagram scale").x + ImGui::GetStyle().ItemInnerSpacing.x);

    if (!ImGui::BeginTabBar("##tabs")) return;

    if (ImGui::BeginTabItem("Model")) {
        ImGui::PushItemWidth(iw);
        ImGui::SeparatorText("Structure");
        flag_grid(fl, "##g_struct", k_structure);
        ImGui::SliderFloat("Line width", &g_line_w, 1.0f, 8.0f, "%.1f px");

        ImGui::SeparatorText("Loads");
        flag_grid(fl, "##g_loads", k_loads, true);
        ImGui::SliderFloat("Arrow scale", &g_arrow, 0.1f, 5.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Results", nullptr, just_solved ? ImGuiTabItemFlags_SetSelected : 0)) {
        ImGui::PushItemWidth(iw);
        if (!g_solved) ImGui::TextDisabled("Solve to enable.");
        ImGui::BeginDisabled(!g_solved);

        ImGui::SeparatorText("Shape");
        flag_grid(fl, "##g_shape", k_shape);
        ImGui::SliderFloat("Deform. scale", &g_deform, 0.1f, 10000.0f, "%.1fx", ImGuiSliderFlags_Logarithmic);
        tip("Logarithmic. Ctrl+click to type a value.");

        ImGui::SeparatorText("Force diagrams");
        flag_grid(fl, "##g_diag", k_diagrams, true);
        ImGui::SliderFloat("Diagram scale", &g_diagram, 0.1f, 5.0f, "%.2f");

        ImGui::SeparatorText("Color map");
        tip("Colors the members by the chosen quantity. One at a time.");
        const unsigned int before = *fl & k_colormaps;
        ImGui::CheckboxFlags("Axial force", fl, COLOR_BY_AXIAL);
        ImGui::CheckboxFlags("Moment", fl, COLOR_BY_MOMENT);
        ImGui::CheckboxFlags("Displacement", fl, COLOR_BY_DISPLACEMENT);
        const unsigned int added = (*fl & k_colormaps) & ~before;
        if (added) *fl = (*fl & ~k_colormaps) | added;   // keep only the newly ticked one

        ImGui::EndDisabled();
        ImGui::PopItemWidth();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Modes")) {
        ImGui::PushItemWidth(iw);
        const int n = UI_MODE_COUNT(w);
        const bool ok = g_solved && n > 0;
        if (!g_solved)   ImGui::TextDisabled("Solve to enable.");
        else if (n <= 0) ImGui::TextDisabled("No modal analysis in this model.");
        ImGui::BeginDisabled(!ok);

        if (g_mode < 0 || g_mode >= n) g_mode = 0;
        auto mode_label = [](char* b, size_t sz, int i) {
            if (g_freq && i < g_freq_n) snprintf(b, sz, "Mode %d  -  %.2f Hz", i + 1, g_freq[i]);
            else                        snprintf(b, sz, "Mode %d", i + 1);
        };
        char prev[48];
        mode_label(prev, sizeof prev, g_mode);
        if (ImGui::BeginCombo("Mode", prev)) {
            for (int i = 0; i < n; i++) {
                char b[48];
                mode_label(b, sizeof b, i);
                if (ImGui::Selectable(b, i == g_mode)) {
                    if (i != g_mode) {
                        g_req |= UI_REQ_MODE_CHANGED;
                        *fl &= ~(unsigned int)SHOW_MODE_SHAPE;   // stop the animation
                    }
                    g_mode = i;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::CheckboxFlags("Show mode shape", fl, SHOW_MODE_SHAPE);
        ImGui::EndDisabled();
        ImGui::PopItemWidth();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}

static void draw_left_panel(World* w, float top, float h)
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, top));
    ImGui::SetNextWindowSize(ImVec2(g_left_w, h));
    const ImGuiWindowFlags f = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (ImGui::Begin("##left", nullptr, f)) draw_left_body(w);
    ImGui::End();
}

// ----------------------------------------------------------------------------
// Right panel: read-only inspector (only fields the UI can read from World)
// ----------------------------------------------------------------------------
static void draw_inspector(World* w)
{
    const int id = g_sel_id;
    char t[32];

    if (g_sel_kind == 1 && id >= 0 && id < UI_NODE_COUNT(w)) {
        snprintf(t, sizeof t, "Node %d", id + 1);
        ImGui::SeparatorText(t);
        if (kv_begin("##nkv")) {
            kv_row("X", "%.5g", (double)UI_NODES(w)[id].x);
            kv_row("Y", "%.5g", (double)UI_NODES(w)[id].y);
            kv_row("Z", "%.5g", (double)UI_NODES(w)[id].z);
            kv_row("Support", "-");
            ImGui::EndTable();
        }
        ImGui::SeparatorText("Results");
        if (kv_begin("##nres")) {
            kv_row("Displacement", "-");
            kv_row("Reaction", "-");
            ImGui::EndTable();
        }
    } else if (g_sel_kind == 2 && id >= 0 && id < UI_ELEM_COUNT(w)) {
        const int a = UI_ELEMS(w)[id].node1, b = UI_ELEMS(w)[id].node2;
        double len = 0.0;
        if (a >= 0 && a < UI_NODE_COUNT(w) && b >= 0 && b < UI_NODE_COUNT(w)) {
            const double dx = (double)UI_NODES(w)[b].x - (double)UI_NODES(w)[a].x;
            const double dy = (double)UI_NODES(w)[b].y - (double)UI_NODES(w)[a].y;
            const double dz = (double)UI_NODES(w)[b].z - (double)UI_NODES(w)[a].z;
            len = std::sqrt(dx * dx + dy * dy + dz * dz);
        }
        snprintf(t, sizeof t, "Element %d", id + 1);
        ImGui::SeparatorText(t);
        if (kv_begin("##ekv")) {
            kv_row("Nodes", "%d - %d", a + 1, b + 1);
            kv_row("Length", "%.5g", len);
            kv_row("Section", "-");
            kv_row("Material", "-");
            kv_row("Releases", "-");
            ImGui::EndTable();
        }
        ImGui::SeparatorText("End forces");
        ImGui::TextDisabled("-");
    } else {
        ImGui::TextDisabled("Nothing selected.");
    }
}

static void draw_right_panel(World* w, float top, float h)
{
    const float W = ImGui::GetIO().DisplaySize.x;
    ImGui::SetNextWindowPos(ImVec2(W - g_right_w, top));
    ImGui::SetNextWindowSize(ImVec2(g_right_w, h));
    const ImGuiWindowFlags f = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (ImGui::Begin("##right", nullptr, f)) {
        const float bh = ImGui::GetFrameHeight();
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("INSPECTOR");
        ImGui::SameLine(ImGui::GetWindowWidth() - bh - ImGui::GetStyle().WindowPadding.x);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        if (ImGui::Button("x", ImVec2(bh, bh))) g_sel_kind = 0;   // clears the selection
        ImGui::PopStyleColor();
        tip("Close");
        draw_inspector(w);
    }
    ImGui::End();
}

// ----------------------------------------------------------------------------
// Status bar
// ----------------------------------------------------------------------------
static void draw_status_bar(void)
{
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, io.DisplaySize.y - g_status_h));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, g_status_h));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f * g_scale, 4.0f * g_scale));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, rgba(18, 20, 24));
    const ImGuiWindowFlags f = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
                               ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (ImGui::Begin("##statusbar", nullptr, f)) {
        ImGui::TextUnformatted(g_status);
        const float xmin = ImGui::GetItemRectMax().x - ImGui::GetWindowPos().x + 16.0f * g_scale;
        const float ymid = ImGui::GetItemRectMin().y + ImGui::GetTextLineHeight() * 0.5f;

        char right[128];
        snprintf(right, sizeof right, "LC %d/%d  |  %d nodes  |  %d elements  |  %s",
                 g_lcs > 0 ? g_lc + 1 : 0, g_lcs, g_nodes, g_elems, g_solved ? "Solved" : "Not solved");
        const float dot = 14.0f * g_scale;
        float x = ImGui::GetWindowWidth() - ImGui::CalcTextSize(right).x - 10.0f * g_scale;
        if (x < xmin + dot) x = xmin + dot;   // never overlap the message

        const ImVec4 dc = g_solved ? rgba(80, 200, 120) : rgba(125, 131, 143);
        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(ImGui::GetWindowPos().x + x - dot * 0.5f, ymid),
                                                    4.0f * g_scale, ImGui::GetColorU32(dc));
        ImGui::SameLine(x);
        ImGui::TextDisabled("%s", right);
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

// ----------------------------------------------------------------------------
// Viewport overlay (drawn behind the panels, on top of your 3D scene)
// ----------------------------------------------------------------------------
static void draw_viewport_overlay(World* w, float left, float right, float top, float bottom)
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const float pad = 12.0f * g_scale, r = 4.0f * g_scale;
    const ImU32 txt = ImGui::GetColorU32(ImGuiCol_Text);
    const ImU32 dim = IM_COL32(125, 131, 143, 220);

    if (UI_NODE_COUNT(w) <= 0) {
        const char* l1 = "No model loaded";
        const char* l2 = "Open a .3dd file  (Ctrl+O)";
        const ImVec2 s1 = ImGui::CalcTextSize(l1), s2 = ImGui::CalcTextSize(l2);
        const float cx = 0.5f * (left + right), cy = 0.5f * (top + bottom);
        dl->AddText(ImVec2(cx - s1.x * 0.5f, cy - s1.y - 2.0f * g_scale), txt, l1);
        dl->AddText(ImVec2(cx - s2.x * 0.5f, cy + 4.0f * g_scale), dim, l2);
    } else {
        char b[64];
        snprintf(b, sizeof b, "LC %d%s", w->load_case + 1, g_solved ? "   Solved" : "");
        const ImVec2 ts = ImGui::CalcTextSize(b);
        const ImVec2 p0(left + pad, top + pad);
        const ImVec2 p1(p0.x + ts.x + 24.0f * g_scale, p0.y + ts.y + 10.0f * g_scale);
        dl->AddRectFilled(p0, p1, IM_COL32(24, 26, 30, 210), r);
        dl->AddRect(p0, p1, IM_COL32(52, 56, 64, 255), r);
        dl->AddRectFilled(p0, ImVec2(p0.x + 3.0f * g_scale, p1.y), ImGui::GetColorU32(accent()), r,
                          ImDrawFlags_RoundCornersLeft);
        dl->AddText(ImVec2(p0.x + 14.0f * g_scale, p0.y + 5.0f * g_scale), txt, b);
    }

    dl->AddText(ImVec2(left + pad, bottom - pad - ImGui::GetTextLineHeight()), dim,
                "WASD move   |   Left-drag rotate");
}

// ----------------------------------------------------------------------------
// File dialog (one implementation for both "Open model" and "Locate Frame3DD")
// ----------------------------------------------------------------------------
struct FileDlg {
    const char* title;
    const char* prompt;      // optional line of text at the top
    const char* ok_label;
    bool        only_3dd;    // list only .3dd files (folders always listed)
    bool        open = false;
    char        buf[512] = "";
    char        err[128] = "";
    fs::path    dir, listed;
    std::vector<fs::path> dirs, files;
};

static FileDlg g_model_dlg  = {"Open model", nullptr, "Open", true};
static FileDlg g_solver_dlg = {"Locate Frame3DD", "Frame3DD was not found. Browse to the frame3dd executable.", "OK", false};

static void open_model_dialog(void)  { g_model_dlg.open = true; }
static void open_solver_dialog(void) { g_solver_dlg.open = true; }

// Returns true once, when the user confirmed an existing file (its path is in d.buf).
static bool file_dialog(FileDlg& d)
{
    if (d.open) {
        d.err[0] = 0;
        d.buf[0] = 0;
        d.listed.clear();                                   // force a fresh listing
        std::error_code ec;
        if (d.dir.empty() || !fs::is_directory(d.dir, ec)) d.dir = home_dir();
        ImGui::OpenPopup(d.title);
        d.open = false;
    }

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(560.0f * g_scale, 440.0f * g_scale), ImGuiCond_Appearing);
    if (!ImGui::BeginPopupModal(d.title, nullptr, ImGuiWindowFlags_NoResize)) return false;

    if (d.prompt) ImGui::TextWrapped("%s", d.prompt);

    if (ImGui::Button("Up")) {
        if (d.dir.has_parent_path() && d.dir.parent_path() != d.dir) d.dir = d.dir.parent_path();
    }
    ImGui::SameLine();
    if (ImGui::Button("Home")) d.dir = home_dir();
    ImGui::SameLine();
    ImGui::TextDisabled("%s", d.dir.string().c_str());

    // directory listing, refreshed only when the folder changes
    if (d.listed != d.dir) {
        d.listed = d.dir;
        d.dirs.clear();
        d.files.clear();
        std::error_code ec;
        fs::directory_iterator it(d.dir, fs::directory_options::skip_permission_denied, ec), end;
        for (; !ec && it != end; it.increment(ec)) {
            const std::string name = it->path().filename().string();
            if (name.empty() || name[0] == '.') continue;   // hide hidden files
            std::error_code e2;
            if (it->is_directory(e2)) d.dirs.push_back(it->path());
            else if (it->is_regular_file(e2) && (!d.only_3dd || is_3dd(it->path()))) d.files.push_back(it->path());
        }
        std::sort(d.dirs.begin(), d.dirs.end());
        std::sort(d.files.begin(), d.files.end());
    }

    const float footer = ImGui::GetFrameHeightWithSpacing() * 3.0f + ImGui::GetStyle().ItemSpacing.y;
    bool ok = false;
    fs::path go_to;
    if (ImGui::BeginChild("##browser", ImVec2(0, -footer), true)) {
        ImGui::PushStyleColor(ImGuiCol_Text, rgba(140, 185, 245));
        for (const fs::path& p : d.dirs) {
            const std::string label = p.filename().string() + "/";
            if (ImGui::Selectable(label.c_str(), false)) go_to = p;
        }
        ImGui::PopStyleColor();
        for (const fs::path& p : d.files) {
            const std::string full = p.string();
            if (ImGui::Selectable(p.filename().string().c_str(), full == d.buf,
                                  ImGuiSelectableFlags_AllowDoubleClick)) {
                snprintf(d.buf, sizeof d.buf, "%s", full.c_str());
                if (ImGui::IsMouseDoubleClicked(0)) ok = true;
            }
        }
        if (d.files.empty() && d.only_3dd) ImGui::TextDisabled("No .3dd files in this folder");
    }
    ImGui::EndChild();
    if (!go_to.empty()) d.dir = go_to;

    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputTextWithHint("##path", "selected file (or paste a path)", d.buf, sizeof d.buf,
                                 ImGuiInputTextFlags_EnterReturnsTrue))
        ok = true;
    if (d.err[0]) ImGui::TextColored(ImVec4(0.95f, 0.40f, 0.40f, 1.0f), "%s", d.err);

    ImGui::BeginDisabled(d.buf[0] == 0);
    if (ImGui::Button(d.ok_label, ImVec2(100.0f * g_scale, 0.0f))) ok = true;
    ImGui::EndDisabled();
    ImGui::SameLine();
    const bool cancel = ImGui::Button("Cancel", ImVec2(100.0f * g_scale, 0.0f));

    bool accepted = false;
    if (ok && d.buf[0]) {
        std::error_code ec;
        if (fs::is_regular_file(fs::path(d.buf), ec)) {
            d.dir = fs::path(d.buf).parent_path();          // reopen in the same folder next time
            accepted = true;
            ImGui::CloseCurrentPopup();
        } else {
            snprintf(d.err, sizeof d.err, "File not found. Check the path.");
        }
    }
    if (cancel) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    return accepted;
}

static void dialogs(World* w)
{
    if (file_dialog(g_solver_dlg)) {
        snprintf((char*)w->solver_path, sizeof w->solver_path, "%s", g_solver_dlg.buf);
        w->solver_found = 1;
        g_req |= UI_REQ_SOLVE;
    }
    if (file_dialog(g_model_dlg)) {
        // same file already loaded: do nothing
        if (!(w->model_loaded && strcmp(w->model_path, g_model_dlg.buf) == 0)) {
            snprintf((char*)w->model_path, sizeof w->model_path, "%s", g_model_dlg.buf);
            g_req |= UI_REQ_OPEN;
        }
    }
}

// ----------------------------------------------------------------------------
// Public interface
// ----------------------------------------------------------------------------
extern "C" {

void ui_init(GLFWwindow* window)
{
    g_window = window;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;   // WASD must always reach the camera

    // Layout scale in ImGui units = content scale / framebuffer scale.
    int ww = 0, wh = 0, fw = 0, fh = 0;
    glfwGetWindowSize(window, &ww, &wh);
    glfwGetFramebufferSize(window, &fw, &fh);
    const float fbs = (ww > 0 && fw > 0) ? (float)fw / (float)ww : 1.0f;
    float sx = 1.0f, sy = 1.0f;
    glfwGetWindowContentScale(window, &sx, &sy);
    g_scale = ((sx > sy) ? sx : sy) / fbs;
    if (g_scale < 1.0f) g_scale = 1.0f;

    load_ui_font(io, 15.0f * g_scale);

    g_left_w *= g_scale;
    g_right_w *= g_scale;
    apply_style();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void ui_begin_frame(void)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ui_draw(World* world)
{
    if (!world) return;
    ImGuiIO& io = ImGui::GetIO();
    g_nodes = UI_NODE_COUNT(world);
    g_elems = UI_ELEM_COUNT(world);
    g_lcs = UI_LC_COUNT(world);
    g_lc = world->load_case;

    g_menu_h = ImGui::GetFrameHeight();
    g_tool_h = ImGui::GetFrameHeight() + 10.0f * g_scale;
    g_status_h = ImGui::GetTextLineHeight() + 8.0f * g_scale;

    // shortcuts (never while typing or while a dialog is open)
    if (!io.WantTextInput && !any_popup_open()) {
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false)) open_model_dialog();
        if (g_nodes > 0 && ImGui::IsKeyPressed(ImGuiKey_F5, false)) request_solve(world);
    }

    draw_menu_bar(world);

    const float top = g_menu_h + g_tool_h;
    draw_toolbar(world, g_menu_h);

    float h = io.DisplaySize.y - top - g_status_h;
    if (h < 1.0f) h = 1.0f;

    update_splitters(top, h);       // widths are final for this frame from here on

    if (g_show_left) draw_left_panel(world, top, h);
    if (g_sel_kind != 0) draw_right_panel(world, top, h);
    g_left_vis = g_show_left;
    g_right_vis = (g_sel_kind != 0);

    draw_viewport_overlay(world,
                          g_left_vis ? g_left_w : 0.0f,
                          io.DisplaySize.x - (g_right_vis ? g_right_w : 0.0f),
                          top, io.DisplaySize.y - g_status_h);
    draw_status_bar();

    dialogs(world);
}

void ui_end_frame(void)
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int ui_wants_mouse(void)
{
    return (ImGui::GetIO().WantCaptureMouse || g_drag != 0 || g_grip_hot) ? 1 : 0;
}
int ui_wants_keyboard(void) { return ImGui::GetIO().WantCaptureKeyboard ? 1 : 0; }

void ui_shutdown(void)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ui_status(const char* msg) { snprintf(g_status, sizeof g_status, "%s", msg ? msg : ""); }

unsigned int ui_take_requests(void) { const unsigned int r = g_req; g_req = 0; return r; }
int  ui_take_camera(void)           { const int c = g_cam; g_cam = 0; return c; }

void ui_set_solved(int solved) { g_solved = solved != 0; }

void ui_set_modes(int count, const double* freq_hz)
{
    g_freq_n = count > 0 ? count : 0;
    g_freq = freq_hz;
}

void ui_set_selection(int kind, int id)
{
    g_sel_kind = (kind == 1 || kind == 2) ? kind : 0;
    g_sel_id = id < 0 ? 0 : id;
}

float ui_line_width(void)    { return g_line_w; }
float ui_arrow_scale(void)   { return g_arrow; }
float ui_deform_scale(void)  { return g_deform; }
float ui_diagram_scale(void) { return g_diagram; }
int   ui_mode(void)          { return g_mode; }
int   ui_mode_animate(void)  { return g_anim ? 1 : 0; }
float ui_mode_speed(void)    { return g_anim_speed; }

float ui_panel_width(void)   // framebuffer pixels, current width
{
    return g_left_vis ? g_left_w * ImGui::GetIO().DisplayFramebufferScale.x : 0.0f;
}

void ui_get_viewport(int* x, int* y, int* w, int* h)   // framebuffer pixels, bottom-left origin
{
    const ImGuiIO& io = ImGui::GetIO();
    const float sx = io.DisplayFramebufferScale.x, sy = io.DisplayFramebufferScale.y;
    const float l = g_left_vis ? g_left_w : 0.0f;
    const float r = g_right_vis ? g_right_w : 0.0f;
    const int x0 = (int)(l * sx + 0.5f);
    const int x1 = (int)((io.DisplaySize.x - r) * sx + 0.5f);
    const int y0 = (int)(g_status_h * sy + 0.5f);
    const int y1 = (int)((io.DisplaySize.y - g_menu_h - g_tool_h) * sy + 0.5f);
    int vw = x1 - x0, vh = y1 - y0;
    if (vw < 1) vw = 1;
    if (vh < 1) vh = 1;
    if (x) *x = x0;
    if (y) *y = y0;
    if (w) *w = vw;
    if (h) *h = vh;
}

} // extern "C"
