#include "DAVAEngine.h"
#include "DAVAClassRegistrator.h"
#include "Render/Highlevel/ShadowVolume.h"
#include "Scene3D/Components/PostEffectComponent.h"
#include "Scene3D/Components/PostEffectDebugComponent.h"
#include "Scene3D/Components/LightRenderComponent.h"
#include "Scene3D/Components/DecalComponent.h"
#include "Scene3D/Components/VTDecalComponent.h"
#include "Scene3D/Components/SplineComponent.h"
#include "Scene3D/Components/EnvironmentComponent.h"
#include "Scene3D/Components/PrefabComponent.h"
#include "Scene3D/Components/MeshComponent.h"
#include "Scene3D/Components/LandscapeComponent.h"
#include "Scene3D/Components/LightmapDataComponent.h"
#include "Scene3D/Components/LightmapComponent.h"
#include "Scene3D/Components/StreamingSettingsComponent.h"
#include "Engine/Engine.h"

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
#include <Physics/StaticBodyComponent.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/BoxShapeComponent.h>
#include <Physics/CapsuleShapeComponent.h>
#include <Physics/SphereShapeComponent.h>
#include <Physics/PlaneShapeComponent.h>
#include "Physics/ConvexHullShapeComponent.h"
#include <Physics/MeshShapeComponent.h>
#include <Physics/HeightFieldShapeComponent.h>
#include <Physics/VehicleCarComponent.h>
#include <Physics/VehicleTankComponent.h>
#include <Physics/VehicleChassisComponent.h>
#include <Physics/VehicleWheelComponent.h>
#include <Physics/BoxCharacterControllerComponent.h>
#include <Physics/CapsuleCharacterControllerComponent.h>
#include <Physics/WASDPhysicsControllerComponent.h>
#endif

namespace DAVA
{
void RegisterDAVAClasses()
{
    // this code does nothing. Needed to compiler generate code from this cpp file
    Logger* log = GetEngineContext()->logger;
    if (log)
        log->Log(Logger::LEVEL__DISABLE, "");
}

REGISTER_CLASS(BaseObject);
REGISTER_CLASS(PolygonGroup);
REGISTER_CLASS(Camera);
REGISTER_CLASS(UIScrollViewContainer);
REGISTER_CLASS(UISlider);
REGISTER_CLASS(UISpinner);
REGISTER_CLASS(UIStaticText);
REGISTER_CLASS(UISwitch);
REGISTER_CLASS(UITextField);
REGISTER_CLASS(Landscape);
REGISTER_CLASS(AnimationData);
REGISTER_CLASS(Light);
REGISTER_CLASS(Mesh);
REGISTER_CLASS(SkinnedMesh);
REGISTER_CLASS(SpeedTreeObject);
REGISTER_CLASS(RenderBatch);
REGISTER_CLASS(RenderObject);
REGISTER_CLASS(ShadowVolume);
REGISTER_CLASS(NMaterial);
REGISTER_CLASS(DataNode);
REGISTER_CLASS(Entity);
REGISTER_CLASS(Scene);
REGISTER_CLASS(UIButton);
REGISTER_CLASS(UIControl);
REGISTER_CLASS(UIList);
REGISTER_CLASS(UIListCell);
REGISTER_CLASS(UIScrollBar);
REGISTER_CLASS(UIScrollView);
REGISTER_CLASS_WITH_ALIAS(PartilceEmitterLoadProxy, "ParticleEmitter3D");
REGISTER_CLASS(UIWebView);
REGISTER_CLASS(UIMovieView);
REGISTER_CLASS(UIParticles);
REGISTER_CLASS(UIJoypad);
REGISTER_CLASS(VegetationRenderObject);
REGISTER_CLASS(BillboardRenderObject);
REGISTER_CLASS(SpriteObject);
REGISTER_CLASS(UI3DView);
REGISTER_CLASS(AnimationComponent);
REGISTER_CLASS(TransformComponent);
REGISTER_CLASS(UpdatableComponent);
REGISTER_CLASS(RenderComponent);
REGISTER_CLASS(CustomPropertiesComponent);
REGISTER_CLASS(ActionComponent);
REGISTER_CLASS(DebugRenderComponent);
REGISTER_CLASS(SoundComponent);
REGISTER_CLASS(BulletComponent);
REGISTER_CLASS(LightComponent);
REGISTER_CLASS(SpeedTreeComponent);
REGISTER_CLASS(WindComponent);
REGISTER_CLASS(WaveComponent);
REGISTER_CLASS(QualitySettingsComponent);
REGISTER_CLASS(UserComponent);
REGISTER_CLASS(SwitchComponent);
REGISTER_CLASS(ParticleEffectComponent);
REGISTER_CLASS(CameraComponent);
REGISTER_CLASS(StaticOcclusionComponent);
REGISTER_CLASS(StaticOcclusionDataComponent);
REGISTER_CLASS(PathComponent);
REGISTER_CLASS(WASDControllerComponent);
REGISTER_CLASS(RotationControllerComponent);
REGISTER_CLASS(SnapToLandscapeControllerComponent);
REGISTER_CLASS(ReflectionComponent);
REGISTER_CLASS(PostEffectComponent);
REGISTER_CLASS(PostEffectDebugComponent);
REGISTER_CLASS(LightRenderComponent);
REGISTER_CLASS(DecalComponent);
REGISTER_CLASS(VTDecalComponent);
REGISTER_CLASS(GeoDecalComponent);
REGISTER_CLASS(SplineComponent);
REGISTER_CLASS(SlotComponent);
REGISTER_CLASS(TextComponent);
REGISTER_CLASS(EnvironmentComponent);
REGISTER_CLASS(PrefabComponent);
REGISTER_CLASS(MeshComponent);
REGISTER_CLASS(LandscapeComponent);
REGISTER_CLASS(LightmapDataComponent);
REGISTER_CLASS(LightmapComponent);
REGISTER_CLASS(StreamingSettingsComponent);

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
REGISTER_CLASS(StaticBodyComponent);
REGISTER_CLASS(DynamicBodyComponent);
REGISTER_CLASS(BoxShapeComponent);
REGISTER_CLASS(CapsuleShapeComponent);
REGISTER_CLASS(SphereShapeComponent);
REGISTER_CLASS(PlaneShapeComponent);
REGISTER_CLASS(ConvexHullShapeComponent);
REGISTER_CLASS(MeshShapeComponent);
REGISTER_CLASS(HeightFieldShapeComponent);
REGISTER_CLASS(VehicleCarComponent);
REGISTER_CLASS(VehicleTankComponent);
REGISTER_CLASS(VehicleChassisComponent);
REGISTER_CLASS(VehicleWheelComponent);
REGISTER_CLASS(BoxCharacterControllerComponent);
REGISTER_CLASS(CapsuleCharacterControllerComponent);
REGISTER_CLASS(WASDPhysicsControllerComponent);
#endif
}
