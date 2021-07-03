#pragma once
#include "FGDClass.h"
#include <string>
class FGDSerializer
{
public:
	static bool BeginFGDFile(const std::string path);

	static bool SerializeClass(FGDClass fgdClass);
	static bool SerializeBrush(FGDBrushEntity fgdClass);
	static bool EndFGDFile();
};