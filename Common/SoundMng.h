///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef SOUNDMNG_H
#define SOUNDMNG_H

class CSoundMng : public QObject
{
	Q_OBJECT

public:
	static CSoundMng* Instance;

	CSoundMng();
	~CSoundMng();

	bool LoadScript(const string& filename);

	bool Play(int soundId);
	void Stop();

	void SetVolume(int volume);
	int GetVolume() const;

	public slots:
	void Update();

private:
	QMap<int, string> m_sounds;
	QTimer m_updateTimer;
	CPtrArray<QSoundEffect> m_playingSounds;
	int m_volume;
};

#define SoundMng	CSoundMng::Instance

#endif // SOUNDMNG_H