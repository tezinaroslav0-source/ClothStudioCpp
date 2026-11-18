#include "catalog.hpp"
#include "dialogs.hpp"
#include "rpf.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cctype>
#include <windows.h>
#include <shellapi.h>

namespace fs = std::filesystem;
static std::mutex scanMutex;
static ScanResult sharedScan;
static std::atomic_bool scanBusy{false};
static std::string scanStatus = "Готово. Выберите DLC или RPF.";
static fs::path rootPath;
static std::vector<CatalogItem> catalog;
static int selected = -1;
static char pathInput[2048]{};
static char searchInput[256]{};
static int genderFilter = 0, typeFilter = 0, categoryFilter = 0;
static bool wireframe = false;
static const char* genders[] = {"Все", "Мужское", "Женское", "Прочее"};
static const char* types[] = {"Все типы", "Модели", "Текстуры"};
static const char* categories[] = {"Все категории", "Голова", "Маски", "Причёски", "Торс / руки", "Штаны", "Сумки / парашюты", "Обувь", "Шарфы / цепи", "Нижняя одежда", "Бронежилеты", "Декали", "Верхняя одежда", "Головные уборы", "Очки", "Серьги", "Часы / браслеты", "Прочее"};
static bool containsCaseInsensitive(std::string a, std::string b) { std::transform(a.begin(),a.end(),a.begin(),[](unsigned char c){return (char)std::tolower(c);}); std::transform(b.begin(),b.end(),b.begin(),[](unsigned char c){return (char)std::tolower(c);}); return a.find(b)!=std::string::npos; }
static void startScan(fs::path p) {
    if (scanBusy.exchange(true)) return;
    rootPath = p; scanStatus = "Сканирование…";
    std::thread([p](){ auto r=scanDlc(p); {std::lock_guard<std::mutex> lock(scanMutex); sharedScan=std::move(r);} scanBusy=false; }).detach();
}
static void copyFileOut(const fs::path& source) {
    if (!fs::exists(source)) return;
    auto dst = openFileDialog(L"Все файлы\0*.*\0\0"); // reused only to obtain a path? replaced below by Save dialog
    (void)dst;
}
static fs::path saveFileDialog(const wchar_t* filter) {
    wchar_t buffer[MAX_PATH]{}; OPENFILENAMEW ofn{}; ofn.lStructSize=sizeof(ofn); ofn.lpstrFilter=filter; ofn.lpstrFile=buffer; ofn.nMaxFile=MAX_PATH; ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
    if (GetSaveFileNameW(&ofn)) return buffer; return {};
}
static void exportSelected() {
    if(selected<0 || selected>= (int)catalog.size()) return;
    const auto& item=catalog[selected];
    auto dest=saveFileDialog(L"Все файлы\0*.*\0\0");
    if(dest.empty()) return;
    std::error_code ec; fs::copy_file(item.path,dest,fs::copy_options::overwrite_existing,ec);
    scanStatus=ec ? ("Ошибка экспорта: "+ec.message()) : "Файл экспортирован.";
}
static void drawViewerGrid(ImDrawList* dl, ImVec2 a, ImVec2 b) {
    dl->AddRectFilled(a,b,IM_COL32(15,18,20,255));
    const float cx=(a.x+b.x)*.5f, cy=(a.y+b.y)*.5f;
    for(int i=-10;i<=10;i++) {
        float x=cx+i*32.0f; dl->AddLine({x,a.y},{x,b.y},IM_COL32(32,39,42,255));
        float y=cy+i*32.0f; dl->AddLine({a.x,y},{b.x,y},IM_COL32(32,39,42,255));
    }
    dl->AddLine({a.x,cy},{b.x,cy},IM_COL32(90,180,100,255),2.0f);
    dl->AddLine({cx,a.y},{cx,b.y},IM_COL32(65,130,190,255),2.0f);
    dl->AddText({a.x+14,a.y+12},IM_COL32(130,145,150,255),"3D ПРОСМОТР — модуль геометрии в разработке");
    dl->AddText({cx+8,cy+8},IM_COL32(85,150,210,255),"Z");
}
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3); glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window=glfwCreateWindow(1420,880,"Cloth Studio — GTA V Clothing Tools",nullptr,nullptr);
    if(!window){glfwTerminate();return 2;} glfwMakeContextCurrent(window); glfwSwapInterval(1);
    IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGuiIO& io=ImGui::GetIO(); io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark(); ImGuiStyle& style=ImGui::GetStyle(); style.WindowRounding=6; style.FrameRounding=4; style.Colors[ImGuiCol_WindowBg]=ImVec4(.055f,.065f,.07f,1); style.Colors[ImGuiCol_Header]=ImVec4(.10f,.25f,.13f,1); style.Colors[ImGuiCol_Button]=ImVec4(.10f,.27f,.14f,1); style.Colors[ImGuiCol_ButtonHovered]=ImVec4(.16f,.39f,.20f,1); style.Colors[ImGuiCol_CheckMark]=ImVec4(.42f,.86f,.32f,1);
    ImGui_ImplGlfw_InitForOpenGL(window,true); ImGui_ImplOpenGL3_Init("#version 330");
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents(); ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();
        if(ImGui::BeginMainMenuBar()) { ImGui::TextColored({.45f,.9f,.35f,1},"CLOTH STUDIO"); ImGui::Separator(); if(ImGui::MenuItem("Открыть RPF…")){auto p=openFileDialog(L"RPF архив\0*.rpf\0Все файлы\0*.*\0\0");if(!p.empty()){auto info=inspectRpf7(p);scanStatus=p.filename().string()+": "+info.message;}} if(ImGui::MenuItem("Выход"))glfwSetWindowShouldClose(window,GLFW_TRUE); ImGui::EndMainMenuBar(); }
        ImGui::Begin("Панель управления",nullptr,ImGuiWindowFlags_NoCollapse);
        if(ImGui::Button("Открыть RPF…")){auto p=openFileDialog(L"RPF архив\0*.rpf\0Все файлы\0*.*\0\0");if(!p.empty()){auto info=inspectRpf7(p);scanStatus=p.filename().string()+" — "+info.message;}}
        ImGui::SameLine(); if(ImGui::Button("Открыть модель…")){auto p=openFileDialog(L"Модели GTA V\0*.ydd;*.ydr;*.yft\0Все файлы\0*.*\0\0");if(!p.empty())scanStatus="Выбрано: "+p.filename().string()+" (декодер модели пока не реализован).";}
        ImGui::SameLine(); ImGui::Checkbox("Каркас",&wireframe);
        ImGui::Separator(); ImGui::TextUnformatted("Путь к DLC или dlc.rpf:"); ImGui::SetNextItemWidth(-140); ImGui::InputText("##dlcpath",pathInput,sizeof(pathInput)); ImGui::SameLine(); if(ImGui::Button("Файл…")){auto p=openFileDialog(L"RPF архив\0*.rpf\0Все файлы\0*.*\0\0");if(!p.empty()){rootPath=p;std::snprintf(pathInput,sizeof(pathInput),"%s",p.string().c_str());scanStatus="Выбран RPF: "+p.filename().string()+". Извлечение TOC ещё в разработке.";}}
        ImGui::SameLine(); if(ImGui::Button("Папка…")){auto p=openFolderDialog();if(!p.empty()){rootPath=p;std::snprintf(pathInput,sizeof(pathInput),"%s",p.string().c_str());}}
        ImGui::SameLine(); if(ImGui::Button("Сканировать")&&!scanBusy){startScan(pathInput);}
        ImGui::End();
        ImGui::Begin("Каталог DLC",nullptr,ImGuiWindowFlags_NoCollapse);
        ImGui::SetNextItemWidth(200); ImGui::Combo("Пол",&genderFilter,genders,4); ImGui::SameLine(); ImGui::SetNextItemWidth(150); ImGui::Combo("Тип",&typeFilter,types,3); ImGui::SameLine(); ImGui::SetNextItemWidth(210); ImGui::Combo("Категория",&categoryFilter,categories,18); ImGui::SameLine(); ImGui::SetNextItemWidth(200); ImGui::InputTextWithHint("##search","Поиск по имени",searchInput,sizeof(searchInput));
        if(scanBusy) ImGui::TextColored({.8f,.8f,.35f,1},"Сканирование…"); else ImGui::Text("Найдено файлов: %d",(int)catalog.size());
        ImGui::BeginChild("catalogList",ImVec2(0,190),true);
        for(int i=0;i<(int)catalog.size();i++) { const auto& item=catalog[i]; if(genderFilter && item.gender!=genders[genderFilter])continue; if(typeFilter==1&&item.texture)continue; if(typeFilter==2&&!item.texture)continue; if(categoryFilter && item.category!=categories[categoryFilter])continue; if(searchInput[0]&&!containsCaseInsensitive(item.name,searchInput))continue;
          if(ImGui::Selectable((item.name+"##"+std::to_string(i)).c_str(),selected==i)){selected=i;} ImGui::SameLine(480); ImGui::TextDisabled("%s | %s",item.gender.c_str(),item.category.c_str()); }
        ImGui::EndChild();
        if(selected>=0&&selected<(int)catalog.size()){ImGui::Text("Выбрано: %s",catalog[selected].name.c_str());ImGui::SameLine();if(ImGui::Button("Сохранить на диск"))exportSelected();}
        ImGui::End();
        ImGui::Begin("3D View",nullptr,ImGuiWindowFlags_NoCollapse); ImVec2 avail=ImGui::GetContentRegionAvail(); ImVec2 origin=ImGui::GetCursorScreenPos(); ImGui::InvisibleButton("viewport",avail,ImGuiButtonFlags_MouseButtonLeft|ImGuiButtonFlags_MouseButtonMiddle|ImGuiButtonFlags_MouseButtonRight); drawViewerGrid(ImGui::GetWindowDrawList(),origin,{origin.x+avail.x,origin.y+avail.y}); ImGui::End();
        ImGui::Begin("Свойства / статус",nullptr,ImGuiWindowFlags_NoCollapse); if(selected>=0&&selected<(int)catalog.size()){auto& f=catalog[selected];ImGui::Text("Имя: %s",f.name.c_str());ImGui::Text("Путь: %s",f.path.string().c_str());ImGui::Text("Формат: %s",f.extension.c_str());ImGui::Text("Пол: %s",f.gender.c_str());ImGui::Text("Категория: %s",f.category.c_str());} ImGui::Separator(); ImGui::TextWrapped("%s",scanStatus.c_str()); ImGui::End();
        {std::lock_guard<std::mutex> lock(scanMutex); if(!scanBusy&&!sharedScan.items.empty()){catalog=std::move(sharedScan.items);sharedScan.items.clear();selected=-1;}}
        ImGui::Render(); int w,h; glfwGetFramebufferSize(window,&w,&h); glViewport(0,0,w,h); glClearColor(.045f,.05f,.055f,1); glClear(GL_COLOR_BUFFER_BIT); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();glfwDestroyWindow(window);glfwTerminate();return 0;
}
