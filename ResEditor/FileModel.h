///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef FILEMODEL_H
#define FILEMODEL_H

enum ESortFunc
{
	SORT_NAME,
	SORT_DATE,
	SORT_TYPE,
	SORT_SIZE
};

class CFileModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CFileModel(QWidget* parent = null);
	virtual ~CFileModel();

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role);

	bool Load(const string& filename);
	bool Save(const string& filename);

	bool IsEncrypted() const {
		return m_encrypted;
	}
	void SetEncrypted(bool encrypted) {
		m_encrypted = encrypted;
	}

	byte EncryptionKey() const {
		return m_key;
	}
	void SetEncryptionKey(byte key) {
		m_key = key;
	}

	void AddFiles(const QStringList& filenames);
	void DeleteFiles(const QModelIndexList& selection);
	void ExtractFiles(const string& dir, const QModelIndexList& selection);
	void ExtractAllFiles(const string& dir, bool replaceFiles);

	bool FileExists(const string& filename) const;

	void SetSortMode(int mode) {
		m_sortFunc = (ESortFunc)mode;
		_sort();
	}
	void Sort()
	{
		beginResetModel();
		_sort();
		endResetModel();
	}

public:
	struct Resource
	{
		string filename;
		uint offset;
		uint time;
		uint size;
		byte* data;
	};

private:
	CPtrArray<Resource> m_resources;
	byte m_key;
	bool m_encrypted;
	ESortFunc m_sortFunc;

	void _encrypt(byte* data, uint size);
	void _decrypt(byte* data, uint size);
	Resource* _getFile(const string& filename);
	void _sort();

public:
	static void InitResources();

private:
	static QIcon s_icons[10];
	static QMap<string, int> s_exts;
};

#endif // FILEMODEL_H