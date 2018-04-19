﻿#ifndef __GMGLGBUFFER_H__
#define __GMGLGBUFFER_H__
#include <gmcommon.h>
BEGIN_NS

// GBuffer 类型，对应pass着色器layout的顺序
enum class GBufferGeometryType
{
	// 顶点属性
	Position,
	Normal,
	Normal_Eye,
	AmbientTexture,
	DiffuseTexture,
	Tangent_Eye,
	Bitangent_Eye,
	NormalMap,
	EndOfGeometryType,
};

enum class GBufferMaterialType
{
	// 材质属性
	Ka,
	Kd,
	Ks_Shininess,
	HasNormalMap_Refractivity,
	EndOfMaterialType,
};

// 当前渲染的状态，渲染到哪一步了
enum class GMGLDeferredRenderState
{
	PassingGeometry, //正在进行普通渲染或geometry pass
	PassingMaterial, //正在传递材质

	PassingLight, // 光照计算
	EndOfRenderState = PassingLight
};

constexpr GMint GMGLGBuffer_TotalTurn = (GMint) GMGLDeferredRenderState::EndOfRenderState;

GM_PRIVATE_OBJECT(GMGLGBuffer)
{
	GMuint fbo[GMGLGBuffer_TotalTurn] = { 0 };
	GMuint textures[(GMint)GBufferGeometryType::EndOfGeometryType] = { 0 };
	GMuint materials[(GMint)GBufferMaterialType::EndOfMaterialType] = { 0 };
	GMuint depthBuffers[(GMint)GMGLDeferredRenderState::EndOfRenderState] = { 0 };
	GMint renderWidth = 0;
	GMint renderHeight = 0;
	GMint currentTurn = 0;
	GMRect clientRect = { 0 };
	GMRect viewport = { 0 };
};

class GMGLShaderProgram;
class GMGLGBuffer : public GMObject
{
	DECLARE_PRIVATE(GMGLGBuffer)

public:
	GMGLGBuffer() = default;
	~GMGLGBuffer();

public:
	void adjustViewport();
	void beginPass();
	bool nextPass();
	void dispose();
	bool init(const GMRect& clientRect);
	void bindForWriting();
	void bindForReading();
	void releaseBind();
	void setReadBuffer(GBufferGeometryType textureType);
	void setReadBuffer(GBufferMaterialType materialType);
	void newFrame();
	void activateTextures();
	void copyDepthBuffer(GMuint target);

public:
	inline const GMint& getWidth() { D(d); return d->renderWidth; }
	inline const GMint& getHeight() { D(d); return d->renderHeight; }

private:
	bool createFrameBuffers(GMGLDeferredRenderState state, GMint textureCount, GMuint* textureArray);
	bool drawBuffers(GMuint count);
};

GM_PRIVATE_OBJECT(GMGLFramebufferDep)
{
	GMuint fbo = 0;
	GMuint texture = 0;
	GMuint depthBuffer = 0;
	GMuint quadVAO = 0;
	GMuint quadVBO = 0;
	GMint renderWidth = 0;
	GMint renderHeight = 0;
	GMfloat sampleOffsets[2] = { 0 };
	GMFilterMode::Mode effects = GMFilterMode::None;
	bool hasBegun = false;
	GMRect clientRect = { 0 };
	GMRect viewport = { 0 };
	GMRenderConfig renderConfig;
};

class GMGLFramebufferDep : public GMObject
{
	DECLARE_PRIVATE(GMGLFramebufferDep)

public:
	GMGLFramebufferDep();
	~GMGLFramebufferDep();

public:
	void dispose();
	bool init(const GMRect& renderRect);
	void beginDrawEffects();
	void endDrawEffects();
	void draw(GMGLShaderProgram* program);
	GMuint framebuffer();
	void bindForWriting();
	void bindForReading();
	void releaseBind();

public:
	inline bool hasBegun() { D(d); return d->hasBegun; }
	inline GMuint getWidth() { D(d); return d->renderWidth; }
	inline GMuint getHeight() { D(d); return d->renderHeight; }

private:
	void newFrame();
	void createQuad();
	void turnOffBlending();
	void blending();
	void renderQuad();
	void disposeQuad();
	const char* useShaderProgramAndApplyFilter(GMGLShaderProgram* program, GMFilterMode::Mode effect);
};

GM_PRIVATE_OBJECT(GMGLFramebuffer)
{
	ITexture* texture = nullptr;
};

class GMGLFramebuffer : public GMObject, public IFramebuffer
{
	DECLARE_PRIVATE(GMGLFramebuffer);

public:
	~GMGLFramebuffer();

public:
	virtual bool init(const GMFramebufferDesc& desc) override;
	virtual ITexture* getTexture() override;

public:
	GMuint getTextureId();
};

GM_PRIVATE_OBJECT(GMGLFramebuffers)
{
	GMuint fbo = 0;
	GMuint depthStencilBuffer = 0;
	Vector<GMGLFramebuffer*> framebuffers;
	bool framebuffersCreated = false;
};

class GMGLFramebuffers : public GMObject, public IFramebuffers
{
	DECLARE_PRIVATE(GMGLFramebuffers)

public:
	GMGLFramebuffers() = default;
	~GMGLFramebuffers();

	virtual bool init(const GMFramebufferDesc& desc) override;
	virtual void addFramebuffer(AUTORELEASE IFramebuffer* framebuffer) override;
	virtual void bind() override;
	virtual void unbind() override;
	virtual void clear() override;
	virtual ITexture* getTexture(GMuint) override;

private:
	void createDepthStencilBuffer(const GMFramebufferDesc& desc);
	bool createFramebuffers();
};

END_NS
#endif