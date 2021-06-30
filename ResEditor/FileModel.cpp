///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "FileModel.h"

QIcon CFileModel::s_icons[10];
QMap<string, int> CFileModel::s_exts;

void CFileModel::InitResources()
{
	s_icons[0].addFile(":/MainFrame/Resources/default.png");
	s_icons[1].addFile(":/MainFrame/Resources/code.png");
	s_icons[2].addFile(":/MainFrame/Resources/csv.png");
	s_icons[3].addFile(":/MainFrame/Resources/header.png");
	s_icons[4].addFile(":/MainFrame/Resources/system.png");
	s_icons[5].addFile(":/MainFrame/Resources/picture.png");
	s_icons[6].addFile(":/MainFrame/Resources/text.png");
	s_exts["inc"] = 1;
	s_exts["c"] = 1;
	s_exts["cpp"] = 1;
	s_exts["lua"] = 1;
	s_exts["csv"] = 2;
	s_exts["h"] = 3;
	s_exts["hpp"] = 3;
	s_exts["dll"] = 4;
	s_exts["exe"] = 4;
	s_exts["ini"] = 4;
	s_exts["png"] = 5;
	s_exts["gif"] = 5;
	s_exts["dds"] = 5;
	s_exts["tga"] = 5;
	s_exts["jpg"] = 5;
	s_exts["jpeg"] = 5;
	s_exts["bmp"] = 5;
	s_exts["txt"] = 6;
}

CFileModel::CFileModel(QWidget* parent)
	: QAbstractListModel(parent),
	m_encrypted(false),
	m_sortFunc(SORT_NAME)
{
	m_key = (byte)(qrand() % 256);
}

CFileModel::~CFileModel()
{
	for (int i = 0; i < m_resources.GetSize(); i++)
	{
		DeleteArray(m_resources[i]->data);
		Delete(m_resources[i]);
	}
}

void CFileModel::_encrypt(byte* data, uint size)
{
	byte temp;
	for (uint i = 0; i < size; i++)
	{
		temp = data[i];
		temp = (temp << 4) | (temp >> 4);
		data[i] = ~temp ^ m_key;
	}
}

void CFileModel::_decrypt(byte* data, uint size)
{
	for (uint i = 0; i < size; i++)
	{
		const byte temp = ~data[i] ^ m_key;
		data[i] = (temp << 4) | (temp >> 4);
	}
}

bool CFileModel::Load(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::ReadOnly))
		return false;

	file.Read(m_key);
	file.Read(m_encrypted);

	int headerSize;
	file.Read(headerSize);

	byte* header = new byte[headerSize];
	file.Read(header, headerSize);
	_decrypt(header, headerSize);

	byte* cur = header;
	char version[8];
	memcpy(version, cur, 7); cur += 7;
	version[7] = '\0';

	short fileCount, nameSize;
	char name[_MAX_FNAME];
	Resource* res;

	memcpy(&fileCount, cur, sizeof(short)); cur += sizeof(short);
	for (short i = 0; i < fileCount; i++)
	{
		memcpy(&nameSize, cur, sizeof(short)); cur += sizeof(short);
		memcpy(name, cur, nameSize); cur += nameSize;
		name[nameSize] = '\0';

		res = new Resource();
		res->filename = name;
		memcpy(&res->size, cur, sizeof(int)); cur += sizeof(int);
		memcpy(&res->time, cur, sizeof(uint)); cur += sizeof(uint);
		memcpy(&res->offset, cur, sizeof(int)); cur += sizeof(int);

		m_resources.Append(res);
	}

	DeleteArray(header);

	byte* data;

	for (short i = 0; i < fileCount; i++)
	{
		res = m_resources[i];
		file.SetPos(res->offset);

		res->data = new byte[res->size];
		file.Read(res->data, res->size);

		if (m_encrypted)
			_decrypt(res->data, res->size);
	}

	file.Close();

	_sort();
	return true;
}

bool CFileModel::Save(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::WriteOnly))
		return false;

	Resource* res;
	int offset = 7 + sizeof(short);
	for (int i = 0; i < m_resources.GetSize(); i++)
	{
		res = m_resources[i];
		offset += sizeof(short);
		offset += res->filename.size();
		offset += sizeof(int);
		offset += sizeof(uint);
		offset += sizeof(int);
	}

	const int headerSize = offset;

	offset += 2 + sizeof(int);
	for (int i = 0; i < m_resources.GetSize(); i++)
	{
		res = m_resources[i];
		res->offset = offset;
		offset += res->size;
	}

	file.Write(m_key);
	file.Write(m_encrypted);
	file.Write(headerSize);

	byte* header = new byte[headerSize];
	byte* cur = header;

	const char version[8] = "\"v0.01\"";
	memcpy(cur, version, 7); cur += 7;

	const short fileCount = (short)m_resources.GetSize();
	memcpy(cur, &fileCount, sizeof(short)); cur += sizeof(short);

	for (int i = 0; i < m_resources.GetSize(); i++)
	{
		res = m_resources[i];
		const QByteArray name = res->filename.toLocal8Bit();
		const short nameSize = (short)name.size();

		memcpy(cur, &nameSize, sizeof(short)); cur += sizeof(short);
		memcpy(cur, name.constData(), name.size()); cur += name.size();
		memcpy(cur, &res->size, sizeof(int)); cur += sizeof(int);
		memcpy(cur, &res->time, sizeof(uint)); cur += sizeof(uint);
		memcpy(cur, &res->offset, sizeof(int)); cur += sizeof(int);
	}

	_encrypt(header, headerSize);
	file.Write(header, headerSize);

	byte* data;
	for (int i = 0; i < m_resources.GetSize(); i++)
	{
		res = m_resources[i];

		if (m_encrypted)
		{
			data = new byte[res->size];
			memcpy(data, res->data, res->size);
			_encrypt(data, res->size);
		}
		else
			data = res->data;

		file.Write(data, (int)res->size);

		if (m_encrypted)
			DeleteArray(data);
	}

	file.Close();
	return true;
}

void CFileModel::AddFiles(const QStringList& filenames)
{
	beginResetModel();

	Resource* res;
	CFile file;

	bool replaceFiles = false;
	bool hasExistingFiles = false;

	for (int i = 0; i < filenames.size(); i++)
		if (FileExists(filenames[i]))
			hasExistingFiles = true;

	if (hasExistingFiles)
		replaceFiles = QMessageBox::question(null, tr("Remplacer fichiers"), tr("Remplacer les fichiers existants ?")) == QMessageBox::Yes;

	for (int i = 0; i < filenames.size(); i++)
	{
		const string filename = filenames[i];
		if (GetExtension(filename) == "res")
			continue;

		if (file.Open(filename, QIODevice::ReadOnly))
		{
			res = null;

			if (FileExists(filename))
			{
				if (replaceFiles)
				{
					res = _getFile(filename);
					if (res)
						DeleteArray(res->data);
				}
			}
			else
			{
				res = new Resource();
				m_resources.Append(res);
			}

			if (res)
			{
				QFileInfo fileInfo(filename);
				res->filename = fileInfo.fileName();
				res->size = (uint)fileInfo.size();
				res->offset = 0;
				res->time = (uint)fileInfo.lastModified().toTime_t();
				res->data = new byte[res->size];
				file.Read(res->data, res->size);
			}

			file.Close();
		}
	}

	_sort();
	endResetModel();
}

void CFileModel::ExtractFiles(const string& dir, const QModelIndexList& selection)
{
	bool replaceFiles = false;
	bool hasExistingFiles = false;

	Resource* res;
	for (auto it = selection.begin(); it != selection.end(); it++)
	{
		res = m_resources[it->row()];
		if (QFileInfo(dir + '/' + res->filename).exists())
		{
			hasExistingFiles = true;
			break;
		}
	}

	if (hasExistingFiles)
		replaceFiles = QMessageBox::question(null, tr("Remplacer fichiers"), tr("Remplacer les fichiers existants ?")) == QMessageBox::Yes;

	CFile file;
	for (auto it = selection.begin(); it != selection.end(); it++)
	{
		res = m_resources[it->row()];
		if (QFileInfo(dir + '/' + res->filename).exists() && !replaceFiles)
			continue;
		
		if (file.Open(dir + '/' + res->filename, QIODevice::WriteOnly))
		{
			file.Write(res->data, res->size);
			file.Close();
		}
	}
}

void CFileModel::ExtractAllFiles(const string& dir, bool replaceFiles)
{
	CFile file;
	Resource* res;
	for (int i = 0; i < m_resources.GetSize(); i++)
	{
		res = m_resources[i];
		if (QFileInfo(dir + '/' + res->filename).exists() && !replaceFiles)
			continue;

		if (file.Open(dir + '/' + res->filename, QIODevice::WriteOnly))
		{
			file.Write(res->data, res->size);
			file.Close();
		}
	}
}

void CFileModel::DeleteFiles(const QModelIndexList& selection)
{
	beginResetModel();

	CPtrArray<Resource> selected;
	for (auto it = selection.begin(); it != selection.end(); it++)
		selected.Append(m_resources[it->row()]);

	int find;
	Resource* res;
	for (int i = 0; i < selected.GetSize(); i++)
	{
		res = selected[i];
		DeleteArray(res->data);

		find = m_resources.Find(res);
		if (find != -1)
			m_resources.RemoveAt(find);


		Delete(res);
	}

	endResetModel();
}

QVariant CFileModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || index.row() >= m_resources.GetSize())
		return QVariant();

	Resource* res = m_resources[index.row()];

	switch (role)
	{
	case Qt::DisplayRole:
	case Qt::EditRole:
		return res->filename;
	case Qt::ToolTipRole:
	{
		string size;
		if (res->size >= 1000000000)
			size = string::number(((double)res->size) / 1000000000.0, 'f', 2) + tr(" Go");
		else if (res->size >= 1000000)
			size = string::number(((double)res->size) / 1000000.0, 'f', 2) + tr(" Mo");
		else if (res->size >= 1000)
			size = string::number(((double)res->size) / 1000.0, 'f', 2) + tr(" Ko");
		else
			size = string::number(res->size) + tr(" o");
		return tr("Nom : ") + res->filename + tr("\nTaille : ") + size + tr("\nDate : ") + QDateTime::fromTime_t(res->time).toString();
	}
	case Qt::DecorationRole:
	{
		auto it = s_exts.find(GetExtension(res->filename));
		if (it != s_exts.end())
			return s_icons[it.value()];
		else
			return s_icons[0];
	}
	}

	return QVariant();
}

int CFileModel::rowCount(const QModelIndex& parent) const
{
	return m_resources.GetSize();
}

Qt::ItemFlags CFileModel::flags(const QModelIndex &index) const
{
	const Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);
	if (!index.isValid())
		return defaultFlags;

	return Qt::ItemIsEditable | defaultFlags;
}

bool CFileModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	const string name = value.toString();

	if (!index.isValid()
		|| role != Qt::EditRole
		|| index.row() >= m_resources.GetSize()
		|| name.length() <= 0)
		return false;

	if (m_resources[index.row()]->filename == name)
		return false;

	if (FileExists(name))
	{
		QMessageBox::critical(null, tr("Erreur"), tr("Un fichier possède déjà ce nom."));
		return false;
	}

	m_resources[index.row()]->filename = name;
	
	if (m_sortFunc == SORT_NAME)
		_sort();
	return true;
}

bool CFileModel::FileExists(const string& filename) const
{
	const string lower = QFileInfo(filename).fileName().toLower();

	for (int i = 0; i < m_resources.GetSize(); i++)
		if (m_resources[i]->filename.toLower() == lower)
			return true;
	return false;
}

CFileModel::Resource* CFileModel::_getFile(const string& filename)
{
	const string lower = QFileInfo(filename).fileName().toLower();

	for (int i = 0; i < m_resources.GetSize(); i++)
		if (m_resources[i]->filename.toLower() == lower)
			return m_resources[i];
	return null;
}

int SortResByName(const CPtrArray<CFileModel::Resource>::Type* res1, const CPtrArray<CFileModel::Resource>::Type* res2)
{
	return (*res1)->filename.compare((*res2)->filename);
}

int SortResByType(const CPtrArray<CFileModel::Resource>::Type* res1, const CPtrArray<CFileModel::Resource>::Type* res2)
{
	return GetExtension((*res1)->filename).compare(GetExtension((*res2)->filename));
}

int SortResByDate(const CPtrArray<CFileModel::Resource>::Type* res1, const CPtrArray<CFileModel::Resource>::Type* res2)
{
	if ((*res1)->time < (*res2)->time)
		return 1;
	else if ((*res1)->time == (*res2)->time)
		return 0;
	else
		return -1;
}

int SortResBySize(const CPtrArray<CFileModel::Resource>::Type* res1, const CPtrArray<CFileModel::Resource>::Type* res2)
{
	if ((*res1)->size < (*res2)->size)
		return 1;
	else if ((*res1)->size == (*res2)->size)
		return 0;
	else
		return -1;
}

void CFileModel::_sort()
{
	if (m_resources.GetSize() < 2)
		return;

	switch (m_sortFunc)
	{
	case SORT_NAME:
		m_resources.Sort(SortResByName);
			break;
	case SORT_TYPE:
		m_resources.Sort(SortResByType);
		break;
	case SORT_DATE:
		m_resources.Sort(SortResByDate);
		break;
	case SORT_SIZE:
		m_resources.Sort(SortResBySize);
		break;
	}
}