#pragma once

#include "CapsHotkeyApp.hpp"

class MainWindow : public CDialogImpl<MainWindow>, public CMessageFilter
{
public:
    enum
    {
        IDD = IDD_DIALOG1
    };

    BEGIN_MSG_MAP(MainWindow)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_COMMAND, OnCommand)
    MESSAGE_HANDLER(WM_CTLCOLORDLG, OnBackgroundColor)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnBackgroundColor)
    MESSAGE_HANDLER(WM_USER + 1, OnUserCmd)
    END_MSG_MAP()

    // BEGIN_UPDATE_UI_MAP(MainWindow)
    // END_UPDATE_UI_MAP()

public:
    MainWindow(HICON icon)
        : icon_(icon)
    {
    }

    ~MainWindow()
    {
    }

protected:
    BOOL PreTranslateMessage(MSG *pMsg) override
    {
        return ::IsDialogMessage(m_hWnd, pMsg);
    }

    int OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
    {
        app_ = std::make_unique<CapsHotkeyApp>(icon_, m_hWnd);

        SetIcon(icon_, FALSE);
        CenterWindow();

        SetDlgItemText(IDC_STATIC, app_->name().c_str());
        auto listvc = (CListViewCtrl)GetDlgItem(IDC_LIST1);
        listvc.SetExtendedListViewStyle(LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT);
        listvc.InsertColumn(0, _T("Key"), LVCFMT_LEFT | TVIS_BOLD, 100, 0);
        listvc.InsertColumn(1, _T("Description"), LVCFMT_LEFT, 400, 1);

        auto row = 0;
        for (auto &&[k, it] : app_->hooks())
        {
            listvc.AddItem(row, 0, std::format(L"[Capslock] + {}", str::wide(KeyCodes::key2str(k))).c_str());
            listvc.AddItem(row, 1, str::wide(it.desc).c_str());
            row++;
        }
        auto loop = wtl::Loop();
        loop->AddMessageFilter(this);
        // loop->AddIdleHandler(this);

        return 0;
    }

    int OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    int OnCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL & /*bHandled*/)
    {
        if (!app_)
            return 0;

        switch (MenuAction(LOWORD(wParam)))
        {
        case MenuAction::Exit:
            app_->quit_current_app();
            break;
        case MenuAction::Help:
            app_->show_main_window();
            break;
        case MenuAction::AutoRun:
            app_->toggle_autorun_enabled();
            break;
        }

        return 0;
    }

    int OnBackgroundColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL & /*bHandled*/)
    {
        return (INT_PTR)GetStockObject(WHITE_BRUSH);
    }

    int OnUserCmd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL & /*bHandled*/)
    {
        if (lParam == WM_RBUTTONUP)
        {
            app_->show_context_menu();
        }
        return 0;
    }

private:
    HICON icon_;
    std::unique_ptr<CapsHotkeyApp> app_{ nullptr };
};
