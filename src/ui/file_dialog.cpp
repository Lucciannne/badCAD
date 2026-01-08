#include "file_dialog.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shobjidl.h>
#endif

namespace badcad {

std::string openFileDialog(bool save, void* windowHandle) {
#ifdef _WIN32
    std::string result;
    
    // Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    bool needsUninit = SUCCEEDED(hr);
    
    IFileDialog* pfd = nullptr;
    
    if (save) {
        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, 
                            IID_IFileSaveDialog, reinterpret_cast<void**>(&pfd));
    } else {
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                            IID_IFileOpenDialog, reinterpret_cast<void**>(&pfd));
    }
    
    if (SUCCEEDED(hr) && pfd) {
        // Set file types
        COMDLG_FILTERSPEC fileTypes[] = {
            { L"badCAD Files", L"*.bCAD" },
            { L"All Files", L"*.*" }
        };
        pfd->SetFileTypes(2, fileTypes);
        pfd->SetFileTypeIndex(1);
        
        if (save) {
            pfd->SetDefaultExtension(L"bCAD");
        }
        
        // Show the dialog
        hr = pfd->Show((HWND)windowHandle);
        
        if (SUCCEEDED(hr)) {
            IShellItem* psiResult = nullptr;
            hr = pfd->GetResult(&psiResult);
            
            if (SUCCEEDED(hr) && psiResult) {
                PWSTR pszFilePath = NULL;
                hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                
                if (SUCCEEDED(hr) && pszFilePath) {
                    // Convert to std::string
                    int size = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, NULL, 0, NULL, NULL);
                    if (size > 0) {
                        result.resize(size - 1);
                        WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, &result[0], size, NULL, NULL);
                    }
                    CoTaskMemFree(pszFilePath);
                }
                psiResult->Release();
            }
        }
        
        pfd->Release();
    }
    
    if (needsUninit) {
        CoUninitialize();
    }
    
    return result;
#else
    return "";
#endif
}

} // namespace badcad
