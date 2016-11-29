﻿#include "stdafx.h"
#include "mtlreaderprivate.h"
#include "objreaderprivate.h"
#include "utilities/scanner.h"
#include "imagereader/imagereader.h"

#define KW_REMARK "#"
#define KW_NEWMATERIAL "newmtl"
#define KW_NS "Ns"
#define KW_D "d"
#define KW_TR "Tr"
#define KW_TF "Tf"
#define KW_ILLUM "illum"
#define KW_KA "Ka"
#define KW_KD "Kd"
#define KW_KS "Ks"
#define KW_MAP_KD "map_Kd"

static bool strEqual(const char* str1, const char* str2)
{
	return !strcmp(str1, str2);
}

static bool isWhiteSpace(char c)
{
	return !!isspace(c);
}

MtlReaderPrivate::~MtlReaderPrivate()
{
	for (auto iter = m_texMap.begin(); iter != m_texMap.end(); iter++)
	{
		TextureInfo& info = (*iter).second;
		if (info.id == TEXTURE_ERROR)
			continue;

		m_pCallback->onRemoveTexture(info.id);
		delete info.texture;
	}
}

Materials& MtlReaderPrivate::getMaterials()
{
	return m_materials;
}

void MtlReaderPrivate::parseLine(const char* line)
{
	Scanner scanner(line, isWhiteSpace);
	char command[LINE_MAX];
	scanner.next(command);

	if (strEqual(command, KW_REMARK))
		return;

	if (strEqual(command, KW_NEWMATERIAL))
	{
		char name[LINE_MAX];
		scanner.next(name);
		m_pCurrentMaterial = &(m_materials[name]);
	}
	else if (strEqual(command, KW_NS))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ns = value;
	}
	else if (strEqual(command, KW_D))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->d = value;
	}
	else if (strEqual(command, KW_TR))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Tr = value;
	}
	else if (strEqual(command, KW_TF))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Tf_r = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Tf_g = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Tf_b = value;
	}
	else if (strEqual(command, KW_ILLUM))
	{
		Fint value;
		scanner.nextInt(&value);
		m_pCurrentMaterial->illum = value;
	}
	else if (strEqual(command, KW_KA))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ka_r = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ka_g = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ka_b = value;
		m_pCurrentMaterial->Ka_switch = true;
	}
	else if (strEqual(command, KW_KD))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Kd_r = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Kd_g = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Kd_b = value;
		m_pCurrentMaterial->Kd_switch = true;
	}
	else if (strEqual(command, KW_KS))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ks_r = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ks_g = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ks_b = value;
		m_pCurrentMaterial->Ks_switch = true;
	}
	else if (strEqual(command, KW_MAP_KD))
	{
		Image* tex = new Image();
		char name[LINE_MAX];
		scanner.next(name);

		std::string filename = m_workingDir;
		filename.append(name);

		if (m_texMap.find(filename) == m_texMap.end())
		{
			ImageReader reader;
			if (reader.load(filename.c_str(), tex))
			{
				m_pCurrentMaterial->hasTexture = true;
				m_pCallback->onAddTexture(tex, &m_pCurrentMaterial->textureID);
			}
			else
			{
				m_pCurrentMaterial->textureID = TEXTURE_ERROR;
			}
		}
		else
		{
			if (m_pCurrentMaterial->textureID != TEXTURE_ERROR)
				m_pCurrentMaterial->hasTexture = true;
			m_pCurrentMaterial->textureID = m_texMap[filename].id;
		}

		TextureInfo info = { tex, m_pCurrentMaterial->textureID };
		m_texMap[filename] = info;
	}
}