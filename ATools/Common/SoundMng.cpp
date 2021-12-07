///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "SoundMng.h"
#include "TextFile.h"

CSoundMng* SoundMng = null;

CSoundMng::CSoundMng()
	: m_volume(75)
{
	SoundMng = this;

	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(Update()));
	m_updateTimer.start(10000);
}

CSoundMng::~CSoundMng()
{
	SoundMng = null;

	Stop();
}

bool CSoundMng::LoadScript(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	int id = file.GetInt();
	while (file.TokenType() != ETokenType::End)
	{
		m_sounds[id] = file.GetString();
		id = file.GetInt();
	}

	return true;
}

bool CSoundMng::Play(int soundId)
{
	if (m_volume == 0)
		return true;

	QMap<int, string>::const_iterator it = m_sounds.constFind(soundId);
	if (it == m_sounds.constEnd())
		return false;

	if (it.value().size() <= 0)
		return true;

	const string filename = QDir::currentPath() + "/Sound/" + it.value();

	if (!QFileInfo(filename).exists())
		return false;

	QSoundEffect* sound = new QSoundEffect();
	sound->setSource(QUrl::fromLocalFile(filename));
	sound->setLoopCount(1);
	sound->setVolume((float)m_volume / 100.0f);

	if (sound->status() == QSoundEffect::Error)
		return false;

	sound->play();
	m_playingSounds.Append(sound);
	return true;
}

void CSoundMng::Stop()
{
	for (int i = 0; i < m_playingSounds.GetSize(); i++)
		Delete(m_playingSounds[i]);
	m_playingSounds.RemoveAll();
}

void CSoundMng::Update()
{
	for (int i = 0; i < m_playingSounds.GetSize(); i++)
	{
		if (!m_playingSounds[i]->isPlaying())
		{
			Delete(m_playingSounds[i]);
			m_playingSounds.RemoveAt(i);
			i--;
		}
	}
}

void CSoundMng::SetVolume(int volume)
{
	m_volume = volume;

	for (int i = 0; i < m_playingSounds.GetSize(); i++)
		if (m_playingSounds[i]->isPlaying())
			m_playingSounds[i]->setVolume((float)m_volume / 100.0f);
}

int CSoundMng::GetVolume() const
{
	return m_volume;
}