#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "UI/NewUI/NewUIManager.h"
#include "UI/NewUI/Inventory/NewUIMyInventory.h"
#include "UI/NewUI/Widgets/NewUIButton.h"
#include "UI/NewUI/Widgets/NewUIScrollBar.h"

namespace SEASON3B
{
    /// <summary>
    /// Shows the players which are currently online on the same game server, together
    /// with their level, class and the map they are on. The list is requested from the
    /// server when the window is opened and refreshed periodically while it's visible.
    /// </summary>
    class CNewUIServerPlayerListWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_SERVER_PLAYER_WINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_SERVER_PLAYER_WINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_SERVER_PLAYER_WINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_SERVER_PLAYER_WINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_SERVER_PLAYER_WINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_SERVER_PLAYER_WINDOW_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };

    private:
        enum WINDOW_LAYOUT
        {
            WINDOW_WIDTH = 240,
            WINDOW_HEIGHT = 320,
            WINDOW_CONTENT_TOP = 46,
            WINDOW_CONTENT_LEFT = 12,
            COLUMN_HEADER_HEIGHT = 16,
            ROW_HEIGHT = 15,
            ROW_MARGIN = 1,
            CONTENT_BOTTOM_MARGIN = 44,
            SCROLLBAR_WIDTH = 15,
            NAME_COLUMN_WIDTH = 100,
            LEVEL_COLUMN_WIDTH = 40,
            CLASS_COLUMN_WIDTH = 52,
            MAP_COLUMN_WIDTH = 36,
        };

        /// <summary>
        /// The interval in which the list is requested again while the window is open.
        /// </summary>
        static constexpr DWORD RefreshIntervalMilliseconds = 5000;

        CNewUIManager* m_pNewUIMng = nullptr;
        POINT m_Pos = { 0, 0 };
        CNewUIButton m_BtnExit;
        CNewUIScrollBar* m_pScrollBar = nullptr;
        DWORD m_dwNextRefresh = 0;

    public:
        CNewUIServerPlayerListWindow();
        virtual ~CNewUIServerPlayerListWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();

        void OpenningProcess();
        void ClosingProcess();

    private:
        void LoadImages();
        void UnloadImages();
        void InitButtons();

        void RequestPlayerList();
        void UpdateScrollBar() const;

        int GetVisibleRowCount() const;
        int GetContentHeight() const;

        void RenderFrame() const;
        void RenderColumnHeader() const;
        void RenderPlayerRows() const;
        void RenderPlayerRow(int rowIndex, int playerIndex, int x, int y) const;
        void RenderEmptyListHint() const;
    };
}
