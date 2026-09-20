#include "stdafx.h"

#include "UI/NewUI/Social/ServerPlayerListWindow.h"

#include "Audio/DSPlaySound.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Social/ServerPlayerList.h"
#include "I18N/All.h"
#include "UI/NewUI/NewUISystem.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <cwchar>
#include <iterator>
#include <utility>

using namespace SEASON3B;

namespace
{
    /// <summary>
    /// Appended to texts which are too wide for their field.
    /// </summary>
    constexpr wchar_t TextElision[] = L"...";

    //. The group box of a party member uses these backdrops; keeping them makes the
    //. rows of this window look exactly like the ones of the party window.
    constexpr unsigned int TitleBackdropColor = 0xE6000000u;
    constexpr unsigned int ContentBackdropColor = 0x99000000u;
}

CNewUIServerPlayerListWindow::CNewUIServerPlayerListWindow()
{
    m_pNewUIMng = nullptr;
    m_Pos.x = m_Pos.y = 0;
    m_pScrollBar = nullptr;
    m_dwNextRefresh = 0;
}

CNewUIServerPlayerListWindow::~CNewUIServerPlayerListWindow()
{
    Release();
}

bool CNewUIServerPlayerListWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (pNewUIMng == nullptr)
    {
        return false;
    }

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_SERVER_PLAYERS, this);

    m_pScrollBar = new CNewUIScrollBar();
    m_pScrollBar->Create(x, y, SCROLLBAR_HEIGHT);

    SetPos(x, y);
    LoadImages();
    InitButtons();

    Show(false);

    return true;
}

void CNewUIServerPlayerListWindow::Release()
{
    UnloadImages();

    if (m_pScrollBar != nullptr)
    {
        m_pScrollBar->Release();
        SAFE_DELETE(m_pScrollBar);
    }

    if (m_pNewUIMng != nullptr)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }
}

void CNewUIServerPlayerListWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_BtnExit.ChangeButtonInfo(
        m_Pos.x + EXIT_BUTTON_LEFT,
        m_Pos.y + EXIT_BUTTON_TOP,
        EXIT_BUTTON_WIDTH,
        EXIT_BUTTON_HEIGHT);

    if (m_pScrollBar != nullptr)
    {
        m_pScrollBar->SetPos(m_Pos.x + SCROLLBAR_LEFT, m_Pos.y + ROWS_TOP);
    }
}

void CNewUIServerPlayerListWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_SERVER_PLAYER_BASE_WINDOW_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_SERVER_PLAYER_BASE_WINDOW_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_SERVER_PLAYER_BASE_WINDOW_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_SERVER_PLAYER_BASE_WINDOW_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_SERVER_PLAYER_BASE_WINDOW_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_SERVER_PLAYER_BASE_WINDOW_BTN_EXIT, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_item_table01(L).tga", IMAGE_SERVER_PLAYER_TABLE_TOP_LEFT);
    LoadBitmap(L"Interface\\newui_item_table01(R).tga", IMAGE_SERVER_PLAYER_TABLE_TOP_RIGHT);
    LoadBitmap(L"Interface\\newui_item_table02(L).tga", IMAGE_SERVER_PLAYER_TABLE_BOTTOM_LEFT);
    LoadBitmap(L"Interface\\newui_item_table02(R).tga", IMAGE_SERVER_PLAYER_TABLE_BOTTOM_RIGHT);
    LoadBitmap(L"Interface\\newui_item_table03(Up).tga", IMAGE_SERVER_PLAYER_TABLE_TOP_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(Dw).tga", IMAGE_SERVER_PLAYER_TABLE_BOTTOM_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(L).tga", IMAGE_SERVER_PLAYER_TABLE_LEFT_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(R).tga", IMAGE_SERVER_PLAYER_TABLE_RIGHT_PIXEL);
}

void CNewUIServerPlayerListWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_SERVER_PLAYER_BASE_WINDOW_BACK);
    DeleteBitmap(IMAGE_SERVER_PLAYER_BASE_WINDOW_TOP);
    DeleteBitmap(IMAGE_SERVER_PLAYER_BASE_WINDOW_LEFT);
    DeleteBitmap(IMAGE_SERVER_PLAYER_BASE_WINDOW_RIGHT);
    DeleteBitmap(IMAGE_SERVER_PLAYER_BASE_WINDOW_BOTTOM);
    DeleteBitmap(IMAGE_SERVER_PLAYER_BASE_WINDOW_BTN_EXIT);

    DeleteBitmap(IMAGE_SERVER_PLAYER_TABLE_RIGHT_PIXEL);
    DeleteBitmap(IMAGE_SERVER_PLAYER_TABLE_LEFT_PIXEL);
    DeleteBitmap(IMAGE_SERVER_PLAYER_TABLE_BOTTOM_PIXEL);
    DeleteBitmap(IMAGE_SERVER_PLAYER_TABLE_TOP_PIXEL);
    DeleteBitmap(IMAGE_SERVER_PLAYER_TABLE_BOTTOM_RIGHT);
    DeleteBitmap(IMAGE_SERVER_PLAYER_TABLE_BOTTOM_LEFT);
    DeleteBitmap(IMAGE_SERVER_PLAYER_TABLE_TOP_RIGHT);
    DeleteBitmap(IMAGE_SERVER_PLAYER_TABLE_TOP_LEFT);
}

void CNewUIServerPlayerListWindow::InitButtons()
{
    m_BtnExit.ChangeButtonImgState(true, IMAGE_SERVER_PLAYER_BASE_WINDOW_BTN_EXIT);
    m_BtnExit.ChangeToolTipText(&I18N::Game::Close, true);
}

void CNewUIServerPlayerListWindow::OpenningProcess()
{
    RequestPlayerList();

    // The list may still be the one of the previous visit, so the derived rows are
    // rebuilt even if the revision didn't change in the meantime.
    RebuildDisplayRows();
    UpdateScrollBarExtent();
    m_lastListRevision = GameLogic::Social::GetServerPlayerListRevision();
}

void CNewUIServerPlayerListWindow::ClosingProcess()
{
}

void CNewUIServerPlayerListWindow::RequestPlayerList()
{
    SocketClient->ToGameServer()->SendServerPlayerListRequest();
    m_dwNextRefresh = GetTickCount() + RefreshIntervalMilliseconds;
}

bool CNewUIServerPlayerListWindow::Update()
{
    if (!IsVisible())
    {
        return true;
    }

    if (GetTickCount() >= m_dwNextRefresh)
    {
        RequestPlayerList();
    }

    const unsigned int revision = GameLogic::Social::GetServerPlayerListRevision();
    if (revision != m_lastListRevision)
    {
        m_lastListRevision = revision;
        RebuildDisplayRows();
        UpdateScrollBarExtent();
    }

    if (m_pScrollBar != nullptr && m_pScrollBar->IsVisible())
    {
        m_pScrollBar->Update();
    }

    return true;
}

void CNewUIServerPlayerListWindow::RebuildDisplayRows()
{
    const int playerCount = GameLogic::Social::GetServerPlayerCount();
    m_displayRows.clear();
    m_displayRows.reserve(playerCount);

    g_pRenderText->SetFont(g_hFont);

    for (int i = 0; i < playerCount; i++)
    {
        const auto& player = GameLogic::Social::GetServerPlayer(i);
        if (wcscmp(player.Name, Hero->ID) == 0)
        {
            // The own character is not part of the list; the player knows where it is.
            continue;
        }

        const auto clientClass = gCharacterManager.ChangeServerClassTypeToClientClassType(
            static_cast<SERVER_CLASS_TYPE>(player.ClassId));

        DisplayRow row;
        row.PlayerIndex = i;
        row.ClassText = FitTextToWidth(gCharacterManager.GetCharacterClassText(clientClass), CLASS_WIDTH);
        row.MapText = FitTextToWidth(gMapManager.GetMapName(player.Map), MAP_WIDTH);
        mu_swprintf_s(row.LevelText, std::size(row.LevelText), L"%d", player.Level);
        mu_swprintf_s(row.PositionText, std::size(row.PositionText), L"(%d,%d)", player.PositionX, player.PositionY);
        m_displayRows.push_back(std::move(row));
    }
}

void CNewUIServerPlayerListWindow::UpdateScrollBarExtent()
{
    if (m_pScrollBar == nullptr)
    {
        return;
    }

    const int scrollableRows = static_cast<int>(m_displayRows.size()) - VISIBLE_ROW_COUNT;
    const bool isScrollable = scrollableRows > 0;

    m_pScrollBar->Show(isScrollable);
    if (isScrollable)
    {
        m_pScrollBar->SetMaxPos(scrollableRows);
        if (m_pScrollBar->GetCurPos() > scrollableRows)
        {
            m_pScrollBar->SetCurPos(scrollableRows);
        }
    }
    else
    {
        m_pScrollBar->SetCurPos(0);
    }
}

bool CNewUIServerPlayerListWindow::UpdateMouseEvent()
{
    // The scroll bar widget renders and handles the mouse regardless of its own
    // visibility, so it's driven by the owner only while it's needed.
    if (m_pScrollBar != nullptr && m_pScrollBar->IsVisible())
    {
        m_pScrollBar->UpdateMouseEvent();
    }

    if (BtnProcess())
    {
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        return false;
    }

    return true;
}

bool CNewUIServerPlayerListWindow::BtnProcess()
{
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, INTERFACE_SERVER_PLAYERS))
    {
        return true;
    }

    if (m_BtnExit.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(INTERFACE_SERVER_PLAYERS);
        return true;
    }

    return false;
}

bool CNewUIServerPlayerListWindow::UpdateKeyEvent()
{
    if (IsVisible() && IsPress(VK_ESCAPE))
    {
        g_pNewUISystem->Hide(INTERFACE_SERVER_PLAYERS);
        PlayBuffer(SOUND_CLICK01);

        return false;
    }

    return true;
}

bool CNewUIServerPlayerListWindow::Render()
{
    if (!IsVisible())
    {
        return true;
    }

    const DWORD previousBackgroundColor = g_pRenderText->GetBgColor();
    g_pRenderText->SetBgColor(RGBA(0, 0, 0, 0));

    EnableAlphaTest();

    RenderImage(IMAGE_SERVER_PLAYER_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), float(WINDOW_HEIGHT));
    RenderImage(IMAGE_SERVER_PLAYER_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_SERVER_PLAYER_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_SERVER_PLAYER_BASE_WINDOW_RIGHT, m_Pos.x + float(WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_SERVER_PLAYER_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(WINDOW_HEIGHT) - 45.f, float(WINDOW_WIDTH), 45.f);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, I18N::Game::ServerPlayers, WINDOW_WIDTH, 0, RT3_SORT_CENTER);

    if (m_displayRows.empty())
    {
        RenderEmptyListHint();
    }
    else
    {
        const int firstRow = m_pScrollBar != nullptr ? m_pScrollBar->GetCurPos() : 0;
        const int totalRows = static_cast<int>(m_displayRows.size());
        for (int rowIndex = 0; rowIndex < VISIBLE_ROW_COUNT; rowIndex++)
        {
            const int displayRowIndex = firstRow + rowIndex;
            if (displayRowIndex >= totalRows)
            {
                break;
            }

            RenderPlayerRow(rowIndex, m_displayRows[displayRowIndex]);
        }
    }

    if (m_pScrollBar != nullptr && m_pScrollBar->IsVisible())
    {
        m_pScrollBar->Render();
    }

    m_BtnExit.Render();

    DisableAlphaBlend();

    g_pRenderText->SetBgColor(previousBackgroundColor);

    return true;
}

void CNewUIServerPlayerListWindow::RenderGroupBox(int x, int y, int width, int height, int titleWidth, int titleHeight) const
{
    EnableAlphaTest();

    RenderColorQuadARGB(float(x + 3), float(y + 2), float(titleWidth - 8), float(titleHeight), TitleBackdropColor);
    RenderColorQuadARGB(float(x + 3), float(y + 2 + titleHeight), float(width - 7), float(height - titleHeight - 7), ContentBackdropColor);

    RenderImage(IMAGE_SERVER_PLAYER_TABLE_TOP_LEFT, x, y, 14, 14);
    RenderImage(IMAGE_SERVER_PLAYER_TABLE_TOP_RIGHT, x + titleWidth - 14, y, 14, 14);
    RenderImage(IMAGE_SERVER_PLAYER_TABLE_TOP_RIGHT, x + width - 14, y + titleHeight, 14, 14);
    RenderImage(IMAGE_SERVER_PLAYER_TABLE_BOTTOM_LEFT, x, y + height - 14, 14, 14);
    RenderImage(IMAGE_SERVER_PLAYER_TABLE_BOTTOM_RIGHT, x + width - 14, y + height - 14, 14, 14);

    RenderImage(IMAGE_SERVER_PLAYER_TABLE_TOP_PIXEL, x + 6, y, titleWidth - 12, 14);
    RenderImage(IMAGE_SERVER_PLAYER_TABLE_RIGHT_PIXEL, x + titleWidth - 14, y + 6, 14, titleHeight - 6);
    RenderImage(IMAGE_SERVER_PLAYER_TABLE_TOP_PIXEL, x + titleWidth - 5, y + titleHeight, width - titleWidth - 6, 14);
    RenderImage(IMAGE_SERVER_PLAYER_TABLE_RIGHT_PIXEL, x + width - 14, y + titleHeight + 6, 14, height - titleHeight - 14);
    RenderImage(IMAGE_SERVER_PLAYER_TABLE_BOTTOM_PIXEL, x + 6, y + height - 14, width - 12, 14);
    RenderImage(IMAGE_SERVER_PLAYER_TABLE_LEFT_PIXEL, x, y + 6, 14, height - 14);
}

void CNewUIServerPlayerListWindow::RenderPlayerRow(int rowIndex, const DisplayRow& row) const
{
    const auto& player = GameLogic::Social::GetServerPlayer(row.PlayerIndex);
    const int rowX = m_Pos.x + ROWS_LEFT;
    const int rowY = m_Pos.y + ROWS_TOP + (rowIndex * ROW_HEIGHT);

    RenderGroupBox(rowX, rowY, ROW_WIDTH, ROW_BOX_HEIGHT, ROW_TITLE_WIDTH, ROW_TITLE_HEIGHT);

    // Same arrangement as a party member row: the name sits in the title band, the
    // map and the coordinates share the second line, and the third line holds the
    // level and the class instead of the health values.
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->RenderText(rowX, rowY + 8, player.Name, ROW_TITLE_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(194, 194, 194, 255);
    g_pRenderText->RenderText(rowX + 10, rowY + 26, row.MapText.c_str(), MAP_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(rowX + 85, rowY + 26, row.PositionText, POSITION_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(rowX + 8, rowY + 51, row.LevelText, LEVEL_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(rowX + 42, rowY + 51, row.ClassText.c_str(), CLASS_WIDTH, 0, RT3_SORT_RIGHT);
}

void CNewUIServerPlayerListWindow::RenderEmptyListHint() const
{
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(194, 194, 194, 255);
    g_pRenderText->RenderText(
        m_Pos.x,
        m_Pos.y + ROWS_TOP + 20,
        I18N::Game::NoPlayersOnline,
        WINDOW_WIDTH,
        0,
        RT3_SORT_CENTER);
}

std::wstring CNewUIServerPlayerListWindow::FitTextToWidth(const wchar_t* text, int maxWidth)
{
    if (text == nullptr || text[0] == L'\0' || maxWidth <= 0)
    {
        return std::wstring();
    }

    const auto length = static_cast<int>(wcslen(text));
    if (g_pRenderText->MeasureText(text, length).cx <= maxWidth)
    {
        return std::wstring(text, length);
    }

    std::wstring shortened(text, length);
    while (!shortened.empty())
    {
        shortened.pop_back();

        std::wstring candidate = shortened;
        candidate += TextElision;
        if (g_pRenderText->MeasureText(candidate.c_str(), static_cast<int>(candidate.size())).cx <= maxWidth)
        {
            return candidate;
        }
    }

    return std::wstring();
}

float CNewUIServerPlayerListWindow::GetLayerDepth()
{
    return 2.4f;
}
