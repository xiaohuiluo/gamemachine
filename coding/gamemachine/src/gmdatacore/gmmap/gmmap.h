﻿#ifndef __GMMAP_H__
#define __GMMAP_H__
#include "common.h"
#include <string>
#include <set>
#include "gmdatacore/object.h"
#include "gmengine/controller/factory.h"
#include "gmengine/elements/gameobjectprivate.h"
#include "gmengine/elements/gerstnerwavegameobject.h"

BEGIN_NS

typedef GMuint ID;

template <typename T>
struct NO_DESTRUCTOR
{
	void operator()(const T& each)
	{
	}
};

template <typename T, typename LESS, typename DESTRUCTOR = NO_DESTRUCTOR<T> >
class GMMapSet : public std::set<T, LESS>
{
	typedef std::set<T, LESS> Base;
public:
	~GMMapSet()
	{
		for (auto iter = begin(); iter != end(); iter++)
		{
			DESTRUCTOR()(*iter);
		}
	}
};

class GMMapString : public std::string
{
public:
	GMMapString& operator = (const char* str)
	{
		if (str)
			this->std::string::operator = (str);
		return *this;
	}
};

template <typename T>
class ID_Less
{
public:
	bool operator ()(const T& left, const T& right)
	{
		return left.id < right.id;
	}
};

template <typename T>
const T* GMMap_find(std::set<T, ID_Less<T>>& set, ID key)
{
	T _key = { key };
	auto it = set.find(_key);
	if (it == set.end())
		return nullptr;

	return &(*it);
}

enum VariantPropertyType
{
	None = 0,
	Waves,
};

union VariantProperty
{
	GerstnerWavesProperties wavesProperites;
};

struct GMMapMeta
{
	GMMapString author;
	GMMapString name;
};

struct GMMapTexture
{
	static TextureType getType(const char* name);

	ID id;
	TextureType type;
	GMMapString path;
};

struct GMMapObject
{
	enum GMMapObjectType
	{
		Error = -1,
		Default,
		Cube,
		Sphere,
		ConvexHull,
		Sky,
		Hallucination,
		Capsule = 6,
		Cylinder,
		Cone,
		GerstnerWave,
		Compound,
	};

	static GMMapObjectType getType(const char* name);

	ID id;
	GMMapObjectType type;
	GMMapString path;
	GMfloat width, height, depth;
	GMfloat slices, stacks, radius;
	GMfloat magnification;
	GMfloat collisionExtents[3];
	VariantPropertyType propertyType;
	VariantProperty property;
};

struct GMMapObject_DESCTRUCTOR
{
	void operator() (const GMMapObject& object)
	{
		if (object.propertyType == Waves)
			delete[] object.property.wavesProperites.waves;
	}
};

struct GMMapMaterial
{
	ID id;
	Material material;
};

struct GMMapEntity
{
	enum
	{
		MAX_REF = 6
	};

	ID id;
	ID objRef;
	ID materialRef[MAX_REF];
	ID textureRef[MAX_REF];
};

struct GMMapInstance
{
	enum
	{
		INVALID_ID = 0,
		MAX_ANIMATION_TYPE = 3
	};

	ID id;
	ID entityRef;
	ID animationRef[MAX_ANIMATION_TYPE];
	GMfloat animationDuration;
	GMfloat position[3], rotation[4];
	GMfloat scale[3];
	GMfloat mass;
	Frictions frictions;
};

struct GMMapLight
{
	static LightType getType(const char* name);

	ID id;
	GMfloat rgb[3];
	GMfloat position[3];
	GMfloat range;
	LightType type;
	GMuint shadow;
};

struct GMMapKeyframe
{
	Keyframe keyframe;
};

class Keyframe_Less
{
public:
	bool operator ()(const GMMapKeyframe& left, const GMMapKeyframe& right)
	{
		return left.keyframe.percentage < right.keyframe.percentage;
	}
};

struct GMMapKeyframes
{
	enum Type
	{
		Rotation,
		Translation,
		Scaling,
	};

	static Keyframes::Interpolation getFunctorType(const char* name);
	static Type getType(const char* name);

	ID id;
	Keyframes::Interpolation functor;
	std::set<GMMapKeyframe, Keyframe_Less> keyframes;
	Type type;
};

struct GMMapSettings
{
	struct Character
	{
		GMfloat position[3];
		GMfloat radius;
		GMfloat height;
		GMfloat stepHeight;
		GMfloat jumpSpeed[3];
		GMfloat eyeOffset[3];
		GMfloat fallSpeed;
		GMuint freemove;
		GMuint movespeed;
	} character;

	struct Gravity
	{
		GMfloat vector[3];
	} gravity;
};

struct GMMap
{
	GMMapMeta meta;
	GMMapSet<GMMapTexture, ID_Less<GMMapTexture>> textures;
	GMMapSet<GMMapObject, ID_Less<GMMapObject>, GMMapObject_DESCTRUCTOR> objects;
	GMMapSet<GMMapMaterial, ID_Less<GMMapMaterial>> materials;
	GMMapSet<GMMapEntity, ID_Less<GMMapEntity>> entities;
	GMMapSet<GMMapInstance, ID_Less<GMMapInstance>> instances;
	GMMapSet<GMMapLight, ID_Less<GMMapLight>> lights;
	GMMapSet<GMMapKeyframes, ID_Less<GMMapKeyframes>> animations;
	GMMapSettings settings;
	std::string workingDir;
};

END_NS
#endif