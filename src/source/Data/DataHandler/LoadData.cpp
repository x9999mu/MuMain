// LoadData.cpp: implementation of the CLoadData class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoadData.h"

#include "Render/Sprites/GlobalBitmap.h"

#include "Render/Models/ZzzBMD.h"
#include "Render/Textures/ZzzTexture.h"

CLoadData gLoadData;

CLoadData::CLoadData() // OK
{
}

CLoadData::~CLoadData() // OK
{
}

namespace
{
// Probes a model file through the same shim the loader opens it with, so the
// answer matches what Open2 sees on every platform: the POSIX shim also
// corrects the separators and the case of these Windows-spelled asset paths.
bool ModelFileExists(const wchar_t* Dir, const wchar_t* Name)
{
    wchar_t ModelPath[260] = {};
    _snwprintf(ModelPath, std::size(ModelPath), L"%ls%ls", Dir, Name);

    FILE* file = _wfopen(ModelPath, L"rb");
    if (file == nullptr)
    {
        return false;
    }

    fclose(file);
    return true;
}
} // namespace

void CLoadData::AccessModel(int Type, const wchar_t* Dir, const wchar_t* FileName, int i, bool bOptional)
{
    wchar_t Name[64];
    if (i == -1)
        mu_swprintf(Name, L"%ls.bmd", FileName);
    else if (i < 10)
        mu_swprintf(Name, L"%ls0%d.bmd", FileName, i);
    else
        mu_swprintf(Name, L"%ls%d.bmd", FileName, i);

    bool Success = false;

    Models[Type].m_iBMDSeqID = Type;

    Success = Models[Type].Open2(Dir, Name);

    if (Success == false)
    {
        const bool bBaseModel = wcscmp(FileName, L"Monster") == 0 || wcscmp(FileName, L"Player") == 0 ||
                                wcscmp(FileName, L"PlayerTest") == 0 || wcscmp(FileName, L"Angel") == 0;

        // An optional model that is simply not shipped is not worth a line in
        // MuError.log: the world object table alone has 160 slots per map and
        // the maps fill only a handful, which put ~150 lines per map load into
        // the log. A file that is there but cannot be read, and a missing base
        // model - which stops the client below - are still reported.
        if (!bOptional || bBaseModel || ModelFileExists(Dir, Name))
        {
            g_ErrorReport.Write(L"AccessModel failed: %ls%ls (Type=%d)\r\n", Dir, Name, Type);
        }

        if (bBaseModel)
        {
            wchar_t Text[256];
            mu_swprintf(Text, L"%ls file does not exist.", Name);
            MessageBox(g_hWnd, Text, NULL, MB_OK);
            SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        }
    }
}

void CLoadData::OpenTexture(int Model, const wchar_t* SubFolder, int Wrap, int Type, bool Check)
{
    BMD* pModel = &Models[Model];

    for (int i = 0; i < pModel->NumMeshs; i++)
    {
        Texture_t* pTexture = &pModel->Textures[i];

        int wchars_num = MultiByteToWideChar(CP_UTF8, 0, pTexture->FileName, -1, NULL, 0);
        auto* textureFileName = new wchar_t[wchars_num];
        MultiByteToWideChar(CP_UTF8, 0, pTexture->FileName, -1, textureFileName, wchars_num);

        wchar_t szFullPath[256] = { 0, };
        wcscpy(szFullPath, L"Data\\");
        wcscat(szFullPath, SubFolder);
        wcscat(szFullPath, textureFileName);

        wchar_t __ext[_MAX_EXT] = { 0, };
        _wsplitpath(textureFileName, NULL, NULL, NULL, __ext);
        if (pTexture->FileName[0] == 'h' && pTexture->FileName[1] == 'i' && pTexture->FileName[2] == 'd')
        {
            pModel->IndexTexture[i] = BITMAP_HIDE;
        }
        else if (tolower(__ext[1]) == 't') // TGA
        {
            pModel->IndexTexture[i] = Bitmaps.LoadImage(szFullPath, GL_NEAREST, Wrap);
        }
        else if (tolower(__ext[1]) == 'j') // JPG
        {
            pModel->IndexTexture[i] = Bitmaps.LoadImage(szFullPath, Type, Wrap);
        }

        bool isSkin = (pTexture->FileName[0] == 's' && pTexture->FileName[1] == 'k' && pTexture->FileName[2] == 'i')
            || !wcsnicmp(textureFileName, L"level", 5);
        bool isHair = pTexture->FileName[0] == 'h' && pTexture->FileName[1] == 'a' && pTexture->FileName[2] == 'i' && pTexture->FileName[3] == 'r';
        
        if (isSkin || isHair)
        {
            BITMAP_t* pBitmap =
                pModel->IndexTexture[i] != BITMAP_UNKNOWN
                ? Bitmaps.FindTexture(pModel->IndexTexture[i])
                : Bitmaps.FindTextureByName(textureFileName);

            if (pBitmap)
            {
                pBitmap->IsSkin = isSkin;
                pBitmap->IsHair = isHair;
            }
        }
        
        if (pModel->IndexTexture[i] == BITMAP_UNKNOWN)
        {
            if (auto pBitmap = Bitmaps.FindTextureByName(textureFileName))
            {
                // we try to find an already loaded texture based on the file name
                Bitmaps.LoadImage(pBitmap->BitmapIndex, pBitmap->FileName);
                pModel->IndexTexture[i] = pBitmap->BitmapIndex;
            }
            else
            {
                wchar_t szErrorMsg[256] = { 0, };
                mu_swprintf(szErrorMsg, L"OpenTexture Failed: %ls of %hs", szFullPath, pModel->Name);
                g_ErrorReport.Write(L"%ls (Model=%d)\r\n", szErrorMsg, Model);
#ifdef FOR_WORK
                PopUpErrorCheckMsgBox(szErrorMsg);
#else // FOR_WORK
                PopUpErrorCheckMsgBox(szErrorMsg, true);
#endif // FOR_WORK
            }
        }

        delete[] textureFileName;
    }
}
