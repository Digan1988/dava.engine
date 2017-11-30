#include "Entity/ComponentManager.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "UI/Components/UIComponent.h"
#include "Utils/CRC32.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
ComponentManager::ComponentManager()
{
    DVASSERT(runtimeTypeIndex == -1 && componentTypeIndex == -1);

    runtimeTypeIndex = Type::AllocUserData();
    componentTypeIndex = Type::AllocUserData();
}

void ComponentManager::RegisterComponent(const Type* type)
{
    DVASSERT(type != nullptr);

    const Type* sceneComponentType = Type::Instance<Component>();
    const Type* uiComponentType = Type::Instance<UIComponent>();

    if (TypeInheritance::CanDownCast(type, sceneComponentType))
    {
        DVASSERT(!TypeInheritance::CanDownCast(type, uiComponentType));

        auto it = std::find(registeredSceneComponents.begin(), registeredSceneComponents.end(), type);

        if (it != registeredSceneComponents.end())
        {
            DVASSERT(false, "Component is already registered.");
            return;
        }

        ++runtimeSceneComponentsCount;

        type->SetUserData(runtimeTypeIndex, Uint32ToVoidPtr(runtimeSceneComponentsCount));
        type->SetUserData(componentTypeIndex, Uint32ToVoidPtr(ComponentType::SCENE_COMPONENT));

        sceneRuntimeIndexToType.push_back(type);

        DVASSERT(sceneRuntimeIndexToType.size() == runtimeSceneComponentsCount);

        DVASSERT(static_cast<size_t>(runtimeSceneComponentsCount) < ComponentMask().size());

        registeredSceneComponents.push_back(type);
    }
    else if (TypeInheritance::CanDownCast(type, uiComponentType))
    {
        DVASSERT(!TypeInheritance::CanDownCast(type, sceneComponentType));

        auto it = std::find(registeredUIComponents.begin(), registeredUIComponents.end(), type);

        if (it != registeredUIComponents.end())
        {
            DVASSERT(false, "UIComponent is already registered.");
            return;
        }

        ++runtimeUIComponentsCount;

        type->SetUserData(runtimeTypeIndex, Uint32ToVoidPtr(runtimeUIComponentsCount));
        type->SetUserData(componentTypeIndex, Uint32ToVoidPtr(ComponentType::UI_COMPONENT));

        registeredUIComponents.push_back(type);
    }
    else
    {
        DVASSERT(false, "'type' should be derived from Component or UIComponent.");
    }
}

void ComponentManager::PreregisterAllDerivedSceneComponentsRecursively()
{
    DVASSERT(!componentsWerePreregistered);
    DVASSERT(runtimeSceneComponentsCount == 0);
    DVASSERT(registeredSceneComponents.empty() && sceneRuntimeIndexToType.empty());

    const TypeInheritance* inheritance = Type::Instance<Component>()->GetInheritance();

    if (inheritance == nullptr || inheritance->GetDerivedTypes().empty())
    {
        DVASSERT(false, "'Component' type has no derived types.");
        return;
    }

    const Vector<TypeInheritance::Info>& derivedTypes = inheritance->GetDerivedTypes();

    Vector<const Type*> componentsToRegister;
    componentsToRegister.reserve(derivedTypes.size() + derivedTypes.size() / 4); // reserve ~125% of derived types, since each type may have its own children

    for (const TypeInheritance::Info& info : derivedTypes)
    {
        CollectSceneComponentRecursively(info.type, componentsToRegister);
    }

    const ReflectedTypeDB* db = ReflectedTypeDB::GetLocalDB();

    std::sort(componentsToRegister.begin(), componentsToRegister.end(), [db](const Type* l, const Type* r) {
        const String& permanentNameL = db->GetByType(l)->GetPermanentName();
        const String& permanentNameR = db->GetByType(r)->GetPermanentName();

        DVASSERT(!permanentNameL.empty() && !permanentNameR.empty());

        return permanentNameL < permanentNameR;
    });

    CRC32 crc;

    for (const Type* type : componentsToRegister)
    {
        RegisterComponent(type);

        const String& permanentName = db->GetByType(type)->GetPermanentName();
        crc.AddData(permanentName.data(), permanentName.size() * sizeof(String::value_type));
    }

    crc32HashOfPreregisteredComponents = crc.Done();

    componentsWerePreregistered = true;
}

uint32 ComponentManager::GetCRC32HashOfPreregisteredSceneComponents()
{
    DVASSERT(componentsWerePreregistered);

    return crc32HashOfPreregisteredComponents;
}

uint32 ComponentManager::GetCRC32HashOfRegisteredSceneComponents()
{
    Vector<const String*> componentsPermanentNames;
    componentsPermanentNames.reserve(registeredSceneComponents.size());

    for (const Type* type : registeredSceneComponents)
    {
        const ReflectedType* refType = ReflectedTypeDB::GetLocalDB()->GetByType(type);

        DVASSERT(refType != nullptr);

        const String& permanentName = refType->GetPermanentName();

        if (!permanentName.empty())
        {
            componentsPermanentNames.push_back(&permanentName);
        }
        else
        {
            DVASSERT(false, "Permanent name is not registered for component class. Hash will be incorrect.");
        }
    }

    DVASSERT(!componentsPermanentNames.empty(), "CRC32 will be 0.");

    std::sort(componentsPermanentNames.begin(), componentsPermanentNames.end(), [](const String* s1, const String* s2) {
        return *s1 < *s2;
    });

    CRC32 crc;

    for (const String* s : componentsPermanentNames)
    {
        crc.AddData(s->data(), s->size());
    }

    return crc.Done();
}

const Type* ComponentManager::GetSceneComponentType(uint32 runtimeIndex) const
{
    const Type* ret = nullptr;

    if (runtimeIndex < sceneRuntimeIndexToType.size())
    {
        ret = sceneRuntimeIndexToType[runtimeIndex];
    }

    return ret;
}

void ComponentManager::CollectSceneComponentRecursively(const Type* type, Vector<const Type*>& components)
{
    components.push_back(type);

    const TypeInheritance* inheritance = type->GetInheritance();

    if (inheritance != nullptr)
    {
        for (const TypeInheritance::Info& info : inheritance->GetDerivedTypes())
        {
            CollectSceneComponentRecursively(info.type, components);
        }
    }
}
}