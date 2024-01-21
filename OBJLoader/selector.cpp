#include "selector.h"

std::string getCurrentDirectory() {
    char buff[FILENAME_MAX]; //create string buffer to hold path
    _getcwd(buff, FILENAME_MAX);
    std::string current_working_dir(buff);
    return current_working_dir;
}

std::string SingleFileSelector() {
    OPENFILENAME ofn;
    WCHAR fileName[1000] = {};
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFilter = L"All Files\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = fileName;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(fileName);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        char ch[260];
        char DefChar = ' ';
        WideCharToMultiByte(CP_ACP, 0, fileName, -1, ch, 260, &DefChar, NULL);

        std::string result(ch);
        return result;
    }
    return "";
}

std::string GetParentPath(std::string filePath) {
    const size_t found = filePath.find_last_of("/\\");
    if (found == std::string::npos) {
        return filePath;
    }
    else
        return filePath.substr(0, found + 1);
}
