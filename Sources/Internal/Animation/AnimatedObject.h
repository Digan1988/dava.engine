#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
/**
	\defgroup animationsystem Animation System
 */
class Animation;
/**
	\ingroup animationsystem
	\brief AnimatedObject is base class for objects that need to be animated. For example UIControl, GameObject and so on. 
	All objects inside the system that has to be animated must be derived from this class. 
	You cannot use the AnimatedObject class directly.  It instead defines the common interface and 
	behavioral structure for all its subclasses.
 */
class AnimatedObject : public BaseObject
{
    DAVA_VIRTUAL_REFLECTION(AnimatedObject, BaseObject);

protected:
    virtual ~AnimatedObject();

public:
    AnimatedObject();

    /**
		\brief stop animations for this given AnimatedObject
		\param[in] track id of track you want to stop or -1 if you want to stop all tracks
	 */
    void StopAnimations(int32 track = -1);
    /**
		\brief check if this animated object is animating right now
		\param[in] track id of track you want to check or -1 if you want to check if any tracks animating now
		\returns true if object is animating on the requested tracks
	 */
    bool IsAnimating(int32 track = -1) const;
    // TODO: void	Pause(int32 track = -1);

    /**
		\brief Returns animation with PLAYING state
		\param[in] track - id of track you want to check, -1 for all tracks. Warning: if track == -1, but multiple animations 
							are playing on different tracks, the function will return only one.
		\returns animation, or 0 if no animations are playing
	 */
    Animation* FindPlayingAnimation(int32 track = -1);

private:
    //AnimationsStorage * animationsStorage;

    /*	
	\TODO optimization of animation subsystem
	
	Animation * activeAnimations; 
	Animation * 
 
 
	// each object support 4 tracks of animations
	static const int32 MAX_NUMBER_OF_TRACKS = 4;
	Animation * headAnimations[MAX_NUMBER_OF_TRACKS];
	Animation * headAllAnimations;

	Animation * FindLastAnimationForTrack(int track);
	
	void AddAnimation(Animation * animation);
	void RemoveAnimation(Animation * animation);

	void Update(float32 timeElapsed);
	
	friend class Animation;*/
};
}
