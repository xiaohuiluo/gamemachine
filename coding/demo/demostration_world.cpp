#include "stdafx.h"
#include "demostration_world.h"
#include <gmcontrolgameobject.h>
#include <gmgl.h>

#include "demo/simple.h"
void loadDemostrations(DemostrationWorld* world)
{
	world->addDemo("Load texture", new Demo_Simple());
	world->init();
}

DemostrationWorld::~DemostrationWorld()
{
	D(d);
	for (auto& demo : d->demos)
	{
		GM_ASSERT(demo.second);
		delete demo.second;
	}
}

void DemostrationWorld::addDemo(const gm::GMString& name, AUTORELEASE gm::IGameHandler* demo)
{
	D(d);
	GM_ASSERT(demo);
	d->demos.push_back(std::make_pair(name, demo));
}

void DemostrationWorld::init()
{
	D(d);
	gm::GMListbox2DGameObject* listbox = new gm::GMListbox2DGameObject();
	gm::GMRect rect = { 10, 10, 200, 600 };
	listbox->setGeometry(rect);
	for (auto& demo : d->demos)
	{
		gm::GMImage2DGameObject* item = listbox->addItem(demo.first);
		item->setHeight(20);
	}
	addControl(listbox);
}

void DemostrationWorld::renderScene()
{
	Base::renderScene();

	gm::IGraphicEngine* engine = GM.getGraphicEngine();
	auto& controls = getControlsGameObject();
	engine->drawObjects(controls.data(), controls.size());
}

//////////////////////////////////////////////////////////////////////////
void DemostrationEntrance::init()
{
	D(d);

	gm::GMGLGraphicEngine* engine = static_cast<gm::GMGLGraphicEngine*> (GM.getGraphicEngine());
	engine->setShaderLoadCallback(this);
	GMSetRenderState(RENDER_MODE, gm::GMStates_RenderOptions::DEFERRED);
	//GMSetRenderState(EFFECTS, GMEffects::Grayscale);
	GMSetRenderState(RESOLUTION_X, 800);
	GMSetRenderState(RESOLUTION_Y, 600);

	gm::GMGamePackage* pk = GM.getGamePackageManager();
#ifdef _DEBUG
	pk->loadPackage("D:/gmpk");
#else
	pk->loadPackage((GMPath::getCurrentPath() + _L("gm.pk0")));
#endif
	d->world = new DemostrationWorld();
}

void DemostrationEntrance::start()
{
	D(d);
	loadDemostrations(d->world);
}

void DemostrationEntrance::event(gm::GameMachineEvent evt)
{
	switch (evt)
	{
	case gm::GameMachineEvent::FrameStart:
		break;
	case gm::GameMachineEvent::FrameEnd:
		break;
	case gm::GameMachineEvent::Simulate:
		break;
	case gm::GameMachineEvent::Render:
		getWorld()->renderScene();
		break;
	case gm::GameMachineEvent::Activate:
		break;
	case gm::GameMachineEvent::Deactivate:
		break;
	case gm::GameMachineEvent::Terminate:
		break;
	default:
		break;
	}
}

DemostrationEntrance::~DemostrationEntrance()
{
	D(d);
	if (d->world)
		delete d->world;
}

void DemostrationEntrance::onLoadForwardShader(const gm::GMMeshType type, gm::GMGLShaderProgram& shader)
{
	gm::GMBuffer vertBuf, fragBuf;
	gm::GMString vertPath, fragPath;
	switch (type)
	{
	case gm::GMMeshType::Model3D:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "model3d.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "model3d.frag", &fragBuf, &fragPath);
		break;
	case gm::GMMeshType::Model2D:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "model2d.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "model2d.frag", &fragBuf, &fragPath);
		break;
	case gm::GMMeshType::Particles:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "particles.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "particles.frag", &fragBuf, &fragPath);
		break;
	default:
		break;
	}

	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	gm::GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	shader.attachShader(shadersInfo[0]);
	shader.attachShader(shadersInfo[1]);
}

void DemostrationEntrance::onLoadDeferredPassShader(gm::GMGLDeferredRenderState state, gm::GMGLShaderProgram& shaderProgram)
{
	gm::GMBuffer vertBuf, fragBuf;
	gm::GMString vertPath, fragPath;
	switch (state)
	{
	case gm::GMGLDeferredRenderState::PassingGeometry:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/geometry_pass.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/geometry_pass.frag", &fragBuf, &fragPath);
		break;
	case gm::GMGLDeferredRenderState::PassingMaterial:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/material_pass.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/material_pass.frag", &fragBuf, &fragPath);
		break;
	default:
		break;
	}
	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	gm::GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	shaderProgram.attachShader(shadersInfo[0]);
	shaderProgram.attachShader(shadersInfo[1]);
}

void DemostrationEntrance::onLoadDeferredLightPassShader(gm::GMGLShaderProgram& lightPassProgram)
{
	gm::GMBuffer vertBuf, fragBuf;
	gm::GMString vertPath, fragPath;
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/light_pass.vert", &vertBuf, &vertPath);
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/light_pass.frag", &fragBuf, &fragPath);
	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	gm::GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	lightPassProgram.attachShader(shadersInfo[0]);
	lightPassProgram.attachShader(shadersInfo[1]);
}

void DemostrationEntrance::onLoadEffectsShader(gm::GMGLShaderProgram& lightPassProgram)
{
	gm::GMBuffer vertBuf, fragBuf;
	gm::GMString vertPath, fragPath;
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "effects/effects.vert", &vertBuf, &vertPath);
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "effects/effects.frag", &fragBuf, &fragPath);
	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	gm::GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	lightPassProgram.attachShader(shadersInfo[0]);
	lightPassProgram.attachShader(shadersInfo[1]);
}
