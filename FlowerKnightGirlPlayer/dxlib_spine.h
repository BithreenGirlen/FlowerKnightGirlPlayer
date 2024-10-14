﻿#ifndef DXLIB_SPINE_H_
#define DXLIB_SPINE_H_

/*avoid conflict between <MathUtils.h> and <Windows.h>*/
#undef min
#undef max
#include <spine/spine.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

class CDxLibSpineDrawer
{
public:
	CDxLibSpineDrawer(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData = nullptr);
	~CDxLibSpineDrawer();

	spine::Skeleton* skeleton = nullptr;
	spine::AnimationState* animationState = nullptr;
	float timeScale = 1.f;

	void Update(float fDelta);
	void Draw(float fDepth = 0.f);

	void SwitchPma(){ m_bAlphaPremultiplied ^= true; }
	void SwitchBlendModeAdoption() { m_bForceBlendModeNormal ^= true; }
	void SetLeaveOutList(spine::Vector<spine::String> &list);
private:
	bool m_bHasOwnAnimationStateData = false;
	bool m_bAlphaPremultiplied = false;
	bool m_bForceBlendModeNormal = true;

	spine::Vector<float> m_worldVertices;
	spine::Vector<DxLib::VERTEX2D> m_dxLibVertices;
	spine::Vector<unsigned short> m_dxLibIndices;
	spine::Vector<unsigned short> m_quadIndices;

	spine::SkeletonClipping m_clipper;

	spine::Vector<spine::String> m_leaveOutList;

	bool IsToBeLeftOut(const spine::String& slotName);
};

class CDxLibTextureLoader : public spine::TextureLoader
{
public:
	CDxLibTextureLoader() {};
	virtual ~CDxLibTextureLoader() {};

	virtual void load(spine::AtlasPage& page, const spine::String& path);
	virtual void unload(void* texture);
};

#endif // DXLIB_SPINE_H_
