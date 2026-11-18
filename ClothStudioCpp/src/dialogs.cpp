#include "dialogs.hpp"
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
std::filesystem::path openFileDialog(const wchar_t* filter) {
    wchar_t buffer[MAX_PATH]{}; OPENFILENAMEW ofn{}; ofn.lStructSize=sizeof(ofn); ofn.lpstrFilter=filter; ofn.lpstrFile=buffer; ofn.nMaxFile=MAX_PATH; ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
    if (GetOpenFileNameW(&ofn)) return buffer; return {};
}
std::filesystem::path openFolderDialog() {
    BROWSEINFOW bi{}; bi.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE; PIDLIST_ABSOLUTE pid=SHBrowseForFolderW(&bi); if(!pid) return {};
    wchar_t path[MAX_PATH]{}; SHGetPathFromIDListW(pid,path); CoTaskMemFree(pid); return path;
}
