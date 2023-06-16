#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include <learnopengl/blender.h>


#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader AnimShader("Shaders/anim_model.vs", "Shaders/anim_model.fs");

	Shader BoneShader("Shaders/bone.vs", "Shaders/bone.fs");


	// load models
	// -----------
	Model Model("resources/objects/Breakdance Ready.fbx");
	Animation PullingAnimation("resources/objects/Breakdance Ready.fbx", &Model);
	Animator Pullinganimator(&PullingAnimation);

	Animation WalkingAnimation("resources/objects/Taking Punch.fbx", &Model);
	Animator Walkinganimator(&WalkingAnimation);

	Blender blender(&Pullinganimator, &Walkinganimator, 0.5);

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
													
	// render loop
	// -----------
	int  first = 0;

	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		blender.update(deltaTime);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		AnimShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		AnimShader.setMat4("projection", projection);
		AnimShader.setMat4("view", view);

		const std::vector<glm::mat4>& Pullingtransforms = Pullinganimator.GetFinalBoneMatrices();
		for (int i = 0; i < Pullingtransforms.size(); ++i)
			AnimShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", Pullingtransforms[i]);

		// render the loaded model
		glm::mat4 model_1 = glm::mat4(1.0f);
		model_1 = glm::translate(model_1, glm::vec3(-1.5f, -1.3f, -2.0f)); // translate it down so it's at the center of the scene
		model_1 = glm::scale(model_1, glm::vec3(.02f, .02f, .02f));	// it's a bit too big for our scene, so scale it down
		AnimShader.setMat4("model", model_1);
		Model.Draw(AnimShader);

	
		const std::vector<glm::mat4>& Walkingtransforms = Walkinganimator.GetFinalBoneMatrices();
		for (int i = 0; i < Walkingtransforms.size(); ++i)
			AnimShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", Walkingtransforms[i]);

		// render the loaded model
		glm::mat4 model_2 = glm::mat4(1.0f);
		model_2 = glm::translate(model_2, glm::vec3(0.0f, -1.3f, -2.0f)); // translate it down so it's at the center of the scene
		model_2 = glm::scale(model_2, glm::vec3(.02f, .02f, .02f));	// it's a bit too big for our scene, so scale it down
		AnimShader.setMat4("model", model_2);
		Model.Draw(AnimShader);

		const std::vector<glm::mat4>& Blendertransforms = blender.GetBlenderBoneMatrices();
		for (int i = 0; i < Blendertransforms.size(); ++i)
			AnimShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", Blendertransforms[i]);

		// render the loaded model
		glm::mat4 model_3 = glm::mat4(1.0f);
		model_3 = glm::translate(model_3, glm::vec3(1.5f, -1.3f, -2.0f)); // translate it down so it's at the center of the scene
		model_3 = glm::scale(model_3, glm::vec3(.02f, .02f, .02f));	// it's a bit too big for our scene, so scale it down
		AnimShader.setMat4("model", model_3);
		Model.Draw(AnimShader);

		BoneShader.use();
		BoneShader.setMat4("projection", projection);
		BoneShader.setMat4("view", view);

		BoneShader.setMat4("model", model_1);
		Pullinganimator.DrawBones();

		BoneShader.setMat4("model", model_2);
		Walkinganimator.DrawBones();

		BoneShader.setMat4("model", model_3);
		blender.DrawBones();
		

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}