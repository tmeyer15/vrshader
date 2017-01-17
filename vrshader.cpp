#include "io.h"
#include <fcntl.h>

#include <stdio.h>

#include <GL/glew.h>

#include "ShaderEditor.h"

#include <SDL.h>
#include <SDL_syswm.h>
void Platform_Initialise(HWND hWnd); // Defined in ScPlatform.cxx
void Platform_Finalise();

#include <openvr.h>

typedef struct {
	// Pass these into the shader.
	GLint timeLocation, resolutionLocation, eyeLocation, rayLocation, originLocation, upLocation;

	// Shader program;
	GLuint shader;
} ShaderStuff;


#include <fstream>
#include <sstream>
std::string loadTextFile(const std::string& filename) {
	std::stringstream buffer;
	buffer << std::ifstream(filename.c_str()).rdbuf();
	return buffer.str();
}

GLuint compileShaderStage(GLenum stage, const std::string& source) {
	GLuint shader = glCreateShader(stage);
	const char* srcArray[] = { source.c_str() };

	glShaderSource(shader, 1, srcArray, NULL);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE) {
		GLint logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

		std::vector<GLchar> errorLog(logSize);
		glGetShaderInfoLog(shader, logSize, &logSize, &errorLog[0]);

		//fprintf(stderr, "Error while compiling\n %s\n\nError: %s\n", source.c_str(), &errorLog[0]);
		printf("Error while compiling\n %s\n\nError: %s\n", source.c_str(), &errorLog[0]);

		glDeleteShader(shader);
		shader = GL_NONE;
	}

	return shader;
}

GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& pixelShaderSource)
{
	GLuint shader = glCreateProgram();

	// Try to compile vertex shader, return none if it fails.
	GLuint vertexShader = compileShaderStage(GL_VERTEX_SHADER, vertexShaderSource);
	if (vertexShader == GL_NONE)
	{
		return GL_NONE;
	}
	glAttachShader(shader, vertexShader);

	// Try to compile the fragment/pixel shader. Return none if it fails.
	GLuint fragmentShader = compileShaderStage(GL_FRAGMENT_SHADER, pixelShaderSource);
	if (fragmentShader == GL_NONE)
	{
		return GL_NONE;
	}
	glAttachShader(shader, fragmentShader);

	// Link and return location of the shader program.
	glLinkProgram(shader);
	return shader;
}

void buildShader(ShaderStuff* shaderStuff, GLint vertexBuffer, const char* vertexShaderFileName, const char* fragmentShaderFileName)
{
	GLuint newShader = createShaderProgram(loadTextFile(vertexShaderFileName), loadTextFile(fragmentShaderFileName));
	if (newShader == GL_NONE)
	{
		// Shader failed to build, don't reset variable locations
		return;
	}

	shaderStuff->shader = newShader;

	// Get the binding location for the variables we will pass into the shader.
	shaderStuff->timeLocation = glGetUniformLocation(shaderStuff->shader, "time");
	shaderStuff->resolutionLocation = glGetUniformLocation(shaderStuff->shader, "resolution");
	shaderStuff->eyeLocation = glGetUniformLocation(shaderStuff->shader, "eye");
	shaderStuff->rayLocation = glGetUniformLocation(shaderStuff->shader, "ray");
	shaderStuff->originLocation = glGetUniformLocation(shaderStuff->shader, "origin");
	shaderStuff->upLocation = glGetUniformLocation(shaderStuff->shader, "up");

	// Bind in the two 
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	GLuint vertexLocation = glGetAttribLocation(shaderStuff->shader, "position");
	glEnableVertexAttribArray(vertexLocation);
	glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, false, 0, 0);
}

// Multiplies a 3x4 matrix by a 4x1 vector to get a new 3x1 vector
void matrixMultiply(vr::HmdMatrix34_t mat, float* vec, float* newVec)
{
	for (int i = 0; i < 3; i++)
	{
		newVec[i] = 0.0;
		for (int j = 0; j < 4; j++)
		{
			newVec[i] += mat.m[i][j] * vec[j];
		}
	}
}

void createConsole()
{
	// From SO
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
}

// Initializes openVR and prints out some useful information about the connected headset
vr::IVRSystem* initializeVRHeadset(uint32_t* renderWidth, uint32_t* renderHeight)
{
	vr::HmdError vrInitError = vr::VRInitError_None;
	vr::IVRSystem *hmd = vr::VR_Init(&vrInitError, vr::VRApplication_Scene);
	if (vrInitError != vr::VRInitError_None)
	{
		printf("Failed to initialize openvr: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(vrInitError));
		return NULL;
	}

	char model[vr::k_unMaxPropertyStringSize], trackingSystem[vr::k_unMaxPropertyStringSize], serial[vr::k_unMaxPropertyStringSize], driver[vr::k_unMaxPropertyStringSize];
	hmd->GetStringTrackedDeviceProperty(0, vr::Prop_ModelNumber_String, model, vr::k_unMaxPropertyStringSize, NULL);
	hmd->GetStringTrackedDeviceProperty(0, vr::Prop_TrackingSystemName_String, trackingSystem, vr::k_unMaxPropertyStringSize, NULL);
	hmd->GetStringTrackedDeviceProperty(0, vr::Prop_SerialNumber_String, serial, vr::k_unMaxPropertyStringSize, NULL);
	hmd->GetStringTrackedDeviceProperty(0, vr::Prop_DriverVersion_String, driver, vr::k_unMaxPropertyStringSize, NULL);
	float refreshRate = hmd->GetFloatTrackedDeviceProperty(0, vr::Prop_DisplayFrequency_Float, NULL);
	hmd->GetRecommendedRenderTargetSize(renderWidth, renderHeight);

	printf("Headset: %s - %s, Tracking System: %s\nDriver: %s, Display: %ux%u (%f hz)\n", model, serial, trackingSystem, driver, *renderWidth, *renderHeight, refreshRate);

	return hmd;
}

// Initializes SDL and glew
SDL_Surface* initializeSDL(uint32_t windowWidth, uint32_t windowHeight)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		printf("Failed to initialize SDL\n");
		return 0;
	}

	uint32_t sdlFlags = SDL_HWSURFACE | SDL_OPENGLBLIT;
	//sdlFlags |= SDL_FULLSCREEN;

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, TRUE);

	// Multiply the window height by two so we can display both eyes
	SDL_Surface* screen = SDL_SetVideoMode(windowWidth * 2, windowHeight, 32, sdlFlags);
	if (screen == NULL)
	{
		SDL_Quit();
		return 0;
	}

	SDL_EnableUNICODE(TRUE);
	SDL_EnableKeyRepeat(500, 100);

	SDL_SysWMinfo info = { { 0, 0 }, 0, 0 };
	SDL_GetWMInfo(&info);

	Platform_Initialise(info.window);

	glewExperimental = GL_TRUE;
	glewInit();
	while (glGetError() != GL_NONE) {} // Clear any start up errors

	return screen;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
	createConsole();
	printf("Starting VR Shader\n");
	
	uint32_t renderWidth, renderHeight;
	vr::IVRSystem* hmd = initializeVRHeadset(&renderWidth, &renderHeight);
	if (hmd == NULL)
	{
		printf("Failed to initialize vr headset system\n");
		return 0;
	}
	
	vr::IVRCompositor* compositor = vr::VRCompositor();
	if (compositor == NULL)
	{
		printf("Failed to create vr compositor\n");
		return 0;
	}

	// Maintain the aspect ratio of the vr display in the mirror.
	uint32_t windowHeight = 960;
	uint32_t windowWidth = (renderWidth * windowHeight) / renderHeight;

	// Initialize SDL
	SDL_Surface* screen = initializeSDL(windowWidth, windowHeight);

	// Create the framebuffers for rendering to each eye
	GLuint framebuffer[2];
	glGenFramebuffers(2, framebuffer);
	assert(glGetError() == GL_NONE);
	GLuint renderTarget[2];
	glGenTextures(2, renderTarget);
	int eye;
	for (eye = 0; eye < 2; eye++) 
	{
		glBindTexture(GL_TEXTURE_2D, renderTarget[eye]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderWidth, renderHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[eye]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget[eye], 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create vertex buffer (2 triangles). These two triangles form the rectangle we render to.
	GLuint triangleVertexBuffer;
	glCreateBuffers(1, &triangleVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVertexBuffer);
	float twoTriangles[] = { -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0 };
	glBufferData(GL_ARRAY_BUFFER, sizeof(twoTriangles), &twoTriangles, GL_STATIC_DRAW);

	ShaderStuff * shaderStuff = (ShaderStuff*)malloc(sizeof(ShaderStuff));
	shaderStuff->shader = GL_NONE;
	buildShader(shaderStuff, triangleVertexBuffer, "vertex.glsl", "fragment.glsl");
	if (shaderStuff->shader == GL_NONE)
	{
		// Shader failed to build, try default.
		printf("Warning: The shader failed to build, attempting to build default shader.\n");
		buildShader(shaderStuff, triangleVertexBuffer, "vertex.glsl", "default_fragment.glsl");
		if (shaderStuff->shader == GL_NONE)
		{
			printf("Error: The default shader failed to build, crashing...\n");
			assert(false);
		}
	}

	ShaderEditor editor;
	editor.initialize(renderWidth, renderHeight);
	editor.loadShaderSource(loadTextFile("fragment.glsl"));
	Scintilla_LinkLexers();

	// VR devices whose poses can be tracked
	vr::TrackedDevicePose_t devicePoses[vr::k_unMaxTrackedDeviceCount];

	int timer = 0;
	time_t lastTime = time(0);
	time_t currentTime;
	int frameCount = 0;
	bool run = true;
	bool editorVisible = true;
	while (run)
	{
		// Check for any openGL errors from the previous frame
		assert(glGetError() == GL_NONE);

		// Handle timing and calculate/print out fps
		currentTime = time(0);
		if (difftime(currentTime, lastTime) >= 1.0)
		{
			fprintf(stderr, "%d frames rendered in %f seconds: %f fps\n", frameCount, currentTime - lastTime, ((float)frameCount) / (currentTime - lastTime));
			frameCount = 0;
			lastTime = currentTime;
		}

		// Handle keypresses/mouse stuff
		SDL_Event E;
		while (SDL_PollEvent(&E))
		{
			if (E.type == SDL_QUIT) run = false;
			else if (E.type == SDL_KEYDOWN)
			{
				if (E.key.keysym.sym == SDLK_ESCAPE)
				{
					run = false;
				}
				else if (E.key.keysym.sym == SDLK_F1)
				{
					editorVisible = !editorVisible;
				}
				else if (E.key.keysym.sym == SDLK_F2)
				{
					std::ofstream tempGLSLFile("tmp.glsl");
					std::string shaderString = editor.getShaderString();
					tempGLSLFile << shaderString;
					tempGLSLFile.close();
					buildShader(shaderStuff, triangleVertexBuffer, "vertex.glsl", "tmp.glsl");
				}
				else if (E.key.keysym.sym == SDLK_F3)
				{
					hmd->ResetSeatedZeroPose();
				}

				if (editorVisible)
				{
					editor.handleKeyPress(E.key);
				}
			}
			else if (E.type == SDL_MOUSEBUTTONUP)
			{
				editor.scrollText(E.button);
			}
		}

		// Get the device position/orientation matrixes.
		compositor->WaitGetPoses(devicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
		vr::HmdMatrix34_t hmdPose = devicePoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;

		// Calculate the position of the headset and the
		// unit vector of the direction the headset is facing
		// and the unit vector point up from the headset.
		float center[3];
		float positionUnit[4] = { 0.0, 0.0, 0.0, 1.0 };
		matrixMultiply(hmdPose, positionUnit, center);
		float ray[3];
		float zUnit[4] = { 0.0, 0.0, -1.0, 0.0 };
		matrixMultiply(hmdPose, zUnit, ray);
		float up[3];
		float yUnit[4] = { 0.0, 1.0, 0.0, 0.0 };
		matrixMultiply(hmdPose, yUnit, up);

		// Render the shader and/or editor for each eye
		for (int eye = 0; eye < 2; ++eye) 
		{
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[eye]);
			glViewport(0, 0, renderWidth, renderHeight);

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(shaderStuff->shader);

			// Pass in the variables that will be used inside the shader.
			glUniform1f(shaderStuff->timeLocation, ((float)timer) / 100.0f);
			glUniform2f(shaderStuff->resolutionLocation, (float)renderWidth, (float)renderHeight);
			glUniform1i(shaderStuff->eyeLocation, eye);
			glUniform3f(shaderStuff->originLocation, center[0], center[1], center[2]);
			glUniform3f(shaderStuff->rayLocation, ray[0], ray[1], ray[2]);
			glUniform3f(shaderStuff->upLocation, up[0], up[1], up[2]);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			if (editorVisible)
			{
				editor.render(eye);
			}

			const vr::Texture_t eyeTexture = { reinterpret_cast<void*>(intptr_t(renderTarget[eye])), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
			vr::VRCompositor()->Submit(vr::EVREye(eye), &eyeTexture);

			// Mirror each eye to the window
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
			glViewport(windowWidth*eye, 0, windowWidth*(eye + 1), windowHeight);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBlitFramebuffer(0, 0, renderWidth, renderHeight, windowWidth*eye, 0, windowWidth*(eye + 1), windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);
		}

		vr::VRCompositor()->PostPresentHandoff();

		// Only render to window every few frames to avoid 60fps cap.
		if (timer % 3 == 0)
		{
			SDL_GL_SwapBuffers();
		}
		timer++;
		frameCount++;
	}

	// Clean up/exit
	Platform_Finalise();
	SDL_Quit();
	vr::VR_Shutdown();
}