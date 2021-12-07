///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "DataManager.h"
#include <TextFile.h>
#include <WndControl.h>

CDataManager* DataMng = null;

CDataManager::CDataManager(QComboBox* controlList)
{
	m_controlList = controlList;
	DataMng = this;
}

CDataManager::~CDataManager()
{
	int i;
	WindowData* window;
	for (auto it = m_windows.begin(); it != m_windows.end();)
	{
		window = it.value();
		for (i = 0; i < window->controls.GetSize(); i++)
			Delete(window->controls[i]);
		Delete(window);
		it = m_windows.erase(it);
	}

	DataMng = null;
}

WindowData* CDataManager::GetWindow(int ID) const
{
	auto it = m_windows.find(ID);
	if (it != m_windows.end())
		return it.value();
	return null;
}

WindowData* CDataManager::GetWindow(const string& ID) const
{
	const int realID = GetWindowID(ID);
	if (realID != -1)
		return GetWindow(realID);
	return null;
}

int CDataManager::GetWindowID(const string& ID) const
{
	auto it = m_windowIDs.find(ID);
	if (it != m_windowIDs.end())
		return it.value();
	it = m_newWindowIDs.find(ID);
	if (it != m_newWindowIDs.end())
		return it.value();
	return -1;
}

int CDataManager::GetNewWindowID() const
{
	int maxID = 0;
	for (auto it = m_windowIDs.begin(); it != m_windowIDs.end(); it++)
	{
		if (it.value() > maxID)
			maxID = it.value();
	}
	for (auto it = m_newWindowIDs.begin(); it != m_newWindowIDs.end(); it++)
	{
		if (it.value() > maxID)
			maxID = it.value();
	}
	return maxID + 1;
}

bool CDataManager::WindowExists(const string& ID)
{
	for (auto it = m_windowIDs.begin(); it != m_windowIDs.end(); it++)
	{
		if (it.key() == ID)
			return true;
	}
	for (auto it = m_newWindowIDs.begin(); it != m_newWindowIDs.end(); it++)
	{
		if (it.key() == ID)
			return true;
	}
	return false;
}

void CDataManager::SetText(const string& ID, const string& text)
{
	auto it = m_texts.find(ID);
	if (it != m_texts.end())
		m_texts[ID] = text;
	else
	{
		it = m_newTexts.find(ID);
		if (it != m_newTexts.end())
			m_newTexts[ID] = text;
	}
}

int CDataManager::GetControlID(const string& ID)
{
	auto it = m_controlIDs.find(ID);
	if (it != m_controlIDs.end())
		return it.value();

	int newID = 0;
	for (it = m_controlIDs.begin(); it != m_controlIDs.end(); it++)
	{
		if (it.value() > newID)
			newID = it.value();
	}
	newID++;
	m_controlIDs[ID] = newID;
	if (CWndControl::s_selection.GetSize() > 0)
	{
		CWndControl* ctrl = CWndControl::s_selection[0];
		CWndControl::s_selection[0] = null;
		FillControlList();
		CWndControl::s_selection[0] = ctrl;
	}
	else
		FillControlList();
	return newID;
}

void CDataManager::AddNewWindow(const string& name, int ID)
{
	m_newWindowIDs[name] = ID;
	WindowData* newWindow = new WindowData();
	newWindow->ID = ID;
	newWindow->define = name;
	newWindow->flags = WBS_MOVE | WBS_SOUND | WBS_CAPTION;
	newWindow->format = D3DFMT_A4R4G4B4;
	newWindow->size = QSize(320, 320);
	newWindow->texture = "WndTile00.tga";
	newWindow->tiles = true;
	newWindow->title = QObject::tr("Nouvelle fenêtre");
	newWindow->titleID = GetNewText(newWindow->title);
	newWindow->tooltip = "";
	newWindow->tooltipID = GetNewText(newWindow->tooltip);
	m_windows[ID] = newWindow;
}

string CDataManager::GetNewText(const string& text)
{
	int newTextID = 0;
	for (auto it = m_texts.begin(); it != m_texts.end(); it++)
	{
		const int ID = it.key().right(6).toInt();
		if (ID > newTextID)
			newTextID = ID;
	}
	for (auto it = m_newTexts.begin(); it != m_newTexts.end(); it++)
	{
		const int ID = it.key().right(6).toInt();
		if (ID > newTextID)
			newTextID = ID;
	}
	newTextID++;
	const string newText = string("IDS_RESDATA_INC_%1").arg(newTextID, 6, 10, QChar('0'));
	m_newTexts[newText] = text;
	return newText;
}

void CDataManager::RemoveText(const string& ID)
{
	auto it = m_newTexts.find(ID);
	if (it != m_newTexts.end())
		m_newTexts.erase(it);
}

void CDataManager::RemoveWindow(int ID)
{
	auto it = m_windows.find(ID);
	if (it == m_windows.end())
		return;

	WindowData* data = it.value();

	while (data->controls.GetSize() > 0)
		RemoveControl(ID, data->controls[0]->ID);

	RemoveText(data->titleID);
	RemoveText(data->tooltipID);

	auto it2 = m_newWindowIDs.find(data->define);
	if (it2 != m_newWindowIDs.end())
		m_newWindowIDs.erase(it2);

	Delete(data);
	m_windows.erase(it);
}

void CDataManager::RemoveControl(int windowID, int controlID)
{
	WindowData* data = GetWindow(windowID);
	if (!data)
		return;

	ControlData* ctrl = null;
	for (int i = 0; i < data->controls.GetSize(); i++)
	{
		ctrl = data->controls[i];
		if (ctrl->ID == controlID)
		{
			RemoveText(ctrl->textID);
			RemoveText(ctrl->tooltipID);

			Delete(ctrl);
			data->controls.RemoveAt(i);
			i--;
		}
	}
}

void CDataManager::FillWindowList(QStringListModel* list)
{
	QStringList stringList;
	for (auto it = m_windows.begin(); it != m_windows.end(); it++)
		stringList.push_back(it.value()->define);
	list->setStringList(stringList);
	list->sort(0);
}

void CDataManager::FillControlList()
{
	m_controlList->clear();
	for (auto it = m_controlIDs.begin(); it != m_controlIDs.end(); it++)
		m_controlList->addItem(it.key(), it.value());
}

bool CDataManager::Load()
{
	if (!CTextFile::LoadDefine("WndStyle.h")
		|| !_loadDefine("resData.h")
		|| !_loadText("resData.txt.txt")
		|| !_loadScript("resData.inc"))
		return false;
	return true;
}

bool CDataManager::Save()
{
	if (!_saveDefine("resData.h")
		|| !_saveText("resData.txt.txt")
		|| !_saveScript("resData.inc"))
		return false;
	return true;
}

bool CDataManager::_saveScript(const string& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical(("Cant create file '" + filename + "'").toLocal8Bit());
		return false;
	}
	const byte header[2] = { 0xff, 0xfe };
	file.write((const char*)header, (qint64)sizeof(header));
	QTextStream out(&file);
	out.setCodec("UTF-16LE");

	const string space = "    ";

	out << "// window resource script" << endl;
	WindowData* window;
	ControlData* control;
	string controlType;
	int i;
	for (auto it = m_windows.begin(); it != m_windows.end(); it++)
	{
		window = it.value();
		out << window->define << ' ';
		out << '"' << window->texture << "\" ";
#if __VER >= 19
		out << '"' << window->icon << "\" ";
#endif
		out << (window->tiles ? 1 : 0) << ' ';
		out << window->size.width() << ' ';
		out << window->size.height() << ' ';
		out << hex << "0x" << window->flags << dec << ' ';
		out << (int)window->format << endl;
		out << '{' << endl << "// Title String" << endl << window->titleID << endl << '}' << endl;
		out << '{' << endl << "// ToolTip" << endl << window->tooltipID << endl << '}' << endl;
		out << '{' << endl;

		for (i = 0; i < window->controls.GetSize(); i++)
		{
			control = window->controls[i];

			switch (control->type)
			{
			case WTYPE_BASE:
				controlType = "WTYPE_BASE";
				break;
			case WTYPE_BUTTON:
				controlType = "WTYPE_BUTTON";
				break;
			case WTYPE_LISTBOX:
				controlType = "WTYPE_LISTBOX";
				break;
			case WTYPE_LISTCTRL:
				controlType = "WTYPE_LISTCTRL";
				break;
			case WTYPE_STATIC:
				controlType = "WTYPE_STATIC";
				break;
			case WTYPE_COMBOBOX:
				controlType = "WTYPE_COMBOBOX";
				break;
			case WTYPE_SCROLLBAR:
				controlType = "WTYPE_SCROLLBAR";
				break;
			case WTYPE_EDITCTRL:
				controlType = "WTYPE_EDITCTRL";
				break;
			case WTYPE_TREECTRL:
				controlType = "WTYPE_TREECTRL";
				break;
			case WTYPE_TABCTRL:
				controlType = "WTYPE_TABCTRL";
				break;
			case WTYPE_CUSTOM:
				controlType = "WTYPE_CUSTOM";
				break;
			case WTYPE_MENU:
				controlType = "WTYPE_MENU";
				break;
			case WTYPE_TEXT:
				controlType = "WTYPE_TEXT";
				break;
			default:
				controlType = "WTYPE_CUSTOM";
				break;
			}

			out << space << controlType << ' ';
			out << control->define << ' ';
			out << '"' << control->texture << "\" ";
			out << (control->tiles ? 1 : 0) << ' ';
			out << control->rect.left() << ' ';
			out << control->rect.top() << ' ';
			out << control->rect.right() << ' ';
			out << control->rect.bottom() << ' ';
			out << hex << "0x" << control->flags << dec << ' ';
			out << (control->visible ? 1 : 0) << ' ';
			out << (control->group ? 1 : 0) << ' ';
			out << (control->disabled ? 1 : 0) << ' ';
			out << (control->tabStop ? 1 : 0);
#if __VER >= 19
			out << ' ' << control->color.red() << ' ' << control->color.green() << ' ' << control->color.blue();
#endif
			out << endl;
			out << space << '{' << endl << space << "// Title String" << endl << space << control->textID << endl << space << '}' << endl;
			out << space << '{' << endl << space << "// ToolTip" << endl << space << control->tooltipID << endl << space << '}' << endl;
		}

		out << endl << '}' << endl;
	}

	file.close();
	return true;
}

bool CDataManager::_saveDefine(const string& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical(("Cant create file '" + filename + "'").toLocal8Bit());
		return false;
	}
	QTextStream out(&file);
	out.setCodec("UTF-8");

	const string space = "             ";

	out << "#ifndef __RESOURCE_H" << endl;
	out << "#define __RESORUCE_H" << endl << endl;
	out << "// Applet Id" << endl;

	for (auto it = m_windowIDs.begin(); it != m_windowIDs.end(); it++)
		out << "#define " << it.key() << space << it.value() << endl;
	for (auto it = m_newWindowIDs.begin(); it != m_newWindowIDs.end(); it++)
		out << "#define " << it.key() << space << it.value() << endl;

	out << endl << "// Control Id" << endl;
	for (auto it = m_controlIDs.begin(); it != m_controlIDs.end(); it++)
		out << "#define " << it.key() << space << it.value() << endl;

	out << endl << "#endif" << endl;

	file.close();
	return true;
}

bool CDataManager::_saveText(const string& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical(("Cant create file '" + filename + "'").toLocal8Bit());
		return false;
	}
	const byte header[2] = { 0xff, 0xfe };
	file.write((const char*)header, (qint64)sizeof(header));
	QTextStream out(&file);
	out.setCodec("UTF-16LE");

	for (auto it = m_texts.begin(); it != m_texts.end(); it++)
	{
		out << it.key() << '\t' << it.value();
		if ((it + 1) != m_texts.end() || m_newTexts.size() > 0)
			out << endl;
	}
	for (auto it = m_newTexts.begin(); it != m_newTexts.end(); it++)
	{
		out << it.key() << '\t' << it.value();
		if ((it + 1) != m_newTexts.end())
			out << endl;
	}

	file.close();
	return true;
}

bool CDataManager::_loadScript(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	static const string beginIDS = "IDS";

	int i;
	int controlType;
	ControlData* control;
	WindowData* window;
	file.NextToken();
	do
	{
		window = new WindowData();
		window->define = file.Token();

		if (m_windowIDs.find(window->define) != m_windowIDs.end())
			window->ID = m_windowIDs[window->define];
		else
			window->ID = -1;

		window->texture = file.GetString();
#if __VER >= 19
		window->icon = file.GetString();
#endif
		window->tiles = file.GetBool();
		window->size.setWidth(file.GetInt());
		window->size.setHeight(file.GetInt());
		window->flags = file.GetInt();
		window->format = (D3DFORMAT)file.GetInt();

		file.NextToken(); // {
		window->titleID = file.GetString();
		if (!window->titleID.startsWith(beginIDS))
		{
			window->title = window->titleID;
			window->titleID.clear();
		}
		else
			window->title = m_texts[window->titleID];
		file.NextToken(); // }

		file.NextToken(); // {
		window->tooltipID = file.GetString();
		if (!window->tooltipID.startsWith(beginIDS))
		{
			window->tooltip = window->tooltipID;
			window->tooltipID.clear();
		}
		else
			window->tooltip = m_texts[window->tooltipID];
		file.NextToken(); // }

		file.NextToken(); // {

		controlType = file.GetInt();
		while (file.TokenType() != ETokenType::EndBlock)
		{
			control = new ControlData();
			control->type = controlType;
			control->define = file.GetString();

			if (m_controlIDs.find(control->define) != m_controlIDs.end())
				control->ID = m_controlIDs[control->define];
			else
				control->ID = -1;

			for (i = 0; i < window->controls.GetSize(); i++)
			{
				if (window->controls[i]->define == control->define)
				{
					qCritical((control->define + " in " + window->define + " already in use").toLocal8Bit());
					break;
				}
			}

			control->texture = file.GetString();
			control->tiles = file.GetBool();
			control->rect.setLeft(file.GetInt());
			control->rect.setTop(file.GetInt());
			control->rect.setRight(file.GetInt());
			control->rect.setBottom(file.GetInt());
			control->flags = file.GetInt();
			control->visible = file.GetBool();
			control->group = file.GetBool();
			control->disabled = file.GetBool();
			control->tabStop = file.GetBool();
#if __VER >= 19
			int r, g, b;
			r = file.GetInt();
			if (r < 0)
				r = 0;
			else if (r > 255)
				r = 255;
			g = file.GetInt();
			if (g < 0)
				g = 0;
			else if (g > 255)
				g = 255;
			b = file.GetInt();
			if (b < 0)
				b = 0;
			else if (b > 255)
				b = 255;
			control->color = QColor(r, g, b);
#endif

			file.NextToken(); // {
			control->textID = file.GetString();
			if (!control->textID.startsWith(beginIDS))
			{
				control->text = control->textID;
				control->textID.clear();
			}
			else
				control->text = m_texts[control->textID];
			file.NextToken(); // }

			file.NextToken(); // {
			control->tooltipID = file.GetString();
			if (!control->tooltipID.startsWith(beginIDS))
			{
				control->tooltip = control->tooltipID;
				control->tooltipID.clear();
			}
			else
				control->tooltip = m_texts[control->tooltipID];
			file.NextToken(); // }

			window->controls.Append(control);
			controlType = file.GetInt();
		}

		m_windows[window->ID] = window;
		file.NextToken();
	} while (file.TokenType() != ETokenType::End);

	for (auto it = m_windows.begin(); it != m_windows.end(); it++)
	{
		window = it.value();
		if (window->titleID.isEmpty())
			window->titleID = GetNewText(window->title);
		if (window->tooltipID.isEmpty())
			window->tooltipID = GetNewText(window->tooltip);

		for (i = 0; i < window->controls.GetSize(); i++)
		{
			control = window->controls[i];
			if (control->textID.isEmpty())
				control->textID = GetNewText(control->text);
			if (control->tooltipID.isEmpty())
				control->tooltipID = GetNewText(control->tooltip);
		}

		if (window->ID == 2028)
		{
			bool b = false;
			b = true;
		}
	}

	file.Close();
	return true;
}

bool CDataManager::_loadDefine(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	int value;
	file.NextToken();
	do
	{
		if (file.TokenType() == ETokenType::Define)
		{
			const string id = file.GetString();
			const string tok = file.GetString();
			const ETokenType tokType = file.TokenType();

			if (tokType == ETokenType::Number)
				value = tok.toInt(null, 10);
			else if (tokType == ETokenType::Hex)
				value = tok.toInt(null, 16);
			else
			{
				file.PutBack();
				file.NextToken();
				continue;
			}

			if (id.startsWith("APP_"))
				m_windowIDs[id] = value;
			else if (id.startsWith("WIDC_"))
				m_controlIDs[id] = value;
		}

		file.NextToken();
	} while (file.TokenType() != ETokenType::End);

	file.Close();
	return true;
}

bool CDataManager::_loadText(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	file.NextToken();
	while (file.TokenType() != ETokenType::End)
	{
		const string id = file.Token();
		if (!id.startsWith("IDS"))
		{
			qCritical(("LoadText : " + id).toLocal8Bit());
			file.NextToken();
			continue;
		}

		m_texts[id] = file.GetLine();
		file.NextToken();
	}

	file.Close();
	return true;
}