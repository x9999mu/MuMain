#include "stdafx.h"

#include "UI/NewUI/Social/ServerPlayerListWindow.h"

#include "Audio/DSPlaySound.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Social/ServerPlayerList.h"
#include "I18N/All.h"
#include "UI/NewUI/NewUISystem.h"
#include "World/MapInfra/MapManager.h"

using namespace SEASON3B;

CNewUIServerPlayerListWindow::CNewUIServerPlayerListWindow() = default;

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
    m_pScrollBar->Create(x, y, GetContentHeight());

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

    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + WINDOW_HEIGHT - 37, 36, 29);

    if (m_pScrollBar != nullptr)
    {
        m_pScrollBar->SetPos(
            m_Pos.x + WINDOW_WIDTH - WINDOW_CONTENT_LEFT - SCROLLBAR_WIDTH,
            m_Pos.y + WINDOW_CONTENT_TOP + COLUMN_HEADER_HEIGHT);
    }
}

void CNewUIServerPlayerListWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_SERVER_PLAYER_WINDOW_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_SERVER_PLAYER_WINDOW_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_SERVER_PLAYER_WINDOW_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_SERVER_PLAYER_WINDOW_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_SERVER_PLAYER_WINDOW_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_SERVER_PLAYER_WINDOW_BTN_EXIT, GL_LINEAR);
}

void CNewUIServerPlayerListWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_SERVER_PLAYER_WINDOW_BACK);
    DeleteBitmap(IMAGE_SERVER_PLAYER_WINDOW_TOP);
    DeleteBitmap(IMAGE_SERVER_PLAYER_WINDOW_LEFT);
    DeleteBitmap(IMAGE_SERVER_PLAYER_WINDOW_RIGHT);
    DeleteBitmap(IMAGE_SERVER_PLAYER_WINDOW_BOTTOM);
    DeleteBitmap(IMAGE_SERVER_PLAYER_WINDOW_BTN_EXIT);
}

void CNewUIServerPlayerListWindow::InitButtons()
{
    m_BtnExit.ChangeButtonImgState(true, IMAGE_SERVER_PLAYER_WINDOW_BTN_EXIT);
    m_BtnExit.ChangeToolTipText(&I18N::Game::Close, true);
}

void CNewUIServerPlayerListWindow::OpenningProcess()
{
    RequestPlayerList();
    UpdateScrollBar();
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

    UpdateScrollBar();

    return true;
}

void CNewUIServerPlayerListWindow::UpdateScrollBar() const
{
    if (m_pScrollBar == nullptr)
    {
        return;
    }

    const int totalRows = static_cast<int>(GameLogic::Social::GetServerPlayerList().size());
    const int scrollableRows = totalRows - GetVisibleRowCount();
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

    m_pScrollBar->Update();
}

bool CNewUIServerPlayerListWindow::UpdateMouseEvent()
{
    if (m_pScrollBar != nullptr)
    {
        m_pScrollBar->UpdateMouseEvent();
    }

    if (m_BtnExit.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(INTERFACE_SERVER_PLAYERS);
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        return false;
    }

    return true;
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

    RenderFrame();
    RenderColumnHeader();

    if (GameLogic::Social::GetServerPlayerList().empty())
    {
        RenderEmptyListHint();
    }
    else
    {
        RenderPlayerRows();
    }

    if (m_pScrollBar != nullptr)
    {
        m_pScrollBar->Render();
    }

    m_BtnExit.Render();

    g_pRenderText->SetBgColor(previousBackgroundColor);

    return true;
}

void CNewUIServerPlayerListWindow::RenderFrame() const
{
    EnableAlphaTest();

    RenderImage(IMAGE_SERVER_PLAYER_WINDOW_BACK, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), float(WINDOW_HEIGHT));
    RenderImage(IMAGE_SERVER_PLAYER_WINDOW_TOP, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_SERVER_PLAYER_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_SERVER_PLAYER_WINDOW_RIGHT, m_Pos.x + float(WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_SERVER_PLAYER_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(WINDOW_HEIGHT) - 45.f, float(WINDOW_WIDTH), 45.f);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(0xFFE0C080);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, I18N::Game::ServerPlayers, WINDOW_WIDTH, 0, RT3_SORT_CENTER);

    DisableAlphaBlend();
}

void CNewUIServerPlayerListWindow::RenderColumnHeader() const
{
    const int x = m_Pos.x + WINDOW_CONTENT_LEFT;
    const int y = m_Pos.y + WINDOW_CONTENT_TOP;

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(0xFFB0B0B0);
    g_pRenderText->SetBgColor(0x00000000);

    g_pRenderText->RenderText(x, y, I18N::Game::Name, NAME_COLUMN_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(x + NAME_COLUMN_WIDTH, y, I18N::Game::Level, LEVEL_COLUMN_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(x + NAME_COLUMN_WIDTH + LEVEL_COLUMN_WIDTH, y, I18N::Game::Class, CLASS_COLUMN_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(x + NAME_COLUMN_WIDTH + LEVEL_COLUMN_WIDTH + CLASS_COLUMN_WIDTH, y, I18N::Game::Map, MAP_COLUMN_WIDTH, 0, RT3_SORT_LEFT);
}

void CNewUIServerPlayerListWindow::RenderPlayerRows() const
{
    const auto& players = GameLogic::Social::GetServerPlayerList();
    const int firstRow = m_pScrollBar != nullptr ? m_pScrollBar->GetCurPos() : 0;
    const int visibleRows = GetVisibleRowCount();
    const int rowX = m_Pos.x + WINDOW_CONTENT_LEFT;
    const int firstRowY = m_Pos.y + WINDOW_CONTENT_TOP + COLUMN_HEADER_HEIGHT;

    for (int rowIndex = 0; rowIndex < visibleRows; rowIndex++)
    {
        const int playerIndex = firstRow + rowIndex;
        if (playerIndex >= static_cast<int>(players.size()))
        {
            break;
        }

        RenderPlayerRow(rowIndex, playerIndex, rowX, firstRowY);
    }
}

void CNewUIServerPlayerListWindow::RenderPlayerRow(int rowIndex, int playerIndex, int x, int y) const
{
    const auto& player = GameLogic::Social::GetServerPlayerList()[playerIndex];
    const int rowY = y + ((ROW_HEIGHT + ROW_MARGIN) * rowIndex);
    const bool isOwnCharacter = player.Name == Hero->ID;

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(isOwnCharacter ? 0xFF60FF60u : 0xFFFFFFFFu);

    g_pRenderText->RenderText(x, rowY, player.Name.c_str(), NAME_COLUMN_WIDTH, 0, RT3_SORT_LEFT);

    wchar_t levelText[8] = { 0, };
    mu_swprintf_s(levelText, std::size(levelText), L"%d", player.Level);
    g_pRenderText->RenderText(x + NAME_COLUMN_WIDTH, rowY, levelText, LEVEL_COLUMN_WIDTH, 0, RT3_SORT_CENTER);

    const auto classText = gCharacterManager.GetCharacterClassText(static_cast<CLASS_TYPE>(player.ClassId));
    g_pRenderText->RenderText(x + NAME_COLUMN_WIDTH + LEVEL_COLUMN_WIDTH, rowY, classText, CLASS_COLUMN_WIDTH, 0, RT3_SORT_LEFT);

    const auto mapText = gMapManager.GetMapName(player.Map);
    g_pRenderText->RenderText(x + NAME_COLUMN_WIDTH + LEVEL_COLUMN_WIDTH + CLASS_COLUMN_WIDTH, rowY, mapText, MAP_COLUMN_WIDTH, 0, RT3_SORT_LEFT);
}

void CNewUIServerPlayerListWindow::RenderEmptyListHint() const
{
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(0xFFB0B0B0);

    g_pRenderText->RenderText(
        m_Pos.x,
        m_Pos.y + WINDOW_CONTENT_TOP + COLUMN_HEADER_HEIGHT,
        I18N::Game::NoPlayersOnline,
        WINDOW_WIDTH,
        0,
        RT3_SORT_CENTER);
}

int CNewUIServerPlayerListWindow::GetContentHeight() const
{
    return WINDOW_HEIGHT - WINDOW_CONTENT_TOP - COLUMN_HEADER_HEIGHT - CONTENT_BOTTOM_MARGIN;
}

int CNewUIServerPlayerListWindow::GetVisibleRowCount() const
{
    return GetContentHeight() / (ROW_HEIGHT + ROW_MARGIN);
}

float CNewUIServerPlayerListWindow::GetLayerDepth()
{
    return 2.4f;
}
