/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_ANIMATION_DATA_H__
#define __DAVAENGINE_ANIMATION_DATA_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneNodeAnimationKey.h"
#include "Scene3D/DataNode.h"
namespace DAVA 
{
	
class AnimationData : public DataNode
{
protected:
	virtual ~AnimationData();
public:
	AnimationData();
	
	SceneNodeAnimationKey Interpolate(float32 t, uint32* startIdxCache);
	
	void AddKey(const SceneNodeAnimationKey & key);
	
	inline int32 GetKeyCount();
	
	void SetDuration(float32 _duration);
	inline float32 GetDuration(); 
	
	void SetInvPose(const Matrix4& mat); 
	const Matrix4& GetInvPose() const;
	
	virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
	virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);

	AnimationData* Clone() const;

	float32 duration;
	
	DAVA::Vector< SceneNodeAnimationKey > keys;

	Matrix4 invPose;
};
	
inline float32 AnimationData::GetDuration()
{
	return duration;
}
	
inline int32 AnimationData::GetKeyCount()
{
	return keys.size();
}
	
};

#endif // __DAVAENGINE_ANIMATION_DATA_H__
