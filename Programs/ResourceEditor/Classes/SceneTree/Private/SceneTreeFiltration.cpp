#include "Classes/SceneTree/SceneTreeFiltration.h"
#include "Classes/Selection/SelectionData.h"

#include <Entity/Component.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>

bool SceneTreeFilterBase::IsEnabled() const
{
    return isEnabled;
}

void SceneTreeFilterBase::SetEnabled(bool isEnabled_)
{
    if (isEnabled != isEnabled_)
    {
        isEnabled = isEnabled_;
        changed.Emit();
    }
}

bool SceneTreeFilterBase::IsInverted() const
{
    return isInverted;
}

void SceneTreeFilterBase::SetInverted(bool isInverted_)
{
    if (isInverted != isInverted_)
    {
        isInverted = isInverted_;
        changed.Emit();
    }
}

const char* SceneTreeFilterBase::titleFieldName = "title";
const char* SceneTreeFilterBase::enabledFieldName = "enabled";
const char* SceneTreeFilterBase::inversedFieldName = "inverted";

DAVA_VIRTUAL_REFLECTION_IMPL(SceneTreeFilterBase)
{
    DAVA::ReflectionRegistrator<SceneTreeFilterBase>::Begin()
    .Field(titleFieldName, &SceneTreeFilterBase::GetTitle, nullptr)
    .Field(enabledFieldName, &SceneTreeFilterBase::IsEnabled, &SceneTreeFilterBase::SetEnabled)
    .Field(inversedFieldName, &SceneTreeFilterBase::IsInverted, &SceneTreeFilterBase::SetInverted)
    .End();
}

class EmptyEntityFilter : public SceneTreeFilterBase
{
public:
    QString GetTitle() const override
    {
        return QStringLiteral("Empty entity");
    }

    bool IsMatched(const Selectable& object, DAVA::TArc::ContextAccessor* /*accessor*/) const override
    {
        if (object.CanBeCastedTo<DAVA::Entity>() == false)
        {
            return false;
        }

        DAVA::Entity* entity = object.Cast<DAVA::Entity>();
        return entity->GetComponentCount() == 0;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(EmptyEntityFilter, SceneTreeFilterBase)
    {
        DAVA::ReflectionRegistrator<EmptyEntityFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Empty entity")]
        .ConstructorByPointer()
        .DestructorByPointer()
        .End();
    }
};

template <DAVA::Component::eType ComponentType>
class ComponentFilter : public SceneTreeFilterBase
{
public:
    ComponentFilter()
    {
        const DAVA::ReflectedType* type = DAVA::ReflectedTypeDB::GetByPointer(this);
        const DAVA::M::DisplayName* displayName = DAVA::TArc::GetReflectedTypeMeta<DAVA::M::DisplayName>(type);
        if (displayName != nullptr)
        {
            title = QString::fromStdString(displayName->displayName);
        }
        else
        {
            title = QString::fromStdString(type->GetPermanentName());
        }
    }

    QString GetTitle() const override
    {
        return title;
    }

    bool IsMatched(const Selectable& object, DAVA::TArc::ContextAccessor* /*accessor*/) const override
    {
        if (object.CanBeCastedTo<DAVA::Entity>() == false)
        {
            return false;
        }

        DAVA::Entity* entity = object.Cast<DAVA::Entity>();
        return DAVA::HasComponent(entity, ComponentType);
    }

private:
    QString title;

    DAVA_VIRTUAL_REFLECTION(ComponentFilter, SceneTreeFilterBase);
};

using LightComponentFilter = ComponentFilter<DAVA::Component::LIGHT_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(LightComponentFilter)
{
    DAVA::ReflectionRegistrator<LightComponentFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Light")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using CameraComponentFilter = ComponentFilter<DAVA::Component::CAMERA_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(CameraComponentFilter)
{
    DAVA::ReflectionRegistrator<CameraComponentFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Camera")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using UserNodeComponentFilter = ComponentFilter<DAVA::Component::USER_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(UserNodeComponentFilter)
{
    DAVA::ReflectionRegistrator<UserNodeComponentFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("User Node")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using SwitchNodeComponentFilter = ComponentFilter<DAVA::Component::SWITCH_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(SwitchNodeComponentFilter)
{
    DAVA::ReflectionRegistrator<SwitchNodeComponentFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Switch")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using ParticleEffectFilter = ComponentFilter<DAVA::Component::PARTICLE_EFFECT_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(ParticleEffectFilter)
{
    DAVA::ReflectionRegistrator<ParticleEffectFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Particle Effect Node")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using WindFilter = ComponentFilter<DAVA::Component::WIND_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(WindFilter)
{
    DAVA::ReflectionRegistrator<WindFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Wind")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using PathFilter = ComponentFilter<DAVA::Component::PATH_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(PathFilter)
{
    DAVA::ReflectionRegistrator<PathFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Path")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

class ParticleEmitterFilter : public SceneTreeFilterBase
{
public:
    bool IsMatched(const Selectable& object, DAVA::TArc::ContextAccessor* accessor) const override
    {
        return object.CanBeCastedTo<DAVA::ParticleEmitterInstance>();
    }

    QString GetTitle() const override
    {
        return QStringLiteral("Particle Emitter");
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ParticleEmitterFilter, SceneTreeFilterBase)
    {
        DAVA::ReflectionRegistrator<ParticleEmitterFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Particle Emitter")]
        .ConstructorByPointer()
        .DestructorByPointer()
        .End();
    }
};

class ParticleLayerFilter : public SceneTreeFilterBase
{
public:
    bool IsMatched(const Selectable& object, DAVA::TArc::ContextAccessor* accessor) const override
    {
        return object.CanBeCastedTo<DAVA::ParticleLayer>();
    }

    QString GetTitle() const override
    {
        return QStringLiteral("Particle Layer");
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ParticleLayerFilter, SceneTreeFilterBase)
    {
        DAVA::ReflectionRegistrator<ParticleLayerFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Particle Layer")]
        .ConstructorByPointer()
        .DestructorByPointer()
        .End();
    }
};

class ParticleForceFilter : public SceneTreeFilterBase
{
public:
    bool IsMatched(const Selectable& object, DAVA::TArc::ContextAccessor* accessor) const override
    {
        return object.CanBeCastedTo<DAVA::ParticleForce>();
    }

    QString GetTitle() const override
    {
        return QStringLiteral("Particle Force");
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ParticleForceFilter, SceneTreeFilterBase)
    {
        DAVA::ReflectionRegistrator<ParticleForceFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Particle Force")]
        .ConstructorByPointer()
        .DestructorByPointer()
        .End();
    }
};

class LandscapeFilter : public SceneTreeFilterBase
{
public:
    bool IsMatched(const Selectable& object, DAVA::TArc::ContextAccessor* accessor) const override
    {
        if (object.CanBeCastedTo<DAVA::Entity>() == false)
        {
            return false;
        }

        DAVA::Entity* entity = object.Cast<DAVA::Entity>();
        DAVA::RenderObject* ro = DAVA::GetRenderObject(entity);
        if (ro == nullptr)
        {
            return false;
        }

        return ro->GetType() == DAVA::RenderObject::TYPE_LANDSCAPE;
    }

    QString GetTitle() const override
    {
        return QStringLiteral("Landscape");
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(LandscapeFilter, SceneTreeFilterBase)
    {
        DAVA::ReflectionRegistrator<LandscapeFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Landscape")]
        .ConstructorByPointer()
        .DestructorByPointer()
        .End();
    }
};

class VegetationFilter : public SceneTreeFilterBase
{
public:
    bool IsMatched(const Selectable& object, DAVA::TArc::ContextAccessor* accessor) const override
    {
        if (object.CanBeCastedTo<DAVA::Entity>() == false)
        {
            return false;
        }

        DAVA::Entity* entity = object.Cast<DAVA::Entity>();
        DAVA::RenderObject* ro = DAVA::GetRenderObject(entity);
        if (ro == nullptr)
        {
            return false;
        }

        return ro->GetType() == DAVA::RenderObject::TYPE_VEGETATION;
    }

    QString GetTitle() const override
    {
        return QStringLiteral("Vegetation");
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(VegetationFilter, SceneTreeFilterBase)
    {
        DAVA::ReflectionRegistrator<VegetationFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Vegetation")]
        .ConstructorByPointer()
        .DestructorByPointer()
        .End();
    }
};

class EditorSpriteFilter : public SceneTreeFilterBase
{
public:
    bool IsMatched(const Selectable& object, DAVA::TArc::ContextAccessor* accessor) const override
    {
        if (object.CanBeCastedTo<DAVA::Entity>() == false)
        {
            return false;
        }

        DAVA::Entity* entity = object.Cast<DAVA::Entity>();
        DAVA::RenderObject* ro = DAVA::GetRenderObject(entity);
        if (ro == nullptr)
        {
            return false;
        }

        return ro->GetType() == DAVA::RenderObject::TYPE_SPRITE;
    }

    QString GetTitle() const override
    {
        return QStringLiteral("Editor Sprite");
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(EditorSpriteFilter, SceneTreeFilterBase)
    {
        DAVA::ReflectionRegistrator<EditorSpriteFilter>::Begin()[DAVA::M::Group("Object Type"), DAVA::M::DisplayName("Editor Sprite")]
        .ConstructorByPointer()
        .DestructorByPointer()
        .End();
    }
};

using ActionFilter = ComponentFilter<DAVA::Component::ACTION_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(ActionFilter)
{
    DAVA::ReflectionRegistrator<ActionFilter>::Begin()[DAVA::M::Group("Component Filter"), DAVA::M::DisplayName("Action")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using QualitySettingsFilter = ComponentFilter<DAVA::Component::QUALITY_SETTINGS_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(QualitySettingsFilter)
{
    DAVA::ReflectionRegistrator<QualitySettingsFilter>::Begin()[DAVA::M::Group("Component Filter"), DAVA::M::DisplayName("Quality Settings")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using StaticOcclusionFilter = ComponentFilter<DAVA::Component::STATIC_OCCLUSION_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(StaticOcclusionFilter)
{
    DAVA::ReflectionRegistrator<StaticOcclusionFilter>::Begin()[DAVA::M::Group("Component Filter"), DAVA::M::DisplayName("Static Occlusion")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using SoundFilter = ComponentFilter<DAVA::Component::SOUND_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(SoundFilter)
{
    DAVA::ReflectionRegistrator<SoundFilter>::Begin()[DAVA::M::Group("Component Filter"), DAVA::M::DisplayName("Sound")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using WaveFilter = ComponentFilter<DAVA::Component::WAVE_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(WaveFilter)
{
    DAVA::ReflectionRegistrator<WaveFilter>::Begin()[DAVA::M::Group("Component Filter"), DAVA::M::DisplayName("Wave")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using SkeletonFilter = ComponentFilter<DAVA::Component::SKELETON_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(SkeletonFilter)
{
    DAVA::ReflectionRegistrator<SkeletonFilter>::Begin()[DAVA::M::Group("Component Filter"), DAVA::M::DisplayName("Skeleton")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using VisibilityFilter = ComponentFilter<DAVA::Component::VISIBILITY_CHECK_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(VisibilityFilter)
{
    DAVA::ReflectionRegistrator<VisibilityFilter>::Begin()[DAVA::M::Group("Component Filter"), DAVA::M::DisplayName("Visibility")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

using LodFilter = ComponentFilter<DAVA::Component::LOD_COMPONENT>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(LodFilter)
{
    DAVA::ReflectionRegistrator<LodFilter>::Begin()[DAVA::M::Group("Component Filter"), DAVA::M::DisplayName("Lod")]
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}

class InSelectionFilter : public SceneTreeFilterBase
{
public:
    bool IsMatched(const Selectable& object, DAVA::TArc::ContextAccessor* accessor) const override
    {
        DAVA::TArc::DataContext* ctx = accessor->GetActiveContext();

        SelectionData* data = ctx->GetData<SelectionData>();
        return data->GetSelection().ContainsObject(object.GetContainedObject());
    }

    QString GetTitle() const override
    {
        if (IsInverted() == false)
            return QStringLiteral("In selection");

        return QStringLiteral("Not in selection");
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(InSelectionFilter, SceneTreeFilterBase)
    {
        DAVA::ReflectionRegistrator<InSelectionFilter>::Begin()[DAVA::M::DisplayName("In selection")]
        .ConstructorByPointer()
        .DestructorByPointer()
        .End();
    }
};

void RegisterPredefinedFilters()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EmptyEntityFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LightComponentFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CameraComponentFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UserNodeComponentFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SwitchNodeComponentFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleEffectFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleEmitterFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleLayerFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleForceFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LandscapeFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WindFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VegetationFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PathFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EditorSpriteFilter);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ActionFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(QualitySettingsFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticOcclusionFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SoundFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WaveFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SkeletonFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisibilityFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LodFilter);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(InSelectionFilter);
}
