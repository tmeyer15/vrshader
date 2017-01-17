#pragma once

#include <new>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>

#include <SDL.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "Platform.h"

#include "ILexer.h"
#include "Scintilla.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "XPM.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "Decoration.h"
#include "CharClassify.h"
#include "Document.h"
#include "Selection.h"
#include "PositionCache.h"
#include "Editor.h"
#include "UniConversion.h"

#include "SciLexer.h"
#include "LexerModule.h"
#include "Catalogue.h"

class LexState;

class ShaderEditor
{
public:
	ShaderEditor();
	~ShaderEditor();

	void initialize(int w, int h);
	void handleKeyPress(SDL_KeyboardEvent& event);
	void scrollText(SDL_MouseButtonEvent& event);
	void loadShaderSource(const std::string shaderSource);
	std::string ShaderEditor::getShaderString();
	void render(int eye);
	
private:
	void initializeShaderEditor(int defaultSize);

private:
	enum ModesEnum
	{
		SELMODE_PROGRAM_LIST,
		SELMODE_SHADER_LIST
	};

private:
	/*static const size_t TICK_INTERVAL = 100;

	bool   mRequireReset;

	size_t mNextTick;

	ModesEnum mSelectionMode;
	GLuint    mSelectedProgram;
	GLuint    mSelectedShader;

	std::vector<GLuint> mPrograms;
	std::vector<GLuint> mAttachedShaders;
	*/

	LexState* mLexer;

	Editor mShaderEditor;

	float mWidth;
	float mHeight;
};
