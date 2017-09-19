#pragma once

#include <TArc/DataProcessing/SettingsNode.h>

#include <FileSystem/FilePath.h>
#include <Math/Color.h>

class DistanceSystemPreferences : public DAVA::TArc::SettingsNode
{
public:
    DAVA::Color linesColor = DAVA::Color(1.0f, 0.0f, 0.0f, 0.9f);
    DAVA::Color textColor = DAVA::Color(1.0f, 0.0f, 0.0f, 0.9f);
    DAVA::FilePath helpLinesTexture = DAVA::FilePath("~res:/QuickEd/UI/HUDControls/MagnetLine/dotline.png");

    DAVA_VIRTUAL_REFLECTION(DistanceSystemPreferences, DAVA::TArc::SettingsNode);
};
