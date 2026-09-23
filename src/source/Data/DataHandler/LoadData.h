// LoadData.h: interface for the CLoadData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CLoadData
{
public:
    CLoadData();
    virtual ~CLoadData();
    // Loads a model from `Dir`/`FileName`. `bOptional` marks a model which the
    // caller may legitimately not ship - the world object table has
    // MAX_WORLD_OBJECTS slots, but a map only places a few of them, so a
    // missing file there is the normal case and is not reported.
    void AccessModel(int Type, const wchar_t* Dir, const wchar_t* FileName, int i = -1, bool bOptional = false);
    void OpenTexture(int Model, const wchar_t* SubFolder, int Wrap = GL_REPEAT, int Type = GL_NEAREST,
                     bool Check = true);

public:
};

extern CLoadData gLoadData;
