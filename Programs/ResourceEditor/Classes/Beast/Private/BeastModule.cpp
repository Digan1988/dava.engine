#include "Classes/Beast/BeastModule.h"

#if defined(__DAVAENGINE_BEAST__)

#include "Classes/Beast/Private/BeastDialog.h"
#include "Classes/Beast/Private/BeastRunner.h"

#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/DataNodes/Settings/RESettings.h>
#include <REPlatform/Global/GlobalOperations.h>
#include <REPlatform/Global/StringConstants.h>
#include <REPlatform/Scene/SceneEditor2.h>

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/Deprecated.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FileSystem.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QMessageBox>

void BeastModule::PostInit()
{
    using namespace DAVA;

    QtAction* action = new QtAction(GetAccessor(), QString("Run Beast"));
    { // enabled/disabled state
        action->SetStateUpdationFunction(QtAction::Enabled, Reflection::Create(ReflectedObject(this)), FastName("beastAvailable"), [](const Any& fieldValue) -> Any {
            return fieldValue.Get<bool>(false);
        });
    }

    connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &BeastModule::OnBeastAndSave));

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint("Scene", { InsertionParams::eInsertionMethod::AfterItem, "actionInvalidateStaticOcclusion" }));

    GetUI()->AddAction(DAVA::mainWindowKey, placementInfo, action);
}

void BeastModule::OnBeastAndSave()
{
    using namespace DAVA;

    SceneData* sceneData = GetAccessor()->GetActiveContext()->GetData<SceneData>();
    DVASSERT(sceneData != nullptr);
    RefPtr<SceneEditor2> scene = sceneData->GetScene();
    DVASSERT(scene);

    if (scene->GetEnabledTools())
    {
        if (QMessageBox::Yes == QMessageBox::question(GetUI()->GetWindow(DAVA::mainWindowKey), "Starting Beast", "Disable landscape editor and start beasting?", (QMessageBox::Yes | QMessageBox::No), QMessageBox::No))
        {
            scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);

            bool success = !scene->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL);
            if (!success)
            {
                Logger::Error(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS.c_str());
                return;
            }
        }
        else
        {
            return;
        }
    }

    InvokeOperation(DAVA::SaveCurrentScene.ID);
    if (!scene->IsLoaded() || scene->IsChanged())
    {
        return;
    }

    BeastDialog dlg(GetUI()->GetWindow(DAVA::mainWindowKey));
    dlg.SetScene(scene.Get());
    const bool run = dlg.Exec();
    if (!run)
        return;

    RunBeast(dlg.GetPath(), dlg.GetMode());

    scene->SetChanged();
    InvokeOperation(DAVA::SaveCurrentScene.ID);
    scene->ClearAllCommands();
}

void BeastModule::RunBeast(const QString& outputPath, Beast::eBeastMode mode)
{
    using namespace DAVA;
    using namespace DAVA;

    SceneData* sceneData = GetAccessor()->GetActiveContext()->GetData<SceneData>();
    DVASSERT(sceneData != nullptr);
    RefPtr<SceneEditor2> scene = sceneData->GetScene();
    DVASSERT(scene);

    const DAVA::FilePath path = outputPath.toStdString();

    WaitDialogParams waitDlgParams;
    waitDlgParams.message = "Starting Beast";
    waitDlgParams.needProgressBar = true;
    waitDlgParams.cancelable = true;
    waitDlgParams.max = 100;
    std::unique_ptr<WaitHandle> waitHandle = GetUI()->ShowWaitDialog(DAVA::mainWindowKey, waitDlgParams);
    BeastRunner beast(scene.Get(), scene->GetScenePath(), path, mode, std::move(waitHandle));
    beast.RunUIMode();

    if (mode == Beast::eBeastMode::MODE_LIGHTMAPS)
    {
        // ReloadTextures should be delayed to give Qt some time for closing wait dialog before we will open new one for texture reloading.
        delayedExecutor.DelayedExecute([this]() {
            InvokeOperation(DAVA::ReloadAllTextures.ID, DAVA::Deprecated::GetDataNode<DAVA::CommonInternalSettings>()->textureViewGPU);
        });
    }
}

bool BeastModule::GetBeastAvailable() const
{
    if (GetAccessor()->GetActiveContext() != nullptr)
    {
        DAVA::FileSystem* fs = DAVA::GetEngineContext()->fileSystem;
        return fs->GetFilenamesTag().empty() == true;
    }

    return false;
}

DAVA_VIRTUAL_REFLECTION_IMPL(BeastModule)
{
    DAVA::ReflectionRegistrator<BeastModule>::Begin()
    .ConstructorByPointer()
    .Field("beastAvailable", &BeastModule::GetBeastAvailable, nullptr)
    .End();
}

DECL_TARC_MODULE(BeastModule);

#endif //#if defined (__DAVAENGINE_BEAST__)
