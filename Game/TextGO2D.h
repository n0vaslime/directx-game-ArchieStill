#ifndef _TEXT_GO_2D_H_
#define _TEXT_GO_2D_H_

//=================================================================
//a Game Object 2D for displaying text
//=================================================================

#include "GameObject2D.h"

class TextGO2D :public GameObject2D
{
public:
	TextGO2D(string _text);

	virtual void Tick(std::shared_ptr<GameData> _GD);
	virtual void Draw(std::shared_ptr<DrawData2D> _DD);
protected:
	string m_text;
};

#endif