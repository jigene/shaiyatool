///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef TEXTFILE_H
#define TEXTFILE_H

#include <iomanip>      // std::setfill, std::setw
#include <sstream>      // std::ostringstream

enum class ETokenType
{
	End,
	String,
	Number,
	Hex,
	StartBlock,
	EndBlock,
	Delimiter,
	Define,
	Identifier,
	Temp
};

class CTextFile
{
public:
	enum
	{
		MaxTokenLength = 2048
	};

	CTextFile();
	~CTextFile();

	bool Load(const string& filename, bool error = true, bool specLoad = false);
	void Close();
	void PutBack();
	string GetLine();
	int GetInt();
	uint GetUInt();
	float GetFloat();
	unsigned long GetLong();

	bool GetBool() {
		return GetInt() != 0;
	}
	string GetString(bool ids = true) {

		string val = "";

		if (!isSpecLoad) {

			_getToken(false, ids);
			val = string(m_token);
		}
		else {
		
			val = retStrVal;
		}

		return val;
	}
	void NextToken(bool ids = true) {
		_getToken(false, ids);
	}
	void SetMark() {
		if (m_cur)
			m_mark = m_cur;
	}
	void GoMark() {
		if (m_mark)
			m_cur = m_mark;
	}
	ETokenType TokenType() const {
		return m_tokenType;
	}
	string Token() const {
		return string(m_token);
	}

	char* CTextFile::GetMapHeight() {

		char* heightData = null;

		if (heightMapStart > 0 && heightMapEnd > 0) {
		
			heightData = new char[heightMapEnd - heightMapStart];
			memcpy(heightData, filedata + heightMapStart, heightMapEnd - heightMapStart);
		}

		return heightData;
	}

	char* CTextFile::GetTextureData() {

		char* textureData = null;

		if (textureDataStart > 0 && textureDataEnd > 0) {

			textureData = new char[textureDataEnd - textureDataStart];
			memcpy(textureData, filedata + textureDataStart, textureDataEnd - textureDataStart);
		}

		return textureData;
	}

private:
	QChar* m_buffer;
	QChar m_token[MaxTokenLength];
	QChar* m_cur;
	ETokenType m_tokenType;
	int m_value;
	QChar* m_mark;

	void _getToken(bool number, bool ids = true);

	bool isSpecLoad = false,
		 waterDataFound = false,
		 skyDataFound = false,
		 heightMapNotSend = true,
		 textureDataNotSend = true;
	char* filedata = null;
	string retStrVal = "";
	float retFloatVal = 0.0;
	unsigned long retLongVal = 0;
	int retIntVal = 0,
		current = 0,
		heightMapStart = 0,
		heightMapEnd = 0,
		textureDataStart = 0,
		textureDataEnd = 0,
		total = 0;

public:
	static bool LoadDefine(const string& filename, bool error = true);
	static bool LoadText(const string& filename);
	static void FillComboBox(QComboBox* comboBox, const string& defBegin, int currentValue);
	static string GetDefineText(int value, const string& defBegin);
	static QStringList GetDefineList(const string& defBegin);
	static int GetDefine(const string& define);

private:
	static QMap<string, int> s_defines;
	static QMap<string, string> s_texts;
};

#endif // TEXTFILE_H