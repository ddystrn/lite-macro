#ifndef UNICODE
#define UNICODE
#endif

#define OEMRESOURCE
#include <windows.h>
#include <commdlg.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <chrono>
#include <cmath>

#define IDI_APP_ICON 101

// ID Control GUI Utama (Lite Macro)
#define ID_BTN_REC 101
#define ID_BTN_PLAY 102
#define ID_CHK_INFINITE 103
#define ID_EDIT_LOOP 104
#define ID_BTN_SAVE 105
#define ID_BTN_LOAD 106
#define ID_BTN_EDIT 107
#define ID_CHK_ALWAYS_TOP 108
#define ID_BTN_PAUSE 109
#define ID_CMB_SPEED 110
#define ID_BTN_TOGGLE_PICKER 111

// ID Control Mouse Picker
#define ID_MP_BTN_COORD 303
#define ID_MP_BTN_COLOR 305
#define ID_MP_BTN_BOTH 306
#define ID_MP_RADIO_STOP 307
#define ID_MP_RADIO_PAUSE 308
#define ID_MP_BTN_START 309
#define ID_MP_BTN_LOAD_CFG 310
#define ID_MP_BTN_SAVE_CFG 312
#define ID_MP_CHK_LOCK 311

// ID Control Dialog Editor Script
#define ID_LST_ACTIONS 201
#define ID_EDIT_X 202
#define ID_EDIT_Y 203
#define ID_EDIT_DELAY 204
#define ID_BTN_UPDATE_ACT 205
#define ID_BTN_DELETE_ACT 206
#define ID_BTN_MOVE_UP 207
#define ID_BTN_MOVE_DOWN 208
#define ID_BTN_SAVE_CLOSE 209
#define ID_LST_DRAG_POINTS 210
#define ID_BTN_EDITOR_PICK_COORD 211

// ID Hotkey Global Lite Macro (ESC dihapus)
#define HOTKEY_F8 1
#define HOTKEY_F9 2
#define HOTKEY_F10 4

enum ActionType { ACTION_CLICK, ACTION_DRAG };

struct DragPoint {
    int x, y;
    double delay;
};

struct MacroEvent {
    ActionType type;
    int start_x, start_y;
    int end_x, end_y;
    std::string button;
    double delay;
    std::vector<DragPoint> path;
};

// Variabel Global App
std::vector<MacroEvent> g_events;
std::vector<MacroEvent> g_eventsBackup;
std::atomic<bool> g_isRecording(false);
std::atomic<bool> g_isPlaying(false);
std::atomic<bool> g_isPaused(false);
HHOOK g_mouseHook = NULL;
HFONT g_hFontSegoeUI = NULL;
HINSTANCE g_hInstance = NULL;

std::chrono::high_resolution_clock::time_point g_lastActionTime;
POINT g_dragStartPos = { -1, -1 };
bool g_isMouseDown = false;
std::vector<DragPoint> g_currentDragPath;
std::chrono::high_resolution_clock::time_point g_lastDragPointTime;

std::wstring g_currentScriptName = L"None";
std::wstring g_lastScriptFullPath = L"";
wchar_t g_iniPath[MAX_PATH] = { 0 };
bool g_hasUnsavedChanges = false;
bool g_editorHasChanges = false;
bool g_isPickerExpanded = false;

// Variabel Mouse Picker
std::atomic<bool> mp_isRunning(false);
bool mp_isPickingCoord = false;
bool mp_isPickingColor = false;
bool mp_isPickingBoth = false;
HANDLE mp_hThread = NULL;
HHOOK mp_mouseHook = NULL;
HCURSOR mp_customCursor = NULL;

// Variabel Tambahan Editor Pick Koordinat
bool g_isEditorPickingCoord = false;
HWND hBtnEditorPickCoord = NULL;

int mp_targetX = 100;
int mp_targetY = 100;
COLORREF mp_targetColor = RGB(255, 165, 0);
int mp_triggerAction = 0; // 0 = Stop, 1 = Pause
wchar_t mp_iniFilePath[MAX_PATH] = { 0 };
wchar_t mp_lastConfigTrackerPath[MAX_PATH] = { 0 };

// Handle UI Utama
HWND hMainWnd;
HWND hStatusLabel, hScriptLabel, hBtnRec, hBtnPlay, hBtnPause, hEditLoop, hChkInfinite, hChkAlwaysTop, hBtnSave, hBtnLoad, hBtnEdit, hCmbSpeed, hBtnTogglePicker;
HWND hGroupLoop, hLblLoopStatic, hLblSpeedStatic;

// Handle UI Editor Dialog
HWND hLstActions = NULL, hEditX = NULL, hEditY = NULL, hEditDelay = NULL, hBtnUpdateAct = NULL, hBtnDeleteAct = NULL, hBtnMoveUp = NULL, hBtnMoveDown = NULL;
HWND hTitleDragDetail = NULL, hLstDragPoints = NULL;

// Handle UI Mouse Picker
HWND mp_hTextX, mp_hTextY, mp_hHexColor, mp_hStatus, mp_hConfigStatus, mp_hBtnStart, mp_hBtnLoadConfig, mp_hBtnSaveConfig, mp_hBtnPickCoord, mp_hBtnPickColor, mp_hBtnPickBoth;
HWND mp_hRadioStop, mp_hRadioPause, mp_hGroupTrigger;
HWND mp_hLblTarget, mp_hChkLock;

// Forward Declarations
void StopRecording();
void StopPlaying();
void PausePlaying();
void UpdatePlayButtonUI(bool playing);
void UpdateRecButtonUI(bool recording);
bool LoadMacroFromFile(const wchar_t* filePath);
void LoadMacro();
bool SaveMacro();
bool SaveMacroToSpecificFile(const wchar_t* filePath);
void SaveConfig();
void ResizeMainWindow(bool expand);
void PopulateDragPointsList(const MacroEvent& evt);
void ClearActionEditFields();
void EnableGlobalCrosshair(bool enable);
void SavePickerConfig();

// Mouse Picker Helper Functions
std::wstring ColorToHex(COLORREF color) {
    int r = GetRValue(color);
    int g = GetGValue(color);
    int b = GetBValue(color);
    wchar_t hexStr[8];
    swprintf_s(hexStr, 8, L"%02X%02X%02X", r, g, b);
    return std::wstring(hexStr);
}

COLORREF HexToColor(const std::wstring& hex) {
    std::wstring cleanHex = hex;
    if (!cleanHex.empty() && cleanHex[0] == L'#') {
        cleanHex = cleanHex.substr(1);
    }
    if (cleanHex.length() != 6) return RGB(255, 165, 0);
    unsigned int r, g, b;
    swscanf_s(cleanHex.c_str(), L"%02x%02x%02x", &r, &g, &b);
    return RGB(r, g, b);
}

void InitIniPath() {
    GetModuleFileName(NULL, g_iniPath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(g_iniPath, L'\\');
    if (lastSlash) {
        wcscpy(lastSlash + 1, L"macro.ini");
    }

    wchar_t modulePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, modulePath, MAX_PATH);
    lastSlash = wcsrchr(modulePath, L'\\');
    if (lastSlash) {
        wcscpy_s(mp_lastConfigTrackerPath, MAX_PATH, modulePath);
        wchar_t* trackerSlash = wcsrchr(mp_lastConfigTrackerPath, L'\\');
        if (trackerSlash) {
            wcscpy_s(trackerSlash + 1, MAX_PATH - (trackerSlash - mp_lastConfigTrackerPath + 1), L"last_picker_config.txt");
        }
        std::wifstream trackerFile(mp_lastConfigTrackerPath);
        if (trackerFile.is_open()) {
            std::wstring savedPath;
            if (std::getline(trackerFile, savedPath) && !savedPath.empty()) {
                wcscpy_s(mp_iniFilePath, MAX_PATH, savedPath.c_str());
            }
            trackerFile.close();
        }
    }
    if (wcslen(mp_iniFilePath) == 0 && lastSlash) {
        wcscpy_s(mp_iniFilePath, MAX_PATH, modulePath);
        lastSlash = wcsrchr(mp_iniFilePath, L'\\');
        if (lastSlash) {
            wcscpy_s(lastSlash + 1, MAX_PATH - (lastSlash - mp_iniFilePath + 1), L"picker.ini");
        }
    }
}

void SetAlwaysOnTop(bool top) {
    SetWindowPos(hMainWnd, top ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

double GetCurrentSpeedMultiplier() {
    int sel = (int)SendMessage(hCmbSpeed, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR) return 1.0;
    wchar_t buf[32];
    SendMessage(hCmbSpeed, CB_GETLBTEXT, sel, (LPARAM)buf);
    std::wstring s(buf);
    if (!s.empty() && s.back() == L'x') s.pop_back();
    double val = _wtof(s.c_str());
    return (val > 0.0) ? val : 1.0;
}

void UpdatePickerConfigStatusLabel() {
    if (mp_hConfigStatus) {
        wchar_t* fileName = wcsrchr(mp_iniFilePath, L'\\');
        std::wstring nameStr = fileName ? fileName + 1 : mp_iniFilePath;
        std::wstring statusText = L"Config: " + nameStr;
        SetWindowTextW(mp_hConfigStatus, statusText.c_str());
    }
}

void SetPickerControlsLock(bool locked) {
    EnableWindow(mp_hTextX, !locked);
    EnableWindow(mp_hTextY, !locked);
    EnableWindow(mp_hHexColor, !locked);
    EnableWindow(mp_hBtnPickCoord, !locked);
    EnableWindow(mp_hBtnPickColor, !locked);
    EnableWindow(mp_hBtnPickBoth, !locked);
}

void SavePickerConfig() {
    wchar_t buf[64];
    GetWindowTextW(mp_hTextX, buf, 64);
    WritePrivateProfileStringW(L"Settings", L"TargetX", buf, mp_iniFilePath);
    GetWindowTextW(mp_hTextY, buf, 64);
    WritePrivateProfileStringW(L"Settings", L"TargetY", buf, mp_iniFilePath);
    GetWindowTextW(mp_hHexColor, buf, 64);
    WritePrivateProfileStringW(L"Settings", L"HexColor", buf, mp_iniFilePath);

    swprintf_s(buf, 64, L"%d", mp_triggerAction);
    WritePrivateProfileStringW(L"Settings", L"TriggerAction", buf, mp_iniFilePath);

    BOOL isLocked = (SendMessage(mp_hChkLock, BM_GETCHECK, 0, 0) == BST_CHECKED);
    WritePrivateProfileStringW(L"Settings", L"PickerLocked", isLocked ? L"1" : L"0", mp_iniFilePath);

    if (wcslen(mp_lastConfigTrackerPath) > 0) {
        std::wofstream trackerFile(mp_lastConfigTrackerPath);
        if (trackerFile.is_open()) {
            trackerFile << mp_iniFilePath;
            trackerFile.close();
        }
    }
}

void LoadPickerConfig() {
    wchar_t buf[64];
    GetPrivateProfileStringW(L"Settings", L"TargetX", L"100", buf, 64, mp_iniFilePath);
    SetWindowTextW(mp_hTextX, buf);
    mp_targetX = _wtoi(buf);

    GetPrivateProfileStringW(L"Settings", L"TargetY", L"100", buf, 64, mp_iniFilePath);
    SetWindowTextW(mp_hTextY, buf);
    mp_targetY = _wtoi(buf);

    GetPrivateProfileStringW(L"Settings", L"HexColor", L"FFA500", buf, 64, mp_iniFilePath);
    SetWindowTextW(mp_hHexColor, buf);
    mp_targetColor = HexToColor(buf);

    mp_triggerAction = GetPrivateProfileIntW(L"Settings", L"TriggerAction", 0, mp_iniFilePath);
    if (mp_triggerAction == 1) {
        SendMessage(mp_hRadioPause, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(mp_hRadioStop, BM_SETCHECK, BST_UNCHECKED, 0);
    } else {
        SendMessage(mp_hRadioStop, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(mp_hRadioPause, BM_SETCHECK, BST_UNCHECKED, 0);
    }

    int isLocked = GetPrivateProfileIntW(L"Settings", L"PickerLocked", 0, mp_iniFilePath);
    if (isLocked == 1) {
        SendMessage(mp_hChkLock, BM_SETCHECK, BST_CHECKED, 0);
        SetPickerControlsLock(true);
    } else {
        SendMessage(mp_hChkLock, BM_SETCHECK, BST_UNCHECKED, 0);
        SetPickerControlsLock(false);
    }

    UpdatePickerConfigStatusLabel();
}

void LoadConfig() {
    UINT isInfinite = GetPrivateProfileInt(L"Settings", L"InfiniteLoop", 0, g_iniPath);
    if (isInfinite == 1) {
        SendMessage(hChkInfinite, BM_SETCHECK, BST_CHECKED, 0);
        EnableWindow(hEditLoop, FALSE);
    } else {
        SendMessage(hChkInfinite, BM_SETCHECK, BST_UNCHECKED, 0);
        EnableWindow(hEditLoop, TRUE);
    }

    UINT isAlwaysTop = GetPrivateProfileInt(L"Settings", L"AlwaysOnTop", 0, g_iniPath);
    if (isAlwaysTop == 1) {
        SendMessage(hChkAlwaysTop, BM_SETCHECK, BST_CHECKED, 0);
        SetAlwaysOnTop(true);
    } else {
        SendMessage(hChkAlwaysTop, BM_SETCHECK, BST_UNCHECKED, 0);
        SetAlwaysOnTop(false);
    }

    int speedIdx = GetPrivateProfileInt(L"Settings", L"SpeedIndex", 5, g_iniPath);
    SendMessage(hCmbSpeed, CB_SETCURSEL, speedIdx, 0);

    g_isPickerExpanded = (GetPrivateProfileInt(L"Settings", L"PickerExpanded", 0, g_iniPath) == 1);

    wchar_t lastScript[MAX_PATH] = { 0 };
    GetPrivateProfileString(L"Settings", L"LastScriptPath", L"", lastScript, MAX_PATH, g_iniPath);
    if (wcslen(lastScript) > 0) {
        if (LoadMacroFromFile(lastScript)) {
            g_hasUnsavedChanges = false;
        }
    }

    if (hMainWnd && IsWindow(hMainWnd)) {
        int savedX = GetPrivateProfileInt(L"Settings", L"WindowX", -9999, g_iniPath);
        int savedY = GetPrivateProfileInt(L"Settings", L"WindowY", -9999, g_iniPath);

        RECT rc;
        GetWindowRect(hMainWnd, &rc);
        int winW = rc.right - rc.left;
        int winH = rc.bottom - rc.top;

        if (savedX != -9999 && savedY != -9999) {
            SetWindowPos(hMainWnd, NULL, savedX, savedY, winW, winH, SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }

    LoadPickerConfig();
}

void SaveConfig() {
    BOOL checkedInfinite = (SendMessage(hChkInfinite, BM_GETCHECK, 0, 0) == BST_CHECKED);
    WritePrivateProfileString(L"Settings", L"InfiniteLoop", checkedInfinite ? L"1" : L"0", g_iniPath);

    BOOL checkedAlwaysTop = (SendMessage(hChkAlwaysTop, BM_GETCHECK, 0, 0) == BST_CHECKED);
    WritePrivateProfileString(L"Settings", L"AlwaysOnTop", checkedAlwaysTop ? L"1" : L"0", g_iniPath);

    int speedIdx = (int)SendMessage(hCmbSpeed, CB_GETCURSEL, 0, 0);
    WritePrivateProfileString(L"Settings", L"SpeedIndex", std::to_wstring(speedIdx).c_str(), g_iniPath);
    WritePrivateProfileString(L"Settings", L"PickerExpanded", g_isPickerExpanded ? L"1" : L"0", g_iniPath);

    WritePrivateProfileString(L"Settings", L"LastScriptPath", g_lastScriptFullPath.c_str(), g_iniPath);

    if (hMainWnd && IsWindow(hMainWnd)) {
        RECT rc;
        GetWindowRect(hMainWnd, &rc);
        WritePrivateProfileStringW(L"Settings", L"WindowX", std::to_wstring(rc.left).c_str(), g_iniPath);
        WritePrivateProfileStringW(L"Settings", L"WindowY", std::to_wstring(rc.top).c_str(), g_iniPath);
    }

    SavePickerConfig();
}

void EnableGlobalCrosshair(bool enable) {
    if (enable) {
        HCURSOR hCross = LoadCursor(NULL, IDC_CROSS);
        mp_customCursor = (HCURSOR)CopyImage(hCross, IMAGE_CURSOR, 0, 0, LR_COPYFROMRESOURCE);
        SetSystemCursor(mp_customCursor, OCR_NORMAL);
    } else {
        SystemParametersInfoA(SPI_SETCURSORS, 0, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    }
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN) {
            MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;
            if (pMouseStruct != NULL) {
                if (g_isEditorPickingCoord) {
                    wchar_t bufX[16], bufY[16];
                    swprintf_s(bufX, 16, L"%d", pMouseStruct->pt.x);
                    swprintf_s(bufY, 16, L"%d", pMouseStruct->pt.y);
                    if (hEditX) SetWindowTextW(hEditX, bufX);
                    if (hEditY) SetWindowTextW(hEditY, bufY);

                    g_isEditorPickingCoord = false;
                    UnhookWindowsHookEx(mp_mouseHook);
                    mp_mouseHook = NULL;
                    EnableGlobalCrosshair(false);
                    if (hBtnEditorPickCoord) SetWindowTextW(hBtnEditorPickCoord, L"Ambil Koordinat");
                    return 1;
                }

                HDC hdc = GetDC(NULL);
                if (mp_isPickingCoord) {
                    mp_targetX = pMouseStruct->pt.x;
                    mp_targetY = pMouseStruct->pt.y;
                    wchar_t bufX[16], bufY[16];
                    swprintf_s(bufX, 16, L"%d", mp_targetX);
                    swprintf_s(bufY, 16, L"%d", mp_targetY);
                    SetWindowTextW(mp_hTextX, bufX);
                    SetWindowTextW(mp_hTextY, bufY);
                    mp_isPickingCoord = false;
                    UnhookWindowsHookEx(mp_mouseHook);
                    mp_mouseHook = NULL;
                    EnableGlobalCrosshair(false);
                    SavePickerConfig();
                    SetWindowTextW(mp_hStatus, L"Status: Koordinat diambil dan disimpan!");
                    SetWindowTextW(mp_hBtnPickCoord, L"Pick Koordinat");
                    ReleaseDC(NULL, hdc);
                    return 1;
                }
                else if (mp_isPickingColor) {
                    COLORREF pickedColor = GetPixel(hdc, pMouseStruct->pt.x, pMouseStruct->pt.y);
                    if (pickedColor != CLR_INVALID) {
                        mp_targetColor = pickedColor;
                        std::wstring hexStr = ColorToHex(mp_targetColor);
                        SetWindowTextW(mp_hHexColor, hexStr.c_str());
                    }
                    mp_isPickingColor = false;
                    UnhookWindowsHookEx(mp_mouseHook);
                    mp_mouseHook = NULL;
                    EnableGlobalCrosshair(false);
                    SavePickerConfig();
                    SetWindowTextW(mp_hStatus, L"Status: Warna diambil dan disimpan!");
                    SetWindowTextW(mp_hBtnPickColor, L"Pick Warna");
                    ReleaseDC(NULL, hdc);
                    return 1;
                }
                else if (mp_isPickingBoth) {
                    mp_targetX = pMouseStruct->pt.x;
                    mp_targetY = pMouseStruct->pt.y;
                    wchar_t bufX[16], bufY[16];
                    swprintf_s(bufX, 16, L"%d", mp_targetX);
                    swprintf_s(bufY, 16, L"%d", mp_targetY);
                    SetWindowTextW(mp_hTextX, bufX);
                    SetWindowTextW(mp_hTextY, bufY);
                    COLORREF pickedColor = GetPixel(hdc, mp_targetX, mp_targetY);
                    if (pickedColor != CLR_INVALID) {
                        mp_targetColor = pickedColor;
                        std::wstring hexStr = ColorToHex(mp_targetColor);
                        SetWindowTextW(mp_hHexColor, hexStr.c_str());
                    }
                    mp_isPickingBoth = false;
                    UnhookWindowsHookEx(mp_mouseHook);
                    mp_mouseHook = NULL;
                    EnableGlobalCrosshair(false);
                    SavePickerConfig();
                    SetWindowTextW(mp_hStatus, L"Status: Koordinat dan Warna diambil!");
                    SetWindowTextW(mp_hBtnPickBoth, L"Pick Keduanya (Koordinat dan Warna)");
                    ReleaseDC(NULL, hdc);
                    return 1;
                }
                ReleaseDC(NULL, hdc);
            }
        }
    }
    return CallNextHookEx(mp_mouseHook, nCode, wParam, lParam);
}

DWORD WINAPI DetectionThread(LPVOID lpParam) {
    HDC hdc = GetDC(NULL);
    if (!hdc) {
        SetWindowTextW(mp_hStatus, L"Status: Gagal mengambil Device Context layar!");
        return 0;
    }
    SetWindowTextW(mp_hStatus, L"Status: Berjalan dan Memantau layar...");

    while (mp_isRunning) {
        wchar_t hexBuf[16];
        GetWindowTextW(mp_hHexColor, hexBuf, 16);
        mp_targetColor = HexToColor(hexBuf);
        
        COLORREF color = GetPixel(hdc, mp_targetX, mp_targetY);

        if (color != CLR_INVALID) {
            int r = GetRValue(color);
            int g = GetGValue(color);
            int b = GetBValue(color);

            int tr = GetRValue(mp_targetColor);
            int tg = GetGValue(mp_targetColor);
            int tb = GetBValue(mp_targetColor);

            double distance = sqrt(pow(r - tr, 2) + pow(g - tg, 2) + pow(b - tb, 2));
            double maxDistance = sqrt(pow(255, 2) + pow(255, 2) + pow(255, 2));
            double similarity = (1.0 - (distance / maxDistance)) * 100.0;

            if (similarity >= 90.0) {
                if (mp_triggerAction == 0) {
                    if (g_isPlaying) StopPlaying();
                    if (g_isRecording) StopRecording();
                    std::wstring statusMsg = L"Status: Triggered Stop! (Kemiripan: " + std::to_wstring((int)similarity) + L"%)";
                    SetWindowTextW(mp_hStatus, statusMsg.c_str());
                } else {
                    if (g_isPlaying && !g_isPaused) {
                        PausePlaying();
                        std::wstring statusMsg = L"Status: Triggered Pause! (Kemiripan: " + std::to_wstring((int)similarity) + L"%)";
                        SetWindowTextW(mp_hStatus, statusMsg.c_str());
                    }
                }
                Sleep(1000);
            }
        }
        Sleep(50);
    }
    ReleaseDC(NULL, hdc);
    SetWindowTextW(mp_hStatus, L"Status: Berhenti.");
    return 0;
}

bool SleepInterruptible(double seconds) {
    if (seconds <= 0) return true;
    auto start = std::chrono::high_resolution_clock::now();
    double targetMs = seconds * 1000.0;
    
    while (g_isPlaying) {
        while (g_isPaused && g_isPlaying) {
            Sleep(50);
        }
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - start;
        if (elapsed.count() >= targetMs) return true;
        Sleep(5);
    }
    return false;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && g_isRecording) {
        MSLLHOOKSTRUCT* pMouse = (MSLLHOOKSTRUCT*)lParam;
        auto now = std::chrono::high_resolution_clock::now();

        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) {
            g_isMouseDown = true;
            g_dragStartPos.x = pMouse->pt.x;
            g_dragStartPos.y = pMouse->pt.y;
            g_currentDragPath.clear();
            g_currentDragPath.push_back({ pMouse->pt.x, pMouse->pt.y, 0.0 });
            g_lastDragPointTime = now;
        }
        else if (wParam == WM_MOUSEMOVE && g_isMouseDown) {
            std::chrono::duration<double> p_delay = now - g_lastDragPointTime;
            g_currentDragPath.push_back({ pMouse->pt.x, pMouse->pt.y, p_delay.count() });
            g_lastDragPointTime = now;
        }
        else if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP) {
            if (g_isMouseDown) {
                std::chrono::duration<double> elapsed = now - g_lastActionTime;
                std::string btn = (wParam == WM_LBUTTONUP) ? "left" : "right";

                int dx = abs(pMouse->pt.x - g_dragStartPos.x);
                int dy = abs(pMouse->pt.y - g_dragStartPos.y);

                MacroEvent evt;
                evt.button = btn;
                evt.delay = elapsed.count();

                if (dx > 5 || dy > 5) {
                    evt.type = ACTION_DRAG;
                    evt.start_x = g_dragStartPos.x;
                    evt.start_y = g_dragStartPos.y;
                    evt.end_x = pMouse->pt.x;
                    evt.end_y = pMouse->pt.y;
                    evt.path = g_currentDragPath;
                }
                else {
                    evt.type = ACTION_CLICK;
                    evt.end_x = pMouse->pt.x;
                    evt.end_y = pMouse->pt.y;
                }

                g_events.push_back(evt);
                g_isMouseDown = false;
                g_lastActionTime = std::chrono::high_resolution_clock::now();
            }
        }
    }
    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

void StartRecording() {
    if (g_isRecording || g_isPlaying) return;
    g_events.clear();
    g_isRecording = true;
    g_isMouseDown = false;
    g_hasUnsavedChanges = true;
    g_lastActionTime = std::chrono::high_resolution_clock::now();

    g_currentScriptName = L"Hasil Rekaman Baru";
    SetWindowText(hScriptLabel, (L"Script: " + g_currentScriptName).c_str());
    SetWindowText(hStatusLabel, L"Status: Merekam... (F8: Stop)");

    UpdateRecButtonUI(true);
    EnableWindow(hBtnPlay, FALSE);

    g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(NULL), 0);
}

void StopRecording() {
    if (g_isRecording) {
        g_isRecording = false;
        if (g_mouseHook) {
            UnhookWindowsHookEx(g_mouseHook);
            g_mouseHook = NULL;
        }
        std::wstring status = L"Status: Selesai (" + std::to_wstring(g_events.size()) + L" aksi terekam)";
        SetWindowText(hStatusLabel, status.c_str());
        UpdateRecButtonUI(false);
        EnableWindow(hBtnPlay, TRUE);
    }
}

void UpdateRecButtonUI(bool recording) {
    if (recording) {
        SetWindowText(hBtnRec, L"STOP (F8)");
    } else {
        SetWindowText(hBtnRec, L"REC (F8)");
    }
}

void PlayMacro() {
    if (g_isRecording) return;
    if (g_isPlaying) {
        StopPlaying();
        return;
    }
    if (g_events.empty()) {
        MessageBox(NULL, L"Belum ada makro yang terekam/dimuat!", L"Peringatan", MB_OK | MB_ICONWARNING);
        return;
    }

    g_isPlaying = true;
    g_isPaused = false;
    EnableWindow(hBtnRec, FALSE);
    UpdatePlayButtonUI(true);
    
    SetWindowPos(hBtnRec, NULL, 12, 54, 80, 30, SWP_NOZORDER);
    SetWindowPos(hBtnPlay, NULL, 98, 54, 80, 30, SWP_NOZORDER);
    ShowWindow(hBtnPause, SW_SHOW);
    SetWindowPos(hBtnPause, NULL, 184, 54, 84, 30, SWP_NOZORDER);

    bool isInfinite = (SendMessage(hChkInfinite, BM_GETCHECK, 0, 0) == BST_CHECKED);
    wchar_t buf[16];
    GetWindowText(hEditLoop, buf, 16);
    int maxLoops = _wtoi(buf);
    if (maxLoops <= 0) maxLoops = 1;

    double speedMultiplier = GetCurrentSpeedMultiplier();

    std::thread([isInfinite, maxLoops, speedMultiplier]() {
        int currentLoop = 0;

        while (g_isPlaying) {
            currentLoop++;
            std::wstring loopText = isInfinite ? L"Loop ke-" + std::to_wstring(currentLoop)
                                               : L"Loop " + std::to_wstring(currentLoop) + L"/" + std::to_wstring(maxLoops);
            SetWindowText(hStatusLabel, (L"Status: Memutar (" + loopText + L")").c_str());

            for (const auto& evt : g_events) {
                if (!g_isPlaying) break;
                while (g_isPaused && g_isPlaying) {
                    Sleep(50);
                }

                double adjustedDelay = evt.delay / speedMultiplier;
                if (!SleepInterruptible(adjustedDelay)) break;

                DWORD downFlag = (evt.button == "left" || evt.button == "1") ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
                DWORD upFlag = (evt.button == "left" || evt.button == "1") ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;

                if (evt.type == ACTION_CLICK) {
                    SetCursorPos(evt.end_x, evt.end_y);
                    if (!SleepInterruptible(0.03 / speedMultiplier)) break;
                    mouse_event(downFlag, 0, 0, 0, 0);
                    if (!SleepInterruptible(0.04 / speedMultiplier)) {
                        mouse_event(upFlag, 0, 0, 0, 0);
                        break;
                    }
                    mouse_event(upFlag, 0, 0, 0, 0);
                }
                else if (evt.type == ACTION_DRAG) {
                    SetCursorPos(evt.start_x, evt.start_y);
                    if (!SleepInterruptible(0.03 / speedMultiplier)) break;
                    mouse_event(downFlag, 0, 0, 0, 0);
                    if (!SleepInterruptible(0.03 / speedMultiplier)) {
                        mouse_event(upFlag, 0, 0, 0, 0);
                        break;
                    }

                    bool cancelledDuringDrag = false;
                    for (const auto& pt : evt.path) {
                        if (!g_isPlaying) {
                            cancelledDuringDrag = true;
                            break;
                        }
                        while (g_isPaused && g_isPlaying) {
                            Sleep(50);
                        }
                        SetCursorPos(pt.x, pt.y);
                        double ptDelay = pt.delay / speedMultiplier;
                        if (!SleepInterruptible(ptDelay)) {
                            cancelledDuringDrag = true;
                            break;
                        }
                    }

                    if (cancelledDuringDrag) {
                        mouse_event(upFlag, 0, 0, 0, 0);
                        break;
                    }

                    SetCursorPos(evt.end_x, evt.end_y);
                    if (!SleepInterruptible(0.03 / speedMultiplier)) {
                        mouse_event(upFlag, 0, 0, 0, 0);
                        break;
                    }
                    mouse_event(upFlag, 0, 0, 0, 0);
                }
            }

            if (!isInfinite && currentLoop >= maxLoops) break;
        }

        std::wstring finStatus = L"Status: Selesai (" + std::to_wstring(currentLoop) + L" loop dijalankan)";
        SetWindowText(hStatusLabel, finStatus.c_str());
        EnableWindow(hBtnRec, TRUE);
        g_isPlaying = false;
        g_isPaused = false;
        UpdatePlayButtonUI(false);
        
        ShowWindow(hBtnPause, SW_HIDE);
        SetWindowPos(hBtnRec, NULL, 12, 54, 124, 30, SWP_NOZORDER);
        SetWindowPos(hBtnPlay, NULL, 144, 54, 124, 30, SWP_NOZORDER);
    }).detach();
}

void StopPlaying() {
    g_isPaused = false;
    g_isPlaying = false;
}

void PausePlaying() {
    if (!g_isPlaying) return;
    g_isPaused = !g_isPaused;
    if (g_isPaused) {
        SetWindowText(hBtnPause, L"RESUME (F10)");
        SetWindowText(hStatusLabel, L"Status: Dijeda (Paused)");
    } else {
        SetWindowText(hBtnPause, L"PAUSE (F10)");
        SetWindowText(hStatusLabel, L"Status: Memutar dilanjutkan...");
    }
}

void UpdatePlayButtonUI(bool playing) {
    if (playing) {
        SetWindowText(hBtnPlay, L"STOP (F9)");
    } else {
        SetWindowText(hBtnPlay, L"PLAY (F9)");
        SetWindowText(hBtnPause, L"PAUSE (F10)");
    }
}

bool SaveMacroToSpecificFile(const wchar_t* filePath) {
    std::wofstream file(filePath);
    if (file.is_open()) {
        for (const auto& e : g_events) {
            file << e.type << L"," << e.start_x << L"," << e.start_y << L","
                 << e.end_x << L"," << e.end_y << L","
                 << (e.button == "left" || e.button == "1" ? 1 : 2) << L"," << e.delay << L"," << e.path.size() << L"\n";
            for (const auto& p : e.path) {
                file << p.x << L"," << p.y << L"," << p.delay << L"\n";
            }
        }
        file.close();
        g_lastScriptFullPath = filePath;
        wchar_t* fileName = wcsrchr((wchar_t*)filePath, L'\\');
        g_currentScriptName = fileName ? fileName + 1 : filePath;
        SetWindowText(hScriptLabel, (L"Script: " + g_currentScriptName).c_str());
        
        g_hasUnsavedChanges = false;
        SaveConfig();
        return true;
    }
    return false;
}

bool SaveMacro() {
    if (g_events.empty()) return false;
    OPENFILENAME ofn = { 0 };
    wchar_t szFile[260] = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Macro Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        return SaveMacroToSpecificFile(ofn.lpstrFile);
    }
    return false;
}

bool LoadMacroFromFile(const wchar_t* filePath) {
    std::wifstream file(filePath);
    if (file.is_open()) {
        g_events.clear();
        std::wstring line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::wstringstream ss(line);
            std::wstring token;
            MacroEvent e;
            int typeInt, btnInt, pathSize;

            std::getline(ss, token, L','); typeInt = _wtoi(token.c_str()); e.type = (ActionType)typeInt;
            std::getline(ss, token, L','); e.start_x = _wtoi(token.c_str());
            std::getline(ss, token, L','); e.start_y = _wtoi(token.c_str());
            std::getline(ss, token, L','); e.end_x = _wtoi(token.c_str());
            std::getline(ss, token, L','); e.end_y = _wtoi(token.c_str());
            std::getline(ss, token, L','); btnInt = _wtoi(token.c_str()); e.button = (btnInt == 1) ? "left" : "right";
            std::getline(ss, token, L','); e.delay = _wtof(token.c_str());
            std::getline(ss, token, L','); pathSize = _wtoi(token.c_str());

            for (int i = 0; i < pathSize; ++i) {
                if (std::getline(file, line)) {
                    std::wstringstream ssP(line);
                    DragPoint p;
                    std::getline(ssP, token, L','); p.x = _wtoi(token.c_str());
                    std::getline(ssP, token, L','); p.y = _wtoi(token.c_str());
                    std::getline(ssP, token, L','); p.delay = _wtof(token.c_str());
                    e.path.push_back(p);
                }
            }
            g_events.push_back(e);
        }
        file.close();
        
        g_lastScriptFullPath = filePath;
        wchar_t* fileName = wcsrchr((wchar_t*)filePath, L'\\');
        g_currentScriptName = fileName ? fileName + 1 : filePath;
        SetWindowText(hScriptLabel, (L"Script: " + g_currentScriptName).c_str());
        SetWindowText(hStatusLabel, (L"Status: Loaded (" + std::to_wstring(g_events.size()) + L" aksi)").c_str());
        return true;
    }
    return false;
}

void LoadMacro() {
    OPENFILENAME ofn = { 0 };
    wchar_t szFile[260] = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Macro Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        if (LoadMacroFromFile(ofn.lpstrFile)) {
            g_hasUnsavedChanges = false;
            SaveConfig();
        }
    }
}

std::wstring FormatActionToHumanLanguage(int index, const MacroEvent& evt) {
    std::wstringstream wss;
    bool isRight = (evt.button == "right" || evt.button == "2");

    wss << (index + 1) << L". ";

    if (evt.type == ACTION_CLICK) {
        wss << (isRight ? L"Klik Kanan" : L"Klik Kiri")
            << L" di Posisi (X: " << evt.end_x << L", Y: " << evt.end_y << L")"
            << L" [Jeda: " << std::fixed << std::setprecision(2) << evt.delay << L"s]";
    } else {
        wss << (isRight ? L"Drag Kanan" : L"Drag Kiri")
            << L" dari (" << evt.start_x << L", " << evt.start_y << L")"
            << L" ke (" << evt.end_x << L", " << evt.end_y << L")"
            << L" [Jeda: " << std::fixed << std::setprecision(2) << evt.delay << L"s]";
    }

    return wss.str();
}

void PopulateActionList() {
    if (!hLstActions) return;
    SendMessage(hLstActions, LB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < g_events.size(); ++i) {
        std::wstring text = FormatActionToHumanLanguage((int)i, g_events[i]);
        SendMessage(hLstActions, LB_ADDSTRING, 0, (LPARAM)text.c_str());
    }
}

void PopulateDragPointsList(const MacroEvent& evt) {
    if (!hLstDragPoints) return;
    SendMessage(hLstDragPoints, LB_RESETCONTENT, 0, 0);
    if (evt.type != ACTION_DRAG) return;

    for (size_t i = 0; i < evt.path.size(); ++i) {
        std::wstringstream wss;
        wss << L"Titik " << (i + 1) << L": (X: " << evt.path[i].x << L", Y: " << evt.path[i].y << L")"
            << L" [Jeda: " << evt.path[i].delay << L"s]";
        SendMessage(hLstDragPoints, LB_ADDSTRING, 0, (LPARAM)wss.str().c_str());
    }
}

void SetActionEditControlsState(bool enable) {
    if (hEditX) EnableWindow(hEditX, enable ? TRUE : FALSE);
    if (hEditY) EnableWindow(hEditY, enable ? TRUE : FALSE);
    if (hEditDelay) EnableWindow(hEditDelay, enable ? TRUE : FALSE);
    if (hBtnUpdateAct) EnableWindow(hBtnUpdateAct, enable ? TRUE : FALSE);
}

void SetActionListOperationButtonsState(bool enable, bool enablePickCoord = true) {
    if (hBtnMoveUp) EnableWindow(hBtnMoveUp, enable ? TRUE : FALSE);
    if (hBtnMoveDown) EnableWindow(hBtnMoveDown, enable ? TRUE : FALSE);
    if (hBtnDeleteAct) EnableWindow(hBtnDeleteAct, enable ? TRUE : FALSE);
    if (hBtnEditorPickCoord) EnableWindow(hBtnEditorPickCoord, (enable && enablePickCoord) ? TRUE : FALSE);
}

void ClearActionEditFields() {
    if (hEditX) SetWindowText(hEditX, L"");
    if (hEditY) SetWindowText(hEditY, L"");
    if (hEditDelay) SetWindowText(hEditDelay, L"");
}

void ResizeEditorDialog(HWND hwnd, bool expand) {
    RECT rc = { 0, 0, expand ? 764 : 412, 300 };
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRectEx(&rc, dwStyle, FALSE, 0);
    int newW = rc.right - rc.left;
    int newH = rc.bottom - rc.top;

    RECT winRc;
    GetWindowRect(hwnd, &winRc);
    SetWindowPos(hwnd, NULL, winRc.left, winRc.top, newW, newH, SWP_NOZORDER | SWP_NOACTIVATE);

    int showCmd = expand ? SW_SHOW : SW_HIDE;
    ShowWindow(hTitleDragDetail, showCmd);
    ShowWindow(hLstDragPoints, showCmd);
}

void UpdateEditorFieldsFromSelectedAction(HWND hDlg, int index) {
    if (index < 0 || index >= (int)g_events.size()) {
        ClearActionEditFields();
        if (hLstDragPoints) SendMessage(hLstDragPoints, LB_RESETCONTENT, 0, 0);
        ResizeEditorDialog(hDlg, false);
        SetActionListOperationButtonsState(false, false);
        return;
    }

    const MacroEvent& evt = g_events[index];
    bool isDragLeft = (evt.type == ACTION_DRAG && (evt.button == "left" || evt.button == "1"));

    if (isDragLeft) {
        ResizeEditorDialog(hDlg, true);
        PopulateDragPointsList(evt);
        SetActionEditControlsState(false);
        ClearActionEditFields();
        SetActionListOperationButtonsState(true, false);
    } else {
        ResizeEditorDialog(hDlg, false);
        if (hLstDragPoints) SendMessage(hLstDragPoints, LB_RESETCONTENT, 0, 0);

        SetActionEditControlsState(true);

        int posX = (evt.type == ACTION_DRAG) ? evt.start_x : evt.end_x;
        int posY = (evt.type == ACTION_DRAG) ? evt.start_y : evt.end_y;

        wchar_t buf[64];
        swprintf_s(buf, 64, L"%d", posX);
        SetWindowTextW(hEditX, buf);

        swprintf_s(buf, 64, L"%d", posY);
        SetWindowTextW(hEditY, buf);

        swprintf_s(buf, 64, L"%.2f", evt.delay);
        SetWindowTextW(hEditDelay, buf);

        SetActionListOperationButtonsState(true, true);
    }
}

void CommitEditorChanges() {
    if (!g_lastScriptFullPath.empty()) {
        SaveMacroToSpecificFile(g_lastScriptFullPath.c_str());
    } else {
        SaveMacro();
    }
    g_editorHasChanges = false;
    SetWindowText(hStatusLabel, (L"Status: Perubahan disimpan (" + std::to_wstring(g_events.size()) + L" aksi)").c_str());
}

void CancelEditorChanges() {
    g_events = g_eventsBackup;
    g_editorHasChanges = false;
}

LRESULT CALLBACK EditorDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        g_eventsBackup = g_events;
        g_editorHasChanges = false;
        g_isEditorPickingCoord = false;

        HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
        if (hIcon) {
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }

        HWND hTitle = CreateWindow(L"STATIC", L"Daftar Aksi Makro:", WS_VISIBLE | WS_CHILD, 12, 12, 350, 18, hwnd, NULL, NULL, NULL);
        hLstActions = CreateWindow(L"LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY, 12, 32, 388, 178, hwnd, (HMENU)ID_LST_ACTIONS, NULL, NULL);

        hTitleDragDetail = CreateWindow(L"STATIC", L"Detail Jalur Drag (Path):", WS_CHILD, 412, 12, 340, 18, hwnd, NULL, NULL, NULL);
        hLstDragPoints = CreateWindow(L"LISTBOX", NULL, WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY, 412, 32, 340, 260, hwnd, (HMENU)ID_LST_DRAG_POINTS, NULL, NULL);

        HWND hSubTitle = CreateWindow(L"STATIC", L"Edit Aksi Terpilih:", WS_VISIBLE | WS_CHILD, 12, 206, 180, 18, hwnd, NULL, NULL, NULL);
        
        HWND hLblX = CreateWindow(L"STATIC", L"Pos X:", WS_VISIBLE | WS_CHILD, 12, 230, 38, 20, hwnd, NULL, NULL, NULL);
        hEditX = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL, 50, 227, 44, 22, hwnd, (HMENU)ID_EDIT_X, NULL, NULL);

        HWND hLblY = CreateWindow(L"STATIC", L"Pos Y:", WS_VISIBLE | WS_CHILD, 102, 230, 38, 20, hwnd, NULL, NULL, NULL);
        hEditY = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL, 140, 227, 44, 22, hwnd, (HMENU)ID_EDIT_Y, NULL, NULL);

        HWND hLblDelay = CreateWindow(L"STATIC", L"Jeda(s):", WS_VISIBLE | WS_CHILD, 192, 230, 48, 20, hwnd, NULL, NULL, NULL);
        hEditDelay = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 240, 227, 62, 22, hwnd, (HMENU)ID_EDIT_DELAY, NULL, NULL);

        hBtnUpdateAct = CreateWindow(L"BUTTON", L"Update Aksi", WS_VISIBLE | WS_CHILD, 308, 225, 92, 26, hwnd, (HMENU)ID_BTN_UPDATE_ACT, NULL, NULL);
        
        hBtnMoveUp = CreateWindow(L"BUTTON", L"Naik ^", WS_VISIBLE | WS_CHILD, 12, 260, 58, 26, hwnd, (HMENU)ID_BTN_MOVE_UP, NULL, NULL);
        hBtnMoveDown = CreateWindow(L"BUTTON", L"Turun v", WS_VISIBLE | WS_CHILD, 74, 260, 58, 26, hwnd, (HMENU)ID_BTN_MOVE_DOWN, NULL, NULL);
        hBtnEditorPickCoord = CreateWindow(L"BUTTON", L"Ambil Koordinat", WS_VISIBLE | WS_CHILD, 136, 260, 96, 26, hwnd, (HMENU)ID_BTN_EDITOR_PICK_COORD, NULL, NULL);
        hBtnDeleteAct = CreateWindow(L"BUTTON", L"Hapus Aksi", WS_VISIBLE | WS_CHILD, 236, 260, 78, 26, hwnd, (HMENU)ID_BTN_DELETE_ACT, NULL, NULL);
        
        HWND btnSaveClose = CreateWindow(L"BUTTON", L"Simpan", WS_VISIBLE | WS_CHILD, 318, 260, 82, 26, hwnd, (HMENU)ID_BTN_SAVE_CLOSE, NULL, NULL);

        HWND controls[] = { hTitle, hTitleDragDetail, hSubTitle, hLblX, hLblY, hLblDelay, hLstActions, hLstDragPoints, hEditX, hEditY, hEditDelay, hBtnUpdateAct, hBtnMoveUp, hBtnMoveDown, hBtnEditorPickCoord, hBtnDeleteAct, btnSaveClose };
        for (HWND hCtrl : controls) {
            SendMessage(hCtrl, WM_SETFONT, (WPARAM)g_hFontSegoeUI, TRUE);
        }

        PopulateActionList();
        SetActionListOperationButtonsState(false, false);
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        if (wmId == ID_LST_ACTIONS && wmEvent == LBN_SELCHANGE) {
            int selIdx = (int)SendMessage(hLstActions, LB_GETCURSEL, 0, 0);
            UpdateEditorFieldsFromSelectedAction(hwnd, selIdx);
        }
        else if (wmId == ID_LST_DRAG_POINTS && wmEvent == LBN_SELCHANGE) {
            int actionIdx = (int)SendMessage(hLstActions, LB_GETCURSEL, 0, 0);
            int pointIdx = (int)SendMessage(hLstDragPoints, LB_GETCURSEL, 0, 0);

            if (actionIdx >= 0 && actionIdx < (int)g_events.size()) {
                const MacroEvent& evt = g_events[actionIdx];
                if (pointIdx >= 0 && pointIdx < (int)evt.path.size()) {
                    const DragPoint& pt = evt.path[pointIdx];

                    SetActionEditControlsState(true);

                    wchar_t buf[64];
                    swprintf_s(buf, 64, L"%d", pt.x);
                    SetWindowTextW(hEditX, buf);

                    swprintf_s(buf, 64, L"%d", pt.y);
                    SetWindowTextW(hEditY, buf);

                    swprintf_s(buf, 64, L"%.2f", pt.delay);
                    SetWindowTextW(hEditDelay, buf);

                    if (hBtnEditorPickCoord) EnableWindow(hBtnEditorPickCoord, TRUE);
                }
            }
        }
        else if (wmId == ID_BTN_EDITOR_PICK_COORD) {
            if (!g_isEditorPickingCoord) {
                g_isEditorPickingCoord = true;
                EnableGlobalCrosshair(true);
                SetWindowTextW(hBtnEditorPickCoord, L"Batal");
                if (!mp_mouseHook) {
                    mp_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);
                }
            } else {
                g_isEditorPickingCoord = false;
                if (mp_mouseHook) { UnhookWindowsHookEx(mp_mouseHook); mp_mouseHook = NULL; }
                EnableGlobalCrosshair(false);
                SetWindowTextW(hBtnEditorPickCoord, L"Ambil Koordinat");
            }
        }
        else if (wmId == ID_BTN_SAVE_CLOSE) {
            CommitEditorChanges();
            DestroyWindow(hwnd);
        }
        break;
    }
    case WM_CLOSE: {
        if (g_isEditorPickingCoord) {
            g_isEditorPickingCoord = false;
            if (mp_mouseHook) { UnhookWindowsHookEx(mp_mouseHook); mp_mouseHook = NULL; }
            EnableGlobalCrosshair(false);
        }
        if (g_editorHasChanges) {
            int res = MessageBox(hwnd, L"Apakah Anda ingin menyimpan perubahan editan script ini?", L"Konfirmasi Simpan Edit", MB_YESNOCANCEL | MB_ICONQUESTION);
            if (res == IDYES) CommitEditorChanges();
            else if (res == IDNO) CancelEditorChanges();
            else if (res == IDCANCEL) return 0;
        } else {
            CancelEditorChanges();
        }
        DestroyWindow(hwnd);
        break;
    }
    case WM_DESTROY: {
        if (g_isEditorPickingCoord) {
            g_isEditorPickingCoord = false;
            if (mp_mouseHook) { UnhookWindowsHookEx(mp_mouseHook); mp_mouseHook = NULL; }
            EnableGlobalCrosshair(false);
        }
        hLstActions = NULL;
        hLstDragPoints = NULL;
        hEditX = NULL;
        hEditY = NULL;
        hEditDelay = NULL;
        hBtnUpdateAct = NULL;
        hBtnMoveUp = NULL;
        hBtnMoveDown = NULL;
        hBtnDeleteAct = NULL;
        hBtnEditorPickCoord = NULL;
        EnableWindow(hMainWnd, TRUE);
        SetForegroundWindow(hMainWnd);
        break;
    }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void OpenScriptEditor(HWND hParent) {
    if (g_events.empty()) {
        MessageBox(hParent, L"Belum ada makro yang terekam/dimuat untuk diedit!", L"Peringatan", MB_OK | MB_ICONWARNING);
        return;
    }

    const wchar_t DLG_CLASS[] = L"ScriptEditorDlgClass";
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = EditorDlgProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = DLG_CLASS;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

    RegisterClass(&wc);
    EnableWindow(hParent, FALSE);

    RECT rc = { 0, 0, 412, 300 };
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRectEx(&rc, dwStyle, FALSE, 0);

    HWND hDlg = CreateWindowEx(
        0, DLG_CLASS, L"Editor Script Makro",
        dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
        hParent, NULL, g_hInstance, NULL
    );

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    SetWindowPos(hDlg, NULL, (screenW - (rc.right - rc.left)) / 2, (screenH - (rc.bottom - rc.top)) / 2, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_SHOWWINDOW);
}

void ResizeMainWindow(bool expand) {
    g_isPickerExpanded = expand;
    int clientH = expand ? 558 : 298; 
    RECT rc = { 0, 0, 280, clientH };
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&rc, dwStyle, FALSE, 0);

    RECT windowRect;
    GetWindowRect(hMainWnd, &windowRect);
    SetWindowPos(hMainWnd, NULL, windowRect.left, windowRect.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);

    int showCmd = expand ? SW_SHOW : SW_HIDE;
    ShowWindow(mp_hTextX, showCmd);
    ShowWindow(mp_hTextY, showCmd);
    ShowWindow(mp_hHexColor, showCmd);
    ShowWindow(mp_hStatus, showCmd);
    ShowWindow(mp_hConfigStatus, showCmd);
    ShowWindow(mp_hBtnStart, showCmd);
    ShowWindow(mp_hBtnSaveConfig, showCmd);
    ShowWindow(mp_hBtnLoadConfig, showCmd);
    ShowWindow(mp_hBtnPickCoord, showCmd);
    ShowWindow(mp_hBtnPickColor, showCmd);
    ShowWindow(mp_hBtnPickBoth, showCmd);
    ShowWindow(mp_hRadioStop, showCmd);
    ShowWindow(mp_hRadioPause, showCmd);
    ShowWindow(mp_hGroupTrigger, showCmd);
    ShowWindow(mp_hLblTarget, showCmd);
    ShowWindow(mp_hChkLock, showCmd);

    HWND hChild = GetWindow(hMainWnd, GW_CHILD);
    while (hChild) {
        char className[64];
        GetClassNameA(hChild, className, 64);
        if (_stricmp(className, "STATIC") == 0) {
            RECT rcChild;
            GetWindowRect(hChild, &rcChild);
            POINT pt = { rcChild.left, rcChild.top };
            ScreenToClient(hMainWnd, &pt);
            if (pt.y >= 285) {
                ShowWindow(hChild, showCmd);
            }
        }
        hChild = GetWindow(hChild, GW_HWNDNEXT);
    }

    SetWindowText(hBtnTogglePicker, expand ? L"Tutup Mouse Picker" : L"Mouse Picker");
    SaveConfig();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        hMainWnd = hwnd;
        InitIniPath();

        RegisterHotKey(hwnd, HOTKEY_F8, 0, VK_F8);
        RegisterHotKey(hwnd, HOTKEY_F9, 0, VK_F9);
        RegisterHotKey(hwnd, HOTKEY_F10, 0, VK_F10);

        g_hFontSegoeUI = CreateFont(
            -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
        );

        hStatusLabel = CreateWindow(L"STATIC", L"Status: Idle (F8: REC, F9: PLAY)", WS_VISIBLE | WS_CHILD | SS_CENTER, 12, 12, 256, 18, hwnd, NULL, NULL, NULL);
        hScriptLabel = CreateWindow(L"STATIC", L"Script: None", WS_VISIBLE | WS_CHILD | SS_CENTER, 12, 32, 256, 18, hwnd, NULL, NULL, NULL);

        hBtnRec = CreateWindow(L"BUTTON", L"REC (F8)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 12, 54, 124, 30, hwnd, (HMENU)ID_BTN_REC, NULL, NULL);
        hBtnPlay = CreateWindow(L"BUTTON", L"PLAY (F9)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 144, 54, 124, 30, hwnd, (HMENU)ID_BTN_PLAY, NULL, NULL);
        
        hBtnPause = CreateWindow(L"BUTTON", L"PAUSE (F10)", WS_CHILD | BS_PUSHBUTTON, 184, 54, 84, 30, hwnd, (HMENU)ID_BTN_PAUSE, NULL, NULL);

        hGroupLoop = CreateWindow(L"BUTTON", L"Pengaturan Loop dan Speed", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 12, 90, 256, 75, hwnd, NULL, NULL, NULL);
        
        hLblLoopStatic = CreateWindow(L"STATIC", L"Loop:", WS_VISIBLE | WS_CHILD, 22, 110, 38, 20, hwnd, NULL, NULL, NULL);
        hEditLoop = CreateWindow(L"EDIT", L"1", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 62, 108, 38, 20, hwnd, (HMENU)ID_EDIT_LOOP, NULL, NULL);
        hChkInfinite = CreateWindow(L"BUTTON", L"Tanpa Henti", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 112, 108, 145, 20, hwnd, (HMENU)ID_CHK_INFINITE, NULL, NULL);

        hLblSpeedStatic = CreateWindow(L"STATIC", L"Speed:", WS_VISIBLE | WS_CHILD, 22, 137, 42, 20, hwnd, NULL, NULL, NULL);
        hCmbSpeed = CreateWindow(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 68, 135, 189, 120, hwnd, (HMENU)ID_CMB_SPEED, NULL, NULL);

        for (int i = 5; i <= 15; ++i) {
            double val = i / 10.0;
            std::wstringstream wss;
            wss << std::fixed << std::setprecision(1) << val << L"x";
            SendMessage(hCmbSpeed, CB_ADDSTRING, 0, (LPARAM)wss.str().c_str());
        }

        hChkAlwaysTop = CreateWindow(L"BUTTON", L"Always on Top", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 12, 172, 160, 20, hwnd, (HMENU)ID_CHK_ALWAYS_TOP, NULL, NULL);

        // Posisi Tombol Save Macro di kiri (12) dan Load Macro di kanan (144)
        hBtnSave = CreateWindow(L"BUTTON", L"Save Macro", WS_VISIBLE | WS_CHILD, 12, 196, 124, 26, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
        hBtnLoad = CreateWindow(L"BUTTON", L"Load Macro", WS_VISIBLE | WS_CHILD, 144, 196, 124, 26, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
        
        hBtnEdit = CreateWindow(L"BUTTON", L"Edit Script", WS_VISIBLE | WS_CHILD, 12, 226, 256, 28, hwnd, (HMENU)ID_BTN_EDIT, NULL, NULL);
        hBtnTogglePicker = CreateWindow(L"BUTTON", L"Mouse Picker", WS_VISIBLE | WS_CHILD, 12, 258, 256, 28, hwnd, (HMENU)ID_BTN_TOGGLE_PICKER, NULL, NULL);

        // --- Kontrol Mouse Picker ---
        CreateWindowW(L"STATIC", L"X:", WS_CHILD, 12, 294, 16, 20, hwnd, NULL, NULL, NULL);
        mp_hTextX = CreateWindowW(L"EDIT", L"100", WS_CHILD | WS_BORDER | ES_NUMBER, 30, 292, 48, 23, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Y:", WS_CHILD, 86, 294, 16, 20, hwnd, NULL, NULL, NULL);
        mp_hTextY = CreateWindowW(L"EDIT", L"100", WS_CHILD | WS_BORDER | ES_NUMBER, 104, 292, 48, 23, hwnd, NULL, NULL, NULL);
        mp_hBtnPickCoord = CreateWindowW(L"BUTTON", L"Pick Koordinat", WS_CHILD, 160, 291, 108, 26, hwnd, (HMENU)ID_MP_BTN_COORD, NULL, NULL);

        mp_hLblTarget = CreateWindowW(L"STATIC", L"Target HEX:", WS_CHILD, 12, 323, 74, 20, hwnd, NULL, NULL, NULL);
        mp_hHexColor = CreateWindowW(L"EDIT", L"FFA500", WS_CHILD | WS_BORDER, 90, 321, 62, 23, hwnd, NULL, NULL, NULL);
        mp_hBtnPickColor = CreateWindowW(L"BUTTON", L"Pick Warna", WS_CHILD, 160, 320, 108, 26, hwnd, (HMENU)ID_MP_BTN_COLOR, NULL, NULL);

        mp_hBtnPickBoth = CreateWindowW(L"BUTTON", L"Pick Keduanya (Koordinat dan Warna)", WS_CHILD, 12, 350, 256, 27, hwnd, (HMENU)ID_MP_BTN_BOTH, NULL, NULL);

        mp_hChkLock = CreateWindowW(L"BUTTON", L"Kunci Koordinat && Warna", WS_CHILD | BS_AUTOCHECKBOX, 12, 381, 256, 20, hwnd, (HMENU)ID_MP_CHK_LOCK, NULL, NULL);

        mp_hGroupTrigger = CreateWindowW(L"BUTTON", L"Choose to trigger", WS_CHILD | BS_GROUPBOX, 12, 404, 256, 43, hwnd, NULL, NULL, NULL);
        mp_hRadioStop = CreateWindowW(L"BUTTON", L"Stop", WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 22, 420, 60, 20, hwnd, (HMENU)ID_MP_RADIO_STOP, NULL, NULL);
        mp_hRadioPause = CreateWindowW(L"BUTTON", L"Pause", WS_CHILD | BS_AUTORADIOBUTTON, 90, 420, 70, 20, hwnd, (HMENU)ID_MP_RADIO_PAUSE, NULL, NULL);

        mp_hBtnStart = CreateWindowW(L"BUTTON", L"Mulai Deteksi", WS_CHILD | BS_DEFPUSHBUTTON, 12, 451, 256, 27, hwnd, (HMENU)ID_MP_BTN_START, NULL, NULL);
        
        mp_hBtnSaveConfig = CreateWindowW(L"BUTTON", L"Save Config", WS_CHILD, 12, 482, 124, 27, hwnd, (HMENU)ID_MP_BTN_SAVE_CFG, NULL, NULL);
        mp_hBtnLoadConfig = CreateWindowW(L"BUTTON", L"Load Config", WS_CHILD, 144, 482, 124, 27, hwnd, (HMENU)ID_MP_BTN_LOAD_CFG, NULL, NULL);

        mp_hConfigStatus = CreateWindowW(L"STATIC", L"Config: picker.ini", WS_CHILD, 12, 513, 256, 18, hwnd, NULL, NULL, NULL);
        mp_hStatus = CreateWindowW(L"STATIC", L"Status: Standby", WS_CHILD, 12, 533, 256, 18, hwnd, NULL, NULL, NULL);

        HWND controls[] = { hStatusLabel, hScriptLabel, hBtnRec, hBtnPlay, hBtnPause, hGroupLoop, hLblLoopStatic, hEditLoop, hChkInfinite, hLblSpeedStatic, hCmbSpeed, hChkAlwaysTop, hBtnLoad, hBtnSave, hBtnEdit, hBtnTogglePicker };
        for (HWND hCtrl : controls) {
            SendMessage(hCtrl, WM_SETFONT, (WPARAM)g_hFontSegoeUI, TRUE);
        }

        HWND pickerControls[] = { mp_hTextX, mp_hTextY, mp_hHexColor, mp_hStatus, mp_hConfigStatus, mp_hBtnStart, mp_hBtnSaveConfig, mp_hBtnLoadConfig, mp_hBtnPickCoord, mp_hBtnPickColor, mp_hBtnPickBoth, mp_hRadioStop, mp_hRadioPause, mp_hGroupTrigger, mp_hLblTarget, mp_hChkLock };
        for (HWND hCtrl : pickerControls) {
            SendMessage(hCtrl, WM_SETFONT, (WPARAM)g_hFontSegoeUI, TRUE);
        }

        HWND hChild = GetWindow(hwnd, GW_CHILD);
        while (hChild) {
            char className[64];
            GetClassNameA(hChild, className, 64);
            if (_stricmp(className, "STATIC") == 0) {
                SendMessage(hChild, WM_SETFONT, (WPARAM)g_hFontSegoeUI, TRUE);
            }
            hChild = GetWindow(hChild, GW_HWNDNEXT);
        }

        LoadConfig();
        if (g_isPickerExpanded) {
            ResizeMainWindow(true);
        } else {
            ResizeMainWindow(false);
        }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    }
    case WM_EXITSIZEMOVE: {
        SaveConfig();
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        if (wmId == ID_BTN_REC) {
            if (g_isRecording) StopRecording();
            else StartRecording();
        }
        else if (wmId == ID_BTN_PLAY) {
            if (g_isPlaying) StopPlaying();
            else PlayMacro();
        }
        else if (wmId == ID_BTN_PAUSE) {
            PausePlaying();
        }
        else if (wmId == ID_BTN_TOGGLE_PICKER) {
            ResizeMainWindow(!g_isPickerExpanded);
        }
        else if (wmId == ID_CHK_INFINITE) {
            BOOL checked = (SendMessage(hChkInfinite, BM_GETCHECK, 0, 0) == BST_CHECKED);
            EnableWindow(hEditLoop, !checked);
            SaveConfig();
        }
        else if (wmId == ID_CMB_SPEED && wmEvent == CBN_SELCHANGE) {
            SaveConfig();
        }
        else if (wmId == ID_CHK_ALWAYS_TOP) {
            BOOL checked = (SendMessage(hChkAlwaysTop, BM_GETCHECK, 0, 0) == BST_CHECKED);
            SetAlwaysOnTop(checked ? true : false);
            SaveConfig();
        }
        else if (wmId == ID_MP_CHK_LOCK) {
            BOOL locked = (SendMessage(mp_hChkLock, BM_GETCHECK, 0, 0) == BST_CHECKED);
            SetPickerControlsLock(locked ? true : false);
            SavePickerConfig();
        }
        else if (wmId == ID_BTN_SAVE) SaveMacro();
        else if (wmId == ID_BTN_LOAD) LoadMacro();
        else if (wmId == ID_BTN_EDIT) OpenScriptEditor(hwnd);
        
        else if (wmId == ID_MP_BTN_START) {
            if (!mp_isRunning) {
                wchar_t bufX[16], bufY[16];
                GetWindowTextW(mp_hTextX, bufX, 16);
                GetWindowTextW(mp_hTextY, bufY, 16);
                mp_targetX = _wtoi(bufX);
                mp_targetY = _wtoi(bufY);
                SavePickerConfig();
                mp_isRunning = true;
                SetWindowTextW(mp_hBtnStart, L"Berhentikan Deteksi");
                mp_hThread = CreateThread(NULL, 0, DetectionThread, NULL, 0, NULL);
            } else {
                mp_isRunning = false;
                SetWindowTextW(mp_hBtnStart, L"Mulai Deteksi");
                if (mp_hThread) {
                    WaitForSingleObject(mp_hThread, 1000);
                    CloseHandle(mp_hThread);
                    mp_hThread = NULL;
                }
            }
        }
        else if (wmId == ID_MP_BTN_COORD) {
            if (!mp_isPickingCoord) {
                mp_isPickingCoord = true;
                mp_isPickingColor = false;
                mp_isPickingBoth = false;
                EnableGlobalCrosshair(true);
                SetWindowTextW(mp_hStatus, L"Status: Klik pada titik koordinat layar...");
                SetWindowTextW(mp_hBtnPickCoord, L"Batal");
                if (!mp_mouseHook) {
                    mp_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);
                }
            } else {
                mp_isPickingCoord = false;
                if (mp_mouseHook) { UnhookWindowsHookEx(mp_mouseHook); mp_mouseHook = NULL; }
                EnableGlobalCrosshair(false);
                SetWindowTextW(mp_hStatus, L"Status: Dibatalkan.");
                SetWindowTextW(mp_hBtnPickCoord, L"Pick Koordinat");
            }
        }
        else if (wmId == ID_MP_BTN_COLOR) {
            if (!mp_isPickingColor) {
                mp_isPickingColor = true;
                mp_isPickingCoord = false;
                mp_isPickingBoth = false;
                EnableGlobalCrosshair(true);
                SetWindowTextW(mp_hStatus, L"Status: Klik pada warna di layar...");
                SetWindowTextW(mp_hBtnPickColor, L"Batal");
                if (!mp_mouseHook) {
                    mp_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);
                }
            } else {
                mp_isPickingColor = false;
                if (mp_mouseHook) { UnhookWindowsHookEx(mp_mouseHook); mp_mouseHook = NULL; }
                EnableGlobalCrosshair(false);
                SetWindowTextW(mp_hStatus, L"Status: Dibatalkan.");
                SetWindowTextW(mp_hBtnPickColor, L"Pick Warna");
            }
        }
        else if (wmId == ID_MP_BTN_BOTH) {
            if (!mp_isPickingBoth) {
                mp_isPickingBoth = true;
                mp_isPickingCoord = false;
                mp_isPickingColor = false;
                EnableGlobalCrosshair(true);
                SetWindowTextW(mp_hStatus, L"Status: Klik pada target untuk pick koordinat dan warna...");
                SetWindowTextW(mp_hBtnPickBoth, L"Batal Pick");
                if (!mp_mouseHook) {
                    mp_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);
                }
            } else {
                mp_isPickingBoth = false;
                if (mp_mouseHook) { UnhookWindowsHookEx(mp_mouseHook); mp_mouseHook = NULL; }
                EnableGlobalCrosshair(false);
                SetWindowTextW(mp_hStatus, L"Status: Dibatalkan.");
                SetWindowTextW(mp_hBtnPickBoth, L"Pick Keduanya (Koordinat dan Warna)");
            }
        }
        else if (wmId == ID_MP_BTN_SAVE_CFG) {
            wchar_t szFile[MAX_PATH] = { 0 };
            OPENFILENAMEW ofn = { 0 };
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = L"Configuration Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = L"Simpan File Konfigurasi Picker (.ini)";
            ofn.lpstrDefExt = L"ini";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileNameW(&ofn) == TRUE) {
                wcscpy_s(mp_iniFilePath, MAX_PATH, szFile);
                SavePickerConfig();
                LoadPickerConfig();
                SetWindowTextW(mp_hStatus, L"Status: Konfigurasi Picker berhasil disimpan!");
            }
        }
        else if (wmId == ID_MP_BTN_LOAD_CFG) {
            wchar_t szFile[MAX_PATH] = { 0 };
            OPENFILENAMEW ofn = { 0 };
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = L"Configuration Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = L"Pilih File Backup Konfigurasi (.ini)";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameW(&ofn) == TRUE) {
                wcscpy_s(mp_iniFilePath, MAX_PATH, szFile);
                LoadPickerConfig();
                SetWindowTextW(mp_hStatus, L"Status: Berhasil memuat config backup!");
            }
        }
        else if (wmId == ID_MP_RADIO_STOP) {
            mp_triggerAction = 0;
            SavePickerConfig();
        }
        else if (wmId == ID_MP_RADIO_PAUSE) {
            mp_triggerAction = 1;
            SavePickerConfig();
        }
        break;
    }
    case WM_HOTKEY: {
        if (wParam == HOTKEY_F8) {
            if (g_isRecording) StopRecording();
            else if (!g_isPlaying) StartRecording();
        }
        else if (wParam == HOTKEY_F9) {
            if (g_isPlaying) StopPlaying();
            else if (!g_isRecording) PlayMacro();
        }
        else if (wParam == HOTKEY_F10) {
            if (g_isPlaying) {
                PausePlaying();
            }
        }
        break;
    }
    case WM_CLOSE: {
        if (!g_events.empty() && g_hasUnsavedChanges) {
            int res = MessageBox(hwnd, L"Apakah Anda ingin menyimpan rekaman makro saat ini sebelum keluar?", L"Konfirmasi Simpan", MB_YESNOCANCEL | MB_ICONQUESTION);
            if (res == IDYES) {
                if (!SaveMacro()) return 0;
            } else if (res == IDCANCEL) return 0;
        }
        SaveConfig();
        DestroyWindow(hwnd);
        break;
    }
    case WM_DESTROY:
        mp_isRunning = false;
        UnregisterHotKey(hwnd, HOTKEY_F8);
        UnregisterHotKey(hwnd, HOTKEY_F9);
        UnregisterHotKey(hwnd, HOTKEY_F10);
        if (g_hFontSegoeUI) DeleteObject(g_hFontSegoeUI);
        if (mp_mouseHook) UnhookWindowsHookEx(mp_mouseHook);
        EnableGlobalCrosshair(false);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void CenterWindow(HWND hwnd, int width, int height) {
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - width) / 2;
    int y = (screenH - height) / 2;
    SetWindowPos(hwnd, NULL, x, y, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInstance = hInstance;

    HANDLE hMutex = CreateMutex(NULL, TRUE, L"LiteMacro_SingleInstance_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, L"Aplikasi Lite Macro sudah berjalan!", L"Peringatan", MB_OK | MB_ICONWARNING);
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }

    const wchar_t CLASS_NAME[] = L"LiteMacroClass";
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = hIcon;

    RegisterClass(&wc);

    InitIniPath();
    bool expandedInit = (GetPrivateProfileInt(L"Settings", L"PickerExpanded", 0, g_iniPath) == 1);
    int initClientH = expandedInit ? 558 : 298;

    RECT rc = { 0, 0, 280, initClientH };
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&rc, dwStyle, FALSE, 0);

    int winW = rc.right - rc.left;
    int winH = rc.bottom - rc.top;

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Lite Macro",
        dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT, winW, winH,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        if (hMutex) ReleaseMutex(hMutex);
        return 0;
    }

    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

    int savedX = GetPrivateProfileInt(L"Settings", L"WindowX", -9999, g_iniPath);
    int savedY = GetPrivateProfileInt(L"Settings", L"WindowY", -9999, g_iniPath);

    if (savedX != -9999 && savedY != -9999) {
        SetWindowPos(hwnd, NULL, savedX, savedY, winW, winH, SWP_NOZORDER | SWP_SHOWWINDOW);
    } else {
        CenterWindow(hwnd, winW, winH);
    }

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    return 0;
}