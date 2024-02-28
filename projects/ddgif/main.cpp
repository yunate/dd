#include "ddgif/stdafx.h"
#if 0
#include "DogGif/DogGif.h"
#include "file_utils_writer.h"
#include "file_utils_reader.h"
#include "simple_wnd/App.h"
#include "simple_wnd/SimpleWnd.h"
#include <time.h>
using namespace DogGifNSP;
POINT g_mouse_point;

RECT g_wnd_rect;

bool g_is_move = false;
class MyWnd :
    public SimpleWnd
{
public:
    MyWnd()
    {
        timer = false;
        needDrawGif = false;
        m_timeDelay = 0;
    }

    ~MyWnd()
    {
        for (auto it : m_gifVec)
        {
            delete it;
        }

        m_gifVec.clear();
    }


    /** 设置window的位置
    @param [in] rect 窗口位置
    */
    inline void SetRect(const RECT& rect)
    {
        SimpleWnd::SetX(rect.left);
        SimpleWnd::SetY(rect.top);
        SimpleWnd::SetWidth(rect.right - rect.left);
        SimpleWnd::SetHeight(rect.bottom - rect.top);
    }

    // 事件处理函数，函数_WndProc间接调用
    virtual LRESULT _OnEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        bool isHandle = false;
        LRESULT result = 0;

        switch (msg)
        {
        case WM_LBUTTONDOWN:
            {
                if (!g_is_move)
                {
                    ::SetCapture(GetWnd());
                    g_is_move = true;
                    ::GetCursorPos(&g_mouse_point);
                    ::GetWindowRect(GetWnd(), &g_wnd_rect);
                    HWND wnd = ::FindWindow(L"maze", L"maze");

                    if (wnd != NULL)
                    {
                        ::PostMessage(wnd, 0xffff, 0, 0);
                    }
                }

                isHandle = true;
                break;
            }
        case WM_LBUTTONUP:
            {
                ::ReleaseCapture();
                g_is_move = false;
                POINT pt;
                isHandle = true;
                HWND wnd = ::FindWindow(L"maze", L"maze");

                if (wnd != NULL)
                {
                    ::PostMessage(wnd, 0xfffd, 0, 0);
                }
                break;
            }
        case WM_MOUSEMOVE:
            {
                if (g_is_move)
                {
                    POINT point;
                    ::GetCursorPos(&point);
                    int delX = (point.x - g_mouse_point.x);
                    int delY = (point.y - g_mouse_point.y);;

                    if (delY != 0 || delX != 0)
                    {
                        RECT rect(g_wnd_rect);
                        rect.left += delX;
                        rect.top += delY;
                        rect.right += delX;
                        rect.bottom += delY;
                        ::MoveWindow(GetWnd(), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
                        HWND wnd = ::FindWindow(L"maze", L"maze");

                        if (wnd != NULL)
                        {
                            ::PostMessage(wnd, 0xfffe, 0, 0);
                        }
                    }
                }
                break;
            }
        case WM_PAINT:
            {
                if (::clock() - m_n64StartTime >= m_timeDelay)
                {
                    m_n64StartTime = ::clock();
                    needDrawGif = true;
                }

                PAINTSTRUCT ps;
                HDC hdc = ::BeginPaint(hWnd, &ps);
                int left = 0;
                int top = 0;

                for (int i = 0; i < m_gifVec.size(); ++i)
                {
                    if (m_gifVec[i]->HasInit())
                    {
                        DogGifColor* pBuffData;
                        u32 buffLen;

                        if (!needDrawGif)
                        {
                            m_gifVec[i]->GetCurrentFrame(&pBuffData, buffLen);
                        }
                        else
                        {
                            m_gifVec[i]->GetNextFrame(&pBuffData, buffLen);
                        }

                        HBITMAP bitMap = ::CreateBitmap((int)m_gifVec[i]->GetGolWidth(),
                            (int)m_gifVec[i]->GetGolHeight(),
                                                        1,
                                                        32,
                                                        pBuffData);

                        if (bitMap)
                        {
                            HBRUSH brush = ::CreatePatternBrush(bitMap);
                            RECT rect = {left, top, left + (int)m_gifVec[i]->GetGolWidth(), top + (int)m_gifVec[i]->GetGolHeight()};

                            if (left + (int)m_gifVec[i]->GetGolWidth() + (int)m_gifVec[i]->GetGolWidth() < GetWidth())
                            {
                                left += (int)m_gifVec[i]->GetGolWidth();
                            }
                            else
                            {
                                left = 0;
                                top += (int)m_gifVec[i]->GetGolHeight();
                            }

                            ::FillRect(hdc, &rect, brush);
                            ::DeleteObject(brush);
                            ::DeleteObject(bitMap);
                        }
                    }
                }

                ::EndPaint(hWnd, &ps);
                isHandle = true;
                needDrawGif = false;
                break;
            }
        case WM_INITDIALOG:
            {
                for (int i = 0; i < 1; ++i)
                {
                    m_gifVec.push_back(new DogGifNSP::DogGif);
                    FileReader reader(L"test\\1.gif", NULL, 0);

                    if (reader.HasError())
                    {
                        continue;
                    }

                    size_t fileSize = reader.GetFileSize();

                    if (fileSize > 0)
                    {
                        u8* pBuff = new u8[fileSize];
                        reader.GetBuff((char*)pBuff, (u32)fileSize);
                        m_gifVec[i]->Init(pBuff, (u32)fileSize);
                        delete[] pBuff;
                    }
                }
               
                m_timeDelay = m_gifVec[0]->GetCurTimeDelay();

                if (!timer)
                {
                    timer = true;
                    ::SetTimer(GetWnd(), 0xffee, 40, NULL);
                }

                break;
            }
        case WM_TIMER:
            switch (wParam)
            {
            case 0xffee:
                {
                    needDrawGif = true;
                    RECT backRect = {0, 0, GetWidth(), GetHeight()};
                    ::InvalidateRect(GetWnd(), &backRect, FALSE);
                    isHandle = true;
                }
            }
        }

        if (!isHandle)
        {
            return SimpleWnd::_OnEvent(hWnd, msg, wParam, lParam);
        }

        return result;
    }

protected:
    DogGif m_gif;
    std::vector<DogGif*> m_gifVec;
    bool timer;
    bool needDrawGif;
    __int64 m_n64StartTime;
    int m_timeDelay;

};

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int       nCmdShow)
{
    App* pApp = &GetAppIns();
    pApp->SetIns(hInstance);
    pApp->RegistAllWnd();

    // 主逻辑
    {
        MyWnd myWnd;
        myWnd.SetRect({ 500, 200, 1500, 900 });
        myWnd.DoModel();
    }

    pApp->UnRegistAllWnd();
    return 0;
}

int main___()
{
    FileReader reader(L"D:\\workspaces\\C++_workspaces\\MyLibrary\\projects\\DogGif\\test\\test.gif", NULL, 0);
    size_t fileSize = reader.GetFileSize();
    u8* pBuff = new u8[fileSize];
    reader.GetBuff((char*)pBuff, fileSize);
    DogGif gif;
    gif.Init(pBuff, (u32)fileSize);
    DogGifColor* pbuff;
    u32 buffLen;
    gif.GetNextFrame(&pbuff, buffLen);

    FileWriter writer(L"D:\\workspaces\\C++_workspaces\\MyLibrary\\projects\\DogGif\\test\\1.bmp", NULL, 0);
    writer.WriteBuffA((const char*)pbuff, buffLen);
    return 1;
}
#endif