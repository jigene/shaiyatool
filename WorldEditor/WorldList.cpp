///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include <World.h>
#include <TextFile.h>
#include <Project.h>
#include <Object.h>
#include <Landscape.h>

void CMainFrame::GenerateList()
{
	if (!m_world)
		return;

	HideDialogs();
	const string filename = QFileDialog::getSaveFileName(this, tr("Enregistrer une liste"), m_filename.isEmpty() ? "" : m_filename.replace(".wld", ".list.txt"), tr("Fichier texte") + " (*.txt)");
	ShowDialogs();

	if (!filename.isEmpty())
	{
		QFile file(filename);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			qCritical(("Cant create file '" + filename + "'").toLocal8Bit());
			return;
		}
		const byte header[2] = { 0xff, 0xfe };
		file.write((const char*)header, (qint64)sizeof(header));
		QTextStream out(&file);
		out.setCodec("UTF-16LE");
		out.setRealNumberNotation(QTextStream::FixedNotation);

		const string listName = QFileInfo(filename).baseName();

		out << "//////" << endl;
		out << tr("// Liste de ") << listName << endl;
		out << tr("// Générée par ATools - WorldEditor") << endl;
		out << "//////" << endl;
		out << endl;

		int i, j;
		CObject* obj;
		CLandscape* land;
		QVector<string> used;
		QVector<string>::iterator it;

		for (i = 0; i < MAX_OBJTYPE; i++)
		{
			for (j = 0; j < m_world->m_objects[i].GetSize(); j++)
			{
				obj = m_world->m_objects[i].GetAt(j);
				if (obj->m_isReal && obj->m_modelProp &&
					(obj->m_modelProp->modelType == MODELTYPE_ANIMATED_MESH || obj->m_modelProp->modelType == MODELTYPE_MESH))
				{
					const string name = obj->GetModelFilename();
					if (!used.contains(name))
						used.push_back(name);
				}
			}
		}

		out << "// " << used.size() << tr(" models utilisés") << endl;
		for (it = used.begin(); it != used.end(); it++)
			out << *it << endl;
		out << endl;

		used.clear();

		for (i = 0; i < MAX_OBJTYPE; i++)
		{
			for (j = 0; j < m_world->m_objects[i].GetSize(); j++)
			{
				obj = m_world->m_objects[i].GetAt(j);
				if (obj->m_isReal && obj->m_modelProp && obj->m_modelProp->modelType == MODELTYPE_SFX)
				{
					const string name = obj->GetModelFilename();
					if (!used.contains(name))
						used.push_back(name);
				}
			}
		}

		out << "// " << used.size() << tr(" SFX utilisés") << endl;
		for (it = used.begin(); it != used.end(); it++)
			out << *it << endl;
		out << endl;

		used.clear();

		for (i = 0; i < m_world->m_width * m_world->m_height; i++)
		{
			land = m_world->m_lands[i];
			if (land)
			{
				for (j = 0; j < land->m_layers.GetSize(); j++)
				{
					const string name = Project->GetTerrainFilename(land->m_layers[j]->textureID);
					if (!used.contains(name))
						used.push_back(name);
				}
			}
		}

		out << "// " << used.size() << tr(" textures de terrain utilisées") << endl;
		for (it = used.begin(); it != used.end(); it++)
			out << *it << endl;
		out << endl;

		QVector<int> terrains;

		for (i = 0; i < m_world->m_width * m_world->m_height; i++)
		{
			land = m_world->m_lands[i];
			if (land)
			{
				for (j = 0; j < land->m_layers.GetSize(); j++)
				{
					if (!terrains.contains(land->m_layers[j]->textureID))
						terrains.push_back(land->m_layers[j]->textureID);
				}
			}
		}

		out << tr("// Code terrain.inc") << endl;
		out << '"' << listName << "\" 0" << endl;
		out << '{' << endl;
		for (i = 0; i < terrains.size(); i++)
			out << '\t' << terrains[i] << " 0 \"" << Project->GetTerrainFilename(terrains[i]) << "\" 0 \"\"" << endl;
		out << '}' << endl << endl;

		out << tr("// Code mdlObj.inc") << endl;
		_writeObjectsList(out, OT_OBJ, listName, "obj", "");
		out << endl;

		out << tr("// Code mdlDyna.inc") << endl;
		_writeObjectsList(out, OT_CTRL, listName, "ctrl", "CI_");
		_writeObjectsList(out, OT_SFX, listName, "sfx", "XI_");
		_writeObjectsList(out, OT_ITEM, listName, "item", "II_");
		_writeObjectsList(out, OT_MOVER, listName, "mvr", "MI_");
		_writeObjectsList(out, OT_SHIP, listName, "ship", "");
		/*_writeObjectsList(out, OT_REGION, listName, "region", "RI_");
		_writeObjectsList(out, OT_PATH, listName, "Path", "RI_");*/

		file.close();
	}
}

void CMainFrame::_writeObjectsList(QTextStream& out, int objType, const string& filename, const string& name, const string& defBegin)
{
	int i, j;
	CObject* obj;
	QVector<ModelProp*> props;

	for (i = 0; i < m_world->m_objects[objType].GetSize(); i++)
	{
		obj = m_world->m_objects[objType].GetAt(i);
		if (obj->m_isReal && obj->m_modelProp)
		{
			if (!props.contains(obj->m_modelProp))
				props.push_back(obj->m_modelProp);
		}
	}

	if (props.size() <= 0)
		return;

	out << '"' << name << "\" " << objType << endl;
	out << '{' << endl;
	out << "\t\"" << filename << '"' << endl;
	out << "\t{" << endl;

	ModelProp* prop;
	for (i = 0; i < props.size(); i++)
	{
		prop = props[i];
		out << "\t\t\"" << prop->filename << "\" ";

		if (defBegin.isEmpty())
			out << prop->ID;
		else
			out << CTextFile::GetDefineText(prop->ID, defBegin);

		switch (prop->modelType)
		{
		case MODELTYPE_BILLBOARD:
			out << " MODELTYPE_BILLBOARD \"";
			break;
		case MODELTYPE_SFX:
			out << " MODELTYPE_SFX \"";
			break;
		case MODELTYPE_ANIMATED_MESH:
			out << " MODELTYPE_ANIMATED_MESH \"";
			break;
		case MODELTYPE_MESH:
			out << " MODELTYPE_MESH \"";
			break;
		}

		out << prop->part << "\" " << (prop->fly ? 1 : 0);

		switch (prop->distant)
		{
		case MD_FAR:
			out << " MD_FAR ";
			break;
		case MD_MID:
			out << " MD_MID ";
			break;
		case MD_NEAR:
			out << " MD_NEAR ";
			break;
		case MD_FIX:
			out << " MD_FIX ";
			break;
		}

		out << (prop->pick ? 1 : 0) << ' ' << prop->scale << ' ' << (prop->trans ? 1 : 0) << ' ' << (prop->shadow ? 1 : 0) << ' ';

		if (prop->textureEx == ATEX_00 || prop->textureEx == ATEX_NONE)
			out << " ATEX_NONE";
		else
			out << CTextFile::GetDefineText(prop->textureEx, "ATEX_");

		out << ' ' << (prop->renderFlag ? 1 : 0) << endl;

		if (prop->motionCount > 0)
		{
			out << "\t\t{" << endl;
			for (j = 0; j < prop->motionCount; j++)
				if (strlen(prop->GetMotion(j)) > 0)
					out << "\t\t\t\"" << prop->GetMotion(j) << "\" " << CTextFile::GetDefineText(j, "MTI_") << endl;
			out << "\t\t}" << endl;
		}
	}

	out << "\t}" << endl;
	out << '}' << endl;
}