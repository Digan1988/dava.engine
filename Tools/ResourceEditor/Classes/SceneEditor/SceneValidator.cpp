#include "SceneValidator.h"
#include "ErrorNotifier.h"
#include "EditorSettings.h"
#include "SceneInfoControl.h"

#include "Render/LibPVRHelper.h"
#include "Render/TextureDescriptor.h"

#include "../Qt/QtUtils.h"
#include "../Qt/SceneDataManager.h"
#include "../Qt/SceneData.h"
#include "../EditorScene.h"

#include "PVRConverter.h"


SceneValidator::SceneValidator()
{
    sceneTextureCount = 0;
    sceneTextureMemory = 0;

    infoControl = NULL;
    
    pathForChecking = String("");
}

SceneValidator::~SceneValidator()
{
    SafeRelease(infoControl);
}

bool SceneValidator::ValidateSceneAndShowErrors(Scene *scene)
{
    errorMessages.clear();

    ValidateScene(scene, errorMessages);

    if(0 < errorMessages.size())
    {
        ShowErrors();
        return true;
    }
    
    return false;
}


void SceneValidator::ValidateScene(Scene *scene, Set<String> &errorsLog)
{
    if(scene) 
    {
        ValidateSceneNode(scene, errorsLog);
        ValidateLodNodes(scene, errorsLog);
        
        for (Set<SceneNode*>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
        {
            SceneNode * node = *it;
            if (node->GetParent())
            {
                node->GetParent()->RemoveNode(node);
            }
        }
        

		for (Set<SceneNode *>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
		{
			SceneNode *node = *it;
			SafeRelease(node);
		}

        emptyNodesForDeletion.clear();
    }
    else 
    {
        errorsLog.insert(String("Scene in NULL!"));
    }
}

void SceneValidator::ValidateScales(Scene *scene, Set<String> &errorsLog)
{
	if(scene) 
	{
		ValidateScalesInternal(scene, errorsLog);
	}
	else 
	{
		errorsLog.insert(String("Scene in NULL!"));
	}
}

void SceneValidator::ValidateScalesInternal(SceneNode *sceneNode, Set<String> &errorsLog)
{
//  Basic algorithm is here
// 	Matrix4 S, T, R; //Scale Transpose Rotation
// 	S.CreateScale(Vector3(1.5, 0.5, 2.0));
// 	T.CreateTranslation(Vector3(100, 50, 20));
// 	R.CreateRotation(Vector3(0, 1, 0), 2.0);
// 
// 	Matrix4 t = R*S*T; //Calculate complex matrix
// 
//	//Calculate Scale components from complex matrix
// 	float32 sx = sqrt(t._00 * t._00 + t._10 * t._10 + t._20 * t._20);
// 	float32 sy = sqrt(t._01 * t._01 + t._11 * t._11 + t._21 * t._21);
// 	float32 sz = sqrt(t._02 * t._02 + t._12 * t._12 + t._22 * t._22);
// 	Vector3 sCalculated(sx, sy, sz);

	if(!sceneNode) return;

	const Matrix4 & t = sceneNode->GetLocalTransform();
	float32 sx = sqrt(t._00 * t._00 + t._10 * t._10 + t._20 * t._20);
	float32 sy = sqrt(t._01 * t._01 + t._11 * t._11 + t._21 * t._21);
	float32 sz = sqrt(t._02 * t._02 + t._12 * t._12 + t._22 * t._22);

	if ((!FLOAT_EQUAL(sx, 1.0f)) 
		|| (!FLOAT_EQUAL(sy, 1.0f))
		|| (!FLOAT_EQUAL(sz, 1.0f)))
	{
 		errorsLog.insert(Format("Node %s: has scale (%.3f, %.3f, %.3f) ! Re-design level.", sceneNode->GetName().c_str(), sx, sy, sz));
	}

	int32 count = sceneNode->GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		ValidateScalesInternal(sceneNode->GetChild(i), errorsLog);
	}
}


void SceneValidator::ValidateSceneNode(SceneNode *sceneNode, Set<String> &errorsLog)
{
    if(!sceneNode) return;
    
    int32 count = sceneNode->GetChildrenCount();
    for(int32 i = 0; i < count; ++i)
    {
        SceneNode *node = sceneNode->GetChild(i);
        MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode*>(node);
        if(mesh)
        {
            ValidateMeshInstance(mesh, errorsLog);
        }
        else 
        {
            LandscapeNode *landscape = dynamic_cast<LandscapeNode*>(node);
            if (landscape) 
            {
                ValidateLandscape(landscape, errorsLog);
            }
            else
            {
                ValidateSceneNode(node, errorsLog);
            }
        }
        
        KeyedArchive *customProperties = node->GetCustomProperties();
        if(customProperties->IsKeyExists("editor.referenceToOwner"))
        {
            String dataSourcePath = EditorSettings::Instance()->GetDataSourcePath();
            if(1 < dataSourcePath.length())
            {
                if('/' == dataSourcePath[0])
                {
                    dataSourcePath = dataSourcePath.substr(1);
                }
                
                String referencePath = customProperties->GetString("editor.referenceToOwner");
                String::size_type pos = referencePath.rfind(dataSourcePath);
                if((String::npos != pos) && (1 != pos))
                {
                    referencePath.replace(pos, dataSourcePath.length(), "");
                    customProperties->SetString("editor.referenceToOwner", referencePath);
                    
                    errorsLog.insert(Format("Node %s: referenceToOwner isn't correct. Re-save level.", node->GetName().c_str()));
                }
            }
        }
    }
    
    if(typeid(SceneNode) == typeid(*sceneNode))
    {
        Set<DataNode*> dataNodeSet;
        sceneNode->GetDataNodes(dataNodeSet);
        if (dataNodeSet.size() == 0)
        {
            if(NodeRemovingDisabled(sceneNode))
            {
                return;
            }
            
            SceneNode * parent = sceneNode->GetParent();
            if (parent)
            {
                emptyNodesForDeletion.insert(SafeRetain(sceneNode));
            }
        }
    }
}

bool SceneValidator::NodeRemovingDisabled(SceneNode *node)
{
    KeyedArchive *customProperties = node->GetCustomProperties();
    return (customProperties && customProperties->IsKeyExists("editor.donotremove"));
}


void SceneValidator::ValidateTextureAndShowErrors(Texture *texture)
{
    errorMessages.clear();

    ValidateTexture(texture, errorMessages);
    ShowErrors();
}

void SceneValidator::ValidateTexture(Texture *texture, Set<String> &errorsLog)
{
	if(!texture) return;

	bool pathIsCorrect = ValidatePathname(texture->GetPathname());
	if(!pathIsCorrect)
	{
		String path = FileSystem::AbsoluteToRelativePath(EditorSettings::Instance()->GetDataSourcePath(), texture->GetPathname());
		errorsLog.insert("Wrong path of: " + path);
	}
	
	if(!IsPowerOf2(texture->GetWidth()) || !IsPowerOf2(texture->GetHeight()))
	{
		String path = FileSystem::AbsoluteToRelativePath(EditorSettings::Instance()->GetDataSourcePath(), texture->GetPathname());
		errorsLog.insert("Wrong size of " + path);
	}
    
	// if there is no descriptor file for this texture - generate it
	if(pathIsCorrect && !IsFBOTexture(texture))
	{
		CreateDescriptorIfNeed(texture->GetPathname());
	}
}


void SceneValidator::ValidateLandscape(LandscapeNode *landscape, Set<String> &errorsLog)
{
    if(!landscape) return;
    
    for(int32 i = 0; i < LandscapeNode::TEXTURE_COUNT; ++i)
    {
        if(LandscapeNode::TEXTURE_DETAIL == (LandscapeNode::eTextureLevel)i)
        {
            continue;
        }

		// TODO:
		// new texture path
		DAVA::String landTexName = landscape->GetTextureName((LandscapeNode::eTextureLevel)i);
		if(!IsTextureDescriptorPath(landTexName))
		{
			landscape->SetTextureName((LandscapeNode::eTextureLevel)i, TextureDescriptor::GetDescriptorPathname(landTexName));
		}
        
        ValidateTexture(landscape->GetTexture((LandscapeNode::eTextureLevel)i), errorsLog);
    }
    
    bool pathIsCorrect = ValidatePathname(landscape->GetHeightmapPathname());
    if(!pathIsCorrect)
    {
        String path = FileSystem::AbsoluteToRelativePath(EditorSettings::Instance()->GetDataSourcePath(), landscape->GetHeightmapPathname());
        errorsLog.insert("Wrong path of Heightmap: " + path);
    }
}

void SceneValidator::ShowErrors()
{
    if(0 < errorMessages.size())
    {
        ErrorNotifier::Instance()->ShowError(errorMessages);
    }
}

void SceneValidator::ValidateMeshInstance(MeshInstanceNode *meshNode, Set<String> &errorsLog)
{
    meshNode->RemoveFlag(SceneNode::NODE_INVALID);
    
    const Vector<PolygonGroupWithMaterial*> & polygroups = meshNode->GetPolygonGroups();
    //Vector<Material *>materials = meshNode->GetMaterials();
    for(int32 iMat = 0; iMat < (int32)polygroups.size(); ++iMat)
    {
        Material * material = polygroups[iMat]->GetMaterial();

        ValidateMaterial(material, errorsLog);

        if (material->Validate(polygroups[iMat]->GetPolygonGroup()) == Material::VALIDATE_INCOMPATIBLE)
        {
            meshNode->AddFlag(SceneNode::NODE_INVALID);
            errorsLog.insert(Format("Material: %s incompatible with node:%s.", material->GetName().c_str(), meshNode->GetFullName().c_str()));
            errorsLog.insert("For lightmapped objects check second coordinate set. For normalmapped check tangents, binormals.");
        }
    }
    
    int32 lightmapCont = meshNode->GetLightmapCount();
    for(int32 iLight = 0; iLight < lightmapCont; ++iLight)
    {
        DAVA::String lightmapName = meshNode->GetLightmapDataForIndex(iLight)->lightmapName;
		if(!IsTextureDescriptorPath(lightmapName))
		{
            meshNode->GetLightmapDataForIndex(iLight)->lightmapName = TextureDescriptor::GetDescriptorPathname(lightmapName);
		}
        
        ValidateTexture(meshNode->GetLightmapDataForIndex(iLight)->lightmap, errorsLog);
    }
}


void SceneValidator::ValidateMaterial(Material *material, Set<String> &errorsLog)
{
    for(int32 iTex = 0; iTex < Material::TEXTURE_COUNT; ++iTex)
    {
        Texture *texture = material->GetTexture((Material::eTextureLevel)iTex);
        if(texture)
        {
            ValidateTexture(texture, errorsLog);
            
            // TODO:
            // new texture path
            String matTexName = material->GetTextureName((Material::eTextureLevel)iTex);
            if(!IsTextureDescriptorPath(matTexName))
            {
                material->SetTexture((Material::eTextureLevel)iTex, TextureDescriptor::GetDescriptorPathname(matTexName));
            }
            
            /*
             if(material->GetTextureName((Material::eTextureLevel)iTex).find(".pvr.png") != String::npos)
             {
             errorsLog.insert(material->GetName() + ": wrong texture name " + material->GetTextureName((Material::eTextureLevel)iTex));
             }
             */
        }
    }
}

void SceneValidator::EnumerateSceneTextures()
{
    sceneTextureCount = 0;
    sceneTextureMemory = 0;
    
    const Map<String, Texture*> textureMap = Texture::GetTextureMap();
    KeyedArchive *settings = EditorSettings::Instance()->GetSettings(); 
    String projectPath = settings->GetString("ProjectPath");
	for(Map<String, Texture *>::const_iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *t = it->second;
        if(String::npos != t->GetPathname().find(projectPath))
        {
            String::size_type pvrPos = t->GetPathname().find(".pvr");
            if(String::npos != pvrPos)
            {   //We need real info about textures size. In Editor on desktop pvr textures are decompressed to RGBA8888, so they have not real size.
                sceneTextureMemory += LibPVRHelper::GetDataLength(t->GetPathname());
            }
            else 
            {
                sceneTextureMemory += t->GetDataSize();
            }
            
            ++sceneTextureCount;
        }
	}
    
    if(infoControl)
    {
        infoControl->InvalidateTexturesInfo(sceneTextureCount, sceneTextureMemory);
    }
}

void SceneValidator::SetInfoControl(SceneInfoControl *newInfoControl)
{
    SafeRelease(infoControl);
    infoControl = SafeRetain(newInfoControl);
    
    sceneStats.Clear();
}

void SceneValidator::CollectSceneStats(const RenderManager::Stats &newStats)
{
    sceneStats = newStats;
    infoControl->SetRenderStats(sceneStats);
}


void SceneValidator::ReloadTextures(int32 asFile)
{
    Map<String, Texture *> textures;
    
    //Enumerate textures
    int32 count = SceneDataManager::Instance()->ScenesCount();
    for(int32 i = 0; i < count; ++i)
    {
        SceneData *sceneData = SceneDataManager::Instance()->GetScene(i);
        EnumerateTextures(textures, sceneData->GetScene());
    }

    //Reload textures
    for(Map<String, Texture *>::iterator it = textures.begin(); it != textures.end(); ++it)
    {
        SafeRetain(it->second);
    }
    
    
	for(Map<String, Texture *>::iterator it = textures.begin(); it != textures.end(); )
	{
        Texture *texture = ReloadTexture(it->first, it->second, asFile);
        if(texture == it->second)
        {
            SafeRelease(it->second);
            textures.erase(it++);
        }
        else
        {
            SafeRelease(it->second);
            it->second = texture;
            ++it;
        }
	}
    
    //reload landscape full tiled texures, which were created as FBO
    for(int32 i = 0; i < count; ++i)
    {
        SceneData *sceneData = SceneDataManager::Instance()->GetScene(i);
        EditorScene *scene = sceneData->GetScene();
        
        RestoreTextures(textures, scene);

        LandscapeNode *landscape = scene->GetLandScape(scene);
        if(landscape)
        {
            Texture *fullTiled = landscape->GetTexture(LandscapeNode::TEXTURE_TILE_FULL);
            if(fullTiled && fullTiled->isRenderTarget)
            {
                landscape->UpdateFullTiledTexture();
            }
        }
    }
    
    //Release textures
    for(Map<String, Texture *>::iterator it = textures.begin(); it != textures.end(); ++it)
    {
        SafeRelease(it->second);
    }

}

Texture * SceneValidator::ReloadTexture(const String &descriptorPathname, Texture *prevTexture, int32 asFile)
{
    if(prevTexture == Texture::GetPinkPlaceholder())
    {
        return Texture::CreateFromFile(descriptorPathname);
    }

    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(descriptorPathname);
    if(descriptor)
    {
        prevTexture->ReloadAs((ImageFileFormat)asFile, descriptor);
        SafeRelease(descriptor);
    }

    return prevTexture;
}



void SceneValidator::FindTexturesForCompression()
{
    Map<String, Texture *> textures;
    
    //Enumerate textures
    int32 count = SceneDataManager::Instance()->ScenesCount();
    for(int32 i = 0; i < count; ++i)
    {
        SceneData *sceneData = SceneDataManager::Instance()->GetScene(i);
        EnumerateTextures(textures, sceneData->GetScene());
    }

    List<Texture *>texturesForPVRCompression;
    List<Texture *>texturesForDXTCompression;

    Map<String, Texture *>::const_iterator endIt = textures.end();
	for(Map<String, Texture *>::const_iterator it = textures.begin(); it != endIt; ++it)
	{
        if(IsTextureChanged(it->first, PVR_FILE))
        {
            texturesForPVRCompression.push_back(SafeRetain(it->second));
        }
        else if(IsTextureChanged(it->first, DXT_FILE))
        {
            texturesForDXTCompression.push_back(SafeRetain(it->second));
        }
	}

    CompressTextures(texturesForPVRCompression, PVR_FILE);
    CompressTextures(texturesForDXTCompression, DXT_FILE);
    
	for_each(texturesForPVRCompression.begin(), texturesForPVRCompression.end(),  SafeRelease<Texture>);
	for_each(texturesForDXTCompression.begin(), texturesForDXTCompression.end(),  SafeRelease<Texture>);
}

void SceneValidator::CompressTextures(const List<DAVA::Texture *> texturesForCompression, DAVA::ImageFileFormat fileFormat)
{
    List<Texture *>::const_iterator endIt = texturesForCompression.end();
	for(List<Texture *>::const_iterator it = texturesForCompression.begin(); it != endIt; ++it)
	{
		Texture *texture = *it;
        //TODO: compress texture
        TextureDescriptor *descriptor = texture->CreateDescriptor();
        if(descriptor)
        {
            if(fileFormat == PVR_FILE)
            {
                PVRConverter::Instance()->ConvertPngToPvr(descriptor->GetSourceTexturePathname(), *descriptor);
            }
            else if(fileFormat == DXT_FILE)
            {
                    
            }
            
			descriptor->UpdateDateAndCrcForFormat(fileFormat);
			descriptor->Save();
            SafeRelease(descriptor);
        }
	}
}


bool SceneValidator::WasTextureChanged(Texture *texture, ImageFileFormat fileFormat)
{
    if(IsFBOTexture(texture))
    {
        return false;
    }
    
    String texturePathname = texture->GetPathname();
    return (IsPathCorrectForProject(texturePathname) && IsTextureChanged(texturePathname, fileFormat));
}

bool SceneValidator::IsFBOTexture(Texture *texture)
{
    if(texture->isRenderTarget)
    {
        return true;
    }

    String::size_type textTexturePos = texture->GetPathname().find("Text texture");
    if(String::npos != textTexturePos)
    {
        return true; //is text texture
    }
    
    return false;
}


void SceneValidator::ValidateLodNodes(Scene *scene, Set<String> &errorsLog)
{
    Vector<LodNode *> lodnodes;
    scene->GetChildNodes(lodnodes); 
    
    for(int32 index = 0; index < (int32)lodnodes.size(); ++index)
    {
        LodNode *ln = lodnodes[index];
        
        int32 layersCount = ln->GetLodLayersCount();
        for(int32 layer = 0; layer < layersCount; ++layer)
        {
            float32 distance = ln->GetLodLayerDistance(layer);
            if(LodNode::INVALID_DISTANCE == distance)
            {
                ln->SetLodLayerDistance(layer, ln->GetDefaultDistance(layer));
                errorsLog.insert(Format("Node %s: lod distances weren't correct. Re-save.", ln->GetName().c_str()));
            }
        }
        
        List<LodNode::LodData *>lodLayers;
        ln->GetLodData(lodLayers);
        
        List<LodNode::LodData *>::const_iterator endIt = lodLayers.end();
        int32 layer = 0;
        for(List<LodNode::LodData *>::iterator it = lodLayers.begin(); it != endIt; ++it, ++layer)
        {
            LodNode::LodData * ld = *it;
            
            if(ld->layer != layer)
            {
                ld->layer = layer;

                errorsLog.insert(Format("Node %s: lod layers weren't correct. Rename childs. Re-save.", ln->GetName().c_str()));
            }
        }
    }
}

String SceneValidator::SetPathForChecking(const String &pathname)
{
    String oldPath = pathForChecking;
    pathForChecking = pathname;
    return oldPath;
}


bool SceneValidator::ValidateTexturePathname(const String &pathForValidation, Set<String> &errorsLog)
{
	DVASSERT(!pathForChecking.empty() && "Need to set pathname for DataSource folder");

	bool pathIsCorrect = IsPathCorrectForProject(pathForValidation);
	if(pathIsCorrect)
	{
		String textureExtension = FileSystem::Instance()->GetExtension(pathForValidation);
		String::size_type extPosition = TextureDescriptor::GetSupportedTextureExtensions().find(textureExtension);
		if(String::npos == extPosition)
		{
			errorsLog.insert(Format("Path %s has incorrect extension", pathForValidation.c_str()));
			return false;
		}

		CreateDescriptorIfNeed(pathForValidation);
	}
	else
	{
		errorsLog.insert(Format("Path %s is incorrect for project %s", pathForValidation.c_str(), pathForChecking.c_str()));
	}

	return pathIsCorrect;
}

bool SceneValidator::ValidateHeightmapPathname(const String &pathForValidation, Set<String> &errorsLog)
{
	DVASSERT(!pathForChecking.empty() && "Need to set pathname for DataSource folder");

	bool pathIsCorrect = IsPathCorrectForProject(pathForValidation);
	if(pathIsCorrect)
	{
		String::size_type posPng = pathForValidation.find(".png");
		String::size_type posHeightmap = pathForValidation.find(Heightmap::FileExtension());
        
        pathIsCorrect = ((String::npos != posPng) || (String::npos != posHeightmap));
        if(!pathIsCorrect)
        {
            errorsLog.insert(Format("Heightmap path %s is wrong", pathForValidation.c_str()));
            return false;
        }
        
        Heightmap *heightmap = new Heightmap();
        if(String::npos != posPng)
        {
            Image *image = CreateTopLevelImage(pathForValidation);
            pathIsCorrect = heightmap->BuildFromImage(image);
            SafeRelease(image);
        }
        else
        {
            pathIsCorrect = heightmap->Load(pathForValidation);
        }

        
        if(!pathIsCorrect)
        {
            SafeRelease(heightmap);
            errorsLog.insert(Format("Can't load Heightmap from path %s", pathForValidation.c_str()));
            return false;
        }
        
        
        pathIsCorrect = IsPowerOf2(heightmap->Size() - 1);
        if(!pathIsCorrect)
        {
            errorsLog.insert(Format("Heightmap %s has wrong size", pathForValidation.c_str()));
        }
        
        SafeRelease(heightmap);
		return pathIsCorrect;
	}
	else
	{
		errorsLog.insert(Format("Path %s is incorrect for project %s", pathForValidation.c_str(), pathForChecking.c_str()));
	}

	return pathIsCorrect;
}


void SceneValidator::CreateDescriptorIfNeed(const String &forPathname)
{
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(forPathname);
    if(!descriptor)
    {
		Logger::Warning("[SceneValidator::CreateDescriptorIfNeed] Need descriptor for file %s", forPathname.c_str());

		descriptor = new TextureDescriptor();
		descriptor->textureFileFormat = PNG_FILE;
        
        String descriptorPathname = TextureDescriptor::GetDescriptorPathname(forPathname);
		descriptor->Save(descriptorPathname);
    }
    
    SafeRelease(descriptor);
    
//	String descriptorPathname = TextureDescriptor::GetDescriptorPathname(forPathname);
//	bool fileExists = FileSystem::Instance()->IsFile(descriptorPathname);
//	if(!fileExists)
//	{
//		Logger::Warning("[SceneValidator::CreateDescriptorIfNeed] Need descriptor for file %s", forPathname.c_str());
//	
//		TextureDescriptor *descriptor = new TextureDescriptor();
//		descriptor->textureFileFormat = PNG_FILE;
//		descriptor->Save(descriptorPathname);
//	}
}


#include "FuckingErrorDialog.h"
bool SceneValidator::ValidatePathname(const String &pathForValidation)
{
    DVASSERT(0 < pathForChecking.length()); 
    //Need to set path to DataSource/3d for path correction  
    //Use SetPathForChecking();
    
    String pathname = FileSystem::GetCanonicalPath(pathForValidation);
    
    String::size_type fboFound = pathname.find(String("FBO"));
    String::size_type resFound = pathname.find(String("~res:"));
    if((String::npos != fboFound) || (String::npos != resFound))
    {
        return true;   
    }
    
    bool pathIsCorrect = IsPathCorrectForProject(pathForValidation);
    if(!pathIsCorrect)
    {
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        
        FuckingErrorDialog *dlg = new FuckingErrorDialog(screen->GetRect(), String("Wrong path: ") + pathForValidation);
        screen->AddControl(dlg);
        SafeRelease(dlg);
    }
    
    return pathIsCorrect;
}

bool SceneValidator::IsPathCorrectForProject(const String &pathname)
{
    String normalizedPath = FileSystem::GetCanonicalPath(pathname);
    String::size_type foundPos = normalizedPath.find(pathForChecking);
    return (String::npos != foundPos);
}


void SceneValidator::EnumerateNodes(DAVA::Scene *scene)
{
    int32 nodesCount = 0;
    if(scene)
    {
        for(int32 i = 0; i < scene->GetChildrenCount(); ++i)
        {
            nodesCount += EnumerateSceneNodes(scene->GetChild(i));
        }
    }
    
    if(infoControl)
        infoControl->SetNodesCount(nodesCount);
}

int32 SceneValidator::EnumerateSceneNodes(DAVA::SceneNode *node)
{
    //TODO: lode node can have several nodes at layer
    
    int32 nodesCount = 1;
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        nodesCount += EnumerateSceneNodes(node->GetChild(i));
    }
    
    return nodesCount;
}


bool SceneValidator::IsTextureChanged(const String &texturePathname, ImageFileFormat fileFormat)
{
    bool isChanged = false;
    
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(texturePathname);
    if(descriptor)
    {
        isChanged = descriptor->IsSourceValidForFormat(fileFormat);
        SafeRelease(descriptor);
    }

    return isChanged;
}

bool SceneValidator::IsTextureDescriptorPath(const String &path)
{
	String ext = FileSystem::GetExtension(path);
	return (ext == TextureDescriptor::GetDescriptorExtension());
}



void SceneValidator::CreateDefaultDescriptors(const String &folderPathname)
{
	FileList * fileList = new FileList(folderPathname);
	for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (fileList->IsDirectory(fi))
		{
            if(0 != CompareCaseInsensitive(String(".svn"), fileList->GetFilename(fi))
               && 0 != CompareCaseInsensitive(String("."), fileList->GetFilename(fi))
                && 0 != CompareCaseInsensitive(String(".."), fileList->GetFilename(fi)))
            {
                CreateDefaultDescriptors(fileList->GetPathname(fi));
            }
		}
        else
        {
            const String pathname = fileList->GetPathname(fi);
            const String extension = FileSystem::Instance()->GetExtension(pathname);
            if(0 == CompareCaseInsensitive(String(".png"), extension))
            {
                CreateDescriptorIfNeed(pathname);
            }
        }
	}

	SafeRelease(fileList);
}

void SceneValidator::EnumerateTextures(Map<String, Texture *> &textures, Scene *scene)
{
    if(!scene)  return;
    
    //materials
    Vector<Material*> materials;
    scene->GetDataNodes(materials);
    for(int32 m = 0; m < (int32)materials.size(); ++m)
    {
        for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
        {
            String path = materials[m]->GetTextureName((Material::eTextureLevel)t);
            if(!path.empty() && IsPathCorrectForProject(path))
            {
                textures[path] = materials[m]->GetTexture((Material::eTextureLevel)t);
            }
        }
    }
    
    //landscapes
    Vector<LandscapeNode *> landscapes;
    scene->GetChildNodes(landscapes);
    for(int32 l = 0; l < (int32)landscapes.size(); ++l)
    {
        for(int32 t = 0; t < LandscapeNode::TEXTURE_COUNT; t++)
        {
            String path = landscapes[l]->GetTextureName((LandscapeNode::eTextureLevel)t);
            if(!path.empty() && IsPathCorrectForProject(path))
            {
                textures[path] = landscapes[l]->GetTexture((LandscapeNode::eTextureLevel)t);
            }
        }
    }
    
    //lightmaps
    Vector<MeshInstanceNode *> meshInstances;
    scene->GetChildNodes(meshInstances);
    for(int32 m = 0; m < (int32)meshInstances.size(); ++m)
    {
        for (int32 li = 0; li < meshInstances[m]->GetLightmapCount(); ++li)
        {
            MeshInstanceNode::LightmapData * ld = meshInstances[m]->GetLightmapDataForIndex(li);
            if (ld)
            {
                if(!ld->lightmapName.empty() && IsPathCorrectForProject(ld->lightmapName))
                {
                    textures[ld->lightmapName] = ld->lightmap;
                }
            }
        }
    }
}

void SceneValidator::RestoreTextures(Map<String, Texture *> &textures, Scene *scene)
{
    if(!scene)  return;
    
    //materials
    Vector<Material*> materials;
    scene->GetDataNodes(materials);
    for(int32 m = 0; m < (int32)materials.size(); ++m)
    {
        for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
        {
            String path = materials[m]->GetTextureName((Material::eTextureLevel)t);
            Map<String, Texture *>::iterator it = textures.find(path);
            if(it != textures.end())
            {
                materials[m]->SetTexture((Material::eTextureLevel)t, path);
            }
        }
    }
    
    //landscapes
    Vector<LandscapeNode *> landscapes;
    scene->GetChildNodes(landscapes);
    for(int32 l = 0; l < (int32)landscapes.size(); ++l)
    {
        for(int32 t = 0; t < LandscapeNode::TEXTURE_COUNT; t++)
        {
            String path = landscapes[l]->GetTextureName((LandscapeNode::eTextureLevel)t);
            Map<String, Texture *>::iterator it = textures.find(path);
            if(it != textures.end())
            {
                landscapes[l]->SetTexture((LandscapeNode::eTextureLevel)t, it->second);
            }
        }
    }
    
    //lightmaps
    Vector<MeshInstanceNode *> meshInstances;
    scene->GetChildNodes(meshInstances);
    for(int32 m = 0; m < (int32)meshInstances.size(); ++m)
    {
        for (int32 li = 0; li < meshInstances[m]->GetLightmapCount(); ++li)
        {
            MeshInstanceNode::LightmapData * ld = meshInstances[m]->GetLightmapDataForIndex(li);
            if (ld)
            {
                Map<String, Texture *>::iterator it = textures.find(ld->lightmapName);
                if(it != textures.end())
                {
                    SafeRelease(ld->lightmap);
                    ld->lightmap = SafeRetain(it->second);
                }
            }
        }
    }
}

