#include "pch.h"
#include "Loop.h"


Loop::Loop(AudioEngine* _audEngine, string _filename) :Sound(_audEngine, _filename)
{
	if (m_sfx)
	{
		m_loop = m_sfx->CreateInstance();
	}
	m_loop->SetVolume(0.25f);
}

Loop::~Loop()
{
	m_loop->Stop(true);
	m_loop.reset();
}

void Loop::Play()
{
	// if (m_playing)
	// {
	// 	m_loop->Stop(true);
	// }
	// else
	// {
	// 	m_loop->Play(true);
	// }
	// m_playing = !m_playing;

	m_loop->Play(true);
}

void Loop::Stop()
{
	m_loop->Stop(true);
}

void Loop::Tick(GameData* _GD)
{
	_GD;
	m_loop->SetVolume(m_volume);
	m_loop->SetPitch(m_pitch);
	m_loop->SetPan(m_pan);
	m_loop->Play(m_playing);
	m_loop->Stop(m_playing);
}
