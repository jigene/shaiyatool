///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef FILE_H
#define FILE_H

class CFile
{
public:
	CFile();
	~CFile();
	
	// ReadOnly or WriteOnly
	bool Open(const string& filename, QIODevice::OpenModeFlag mode, bool error = true);
	void Close();

	// ReadOnly functions
	void Skip(int byteCount);
	void SetPos(int bytePos);
	int GetSize();
	const char* GetBuffer() const;

	template<class T> void Read(T& val)
	{
		memcpy(&val, m_cur, sizeof(T));
		m_cur += sizeof(T);
	}

	template<class T> void Read(T* val, int count)
	{
		memcpy(val, m_cur, sizeof(T) * count);
		m_cur += sizeof(T) * count;
	}

	// WriteOnly functions
	template<class T> void Write(const T& val)
	{
		m_file.write((const char*)&val, (qint64)sizeof(T));
	}

	template<class T> void Write(const T* val, int count)
	{
		m_file.write((const char*)val, (qint64)(sizeof(T) * count));
	}

private:
	QIODevice::OpenModeFlag m_mode;
	QFile m_file;
	QByteArray m_buffer;
	char* m_cur;
};

#endif // FILE_H