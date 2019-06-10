#include "stdafx.h"

#include "settings.h"
#include "input.h"
#include "sys_window.h"
#include "load_image.h"

static void error_cb(int error, char const* description)
{
	std::cerr << "GLFW error: " << description << std::endl;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	//window = window_;
	//Editor::Get().setInit(true);

	//Render::Dimension(Settings::Get().Graphics.ScreenX(),
	//	Settings::Get().Graphics.ScreenY() -
	//	(Settings::Get().Graphics.fullscreen ? 0 : 63));
}

static void iconify_callback(GLFWwindow * window, int mode)
{
	//if (mode == GLFW_TRUE)
	//	Game_::game->currLevel->paused = true;
}

void cursor_enter_callback(GLFWwindow* window, int entered)
{
	if (entered)
	{
		// The cursor entered the client area of the window
	}
	else
	{
		// The cursor left the client area of the window
	}
}

void set_glfw_callbacks(GLFWwindow* window)
{
	Input::init_glfw_input_cbs(window);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetWindowIconifyCallback(window, iconify_callback);
	glfwSetCursorEnterCallback(window, cursor_enter_callback);
}

static void gen_app_icon(GLFWwindow* window)
{
	GLFWimage icon;
	icon.pixels = loadImage("./resources/logo.png");
	icon.width = 128;
	icon.height = 128;
	if (icon.pixels)
	{
		glfwSetWindowIcon(window, 1, &icon);
		deleteImage(icon.pixels);
	}
}

GLFWwindow* init_glfw_context()
{
	if (!glfwInit())
		return nullptr;
	glfwSetErrorCallback(error_cb);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);

	// MSAA
	//glfwWindowHint(GLFW_SAMPLES, Settings::Get().Graphics.multisamples);
	//glEnable(GL_MULTISAMPLE);

	// vertical sync
	//if (Settings::Get().Graphics.vsync)
	//	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	//else
	//	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);

	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	int window_width = Settings::Graphics.screenX; //mode->width;
	int window_height = Settings::Graphics.screenY; //mode->height;

	// for reinitializing glfw
	//if (window_width == 0 || window_height == 0)
	//{
	//	window_width = 1920;
	//	window_height = 1080;
	//	Settings::Graphics.screenX;
	//	Settings::Graphics.screenY;
	//}

	GLFWwindow* window;
	if (Settings::Graphics.fullscreen)
		window = glfwCreateWindow(window_width, window_height, "Sicko Engine", glfwGetPrimaryMonitor(), NULL);
	else
	{
		window = glfwCreateWindow(window_width, window_height - 63, "Sicko Engine", NULL, NULL);
		Settings::Graphics.screenY = window_height;
	}
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	gen_app_icon(window);

	glfwSetWindowPos(window, 0, 25);
	glfwMakeContextCurrent(window);
	set_glfw_callbacks(window);

	glewExperimental = GL_FALSE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return nullptr;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glViewport(0, 0, window_width, Settings::Graphics.screenY);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND_COLOR);

	glClearColor(0.3f, 0.3f, 0.5f, 1.0f);

	return window;
}