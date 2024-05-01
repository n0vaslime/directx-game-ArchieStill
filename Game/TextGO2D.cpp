#include "pch.h"
#include "TextGO2D.h"
#include "DrawData2D.h"
#include "helper.h"

TextGO2D::TextGO2D(string _text)
{
	m_text = _text;
}

void TextGO2D::Tick(std::shared_ptr<GameData> _GD)
{
}

void TextGO2D::Draw(std::shared_ptr<DrawData2D> _DD)
{
	//right click and "Go to Defintion/Declaration" to see other version of this in DXTK
	_DD->m_Font->DrawString(_DD->m_Sprites.get(), Helper::charToWChar(m_text.c_str()), m_pos, m_colour,m_rotation,m_origin,m_scale);
}