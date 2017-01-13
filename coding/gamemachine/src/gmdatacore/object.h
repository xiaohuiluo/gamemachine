﻿#ifndef __OBJSTRUCT_H__
#define __OBJSTRUCT_H__
#include "common.h"
#include <vector>
#include "image.h"
#include "utilities/assert.h"
#include "utilities/autoptr.h"
#include "utilities/vmath.h"
#include "gmdatacore/texture.h"

BEGIN_NS

class Object;
class GMGLShaders;
class ObjectPainter
{
public:
	ObjectPainter(Object* obj);

public:
	virtual void transfer() = 0;
	virtual void draw() = 0;
	virtual void dispose() = 0;
	virtual void clone(Object* obj, OUT ObjectPainter** painter) = 0;

protected:
	Object* getObject();

private:
	Object* m_object;
};

enum TextureType
{
	TextureTypeUnknown = -1,
	TextureTypeShadow = 0,

	// 从Start到End表示每个对象绘制时需要清理掉的纹理
	TextureTypeResetStart,
	TextureTypeAmbient = TextureTypeResetStart,
	TextureTypeCubeMap,
	TextureTypeResetEnd,

	// 由于反射的天空纹理存在于环境，所以不需要清理
	TextureTypeReflectionCubeMap,
};

struct TextureInfo
{
	ITexture* texture;
	TextureType type;
	GMuint autorelease : 1;
};

enum
{
	MaxTextureCount = 5
};

struct Material
{
	GMfloat Ka[3];
	GMfloat Kd[3];
	GMfloat Ks[3];
	GMfloat Ke[3];	//对于环境（如天空）的反射系数
	GMfloat shininess;
	TextureInfo textures[MaxTextureCount];
};

class Component
{
	friend Object;

public:
	enum
	{
		DefaultEdgesCount = 3,
	};

	Component();
	Component(GMuint edgeCountPerPolygon);
	~Component();

	void setEdgeCountPerPolygon(GMuint edgeCountPerPolygon)
	{
		if (edgeCountPerPolygon > m_edgeCountPerPolygon)
			m_edgeCountPerPolygon = edgeCountPerPolygon;
	}

	GMuint getEdgeCountPerPolygon()
	{
		return m_edgeCountPerPolygon;
	}

	Material& getMaterial()
	{
		return m_material;
	}

	void setOffset(GMvertexoffset offset)
	{
		m_offset = offset;
	}

	GMvertexoffset getOffset()
	{
		return m_offset;
	}

	GMuint getCount()
	{
		return m_verticesCount;
	}

	GMint* getFirstPtr()
	{
		return m_firstPtr;
	}

	GMint* getCountPtr()
	{
		return m_countPtr;
	}

	GMuint getPolygonCount()
	{
		return m_verticesCount / m_edgeCountPerPolygon;
	}

	void generatePolygonProperties();

private:
	GMuint m_verticesCount;
	GMvertexoffset m_offset;
	Material m_material;
	GMuint m_edgeCountPerPolygon;

// 多边形属性
	GMint* m_firstPtr;
	GMint* m_countPtr;
};

class Object
{
public:
	typedef GMfloat DataType;

	enum ObjectType
	{
		ObjectTypeBegin,

		// 表示一个不透明的对象
		NormalObject,

		// 表示一个天空
		Sky,

		ObjectTypeEnd,
	};

	// 绘制时候的排列方式
	enum ArrangementMode
	{
		// 默认排列，按照Components，并按照一个个三角形来画
		Triangle_Fan,

		Triangle_Strip,
	};

	Object();
	~Object();

	void disposeMemory();

	void setPainter(AUTORELEASE ObjectPainter* painter)
	{
		m_painter.reset(painter);
	}

	ObjectPainter* getPainter()
	{
		return m_painter;
	}

	void appendComponent(AUTORELEASE Component* component, GMuint verticesCount);

	std::vector<AUTORELEASE Component*>& getComponents()
	{
		return m_components;
	}

	std::vector<DataType>& vertices()
	{
		return m_vertices;
	}

	std::vector<DataType>& normals()
	{
		return m_normals;
	}

	std::vector<DataType>& uvs()
	{
		return m_uvs;
	}

	ObjectType getType()
	{
		return m_type;
	}

	void setType(ObjectType type)
	{
		m_type = type;
	}

	void setArrangementMode(ArrangementMode mode)
	{
		m_mode = mode;
	}

	ArrangementMode getArrangementMode()
	{
		return m_mode;
	}

	GMuint getBufferId() { return m_bufferId; }
	GMuint getArrayId() { return m_arrayId; }
	void setBufferId(GMuint id) { m_bufferId = id; }
	void setArrayId(GMuint id) { m_arrayId = id; }

private:
	std::vector<DataType> m_vertices;
	std::vector<DataType> m_normals;
	std::vector<DataType> m_uvs;
	GMuint m_arrayId;
	GMuint m_bufferId;
	AutoPtr<ObjectPainter> m_painter;
	std::vector<Component*> m_components;
	ObjectType m_type;
	ArrangementMode m_mode;
};

END_NS
#endif