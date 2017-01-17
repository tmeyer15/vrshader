#include "windows.h"
#include "ShaderEditor.h"
#include <GL/glew.h>

class LexState : public LexInterface {
	const LexerModule *lexCurrent;
public:
	int lexLanguage;

	LexState(Document *pdoc_) : LexInterface(pdoc_) {
		lexCurrent = 0;
		performingStyle = false;
		lexLanguage = SCLEX_CONTAINER;
	}

	~LexState() {
		if (instance) {
			instance->Release();
			instance = 0;
		}
	}

	void SetLexerModule(const LexerModule *lex) {
		if (lex != lexCurrent) {
			if (instance) {
				instance->Release();
				instance = 0;
			}
			lexCurrent = lex;
			if (lexCurrent)
				instance = lexCurrent->Create();
			pdoc->LexerChanged();
		}
	}

	void SetLexer(uptr_t wParam) {
		lexLanguage = wParam;
		if (lexLanguage == SCLEX_CONTAINER) {
			SetLexerModule(0);
		}
		else {
			const LexerModule *lex = Catalogue::Find(lexLanguage);
			if (!lex)
				lex = Catalogue::Find(SCLEX_NULL);
			SetLexerModule(lex);
		}
	}

	void SetLexerLanguage(const char *languageName) {
		const LexerModule *lex = Catalogue::Find(languageName);
		if (!lex)
			lex = Catalogue::Find(SCLEX_NULL);
		if (lex)
			lexLanguage = lex->GetLanguage();
		SetLexerModule(lex);
	}

	void SetWordList(int n, const char *wl) {
		if (instance) {
			int firstModification = instance->WordListSet(n, wl);
			if (firstModification >= 0) {
				pdoc->ModifiedAt(firstModification);
			}
		}
	}

	void PropSet(const char *key, const char *val) {
		if (instance) {
			int firstModification = instance->PropertySet(key, val);
			if (firstModification >= 0) {
				pdoc->ModifiedAt(firstModification);
			}
		}
	}
};

const char glslKeyword[] =
"discard struct if else switch case default break goto return for while do continue";

const char glslType[] =
"attribute const in inout out uniform varying invariant "
"centroid flat smooth noperspective layout patch sample "
"subroutine lowp mediump highp precision "
"void float vec2 vec3 vec4 bvec2 bvec3 bvec4 ivec2 ivec3 ivec4 "
"uvec2 uvec3 uvec4 dvec2 dvec3 dvec4 "
"sampler1D sampler2D sampler3D isampler2D isampler1D isampler3D "
"usampler1D usampler2D usampler3D "
"sampler1DShadow sampler2DShadow sampler1DArray sampler2DArray "
"sampler1DArrayShadow sampler2DArrayShadow "
"samplerCube samperCubeShadow samperCubeArrayShadow ";

const char glslBuiltin[] =
"radians degrees sin cos tan asin acos atan sinh "
"cosh tanh asinh acosh atanh pow exp log exp2 "
"log2 sqrt inversesqrt abs sign floor trunc round "
"roundEven ceil fract mod modf min max clamp mix "
"step smoothstep isnan isinf floatBitsToInt floatBitsToUint "
"intBitsToFloat uintBitsToFloat fma frexp ldexp packUnorm2x16 "
"packUnorm4x8 packSnorm4x8 unpackUnorm2x16 unpackUnorm4x8 "
"unpackSnorm4x8 packDouble2x32 unpackDouble2x32 length distance "
"dot cross normalize ftransform faceforward reflect "
"refract matrixCompMult outerProduct transpose determinant "
"inverse lessThan lessThanEqual greaterThan greaterThanEqual "
"equal notEqual any all not uaddCarry usubBorrow "
"umulExtended imulExtended bitfieldExtract bitfildInsert "
"bitfieldReverse bitCount findLSB findMSB textureSize "
"textureQueryLOD texture textureProj textureLod textureOffset "
"texelFetch texelFetchOffset textureProjOffset textureLodOffset "
"textureProjLod textureProjLodOffset textureGrad textureGradOffset "
"textureProjGrad textureProjGradOffset textureGather "
"textureGatherOffset texture1D texture2D texture3D texture1DProj "
"texture2DProj texture3DProj texture1DLod texture2DLod "
"texture3DLod texture1DProjLod texture2DProjLod texture3DProjLod "
"textureCube textureCubeLod shadow1D shadow2D shadow1DProj "
"shadow2DProj shadow1DLod shadow2DLod shadow1DProjLod "
"shadow2DProjLod dFdx dFdy fwidth interpolateAtCentroid "
"interpolateAtSample interpolateAtOffset noise1 noise2 noise3 "
"noise4 EmitStreamVertex EndStreamPrimitive EmitVertex "
"EndPrimitive barrier "
"gl_VertexID gl_InstanceID gl_Position gl_PointSize "
"gl_ClipDistance gl_PrimitiveIDIn gl_InvocationID gl_PrimitiveID "
"gl_Layer gl_PatchVerticesIn gl_TessLevelOuter gl_TessLevelInner "
"gl_TessCoord gl_FragCoord gl_FrontFacing gl_PointCoord "
"gl_SampleID gl_SamplePosition gl_FragColor gl_FragData "
"gl_FragDepth gl_SampleMask gl_ClipVertex gl_FrontColor "
"gl_BackColor gl_FrontSecondaryColor gl_BackSecondaryColor "
"gl_TexCoord gl_FogFragCoord gl_Color gl_SecondaryColor "
"gl_Normal gl_Vertex gl_MultiTexCoord0 gl_MultiTexCoord1 "
"gl_MultiTexCoord2 gl_MultiTexCoord3 gl_MultiTexCoord4 "
"gl_MultiTexCoord5 gl_MultiTexCoord6 gl_MultiTexCoord7 gl_FogCoord "
"gl_MaxVertexAttribs gl_MaxVertexUniformComponents gl_MaxVaryingFloats "
"gl_MaxVaryingComponents gl_MaxVertexOutputComponents "
"gl_MaxGeometryInputComponents gl_MaxGeometryOutputComponents "
"gl_MaxFragmentInputComponents gl_MaxVertexTextureImageUnits "
"gl_MaxCombinedTextureImageUnits gl_MaxTextureImageUnits "
"gl_MaxFragmentUniformComponents gl_MaxDrawBuffers gl_MaxClipDistances "
"gl_MaxGeometryTextureImageUnits gl_MaxGeometryOutputVertices "
"gl_MaxGeometryTotalOutputComponents gl_MaxGeometryUniformComponents "
"gl_MaxGeometryVaryingComponents gl_MaxTessControlInputComponents "
"gl_MaxTessControlOutputComponents gl_MaxTessControlTextureImageUnits "
"gl_MaxTessControlUniformComponents "
"gl_MaxTessControlTotalOutputComponents "
"gl_MaxTessEvaluationInputComponents gl_MaxTessEvaluationOutputComponents "
"gl_MaxTessEvaluationTextureImageUnits "
"gl_MaxTessEvaluationUniformComponents gl_MaxTessPatchComponents "
"gl_MaxPatchVertices gl_MaxTessGenLevel gl_MaxTextureUnits "
"gl_MaxTextureCoords gl_MaxClipPlanes "
"gl_DepthRange gl_ModelViewMatrix gl_ProjectionMatrix "
"gl_ModelViewProjectionMatrix gl_TextureMatrix gl_NormalMatrix "
"gl_ModelViewMatrixInverse gl_ProjectionMatrixInverse "
"gl_ModelViewProjectionMatrixInverse gl_TextureMatrixInverse "
"gl_ModelViewMatrixTranspose gl_ProjectionMatrixTranspose "
"gl_ModelViewProjectionMatrixTranspose gl_TextureMatrixTranspose "
"gl_ModelViewMatrixInverseTranspose gl_ProjectionMatrixInverseTranspose "
"gl_ModelViewProjectionMatrixInverseTranspose "
"gl_TextureMatrixInverseTranspose gl_NormalScale gl_ClipPlane "
"gl_Point gl_FrontMaterial gl_BackMaterial gl_LightSource "
"gl_LightModel gl_FrontLightModelProduct gl_BackLightModelProduct "
"gl_FrontLightProduct gl_BackLightProduct gl_TextureEnvColor "
"gl_EyePlaneS gl_EyePlaneT gl_EyePlaneR gl_EyePlaneQ "
"gl_ObjectPlaneS gl_ObjectPlaneT gl_ObjectPlaneR gl_ObjectPlaneQ "
"gl_Fog";

const size_t NB_FOLDER_STATE = 7;
const size_t FOLDER_TYPE = 0;
const int markersArray[][NB_FOLDER_STATE] = {
	{ SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB, SC_MARKNUM_FOLDERTAIL, SC_MARKNUM_FOLDEREND,        SC_MARKNUM_FOLDEROPENMID,     SC_MARKNUM_FOLDERMIDTAIL },
	{ SC_MARK_MINUS,         SC_MARK_PLUS,      SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY },
	{ SC_MARK_ARROWDOWN,     SC_MARK_ARROW,     SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY },
	{ SC_MARK_CIRCLEMINUS,   SC_MARK_CIRCLEPLUS,SC_MARK_VLINE,        SC_MARK_LCORNERCURVE,  SC_MARK_CIRCLEPLUSCONNECTED, SC_MARK_CIRCLEMINUSCONNECTED, SC_MARK_TCORNERCURVE },
	{ SC_MARK_BOXMINUS,      SC_MARK_BOXPLUS,   SC_MARK_VLINE,        SC_MARK_LCORNER,       SC_MARK_BOXPLUSCONNECTED,    SC_MARK_BOXMINUSCONNECTED,    SC_MARK_TCORNER }
};

ShaderEditor::ShaderEditor()
{
	mLexer = new LexState(mShaderEditor.GetDocument());
	mShaderEditor.SetLexer(mLexer);
}

ShaderEditor::~ShaderEditor()
{

}

void SetAStyle(Editor& ed, int style, int fore, int back = 0xFFFFFFFF, int size = -1, const char *face = 0)
{
	ed.Command(SCI_STYLESETFORE, style, fore);
	ed.Command(SCI_STYLESETBACK, style, back);
	if (size >= 1)
		ed.Command(SCI_STYLESETSIZE, style, size);
	if (face)
		ed.Command(SCI_STYLESETFONT, style, reinterpret_cast<sptr_t>(face));
}

void ShaderEditor::initializeShaderEditor(int defaultSize=18) {

	mShaderEditor.Command(SCI_SETSTYLEBITS, 7);

	mLexer->SetLexer(SCLEX_CPP);
	mLexer->SetWordList(0, glslKeyword);
	mLexer->SetWordList(1, glslType);
	mLexer->SetWordList(4, glslBuiltin);
	mLexer->PropSet("fold", "1");

	mShaderEditor.Command(SCI_SETSTYLEBITS, 7);
	int baseAlpha = 0xE0000000;

	// Set up the global default style. These attributes are used wherever no explicit choices are made.
	SetAStyle(mShaderEditor, STYLE_DEFAULT, 0xFFFFFFFF, baseAlpha | 0x00000000, defaultSize, "c:/windows/fonts/cour.ttf");
	mShaderEditor.Command(SCI_STYLECLEARALL);	// Copies global style to all others
	SetAStyle(mShaderEditor, STYLE_INDENTGUIDE, 0xFFC0C0C0, baseAlpha | 0x00000000, defaultSize, "c:/windows/fonts/cour.ttf");
	SetAStyle(mShaderEditor, STYLE_BRACELIGHT, 0xFF00FF00, baseAlpha | 0x00000000, defaultSize, "c:/windows/fonts/cour.ttf");
	SetAStyle(mShaderEditor, STYLE_BRACEBAD, 0xFF0000FF, baseAlpha | 0x00000000, defaultSize, "c:/windows/fonts/cour.ttf");
	SetAStyle(mShaderEditor, STYLE_LINENUMBER, 0xFFC0C0C0, baseAlpha | 0x00333333, defaultSize, "c:/windows/fonts/cour.ttf");
	mShaderEditor.Command(SCI_SETFOLDMARGINCOLOUR, 1, baseAlpha | 0x001A1A1A);
	mShaderEditor.Command(SCI_SETFOLDMARGINHICOLOUR, 1, baseAlpha | 0x001A1A1A);
	mShaderEditor.Command(SCI_SETSELBACK, 1, baseAlpha | 0x00CC9966);
	mShaderEditor.Command(SCI_SETCARETFORE, 0xFFFFFFFF, 0);
	mShaderEditor.Command(SCI_SETCARETLINEVISIBLE, 1);
	mShaderEditor.Command(SCI_SETCARETLINEBACK, 0xFFFFFFFF);
	mShaderEditor.Command(SCI_SETCARETLINEBACKALPHA, 0x20);

	mShaderEditor.Command(SCI_SETMARGINWIDTHN, 0, defaultSize + 4);//Calculate correct width
	mShaderEditor.Command(SCI_SETMARGINWIDTHN, 1, 6);//Calculate correct width
	mShaderEditor.Command(SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);//Calculate correct width

	for (int i = 0; i < NB_FOLDER_STATE; i++)
	{
		mShaderEditor.Command(SCI_MARKERDEFINE, markersArray[FOLDER_TYPE][i], markersArray[4][i]);
		mShaderEditor.Command(SCI_MARKERSETBACK, markersArray[FOLDER_TYPE][i], 0xFF6A6A6A);
		mShaderEditor.Command(SCI_MARKERSETFORE, markersArray[FOLDER_TYPE][i], 0xFF333333);
	}

	mShaderEditor.Command(SCI_SETUSETABS, 1);
	mShaderEditor.Command(SCI_SETTABWIDTH, 4);
	mShaderEditor.Command(SCI_SETINDENTATIONGUIDES, SC_IV_REAL);

	SetAStyle(mShaderEditor, SCE_C_DEFAULT, 0xDFFFFFFF, baseAlpha | 0x00000000, defaultSize, "c:/windows/fonts/cour.ttf");
	SetAStyle(mShaderEditor, SCE_C_WORD, 0xFF0066FF, baseAlpha | 0x00000000);
	SetAStyle(mShaderEditor, SCE_C_WORD2, 0xFFFFFF00, baseAlpha | 0x00000000);
	// WTF??? SetAStyle(mShaderEditor SCE_C_GLOBALCLASS, 0xFF0000FF, 0xFF000000);
	SetAStyle(mShaderEditor, SCE_C_PREPROCESSOR, 0xFFC0C0C0, baseAlpha | 0x00000000);
	SetAStyle(mShaderEditor, SCE_C_NUMBER, 0xFF0080FF, baseAlpha | 0x00000000);
	SetAStyle(mShaderEditor, SCE_C_OPERATOR, 0xFF00CCFF, baseAlpha | 0x00000000);
	SetAStyle(mShaderEditor, SCE_C_COMMENT, 0xFF00FF00, baseAlpha | 0x00000000);
	SetAStyle(mShaderEditor, SCE_C_COMMENTLINE, 0xFF00FF00, baseAlpha | 0x00000000);
}

void ShaderEditor::initialize(int w, int h)
{
	initializeShaderEditor();

	mWidth = w; mHeight = h;

	mShaderEditor.SetSize(mWidth * 0.5f, mHeight * 0.5f);

	mShaderEditor.Command(SCI_SETFOCUS, true);
}

void ShaderEditor::loadShaderSource(const std::string shaderSource)
{
	mShaderEditor.Command(SCI_CANCEL);
	mShaderEditor.Command(SCI_CLEARALL);
	mShaderEditor.Command(SCI_SETUNDOCOLLECTION, 0);
	mShaderEditor.Command(SCI_ADDTEXT, shaderSource.length(), reinterpret_cast<LPARAM>(shaderSource.data()));
	mShaderEditor.Command(SCI_SETUNDOCOLLECTION, 1);
	mShaderEditor.Command(SCI_EMPTYUNDOBUFFER);
	mShaderEditor.Command(SCI_SETSAVEPOINT);
	mShaderEditor.Command(SCI_GOTOPOS, 0);
}

std::string ShaderEditor::getShaderString()
{
	GLint lengthDoc;
	TextRange tr;

	lengthDoc = mShaderEditor.Command(SCI_GETLENGTH);
	tr.chrg.cpMin = 0;
	tr.chrg.cpMax = lengthDoc;
	tr.lpstrText = (char*)alloca(lengthDoc + 1);
	mShaderEditor.Command(SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));

	return std::string(tr.lpstrText);
}

void ShaderEditor::render(int eye)
{
	// Clear shader
	glUseProgram(0);
	
	
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0, mWidth, 0, mHeight, 0, 500);
	glOrtho(0, mWidth, 0, mHeight, 0, 500);
	glTranslatef(0, mHeight, 0);
	glScalef(1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_CLIP_PLANE0);
	glEnable(GL_CLIP_PLANE1);
	glEnable(GL_CLIP_PLANE2);
	glEnable(GL_CLIP_PLANE3);
	

	float w1 = mWidth - 80.0f, h1 = mHeight - 80.0f;

	// hacky offset for stereo view of the editor.
	float eyeOffset = mWidth * 0.15f * (eye == 0 ? 1.0f : -1.0f);

	glTranslatef(mWidth * 0.25f + eyeOffset , 0.25f * mHeight, 0);
	
	// paints the editor in opengl
	mShaderEditor.Paint();

	glPopAttrib();
}

// Handle mouse scroll up/down to scroll the cursor up/down.
void ShaderEditor::scrollText(SDL_MouseButtonEvent& event)
{
	int direction = 0;
	if (event.button == SDL_BUTTON_WHEELUP)
	{
		direction = SCK_UP;
	}
	else if (event.button == SDL_BUTTON_WHEELDOWN)
	{
		direction = SCK_DOWN;
	}
	mShaderEditor.KeyDown(direction, false, false, false);
}

void ShaderEditor::handleKeyPress(SDL_KeyboardEvent& event)
{
	int sciKey;
	switch (event.keysym.sym)
	{
	case SDLK_DOWN:				sciKey = SCK_DOWN;          break;
	case SDLK_UP:				sciKey = SCK_UP;            break;
	case SDLK_LEFT:				sciKey = SCK_LEFT;          break;
	case SDLK_RIGHT:			sciKey = SCK_RIGHT;         break;
	case SDLK_HOME:				sciKey = SCK_HOME;          break;
	case SDLK_END:				sciKey = SCK_END;           break;
	case SDLK_PAGEUP:			sciKey = SCK_PRIOR;         break;
	case SDLK_PAGEDOWN:			sciKey = SCK_NEXT;	        break;
	case SDLK_DELETE:			sciKey = SCK_DELETE;        break;
	case SDLK_INSERT:			sciKey = SCK_INSERT;        break;
	case SDLK_ESCAPE:			sciKey = SCK_ESCAPE;        break;
	case SDLK_BACKSPACE:		sciKey = SCK_BACK;	        break;
	case SDLK_TAB:				sciKey = SCK_TAB;	        break;
	case SDLK_RETURN:			sciKey = SCK_RETURN;        break;
	case SDLK_KP_PLUS:			sciKey = SCK_ADD;	        break;
	case SDLK_KP_MINUS:			sciKey = SCK_SUBTRACT;      break;
	case SDLK_KP_DIVIDE:		sciKey = SCK_DIVIDE;        break;
	case SDLK_LSUPER:			sciKey = SCK_WIN;	        break;
	case SDLK_RSUPER:			sciKey = SCK_RWIN;	        break;
	case SDLK_MENU:				sciKey = SCK_MENU;	        break;
	case SDLK_SLASH:			sciKey = '/';		        break;
	case SDLK_ASTERISK:			sciKey = '`';		        break;
	case SDLK_LEFTBRACKET:		sciKey = '[';		        break;
	case SDLK_BACKSLASH:		sciKey = '\\';		        break;
	case SDLK_RIGHTBRACKET:		sciKey = ']';		        break;
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
	case SDLK_LALT:
	case SDLK_RALT:
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		sciKey = 0;
		break;
	default:
		sciKey = event.keysym.sym;
	}

	if (sciKey)
	{
		bool consumed;
		bool ctrlPressed = event.keysym.mod&KMOD_LCTRL || event.keysym.mod&KMOD_RCTRL;
		bool altPressed = event.keysym.mod&KMOD_LALT || event.keysym.mod&KMOD_RALT;
		bool shiftPressed = event.keysym.mod&KMOD_LSHIFT || event.keysym.mod&KMOD_RSHIFT;
		mShaderEditor.KeyDown((SDLK_a <= sciKey && sciKey <= SDLK_z) ? sciKey - 'a' + 'A' : sciKey,
			shiftPressed, ctrlPressed, altPressed,
			&consumed
		);
		if (!consumed && event.keysym.unicode >= 32 && !ctrlPressed && !altPressed)
		{
			char    utf8[5];
			wchar_t utf16[2] = { event.keysym.unicode, 0 };
			UTF8FromUTF16(utf16, 1, utf8, sizeof(utf8));
			mShaderEditor.AddCharUTF(utf8, strlen(utf8));
		}
	}
}
