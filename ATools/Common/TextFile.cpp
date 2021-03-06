///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "TextFile.h"
#include <iostream>
#include <fstream>

QMap<string, int> CTextFile::s_defines;
QMap<string, string> CTextFile::s_texts;

static const string delimiters = " !:;,+-<>'/*%^=()&|\"{}\n\t\r";
static const string define = "#define";

CTextFile::CTextFile()
	: m_buffer(null)
{
	Close();
}

CTextFile::~CTextFile()
{
	Close();
}

bool CTextFile::Load(const string& filename, bool error, bool specLoad)
{
	isSpecLoad = specLoad;

	if (specLoad) {

		std::ifstream reader(filename.toUtf8().constData(), std::ios::binary);

		if (reader.is_open()) {

			reader.seekg(0, reader.end);

			total = reader.tellg();

			reader.seekg(0, reader.beg);

			filedata = new char[total + 1];
			reader.read(filedata, total);

//			std::string fileHex = ToHex(std::string(filedata, total + 1), true);
//			free(filedata);

//			total = (int)fileHex.length();
//			filedata = new char[total + 1];
//			memcpy(filedata, fileHex.c_str(), total + 1);

			reader.close();
		}
	}
	else {
		CFile file;
		if (!file.Open(filename, QIODevice::ReadOnly, error))
			return false;

		bool unicode = false;
		if (file.GetSize() > 2)
		{
			byte header[2];
			file.Read(header);
			if (header[0] == 0xff && header[1] == 0xfe)
				unicode = true;
		}

		string data;
		if (unicode)
			data = string::fromUtf16((const ushort*)file.GetBuffer(), file.GetSize() / 2);
		else
			data = string::fromLocal8Bit(file.GetBuffer(), file.GetSize());

		m_buffer = new QChar[data.size() + 1];
		memcpy(m_buffer, data.constData(), data.size() * sizeof(QChar));
		m_buffer[data.size()] = '\0';

		m_cur = &m_buffer[0];

		file.Close();
	}
	return true;
}

void CTextFile::Close()
{
	DeleteArray(m_buffer);
	m_token[0] = '\0';
	m_cur = null;
	m_tokenType = ETokenType::Temp;
	m_value = -1;
	m_mark = null;
}

void CTextFile::_getToken(bool number, bool ids)
{
	if (!isSpecLoad) {

		m_token[0] = '\0';
		QChar* curTok = m_token;
		m_value = -1;
		m_tokenType = ETokenType::Temp;

		bool loop;
		do
		{
			loop = false;

			while (m_cur->isSpace() && *m_cur != '\0')
				++m_cur;

			while (*m_cur == '/')
			{
				++m_cur;
				if (*m_cur == '/')
				{
					++m_cur;
					while (*m_cur != '\r' && *m_cur != '\0')
						++m_cur;
					if (*m_cur == '\r')
						m_cur += 2;
				}
				else if (*m_cur == '*')
				{
					++m_cur;
					do
					{
						while (*m_cur != '*' && *m_cur != '\0')
							++m_cur;

						if (*m_cur == '\0')
						{
							m_tokenType = ETokenType::End;
							return;
						}
						++m_cur;
					} while (*m_cur != '/');

					++m_cur;
					while (m_cur->isSpace() && *m_cur != '\0')
						++m_cur;
				}
				else
				{
					--m_cur;
					break;
				}
				loop = true;
			}
		} while (loop);


		if (*m_cur == '\0')
		{
			m_tokenType = ETokenType::End;
		}
		else if (*m_cur == '"')
		{
			++m_cur;
			while (*m_cur != '"' && *m_cur != '\r' && *m_cur != '\0' && (curTok - m_token) < MaxTokenLength)
				*curTok++ = *m_cur++;
			++m_cur;
			m_tokenType = ETokenType::String;
			if (*(m_cur - 1) != '"')
			{
				if (*(m_cur - 1) == '\0')
					--m_cur;
			}
			*curTok = '\0';
		}
		else if ((delimiters.contains(*m_cur) || *m_cur == 0))
		{
			if (*m_cur == '{')
				m_tokenType = ETokenType::StartBlock;
			else if (*m_cur == '}')
				m_tokenType = ETokenType::EndBlock;
			else
				m_tokenType = ETokenType::Delimiter;

			*curTok++ = *m_cur++;
			if (*m_cur == '=' || *m_cur == '&' || *m_cur == '|')
				*curTok++ = *m_cur++;
			*curTok = '\0';
		}
		else if (*m_cur == '0' && *(m_cur + 1) == 'x')
		{
			m_cur += 2;
			while (!(delimiters.contains(*m_cur) || *m_cur == 0))
				*curTok++ = *m_cur++;
			*curTok = '\0';
			m_tokenType = ETokenType::Hex;
		}
		else if (m_cur->isDigit())
		{
			while (!(delimiters.contains(*m_cur) || *m_cur == 0))
				*curTok++ = *m_cur++;
			*curTok = '\0';
			m_tokenType = ETokenType::Number;
		}
		else
		{
			while (!(delimiters.contains(*m_cur) || *m_cur == 0))
				*curTok++ = *m_cur++;
			*curTok = '\0';
		}

		if (m_tokenType == ETokenType::Temp)
		{
			const int tokenSize = (int)(curTok - m_token);
			const string tok = string::fromRawData(m_token, tokenSize);

			if (tokenSize > 3)
			{
				if (tokenSize == 7 && m_token[0] == '#' && tok == define)
					m_tokenType = ETokenType::Define;
				else if (m_token[0] == 'I' && m_token[1] == 'D' && m_token[2] == 'S')
				{
					if (ids)
					{
						const QMap<string, string>::const_iterator it = s_texts.constFind(tok);
						if (it != s_texts.constEnd())
						{
							const string& text = it.value();
							memcpy(m_token, text.constData(), text.size() * sizeof(QChar));
							m_token[text.size()] = '\0';
							m_tokenType = ETokenType::String;
						}
					}
					else
						m_tokenType = ETokenType::String;
				}
			}

			if (m_tokenType == ETokenType::Temp)
			{
				const QMap<string, int>::const_iterator it = s_defines.constFind(tok);
				if (it != s_defines.constEnd())
				{
					if (number)
						m_value = it.value();
					else
					{
						const string number = string::number(it.value());
						memcpy(m_token, number.constData(), number.size() * sizeof(QChar));
						m_token[number.size()] = '\0';
					}
					m_tokenType = ETokenType::Number;
				}
				else
				{
					if (number && *m_token != '\0' && *m_token != '=' && *m_token != '-' && *m_token != '+')
						qCritical((tok + " not found").toLocal8Bit());

					m_tokenType = ETokenType::Identifier;
				}
			}
		}
	}
	else {

		std::string filetext("");

		for (int counter = current; counter < total; counter++) {

			filetext += *(filedata + textureDataStart + 512);

			if (counter == 5) {

				retLongVal = (((unsigned long)*(filedata + 5)) << 8) |
							  ((unsigned long)*(filedata + 6));

				m_tokenType = ETokenType::Number;
				memcpy(m_token, string(std::string("size").c_str()).constData(), 4 * sizeof(QChar));
				heightMapStart = 8;

//				if (retLongVal == 2048) {

				//	lightMapEnd = 2101257;
				//	heightMapStart = 2101258;
//					heightMapEnd = 3151883;
//				}
//				else if (retLongVal == 1024) {

//					heightMapEnd = 789515;
//				}

				heightMapEnd = ((((retLongVal / 2) + 1) * ((retLongVal / 2) + 1)) * 2) + heightMapStart;
				textureDataStart = heightMapEnd + 1;
				textureDataEnd = (((retLongVal / 2) + 1) * ((retLongVal / 2) + 1)) + textureDataStart;

				current = textureDataEnd + 1;

				break;
			}
			else if (heightMapNotSend && heightMapStart > 0 && heightMapEnd > 0) {

				heightMapNotSend = false;
				m_tokenType = ETokenType::Number;
				retIntVal = heightMapEnd - heightMapStart;
				memcpy(m_token, string(std::string("heightmap").c_str()).constData(), 9 * sizeof(QChar));
				filetext = "";
				break;
			}
			else if (textureDataNotSend && textureDataStart > 0 && textureDataEnd > 0) {

				textureDataNotSend = false;
				m_tokenType = ETokenType::Number;
				retIntVal = textureDataEnd - textureDataStart;
				memcpy(m_token, string(std::string("textureData").c_str()).constData(), 11 * sizeof(QChar));
				filetext = "";
				break;
			}
			else if (filetext.find(".tga") != std::string::npos && (skyDataFound || !waterDataFound)) {

				m_tokenType = ETokenType::String;

				if (skyDataFound) {

					memcpy(m_token, string(std::string("cloud").c_str()).constData(), 5 * sizeof(QChar));
				}
				else {

					memcpy(m_token, string(std::string("ground").c_str()).constData(), 6 * sizeof(QChar));
				}

				retStrVal = string(filetext.c_str());

				filetext = "";
				current = counter;
				break;
			}
			else if (filetext.find(".wtr") != std::string::npos) {
				
				waterDataFound = true;
				m_tokenType = ETokenType::String;
				memcpy(m_token, string(std::string("seacloud").c_str()).constData(), 8 * sizeof(QChar));
				retStrVal = string(filetext.c_str());
				filetext = "";
				current = counter;
				break;
			}
			else if (filetext.find(".bmp") != std::string::npos) {

				skyDataFound = true;
				m_tokenType = ETokenType::String;
				memcpy(m_token, string(std::string("sky").c_str()).constData(), 3 * sizeof(QChar));
				retStrVal = string(filetext.c_str());

				filetext = "";
				current = counter;
				break;
			}
			else {

				memset(m_token, '\0', sizeof(m_token));
			}

			if (*(filedata + counter) == '\0') {

				filetext = "";
			}

			if (counter >= total - 1) {
			
				m_tokenType = ETokenType::End;
			}
		}
	}
}

string CTextFile::GetLine()
{
	while (m_cur->isSpace() && *m_cur != '\0' && *m_cur != '\r')
		++m_cur;

	QChar* curTok = m_token;
	while (*m_cur != '\0' && *m_cur != '\r')
		*curTok++ = *m_cur++;

	if (curTok != m_token)
	{
		--curTok;
		while (curTok->isSpace() && *curTok != '\0')
			--curTok;
		++curTok;
	}

	*curTok = '\0';
	return string(m_token);
}

int CTextFile::GetInt()
{
	if (!isSpecLoad) {
		_getToken(true);

		if (m_tokenType == ETokenType::Hex)
		{
			bool ok = false;
			const int value = string(m_token).toInt(&ok, 16);
			if (ok)
				return value;
			else
				return 0;
		}
		else if (*m_token != '\0')
		{
			if (m_token[0] == '=')
				return -1;
			else if (m_token[0] == '-')
			{
				_getToken(true);

				if (m_value != -1)
					return -m_value;
				else
				{
					bool ok = false;
					const int value = string(m_token).toInt(&ok, 10);
					if (ok)
						return -value;
					else
						return 0;
				}
			}
			else if (m_token[0] == '+')
			{
				_getToken(true);
			}

			if (m_value != -1)
				return m_value;
			else
			{
				bool ok = false;
				const int value = string(m_token).toInt(&ok, 10);
				if (ok)
					return value;
				else
					return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else {
	
		return retIntVal;
	}
}

uint CTextFile::GetUInt()
{
	_getToken(true);

	if (m_tokenType == ETokenType::Hex)
	{
		bool ok = false;
		const int ret = string(m_token).toUInt(&ok, 16);
		if (ok)
			return ret;
		else
			return 0;
	}
	else
		return 0;
}

float CTextFile::GetFloat()
{
	if (!isSpecLoad) {
		_getToken(true);

		if (*m_token != '\0')
		{
			if (m_token[0] == '=')
				return -1.0f;
			else if (m_token[0] == '-')
			{
				_getToken(true);

				if (m_value != -1)
					return (float)-m_value;
				else
				{
					bool ok = false;
					const float value = string(m_token).remove('f').toFloat(&ok);
					if (ok)
						return -value;
					else
						return 0.0f;
				}
			}
			else if (m_token[0] == '+')
			{
				_getToken(true);
			}

			if (m_value != -1)
				return (float)m_value;
			else
			{
				bool ok = false;
				const float value = string(m_token).remove('f').toFloat(&ok);
				if (ok)
					return value;
				else
					return 0.0f;
			}
		}
		else
		{
			return 0.0f;
		}
	}
	else {
	
		return retFloatVal;
	}
}

unsigned long CTextFile::GetLong()
{
	return retLongVal;

}

void CTextFile::PutBack()
{
	for (QChar*t = m_token; *t != '\0'; t++)
		m_cur--;
	if (m_tokenType == ETokenType::String)
		m_cur -= 2;
}

bool CTextFile::LoadDefine(const string& filename, bool error)
{
	CTextFile file;
	if (!file.Load(filename, error))
		return false;

	int value;
	file.NextToken();
	do
	{
		if (file.TokenType() == ETokenType::Define)
		{
			const string id = file.GetString();
			file.SetMark();
			const string tok = file.GetString();
			const ETokenType tokType = file.TokenType();

			if (tokType == ETokenType::Number)
				value = tok.toInt(null, 10);
			else if (tokType == ETokenType::Hex)
				value = tok.toInt(null, 16);
			else
			{
				file.GoMark();
				file.NextToken();
				continue;
			}

			s_defines[id] = value;
		}

		file.NextToken();
	} while (file.TokenType() != ETokenType::End);

	file.Close();
	return true;
}

bool CTextFile::LoadText(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename, false))
		return false;

	file.NextToken();
	while (file.TokenType() != ETokenType::End)
	{
		const string id = file.Token();
		if (id.size() <= 3 || id[0] != 'I' || id[1] != 'D' || id[2] != 'S')
		{
			file.NextToken();
			continue;
		}

		s_texts[id] = file.GetLine();
		file.NextToken();
	}

	file.Close();
	return true;
}

void CTextFile::FillComboBox(QComboBox* comboBox, const string& defBegin, int currentValue)
{
	comboBox->clear();
	int i = 0;
	for (auto it = s_defines.begin(); it != s_defines.end(); it++)
	{
		if (it.key().startsWith(defBegin))
		{
			comboBox->addItem(it.key(), it.value());
			if (it.value() == currentValue)
				comboBox->setCurrentIndex(i);
			i++;
		}
	}
}

string CTextFile::GetDefineText(int value, const string& defBegin)
{
	for (auto it = s_defines.begin(); it != s_defines.end(); it++)
	{
		if (it.value() == value && it.key().startsWith(defBegin))
			return it.key();
	}
	return string::number(value);
}

QStringList CTextFile::GetDefineList(const string& defBegin)
{
	QStringList list;
	for (auto it = s_defines.begin(); it != s_defines.end(); it++)
		if (it.key().startsWith(defBegin))
			list.push_back(it.key());
	list.sort();
	return list;
}

int CTextFile::GetDefine(const string& define)
{
	auto it = s_defines.constFind(define);
	if (it != s_defines.constEnd())
		return it.value();
	else
	{
		qCritical((define + " not found").toLocal8Bit());
		return 0;
	}
}