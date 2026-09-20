#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "UI/NewUI/NewUIManager.h"
#include "UI/NewUI/Inventory/NewUIMyInventory.h"
#include "UI/NewUI/Widgets/NewUIButton.h"
#include "UI/NewUI/Widgets/NewUIScrollBar.h"

#include <string>
#include <vector>

namespace SEASON3B
{
    /// <summary>
    /// Shows the players which are currently online on the same game server. It uses
    /// the layout of the party window, but without the buttons which would kick a
    /// player out of a party.
    /// </summary>
    class CNewUIServerPlayerListWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            // Base window, same images as the party window.
            IMAGE_SERVER_PLAYER_BASE_WINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_SERVER_PLAYER_BASE_WINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_SERVER_PLAYER_BASE_WINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_SERVER_PLAYER_BASE_WINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_SERVER_PLAYER_BASE_WINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_SERVER_PLAYER_BASE_WINDOW_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,

            // Group box of a single row, same images as a party member entry.
            IMAGE_SERVER_PLAYER_TABLE_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,
            IMAGE_SERVER_PLAYER_TABLE_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,
            IMAGE_SERVER_PLAYER_TABLE_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,
            IMAGE_SERVER_PLAYER_TABLE_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,
            IMAGE_SERVER_PLAYER_TABLE_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,
            IMAGE_SERVER_PLAYER_TABLE_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,
            IMAGE_SERVER_PLAYER_TABLE_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,
            IMAGE_SERVER_PLAYER_TABLE_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,
        };

    private:
        enum WINDOW_LAYOUT
        {
            WINDOW_WIDTH = 190,
            WINDOW_HEIGHT = 429,

            ROWS_LEFT = 10,
            ROWS_TOP = 40,
            ROW_HEIGHT = 71,
            VISIBLE_ROW_COUNT = 5,

            ROW_WIDTH = 158,
            ROW_BOX_HEIGHT = 70,
            ROW_TITLE_WIDTH = 70,
            ROW_TITLE_HEIGHT = 20,

            SCROLLBAR_LEFT = 170,
            SCROLLBAR_HEIGHT = 340,

            MAP_WIDTH = 70,
            POSITION_WIDTH = 60,
            LEVEL_WIDTH = 30,
            CLASS_WIDTH = 116,

            EXIT_BUTTON_LEFT = 13,
            EXIT_BUTTON_TOP = 392,
            EXIT_BUTTON_WIDTH = 36,
            EXIT_BUTTON_HEIGHT = 29,
        };

        /// <summary>
        /// The interval in which the list is requested again while the window is open.
        /// </summary>
        static constexpr DWORD RefreshIntervalMilliseconds = 5000;

        /// <summary>
        /// A revision which the received list never has, so the first scroll bar
        /// update always runs.
        /// </summary>
        static constexpr unsigned int NoRevisionYet = ~0u;

        /// <summary>
        /// A row which is ready to be rendered. The texts which can be wider than
        /// their field are shortened once per received list, so that rendering
        /// neither measures nor allocates anything.
        /// </summary>
        struct DisplayRow
        {
            int PlayerIndex = 0;
            std::wstring ClassText;
            std::wstring MapText;
            wchar_t LevelText[8] = { 0, };
            wchar_t PositionText[12] = { 0, };
        };

        CNewUIManager* m_pNewUIMng = nullptr;
        POINT m_Pos = { 0, 0 };
        CNewUIButton m_BtnExit;
        CNewUIScrollBar* m_pScrollBar = nullptr;
        DWORD m_dwNextRefresh = 0;
        unsigned int m_lastListRevision = NoRevisionYet;
        std::vector<DisplayRow> m_displayRows;

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

        bool BtnProcess();

        float GetLayerDepth();

        void OpenningProcess();
        void ClosingProcess();

    private:
        void LoadImages();
        void UnloadImages();
        void InitButtons();

        void RequestPlayerList();
        void RebuildDisplayRows();
        void UpdateScrollBarExtent();

        void RenderGroupBox(int x, int y, int width, int height, int titleWidth, int titleHeight) const;
        void RenderPlayerRow(int rowIndex, const DisplayRow& row) const;
        void RenderEmptyListHint() const;

        static std::wstring FitTextToWidth(const wchar_t* text, int maxWidth);
    };
}
