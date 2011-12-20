#include "SceneEditorScreenMain.h"

#include "EditorBodyControl.h"
#include "LibraryControl.h"

#include "ControlsFactory.h"

void SceneEditorScreenMain::LoadResources()
{
    //RenderManager::Instance()->EnableOutputDebugStatsEveryNFrame(30);
    ControlsFactory::CustomizeScreenBack(this);

    font = ControlsFactory::CreateFontLight();
    
    //init file system dialog
    fileSystemDialog = new UIFileSystemDialog("~res:/Fonts/MyriadPro-Regular.otf");
    fileSystemDialog->SetDelegate(this);
    
    keyedArchieve = new KeyedArchive();
    keyedArchieve->Load("~doc:/ResourceEditorOptions.archive");
    String path = keyedArchieve->GetString("LastSavedPath", "/");
    if(path.length())
    fileSystemDialog->SetCurrentDir(path);
    
    // add line after menu
    Rect fullRect = GetRect();
    AddLineControl(Rect(0, MENU_HEIGHT, fullRect.dx, LINE_HEIGHT));
    CreateTopMenu();
    
    //add line before body
    AddLineControl(Rect(0, BODY_Y_OFFSET, fullRect.dx, LINE_HEIGHT));
    
    //Library
    libraryButton = ControlsFactory::CreateButton(
                        Rect(fullRect.dx - BUTTON_WIDTH, BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT), 
                        L"Library");
    libraryButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnLibraryPressed));
    AddControl(libraryButton);
    
    libraryControl = new LibraryControl(
                            Rect(fullRect.dx - LIBRARY_WIDTH, BODY_Y_OFFSET + 1, LIBRARY_WIDTH, fullRect.dy - BODY_Y_OFFSET - 1)); 
    libraryControl->SetDelegate(this);
    libraryControl->SetPath(path);

    //properties
    propertiesButton = ControlsFactory::CreateButton(
                        Rect(fullRect.dx - (BUTTON_WIDTH*2 + 1), BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT), 
                        L"Properties");
    propertiesButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnPropertiesPressed));
    AddControl(propertiesButton);
    
    
    hierarhyButton = ControlsFactory::CreateButton(
                         Rect(0, BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT), 
                         L"Hierarhy");
    hierarhyButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnHierarhyPressed));
    AddControl(hierarhyButton);
    

    InitializeBodyList();
}

void SceneEditorScreenMain::UnloadResources()
{
    SafeRelease(keyedArchieve);
    
    SafeRelease(propertiesButton);
    
    SafeRelease(libraryControl);
    SafeRelease(libraryButton);
    
    SafeRelease(fileSystemDialog);
    
    ReleaseBodyList();
    
    ReleaseTopMenu();
    
    SafeRelease(font);
}


void SceneEditorScreenMain::WillAppear()
{
}

void SceneEditorScreenMain::WillDisappear()
{
	
}

void SceneEditorScreenMain::Update(float32 timeElapsed)
{
}

void SceneEditorScreenMain::Draw(const UIGeometricData &geometricData)
{
    UIScreen::Draw(geometricData);
}

void SceneEditorScreenMain::CreateTopMenu()
{
    int32 x = 0;
    int32 y = 0;
    int32 dx = BUTTON_WIDTH;
    int32 dy = BUTTON_HEIGHT;
    btnOpen = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Open");
    x += dx + 1;
    btnSave = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Save");
    x += dx + 1;
    btnMaterials = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Materials");
    x += dx + 1;
    btnCreate = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Create");
    x += dx + 1;
    btnNew = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"New");
    x += dx + 1;
    btnProject = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Open Project");
	x += dx + 1;
	btnBeast = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Beast");
    
    

    AddControl(btnOpen);
    AddControl(btnSave);
    AddControl(btnMaterials);
    AddControl(btnCreate);
    AddControl(btnNew);
    AddControl(btnProject);
#ifdef __DAVAENGINE_BEAST__
	AddControl(btnBeast);
#endif

    btnOpen->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnOpenPressed));
    btnSave->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSavePressed));
    btnMaterials->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnMaterialsPressed));
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnCreatePressed));
    btnNew->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnNewPressed));
    btnProject->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnOpenProjectPressed));
	btnBeast->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnBeastPressed));
}

void SceneEditorScreenMain::ReleaseTopMenu()
{
    SafeRelease(btnOpen);
    SafeRelease(btnSave);
    SafeRelease(btnMaterials);
    SafeRelease(btnCreate);
    SafeRelease(btnNew);
    SafeRelease(btnProject);
	SafeRelease(btnBeast);
}

void SceneEditorScreenMain::AddLineControl(DAVA::Rect r)
{
    UIControl *lineControl = ControlsFactory::CreateLine(r);
    AddControl(lineControl);
    SafeRelease(lineControl);
}

void SceneEditorScreenMain::OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile)
{
    switch (fileSystemDialogOpMode) 
    {
        case DIALOG_OPERATION_MENU_OPEN:
        {
            int32 iBody = FindCurrentBody();
            if(-1 != iBody)
            {
                bodies[iBody].bodyControl->OpenScene(pathToFile);
            }
            
            break;
        }
            
        case DIALOG_OPERATION_MENU_SAVE:
        {
            break;
        }
            
        case DIALOG_OPERATION_MENU_PROJECT:
        {
            keyedArchieve->SetString("LastSavedPath", pathToFile);
            keyedArchieve->Save("~doc:/ResourceEditorOptions.archive");
            
            libraryControl->SetPath(pathToFile);
            break;
        }

        default:
            break;
    }

    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
}

void SceneEditorScreenMain::OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog)
{
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
}


void SceneEditorScreenMain::OnOpenPressed(BaseObject * obj, void *, void *)
{
    if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetExtensionFilter(".sce");
        fileSystemDialog->Show(this);
        fileSystemDialogOpMode = DIALOG_OPERATION_MENU_OPEN;
    }
}


void SceneEditorScreenMain::OnSavePressed(BaseObject * obj, void *, void *)
{
    if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetExtensionFilter(".dae");
        fileSystemDialog->Show(this);
        fileSystemDialogOpMode = DIALOG_OPERATION_MENU_SAVE;
    }
}


void SceneEditorScreenMain::OnMaterialsPressed(BaseObject * obj, void *, void *)
{
    
}


void SceneEditorScreenMain::OnCreatePressed(BaseObject * obj, void *, void *)
{
    
}


void SceneEditorScreenMain::OnNewPressed(BaseObject * obj, void *, void *)
{
}


void SceneEditorScreenMain::OnOpenProjectPressed(BaseObject * obj, void *, void *)
{
    if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_CHOOSE_DIR);
        fileSystemDialog->Show(this);
        fileSystemDialogOpMode = DIALOG_OPERATION_MENU_PROJECT;
    }
}


void SceneEditorScreenMain::InitializeBodyList()
{
    AddBodyItem(L"Level", false);
}

void SceneEditorScreenMain::ReleaseBodyList()
{
    for(int32 i = 0; i < bodies.size(); ++i)
    {
        RemoveControl(bodies[i].headerButton);
        RemoveControl(bodies[i].bodyControl);
        
        SafeRelease(bodies[i].headerButton);
        SafeRelease(bodies[i].closeButton);
        SafeRelease(bodies[i].bodyControl);
    }
    bodies.clear();
}

void SceneEditorScreenMain::AddBodyItem(const WideString &text, bool isCloseable)
{
    BodyItem c;
    
    int32 count = bodies.size();
    c.headerButton = ControlsFactory::CreateButton(
                        Rect(TAB_BUTTONS_OFFSET + count * (BUTTON_WIDTH + 1), 
                             BODY_Y_OFFSET - BUTTON_HEIGHT, 
                             BUTTON_WIDTH, 
                             BUTTON_HEIGHT), 
                        text);

    c.headerButton->SetTag(count);
    c.headerButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSelectBody));
    if(isCloseable)
    {
        c.closeButton = ControlsFactory::CreateCloseWindowButton(Rect(BUTTON_WIDTH - BUTTON_HEIGHT, 0, BUTTON_HEIGHT, BUTTON_HEIGHT));
        c.closeButton->SetTag(count);
        c.closeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnCloseBody));
        c.headerButton->AddControl(c.closeButton);
    }
    else 
    {
        c.closeButton = NULL;
    }


    Rect fullRect = GetRect();
    c.bodyControl = new EditorBodyControl(Rect(0, BODY_Y_OFFSET + 1, fullRect.dx, fullRect.dy - BODY_Y_OFFSET - 1));
    c.bodyControl->SetTag(count);    
    
    AddControl(c.headerButton);    
    bodies.push_back(c);
    
    //set as current
    c.headerButton->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
}


void SceneEditorScreenMain::OnSelectBody(BaseObject * owner, void * userData, void * callerData)
{
    UIButton *btn = (UIButton *)owner;
    
    for(int32 i = 0; i < bodies.size(); ++i)
    {
        if(bodies[i].bodyControl->GetParent())
        {
            if(btn == bodies[i].headerButton)
            {
                // click on selected body - nothing to do
                return;
            }
            
            if(libraryControl->GetParent())
            {
                RemoveControl(libraryControl);
                bodies[i].bodyControl->UpdateLibraryState(false, libraryControl->GetRect().dx);
            }
            
            RemoveControl(bodies[i].bodyControl);
            bodies[i].headerButton->SetSelected(false, false);
        }
    }
    AddControl(bodies[btn->GetTag()].bodyControl);
    bodies[btn->GetTag()].headerButton->SetSelected(true, false);    
    
    if(libraryControl->GetParent())
    {
        BringChildFront(libraryControl);
    }
}
void SceneEditorScreenMain::OnCloseBody(BaseObject * owner, void * userData, void * callerData)
{
    UIButton *btn = (UIButton *)owner;
    int32 tag = btn->GetTag();
    
    bool needToSwitchBody = false;
    Vector<BodyItem>::iterator it = bodies.begin();
    for(int32 i = 0; i < bodies.size(); ++i, ++it)
    {
        if(btn == bodies[i].closeButton)
        {
            if(bodies[i].bodyControl->GetParent())
            {
                RemoveControl(libraryControl);
                
                RemoveControl(bodies[i].bodyControl);
                needToSwitchBody = true;
            }
            RemoveControl(bodies[i].headerButton);
            
            SafeRelease(bodies[i].headerButton);
            SafeRelease(bodies[i].closeButton);
            SafeRelease(bodies[i].bodyControl);            

            bodies.erase(it);
            break;
        }
    }

    for(int32 i = 0; i < bodies.size(); ++i, ++it)
    {
        bodies[i].headerButton->SetRect(Rect(TAB_BUTTONS_OFFSET + i * (BUTTON_WIDTH + 1), BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT));
        
        bodies[i].headerButton->SetTag(i);
        if(bodies[i].closeButton)
        {
            bodies[i].closeButton->SetTag(i);
        }
        bodies[i].bodyControl->SetTag(i);
    }
    
    if(bodies.size())
    {
        if(tag)
        {
            --tag;
        }

        //set as current
        bodies[tag].headerButton->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
    }
}


void SceneEditorScreenMain::OnLibraryPressed(DAVA::BaseObject *obj, void *, void *)
{
    if(libraryControl->GetParent())
    {
        RemoveControl(libraryControl);
    }
    else
    {
        AddControl(libraryControl);
    }

    int32 iBody = FindCurrentBody();
    if(-1 != iBody)
    {
        bodies[iBody].bodyControl->ShowProperties(false);
        bodies[iBody].bodyControl->UpdateLibraryState(libraryControl->GetParent(), libraryControl->GetRect().dx);
    }
}

int32 SceneEditorScreenMain::FindCurrentBody()
{
    for(int32 i = 0; i < bodies.size(); ++i)
    {
        if(bodies[i].bodyControl->GetParent())
        {
            return i;
        }
    }
    
    return -1;
}

void SceneEditorScreenMain::OnPropertiesPressed(DAVA::BaseObject *obj, void *, void *)
{
    int32 iBody = FindCurrentBody();
    if(-1 != iBody)
    {
        bool areShown = bodies[iBody].bodyControl->PropertiesAreShown();
        bodies[iBody].bodyControl->ShowProperties(!areShown);
        
        if(!areShown)
        {
            if(libraryControl->GetParent())
            {
                RemoveControl(libraryControl);
                bodies[iBody].bodyControl->UpdateLibraryState(false, libraryControl->GetRect().dx);
            }
        }
    }
}

void SceneEditorScreenMain::OnEditSCE(const String &pathName, const String &name)
{
    AddBodyItem(StringToWString(name), true);
    int32 iBody = bodies.size() - 1;
    bodies[iBody].bodyControl->OpenScene(pathName);
}

void SceneEditorScreenMain::OnAddSCE(const String &pathName)
{
    int32 iBody = FindCurrentBody();
    if(-1 != iBody)
    {
        bodies[iBody].bodyControl->OpenScene(pathName);
    }
}

void SceneEditorScreenMain::OnHierarhyPressed(BaseObject * obj, void *, void *)
{
    int32 iBody = FindCurrentBody();
    if(-1 != iBody)
    {
        bool areShown = bodies[iBody].bodyControl->HierarhyAreShown();
        bodies[iBody].bodyControl->ShowHierarhy(!areShown);
    }
}

void SceneEditorScreenMain::OnBeastPressed(BaseObject * obj, void *, void *)
{
	bodies[0].bodyControl->BeastProcessScene();
}
