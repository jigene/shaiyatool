///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "DAEExporter.h"
#include <Object3D.h>
#include <Motion.h>

CDAEExporter::CDAEExporter(CAnimatedMesh* mesh)
	: CExporter(mesh)
{
}

bool CDAEExporter::Export(const string& filename)
{
	m_doc.appendChild(m_doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\""));

	m_colladaNode = m_doc.createElement("COLLADA");
	m_colladaNode.setAttribute("version", "1.4.1");
	m_colladaNode.setAttribute("xmlns", "http://www.collada.org/2005/11/COLLADASchema");
	m_doc.appendChild(m_colladaNode);

	_writeAsset();
	_writeCameras();
	_writeLights();
	_writeImages();
	_writeEffects();
	_writeMaterials();
	_writeGeometries();
	_writeAnimations();
	_writeControllers();
	_writeVisualScenes();
	_writeScene();

	m_file.setFileName(filename);
	if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	QTextStream out(&m_file);
	out.setCodec("UTF-8");
	out << m_doc.toString();

	return true;
}

void CDAEExporter::_writeAsset()
{
	QDomElement asset = m_doc.createElement("asset");
	m_colladaNode.appendChild(asset);

	QDomElement unit = m_doc.createElement("unit");
	unit.setAttribute("meter", 0.025400);
	unit.setAttribute("name", "centimeter");
	asset.appendChild(unit);

	QDomElement upAxis = m_doc.createElement("up_axis");
	upAxis.appendChild(m_doc.createTextNode("Y_UP"));
	asset.appendChild(upAxis);
}

void CDAEExporter::_writeCameras()
{
	QDomElement cameras = m_doc.createElement("library_cameras");
	m_colladaNode.appendChild(cameras);
}

void CDAEExporter::_writeLights()
{
	QDomElement lights = m_doc.createElement("library_lights");
	m_colladaNode.appendChild(lights);
}

void CDAEExporter::_writeImages()
{
	QDomElement images = m_doc.createElement("library_images");
	m_colladaNode.appendChild(images);

	for (auto it = m_materials.begin(); it != m_materials.end(); it++)
	{
		const string texId = string(it.value()->textureName).toLower().replace('.', '_').replace('-', '_').replace(' ', '_') + '-' + it.key();
		QDomElement texture = m_doc.createElement("image");
		texture.setAttribute("id", texId);
		texture.setAttribute("name", texId);
		QDomElement init_from = m_doc.createElement("init_from");
		init_from.appendChild(m_doc.createTextNode(it.value()->textureName));
		texture.appendChild(init_from);
		images.appendChild(texture);
	}
}

void CDAEExporter::_writeEffects()
{
	QDomElement effects = m_doc.createElement("library_effects");
	m_colladaNode.appendChild(effects);

	for (auto it = m_materials.begin(); it != m_materials.end(); it++)
	{
		const string texID = string(it.value()->textureName).toLower().replace('.', '_').replace('-', '_').replace(' ', '_') + '-' + it.key();

		QDomElement effect = m_doc.createElement("effect");
		effect.setAttribute("id", it.key() + "-effect");
		QDomElement profile_COMMON = m_doc.createElement("profile_COMMON");
		{
			QDomElement newparam = m_doc.createElement("newparam");
			newparam.setAttribute("sid", texID + "-surface");
			QDomElement surface = m_doc.createElement("surface");
			surface.setAttribute("type", "2D");
			QDomElement init_from = m_doc.createElement("init_from");
			init_from.appendChild(m_doc.createTextNode(texID));
			surface.appendChild(init_from);
			newparam.appendChild(surface);
			profile_COMMON.appendChild(newparam);
		}
		{
			QDomElement newparam = m_doc.createElement("newparam");
			newparam.setAttribute("sid", texID + "-sampler");
			QDomElement sampler2D = m_doc.createElement("sampler2D");
			sampler2D.setAttribute("type", "2D");
			QDomElement source = m_doc.createElement("source");
			source.appendChild(m_doc.createTextNode(texID + "-surface"));
			sampler2D.appendChild(source);
			newparam.appendChild(sampler2D);
			profile_COMMON.appendChild(newparam);
		}
		QDomElement technique = m_doc.createElement("technique");
		technique.setAttribute("sid", "common");
		QDomElement phong = m_doc.createElement("phong");
		QDomElement emission = m_doc.createElement("emission");
		{
			QDomElement color = m_doc.createElement("color");
			color.setAttribute("sid", "emission");
			color.appendChild(m_doc.createTextNode("0 0 0 1"));
			emission.appendChild(color);
		}
		phong.appendChild(emission);
		QDomElement ambient = m_doc.createElement("ambient");
		{
			QDomElement color = m_doc.createElement("color");
			color.setAttribute("sid", "ambient");
			color.appendChild(m_doc.createTextNode("0 0 0 1"));
			ambient.appendChild(color);
		}
		phong.appendChild(ambient);
		QDomElement diffuse = m_doc.createElement("diffuse");
		QDomElement texture = m_doc.createElement("texture");
		texture.setAttribute("texcoord", "UVMap");
		texture.setAttribute("texture", texID + "-sampler");
		diffuse.appendChild(texture);
		phong.appendChild(diffuse);
		QDomElement specular = m_doc.createElement("specular");
		{
			QDomElement color = m_doc.createElement("color");
			color.setAttribute("sid", "specular");
			color.appendChild(m_doc.createTextNode("0.5 0.5 0.5 1"));
			specular.appendChild(color);
		}
		phong.appendChild(specular);
		QDomElement shininess = m_doc.createElement("shininess");
		QDomElement f = m_doc.createElement("float");
		f.setAttribute("sid", "shininess");
		f.appendChild(m_doc.createTextNode("50"));
		shininess.appendChild(f);
		phong.appendChild(shininess);

		MaterialBlock* block = _getMaterialBlock(it.value());
		if (block)
		{
			if (block->effect & XE_2SIDE)
			{
				QDomElement double_sided = m_doc.createElement("double_sided");
				double_sided.appendChild(m_doc.createTextNode("true"));
				phong.appendChild(double_sided);
			}
			if (block->effect & XE_OPACITY)
			{
				QDomElement transparent = m_doc.createElement("transparent");
				transparent.setAttribute("opaque", "RGB_ZERO");

				QDomElement color = m_doc.createElement("color");
				color.setAttribute("sid", "transparent");
				color.appendChild(m_doc.createTextNode("1.0 1.0 1.0 1.0"));
				transparent.appendChild(color);

				QDomElement texture = m_doc.createElement("texture");
				texture.setAttribute("texcoord", "UVMap");
				texture.setAttribute("texture", texID + "-sampler");
				transparent.appendChild(texture);

				phong.appendChild(transparent);

				QDomElement transparency = m_doc.createElement("transparency");
				QDomElement f = m_doc.createElement("float");
				f.setAttribute("sid", "transparency");
				f.appendChild(m_doc.createTextNode(string::number(1.0f - (float)block->amount / 255.0f)));
				transparency.appendChild(f);
				phong.appendChild(transparency);
			}
		}

		QDomElement index_of_refraction = m_doc.createElement("index_of_refraction");
		{
			QDomElement f = m_doc.createElement("float");
			f.setAttribute("sid", "index_of_refraction");
			f.appendChild(m_doc.createTextNode("1"));
			index_of_refraction.appendChild(f);
		}
		phong.appendChild(index_of_refraction);
		technique.appendChild(phong);
		profile_COMMON.appendChild(technique);
		effect.appendChild(profile_COMMON);
		effects.appendChild(effect);
	}
}

void CDAEExporter::_writeMaterials()
{
	QDomElement materials = m_doc.createElement("library_materials");
	m_colladaNode.appendChild(materials);

	for (auto it = m_materials.begin(); it != m_materials.end(); it++)
	{
		QDomElement material = m_doc.createElement("material");
		material.setAttribute("id", it.key() + "-material");
		material.setAttribute("name", it.key());
		QDomElement instance_effect = m_doc.createElement("instance_effect");
		instance_effect.setAttribute("url", '#' + it.key() + "-effect");
		material.appendChild(instance_effect);
		materials.appendChild(material);
	}
}

void CDAEExporter::_writeGeometries()
{
	QDomElement geomerties = m_doc.createElement("library_geometries");

	GMObject* obj;
	for (auto it = m_objects.begin(); it != m_objects.end(); it++)
	{
		obj = it.value();
		const QString meshID = it.key() + "-mesh";

		QDomElement geometry = m_doc.createElement("geometry");
		geometry.setAttribute("id", meshID);
		geometry.setAttribute("name", it.key());
		geomerties.appendChild(geometry);

		QDomElement mesh = m_doc.createElement("mesh");
		geometry.appendChild(mesh);

		const int vertexCount = obj->vertexCount;
		D3DXVECTOR3 v;

		{
			const QString srcID = meshID + "-positions";
			QDomElement source = m_doc.createElement("source");
			source.setAttribute("id", srcID);
			mesh.appendChild(source);

			QDomElement float_array = m_doc.createElement("float_array");
			float_array.setAttribute("id", srcID + "-array");
			float_array.setAttribute("count", vertexCount * 3);
			QString content;

			for (int i = 0; i < vertexCount; i++)
			{
				if (obj->type == GMT_SKIN)
					v = ((SkinVertex*)obj->vertices)[i].p;
				else
					v = ((NormalVertex*)obj->vertices)[i].p;
				content.append(QString::number(v.x, 'g', 7));
				content.append(' ');
				content.append(QString::number(v.y, 'g', 7));
				content.append(' ');
				content.append(QString::number(-v.z, 'g', 7));
				content.append(' ');
			}
			float_array.appendChild(m_doc.createTextNode(content));
			source.appendChild(float_array);

			QDomElement technique_common = m_doc.createElement("technique_common");
			QDomElement accessor = m_doc.createElement("accessor");
			accessor.setAttribute("stride", "3");
			accessor.setAttribute("count", vertexCount);
			accessor.setAttribute("source", '#' + srcID + "-array");
			QDomElement paramX = m_doc.createElement("param");
			paramX.setAttribute("name", "X"); paramX.setAttribute("type", "float");
			accessor.appendChild(paramX);
			QDomElement paramY = m_doc.createElement("param");
			paramY.setAttribute("name", "Y"); paramY.setAttribute("type", "float");
			accessor.appendChild(paramY);
			QDomElement paramZ = m_doc.createElement("param");
			paramZ.setAttribute("name", "Z"); paramZ.setAttribute("type", "float");
			accessor.appendChild(paramZ);
			technique_common.appendChild(accessor);
			source.appendChild(technique_common);
		}

		{
			const QString srcID = meshID + "-normals";
			QDomElement source = m_doc.createElement("source");
			source.setAttribute("id", srcID);
			mesh.appendChild(source);

			QDomElement float_array = m_doc.createElement("float_array");
			float_array.setAttribute("id", srcID + "-array");
			float_array.setAttribute("count", vertexCount * 3);
			QString content;

			for (int i = 0; i < vertexCount; i++)
			{
				if (obj->type == GMT_SKIN)
					v = ((SkinVertex*)obj->vertices)[i].n;
				else
					v = ((NormalVertex*)obj->vertices)[i].n;
				content.append(QString::number(v.x, 'g', 7));
				content.append(' ');
				content.append(QString::number(v.y, 'g', 7));
				content.append(' ');
				content.append(QString::number(-v.z, 'g', 7));
				content.append(' ');
			}
			float_array.appendChild(m_doc.createTextNode(content));
			source.appendChild(float_array);

			QDomElement technique_common = m_doc.createElement("technique_common");
			QDomElement accessor = m_doc.createElement("accessor");
			accessor.setAttribute("stride", "3");
			accessor.setAttribute("count", vertexCount);
			accessor.setAttribute("source", '#' + srcID + "-array");
			QDomElement paramX = m_doc.createElement("param");
			paramX.setAttribute("name", "X"); paramX.setAttribute("type", "float");
			accessor.appendChild(paramX);
			QDomElement paramY = m_doc.createElement("param");
			paramY.setAttribute("name", "Y"); paramY.setAttribute("type", "float");
			accessor.appendChild(paramY);
			QDomElement paramZ = m_doc.createElement("param");
			paramZ.setAttribute("name", "Z"); paramZ.setAttribute("type", "float");
			accessor.appendChild(paramZ);
			technique_common.appendChild(accessor);
			source.appendChild(technique_common);
		}

		{
			const QString srcID = meshID + "-map-0";
			QDomElement source = m_doc.createElement("source");
			source.setAttribute("id", srcID);
			mesh.appendChild(source);

			QDomElement float_array = m_doc.createElement("float_array");
			float_array.setAttribute("count", vertexCount * 2);
			float_array.setAttribute("id", srcID + "-array");
			QString content;
			D3DXVECTOR2 v2;
			for (int i = 0; i < vertexCount; i++)
			{
				if (obj->type == GMT_SKIN)
					v2 = ((SkinVertex*)obj->vertices)[i].t;
				else
					v2 = ((NormalVertex*)obj->vertices)[i].t;

				content.append(QString::number(v2.x));
				content.append(' ');
				content.append(QString::number(1.0f - v2.y));
				content.append(' ');
			}
			float_array.appendChild(m_doc.createTextNode(content));
			source.appendChild(float_array);

			QDomElement technique_common = m_doc.createElement("technique_common");
			QDomElement accessor = m_doc.createElement("accessor");
			accessor.setAttribute("stride", "2");
			accessor.setAttribute("count", vertexCount);
			accessor.setAttribute("source", '#' + srcID + "-array");
			QDomElement paramX = m_doc.createElement("param");
			paramX.setAttribute("name", "S"); paramX.setAttribute("type", "float");
			accessor.appendChild(paramX);
			QDomElement paramY = m_doc.createElement("param");
			paramY.setAttribute("name", "T"); paramY.setAttribute("type", "float");
			accessor.appendChild(paramY);
			technique_common.appendChild(accessor);
			source.appendChild(technique_common);
		}

		QDomElement vertices = m_doc.createElement("vertices");
		vertices.setAttribute("id", meshID + "-vertices");
		{
			QDomElement input = m_doc.createElement("input");
			input.setAttribute("semantic", "POSITION");
			input.setAttribute("source", '#' + meshID + "-positions");
			vertices.appendChild(input);
		}
		mesh.appendChild(vertices);

		MaterialBlock* block;
		for (int i = 0; i < obj->materialBlockCount; i++)
		{
			block = &obj->materialBlocks[i];
			Material* mat = null;

			if (obj->material)
				mat = &obj->materials[block->materialID];

			QDomElement polylist = m_doc.createElement("polylist");
			if (mat)
				polylist.setAttribute("material", _getMaterialID(mat) + "-material");
			polylist.setAttribute("count", block->primitiveCount);
			mesh.appendChild(polylist);

			{
				QDomElement input = m_doc.createElement("input");
				input.setAttribute("offset", "0");
				input.setAttribute("semantic", "VERTEX");
				input.setAttribute("source", '#' + meshID + "-vertices");
				polylist.appendChild(input);
			}
			{
				QDomElement input = m_doc.createElement("input");
				input.setAttribute("offset", "1");
				input.setAttribute("semantic", "NORMAL");
				input.setAttribute("source", '#' + meshID + "-normals");
				polylist.appendChild(input);
			}
			{
				QDomElement input = m_doc.createElement("input");
				input.setAttribute("set", "0");
				input.setAttribute("offset", "2");
				input.setAttribute("semantic", "TEXCOORD");
				input.setAttribute("source", '#' + meshID + "-map-0");
				polylist.appendChild(input);
			}

			QDomElement vcount = m_doc.createElement("vcount");
			QString vcountList;
			for (int i = 0; i < block->primitiveCount; i++)
				vcountList.append("3 ");
			vcount.appendChild(m_doc.createTextNode(vcountList));
			polylist.appendChild(vcount);

			QDomElement p = m_doc.createElement("p");
			QString pList;

			for (int i = 0; i < block->primitiveCount * 3; i++)
			{
				const QString s = QString::number(obj->indices[i + block->startVertex]) + ' ';
				pList.append(s);
				pList.append(s);
				pList.append(s);
			}

			p.appendChild(m_doc.createTextNode(pList));
			polylist.appendChild(p);
		}
	}

	m_colladaNode.appendChild(geomerties);
}

void CDAEExporter::_writeAnimations()
{
	QDomElement animations = m_doc.createElement("library_animations");
	m_colladaNode.appendChild(animations);

	TMAnimation* frames = null;
	for (auto it = m_animations.begin(); it != m_animations.end(); it++)
	{
		const string aniID = it.key();
		frames = it.value();

		QDomElement animation = m_doc.createElement("animation");
		animation.setAttribute("id", aniID);
		animations.appendChild(animation);

		{
			const QString srcID = aniID + "-input";
			QDomElement source = m_doc.createElement("source");
			source.setAttribute("id", srcID);
			animation.appendChild(source);

			QDomElement float_array = m_doc.createElement("float_array");
			float_array.setAttribute("id", srcID + "-array");
			float_array.setAttribute("count", QString::number(m_frameCount));
			QString fList;
			for (int i = 0; i < m_frameCount; i++)
			{
				fList.append(QString::number((1.0f / 30.0f) * (float)i));
				fList.append(' ');
			}
			float_array.appendChild(m_doc.createTextNode(fList));
			source.appendChild(float_array);

			QDomElement technique_common = m_doc.createElement("technique_common");
			source.appendChild(technique_common);

			QDomElement accessor = m_doc.createElement("accessor");
			accessor.setAttribute("source", '#' + srcID + "-array");
			accessor.setAttribute("count", QString::number(m_frameCount));
			accessor.setAttribute("stride", "1");
			technique_common.appendChild(accessor);

			QDomElement param = m_doc.createElement("param");
			param.setAttribute("name", "TIME");
			param.setAttribute("type", "float");
			accessor.appendChild(param);
		}
		{
			const QString srcID = aniID + "-output";
			QDomElement source = m_doc.createElement("source");
			source.setAttribute("id", srcID);
			animation.appendChild(source);

			QDomElement float_array = m_doc.createElement("float_array");
			float_array.setAttribute("id", srcID + "-array");
			float_array.setAttribute("count", QString::number(m_frameCount * 16));
			QString fList;
			D3DXMATRIX m1, m2;
			D3DXVECTOR3 trans;
			D3DXQUATERNION quat;

			for (int i = 0; i < m_frameCount; i++)
			{
				trans = frames[i].pos;
				quat = frames[i].rot;

				D3DXMatrixTranslation(&m1, trans.x, trans.y, trans.z);
				m2 = _getRotationMatrix(quat);

				m2 *= m1;

				fList.append(_matToString(m2));
			}
			float_array.appendChild(m_doc.createTextNode(fList));
			source.appendChild(float_array);

			QDomElement technique_common = m_doc.createElement("technique_common");
			source.appendChild(technique_common);

			QDomElement accessor = m_doc.createElement("accessor");
			accessor.setAttribute("source", '#' + srcID + "-array");
			accessor.setAttribute("count", QString::number(m_frameCount));
			accessor.setAttribute("stride", "16");
			technique_common.appendChild(accessor);

			QDomElement param = m_doc.createElement("param");
			param.setAttribute("name", "TRANSFORM");
			param.setAttribute("type", "float4x4");
			accessor.appendChild(param);
		}
		{
			const QString srcID = aniID + "-interpolation";
			QDomElement source = m_doc.createElement("source");
			source.setAttribute("id", srcID);
			animation.appendChild(source);

			QDomElement Name_array = m_doc.createElement("Name_array");
			Name_array.setAttribute("id", srcID + "-array");
			Name_array.setAttribute("count", QString::number(m_frameCount));
			QString nameList;
			for (int i = 0; i < m_frameCount; i++)
				nameList.append("LINEAR ");
			Name_array.appendChild(m_doc.createTextNode(nameList));
			source.appendChild(Name_array);

			QDomElement technique_common = m_doc.createElement("technique_common");
			source.appendChild(technique_common);

			QDomElement accessor = m_doc.createElement("accessor");
			accessor.setAttribute("source", '#' + srcID + "-array");
			accessor.setAttribute("count", QString::number(m_frameCount));
			accessor.setAttribute("stride", "1");
			technique_common.appendChild(accessor);

			QDomElement param = m_doc.createElement("param");
			param.setAttribute("name", "INTERPOLATION");
			param.setAttribute("type", "name");
			accessor.appendChild(param);
		}

		QDomElement sampler = m_doc.createElement("sampler");
		sampler.setAttribute("id", aniID + "-sampler");
		animation.appendChild(sampler);

		{
			QDomElement input = m_doc.createElement("input");
			input.setAttribute("semantic", "INPUT");
			input.setAttribute("source", '#' + aniID + "-input");
			sampler.appendChild(input);
		}
		{
			QDomElement input = m_doc.createElement("input");
			input.setAttribute("semantic", "OUTPUT");
			input.setAttribute("source", '#' + aniID + "-output");
			sampler.appendChild(input);
		}
		{
			QDomElement input = m_doc.createElement("input");
			input.setAttribute("semantic", "INTERPOLATION");
			input.setAttribute("source", '#' + aniID + "-interpolation");
			sampler.appendChild(input);
		}

		QDomElement channel = m_doc.createElement("channel");
		channel.setAttribute("source", '#' + aniID + "-sampler");

		string objID = aniID;
		objID.remove(aniID.size() - 10, 10);
		channel.setAttribute("target", objID + "/transform");
		animation.appendChild(channel);
	}
}

void CDAEExporter::_writeControllers()
{
	QDomElement controllers = m_doc.createElement("library_controllers");
	m_colladaNode.appendChild(controllers);

	GMObject* obj;
	for (auto it = m_objects.begin(); it != m_objects.end(); it++)
	{
		obj = it.value();
		if (obj->type != GMT_SKIN)
			continue;

		SkinVertex* vertices = (SkinVertex*)obj->vertices;
		const int vertexCount = obj->vertexCount;

		const QString objID = it.key();
		const QString skinID = objID + "-skin";

		QDomElement controller = m_doc.createElement("controller");
		controller.setAttribute("id", skinID);
		controller.setAttribute("name", objID + "_armature");
		controllers.appendChild(controller);

		QDomElement skin = m_doc.createElement("skin");
		skin.setAttribute("source", '#' + objID + "-mesh");
		controller.appendChild(skin);

		QDomElement bind_shape_matrix = m_doc.createElement("bind_shape_matrix");
		bind_shape_matrix.appendChild(m_doc.createTextNode(_identityMat()));
		skin.appendChild(bind_shape_matrix);

		QVector<Bone*> bones;
		for (int i = 0; i < m_boneIDs.size(); i++)
		{
			for (auto it2 = m_boneIDs.begin(); it2 != m_boneIDs.end(); it2++)
			{
				if (it2.value() == i)
				{
					bones.push_back(it2.key());
					break;
				}
			}
		}

		{
			const QString srcID = skinID + "-joints";
			QDomElement source = m_doc.createElement("source");
			source.setAttribute("id", srcID);
			skin.appendChild(source);

			QDomElement Name_array = m_doc.createElement("Name_array");
			Name_array.setAttribute("id", srcID + "-array");
			Name_array.setAttribute("count", QString::number(m_bones.size()));
			QString names;
			for (int i = 0; i < bones.size(); i++)
			{
				names.append(string(bones[i]->name).toLower().replace('.', '_').replace('-', '_').replace(' ', '_'));
				names.append(' ');
			}
			Name_array.appendChild(m_doc.createTextNode(names));
			source.appendChild(Name_array);

			QDomElement technique_common = m_doc.createElement("technique_common");
			source.appendChild(technique_common);

			QDomElement accessor = m_doc.createElement("accessor");
			accessor.setAttribute("source", '#' + srcID + "-array");
			accessor.setAttribute("count", QString::number(m_bones.size()));
			accessor.setAttribute("stride", "1");
			technique_common.appendChild(accessor);

			QDomElement param = m_doc.createElement("param");
			param.setAttribute("name", "JOINT");
			param.setAttribute("type", "name");
			accessor.appendChild(param);
		}
		{
			const QString srcID = skinID + "-bind_poses";
			QDomElement source = m_doc.createElement("source");
			source.setAttribute("id", srcID);
			skin.appendChild(source);

			QDomElement float_array = m_doc.createElement("float_array");
			float_array.setAttribute("id", srcID + "-array");
			float_array.setAttribute("count", QString::number(m_bones.size() * 16));
			QString transforms;
			for (int i = 0; i < bones.size(); i++)
				transforms.append(_matToString(bones[i]->inverseTM));
			float_array.appendChild(m_doc.createTextNode(transforms));
			source.appendChild(float_array);

			QDomElement technique_common = m_doc.createElement("technique_common");
			source.appendChild(technique_common);

			QDomElement accessor = m_doc.createElement("accessor");
			accessor.setAttribute("source", '#' + srcID + "-array");
			accessor.setAttribute("count", QString::number(m_bones.size()));
			accessor.setAttribute("stride", "16");
			technique_common.appendChild(accessor);

			QDomElement param = m_doc.createElement("param");
			param.setAttribute("name", "TRANSFORM");
			param.setAttribute("type", "float4x4");
			accessor.appendChild(param);
		}
		{
			const QString srcID = skinID + "-weights";
			QDomElement source = m_doc.createElement("source");
			source.setAttribute("id", srcID);
			skin.appendChild(source);

			QDomElement float_array = m_doc.createElement("float_array");
			float_array.setAttribute("id", srcID + "-array");
			float_array.setAttribute("count", QString::number(vertexCount * 2));
			QString weights;
			for (int i = 0; i < vertexCount; i++)
			{
				weights.append(QString::number(vertices[i].w1));
				weights.append(' ');
				weights.append(QString::number(vertices[i].w2));
				weights.append(' ');
			}
			float_array.appendChild(m_doc.createTextNode(weights));
			source.appendChild(float_array);

			QDomElement technique_common = m_doc.createElement("technique_common");
			source.appendChild(technique_common);

			QDomElement accessor = m_doc.createElement("accessor");
			accessor.setAttribute("source", '#' + srcID + "-array");
			accessor.setAttribute("count", QString::number(vertexCount * 2));
			accessor.setAttribute("stride", "1");
			technique_common.appendChild(accessor);

			QDomElement param = m_doc.createElement("param");
			param.setAttribute("name", "WEIGHT");
			param.setAttribute("type", "float");
			accessor.appendChild(param);
		}

		QDomElement joints = m_doc.createElement("joints");
		skin.appendChild(joints);

		{
			QDomElement input = m_doc.createElement("input");
			input.setAttribute("semantic", "JOINT");
			input.setAttribute("source", '#' + skinID + "-joints");
			joints.appendChild(input);
		}
		{
			QDomElement input = m_doc.createElement("input");
			input.setAttribute("semantic", "INV_BIND_MATRIX");
			input.setAttribute("source", '#' + skinID + "-bind_poses");
			joints.appendChild(input);
		}

		QDomElement vertex_weights = m_doc.createElement("vertex_weights");
		vertex_weights.setAttribute("count", vertexCount);
		skin.appendChild(vertex_weights);

		{
			QDomElement input = m_doc.createElement("input");
			input.setAttribute("semantic", "JOINT");
			input.setAttribute("source", '#' + skinID + "-joints");
			input.setAttribute("offset", "0");
			vertex_weights.appendChild(input);
		}
		{
			QDomElement input = m_doc.createElement("input");
			input.setAttribute("semantic", "WEIGHT");
			input.setAttribute("source", '#' + skinID + "-weights");
			input.setAttribute("offset", "1");
			vertex_weights.appendChild(input);
		}

		QDomElement vcount = m_doc.createElement("vcount");
		vertex_weights.appendChild(vcount);
		QString vList;
		SkinVertex* vertex;
		for (int i = 0; i < vertexCount; i++)
		{
			vertex = &vertices[i];

			if (vertex->w1 != 0.0f && vertex->w2 != 0.0f)
				vList.append("2 ");
			else if (vertex->w1 != 0.0f || vertex->w2 != 0.0f)
				vList.append("1 ");
			else
				vList.append("0 ");
		}
		vcount.appendChild(m_doc.createTextNode(vList));

		QDomElement v = m_doc.createElement("v");
		vertex_weights.appendChild(v);
		vList.clear();
		VertexWeight* vertexWeights = new VertexWeight[vertexCount];

		for (int i = 0; i < vertexCount; i++)
		{
			vertexWeights[i].boneIds[0] = -1;
			vertexWeights[i].boneIds[1] = -1;
		}

		int boneIds[MAX_BONES];
		MaterialBlock* block;
		for (int i = 0; i < obj->materialBlockCount; i++)
		{
			block = &obj->materialBlocks[i];

			for (int j = 0; j < MAX_BONES; j++)
				boneIds[j] = j;

			if (obj->usedBoneCount > 0)
			{
				for (int j = 0; j < obj->usedBoneCount; j++)
					boneIds[j] = obj->usedBones[j];
			}

			if (block->usedBoneCount > 0)
			{
				for (int j = 0; j < block->usedBoneCount; j++)
					boneIds[j] = block->usedBones[j];
			}

			for (int j = 0; j < block->primitiveCount * 3; j++)
			{
				const ushort vertexId = obj->indices[j + block->startVertex];
				vertex = &vertices[vertexId];

				if (vertex->w1 != 0.0f)
				{
					vertexWeights[vertexId].boneIds[0] = boneIds[vertex->id1 / 3];
					vertexWeights[vertexId].weightIDs[0] = vertexId * 2;
				}

				if (vertex->w2 != 0.0f)
				{
					vertexWeights[vertexId].boneIds[1] = boneIds[vertex->id2 / 3];
					vertexWeights[vertexId].weightIDs[1] = vertexId * 2 + 1;
				}
			}
		}

		for (int i = 0; i < vertexCount; i++)
		{
			if (vertexWeights[i].boneIds[0] != -1)
			{
				vList.append(QString::number(vertexWeights[i].boneIds[0]));
				vList.append(' ');
				vList.append(QString::number(vertexWeights[i].weightIDs[0]));
				vList.append(' ');
			}

			if (vertexWeights[i].boneIds[1] != -1)
			{
				vList.append(QString::number(vertexWeights[i].boneIds[1]));
				vList.append(' ');
				vList.append(QString::number(vertexWeights[i].weightIDs[1]));
				vList.append(' ');
			}
		}

		DeleteArray(vertexWeights);
		v.appendChild(m_doc.createTextNode(vList));
	}
}

void CDAEExporter::_writeVisualScenes()
{
	QDomElement visual_scenes = m_doc.createElement("library_visual_scenes");
	m_colladaNode.appendChild(visual_scenes);

	QDomElement visual_scene = m_doc.createElement("visual_scene");
	visual_scene.setAttribute("id", "Scene");
	visual_scene.setAttribute("name", "Scene");
	visual_scenes.appendChild(visual_scene);

	for (auto it = m_objects.begin(); it != m_objects.end(); it++)
	{
		if (it.value()->parentID == -1)
			_writeNode(&visual_scene, it.key(), it.value());
	}

	for (int i = 0; i < m_bones.size(); i++)
	{
		if (m_bones[i]->parentID == -1)
			_writeNode(&visual_scene, string(m_bones[i]->name).toLower().replace('.', '_').replace('-', '_').replace(' ', '_'), m_bones[i]);
	}
}

void CDAEExporter::_writeNode(QDomElement* parent, const string& name, GMObject* obj)
{
	QDomElement node = m_doc.createElement("node");
	node.setAttribute("id", name);
	node.setAttribute("name", name);
	node.setAttribute("type", "NODE");

	QDomElement matrix = m_doc.createElement("matrix");
	matrix.setAttribute("sid", "transform");
	matrix.appendChild(m_doc.createTextNode(_matToString(obj->transform)));
	node.appendChild(matrix);

	QDomElement instance_geometry;
	if (obj->type == GMT_SKIN)
	{
		instance_geometry = m_doc.createElement("instance_controller");
		instance_geometry.setAttribute("url", '#' + name + "-skin");

		QDomElement skeleton = m_doc.createElement("skeleton");
		skeleton.appendChild(m_doc.createTextNode('#' + m_rootBoneID));
		instance_geometry.appendChild(skeleton);
	}
	else
	{
		instance_geometry = m_doc.createElement("instance_geometry");
		instance_geometry.setAttribute("url", '#' + name + "-mesh");
	}

	if (obj->material)
	{
		QDomElement bind_material = m_doc.createElement("bind_material");
		QDomElement technique_common = m_doc.createElement("technique_common");

		Material* mat;
		for (int i = 0; i < obj->materialBlockCount; i++)
		{
			mat = &obj->materials[obj->materialBlocks[i].materialID];
			QDomElement instance_material = m_doc.createElement("instance_material");
			const QString matID = _getMaterialID(mat) + "-material";
			instance_material.setAttribute("symbol", matID);
			instance_material.setAttribute("target", '#' + matID);
			QDomElement bind_vertex_input = m_doc.createElement("bind_vertex_input");
			bind_vertex_input.setAttribute("semantic", "UVMap");
			bind_vertex_input.setAttribute("input_semantic", "TEXCOORD");
			bind_vertex_input.setAttribute("input_set", "0");
			instance_material.appendChild(bind_vertex_input);
			technique_common.appendChild(instance_material);
		}

		bind_material.appendChild(technique_common);
		instance_geometry.appendChild(bind_material);
	}

	node.appendChild(instance_geometry);

	for (auto it = m_objects.begin(); it != m_objects.end(); it++)
	{
		if (it.value()->parentID == m_objectIDs[obj]
			&& m_objectLODs[it.value()] == m_objectLODs[obj]
			&& it.value()->parentType != GMT_BONE)
			_writeNode(&node, it.key(), it.value());
	}

	parent->appendChild(node);
}

void CDAEExporter::_writeNode(QDomElement* parent, const string& name, Bone* bone)
{
	QDomElement node = m_doc.createElement("node");
	node.setAttribute("id", name);
	node.setAttribute("name", name);
	node.setAttribute("sid", name);
	node.setAttribute("type", "JOINT");

	QDomElement matrix = m_doc.createElement("matrix");
	matrix.setAttribute("sid", "transform");

	auto it = m_boneAnimTMs.constFind(bone);
	if (it == m_boneAnimTMs.constEnd())
		matrix.appendChild(m_doc.createTextNode(_matToString(bone->localTM)));
	else
		matrix.appendChild(m_doc.createTextNode(_matToString(it.value())));

	node.appendChild(matrix);

	for (int i = 0; i < m_bones.size(); i++)
	{
		if (m_bones[i]->parentID == m_boneIDs[bone])
			_writeNode(&node, string(m_bones[i]->name).toLower().replace('.', '_').replace('-', '_').replace(' ', '_'), m_bones[i]);
	}

	for (auto it = m_objects.begin(); it != m_objects.end(); it++)
	{
		if (it.value()->parentID == m_boneIDs[bone]
			&& it.value()->parentType == GMT_BONE)
			_writeNode(&node, it.key(), it.value());
	}

	parent->appendChild(node);
}

void CDAEExporter::_writeScene()
{
	QDomElement scene = m_doc.createElement("scene");
	m_colladaNode.appendChild(scene);

	QDomElement visual_scene = m_doc.createElement("instance_visual_scene");
	visual_scene.setAttribute("url", "#Scene");
	scene.appendChild(visual_scene);
}

string CDAEExporter::_matToString(const D3DXMATRIX& mat)
{
	const D3DXMATRIX tempMat(
		mat._11, mat._12, -mat._13, mat._14,
		mat._21, mat._22, -mat._23, mat._24,
		-mat._31, -mat._32, mat._33, -mat._34,
		mat._41, mat._42, -mat._43, mat._44
		);

	string s;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			s.append(string::number(tempMat[j * 4 + i]));
			s.append(' ');
		}
	}
	return s;
}

string CDAEExporter::_identityMat()
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	return _matToString(mat);
}