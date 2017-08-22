﻿#include "stdafx.h"
#include "gmglgbuffer.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"

constexpr GMuint GEOMETRY_NUM = (GMuint)GBufferGeometryType::EndOfGeometryType;
constexpr GMuint MATERIAL_NUM = (GMuint)GBufferMaterialType::EndOfMaterialType;
constexpr GMuint FLAG_NUM = (GMuint)GBufferFlags::EndOfFlags;

Array<const char*, GEOMETRY_NUM> g_GBufferGeometryUniformNames =
{
	"gPosition",
	"gNormal",
	"gTexAmbient",
	"gTexDiffuse",
	"gTangent",
	"gBitangent",
	"gNormalMap",
};

Array<const char*, MATERIAL_NUM> g_GBufferMaterialUniformNames =
{
	"gKa",
	"gKd",
	"gKs",
};

Array<const char*, FLAG_NUM> g_GBufferFlagUniformNames =
{
	"gHasNormalMap",
};

GMGLGBuffer::GMGLGBuffer()
{
}

GMGLGBuffer::~GMGLGBuffer()
{
	dispose();
}

void GMGLGBuffer::beginPass()
{
	D(d);
	d->currentTurn = (GMint)GMGLDeferredRenderState::GeometryPass;
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	engine->setRenderState((GMGLDeferredRenderState)d->currentTurn);
}

bool GMGLGBuffer::nextPass()
{
	D(d);
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	++d->currentTurn;
	engine->setRenderState((GMGLDeferredRenderState)d->currentTurn);
	if (d->currentTurn == GMGLGBuffer_TotalTurn)
		return false;
	return true;
}

void GMGLGBuffer::dispose()
{
	D(d);
	if (d->fbo)
	{
		glDeleteFramebuffers(GMGLGBuffer_TotalTurn, d->fbo);
		GM_ZeroMemory(d->fbo);
	}

	if (d->textures[0] != 0)
	{
		glDeleteTextures(GEOMETRY_NUM, d->textures);
		d->textures[0] = 0;
	}

	if (d->depthTexture != 0)
	{
		glDeleteTextures(1, &d->depthTexture);
		d->depthTexture = 0;
	}
}

bool GMGLGBuffer::init(GMuint windowWidth, GMuint windowHeight)
{
	D(d);
	GLenum errCode;

	d->windowWidth = windowWidth;
	d->windowHeight = windowHeight;

	// Create the FBO
	glGenFramebuffers(GMGLGBuffer_TotalTurn, d->fbo);

	// Vertex data
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo[(GMuint)GMGLDeferredRenderState::GeometryPass]);
	glGenTextures(GEOMETRY_NUM, d->textures);
	for (GMuint i = 0; i < GEOMETRY_NUM; i++)
	{
		glBindTexture(GL_TEXTURE_2D, d->textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, d->textures[i], 0);
		ASSERT((errCode = glGetError()) == GL_NO_ERROR);
	}
	if (!drawBuffers(GEOMETRY_NUM))
		return false;

	// Material data
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo[(GMuint)GMGLDeferredRenderState::PassingMaterial]);
	glGenTextures(MATERIAL_NUM, d->materials);
	for (GMuint i = 0; i < MATERIAL_NUM; i++)
	{
		glBindTexture(GL_TEXTURE_2D, d->materials[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, d->materials[i], 0);
		ASSERT((errCode = glGetError()) == GL_NO_ERROR);
	}
	if (!drawBuffers(MATERIAL_NUM))
		return false;

	// Flags
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo[(GMuint)GMGLDeferredRenderState::PassingFlags]);
	glGenTextures(FLAG_NUM, d->flags);
	for (GMuint i = 0; i < FLAG_NUM; i++)
	{
		glBindTexture(GL_TEXTURE_2D, d->flags[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, d->flags[i], 0);
		ASSERT((errCode = glGetError()) == GL_NO_ERROR);
	}
	if (!drawBuffers(FLAG_NUM))
		return false;

	// depth
	/*
	glGenTextures(1, &d->depthTexture);
	glBindTexture(GL_TEXTURE_2D, d->depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, d->depthTexture, 0);
	ASSERT((errCode = glGetError()) == GL_NO_ERROR);
	*/

	// restore default FBO
	releaseBind();

	return true;
}

void GMGLGBuffer::bindForWriting()
{
	D(d);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo[d->currentTurn]);
}

void GMGLGBuffer::bindForReading()
{
	D(d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, d->fbo[d->currentTurn]);
}

void GMGLGBuffer::releaseBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GMGLGBuffer::setReadBuffer(GBufferGeometryType textureType)
{
	D(d);
	ASSERT(d->currentTurn == 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (GMuint)textureType);
}

void GMGLGBuffer::setReadBuffer(GBufferMaterialType materialType)
{
	D(d);
	ASSERT(d->currentTurn == 1);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (GMuint)materialType);
}

void GMGLGBuffer::newFrame()
{
	bindForWriting();
	GMGLGraphicEngine::newFrameOnCurrentContext();
	releaseBind();
}

void GMGLGBuffer::activateTextures(GMGLShaderProgram* shaderProgram)
{
	D(d);
	GLenum errCode;

	{
		for (GMuint i = 0; i < GEOMETRY_NUM; i++)
		{
			shaderProgram->setInt(g_GBufferGeometryUniformNames[i], i);
			ASSERT((errCode = glGetError()) == GL_NO_ERROR);
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, d->textures[i]);
			ASSERT((errCode = glGetError()) == GL_NO_ERROR);
		}
	}

	constexpr GMuint GEOMETRY_OFFSET = GEOMETRY_NUM;
	{
		for (GMuint i = 0; i < MATERIAL_NUM; i++)
		{
			shaderProgram->setInt(g_GBufferMaterialUniformNames[i], GEOMETRY_OFFSET + i);
			ASSERT((errCode = glGetError()) == GL_NO_ERROR);
			glActiveTexture(GL_TEXTURE0 + GEOMETRY_OFFSET + i);
			glBindTexture(GL_TEXTURE_2D, d->materials[i]);
			ASSERT((errCode = glGetError()) == GL_NO_ERROR);
		}
	}

	constexpr GMuint FLAG_OFFSET = GEOMETRY_OFFSET + MATERIAL_NUM;
	{
		for (GMuint i = 0; i < FLAG_NUM; i++)
		{
			shaderProgram->setInt(g_GBufferFlagUniformNames[i], FLAG_OFFSET + i);
			ASSERT((errCode = glGetError()) == GL_NO_ERROR);
			glActiveTexture(GL_TEXTURE0 + FLAG_OFFSET + i);
			glBindTexture(GL_TEXTURE_2D, d->flags[i]);
			ASSERT((errCode = glGetError()) == GL_NO_ERROR);
		}
	}
}

bool GMGLGBuffer::drawBuffers(GMuint count)
{
	GLenum errCode;
	Vector<GLuint> attachments;
	for (GMuint i = 0; i < count; i++)
	{
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	glDrawBuffers(count, attachments.data());
	ASSERT((errCode = glGetError()) == GL_NO_ERROR);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		gm_error("FB error, status: 0x%x\n", status);
		return false;
	}
	return true;
}