#include <glutil.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <map>
#include <unordered_map>
#include <time.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;
const int NUM_OBJS = 10;

GLfloat rot_angle = 1.5; // in degrees, rotation per frame
GLfloat blueAlpha = .5f;

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec4 color;
	Vertex() {
		color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
	}
	void set_position(glm::vec3 p) {
		position = p;
	}
	void set_normal(glm::vec3 a, glm::vec3 b) {
		normal = glm::normalize(glm::cross(position - a, position - b));
		//normal = glm::normalize(glm::cross(a - position, b - position));
	}
	bool operator==(const Vertex& other) const {
		return position == other.position;
	}
};

// need to adjust it based on offset of 
struct Icosphere {
	// magic constants
	// source: https://schneide.blog/2016/07/15/generating-an-icosphere-in-c/
	const float X = (float)((1.0f + sqrt(5.0f)) / 2.0f); //.525731112119133606f;
	const float Z = 1.f; //.850650808352039932f;
	const float N = 0.f;
	// sphere vertices
	std::vector<Vertex> icosphere_vertices;
	std::unordered_map<glm::vec3, int, std::hash<glm::vec3>> icosphere_vertices_lookup_table;
	// sphere indexing
	std::vector<GLuint> icosphere_triangle_elements;
	std::map<long long, int> cache;
	int index;
	float scale;
	int recursion_level;
	glm::vec3 center;

	Icosphere() : scale(1.f), recursion_level(0), center(glm::vec3(0,0,0)) {
		index = 0;
	}
	Icosphere(float _scale, int _recursion_level, glm::vec3 _center) : scale(_scale), recursion_level(_recursion_level), center(_center) {
		index = 0;
	}
	
	void generate_icosphere() {
		// adding the vertices
		{
			add_vertex(glm::vec3(-X, N, Z));
			add_vertex(glm::vec3(X, N, Z));
			add_vertex(glm::vec3(-X, N, -Z));
			add_vertex(glm::vec3(X, N, -Z));

			add_vertex(glm::vec3(N, Z, X));
			add_vertex(glm::vec3(N, Z, -X));
			add_vertex(glm::vec3(N, -Z, X));
			add_vertex(glm::vec3(N, -Z, -X));

			add_vertex(glm::vec3(Z, X, N));
			add_vertex(glm::vec3(-Z, X, N));
			add_vertex(glm::vec3(Z, -X, N));
			add_vertex(glm::vec3(-Z, -X, N));
		}
		std::vector<glm::vec3> temp_elems;
		// adding the faces
		{
			temp_elems.push_back(glm::vec3(0, 4, 1));
			temp_elems.push_back(glm::vec3(0, 9, 4));
			temp_elems.push_back(glm::vec3(9, 5, 4));
			temp_elems.push_back(glm::vec3(4, 5, 8));
			temp_elems.push_back(glm::vec3(4, 8, 1));

			temp_elems.push_back(glm::vec3(8, 10, 1));
			temp_elems.push_back(glm::vec3(8, 3, 10));
			temp_elems.push_back(glm::vec3(5, 3, 8));
			temp_elems.push_back(glm::vec3(5, 2, 3));
			temp_elems.push_back(glm::vec3(2, 7, 3));

			temp_elems.push_back(glm::vec3(7, 10, 3));
			temp_elems.push_back(glm::vec3(7, 6, 10));
			temp_elems.push_back(glm::vec3(7, 11, 6));
			temp_elems.push_back(glm::vec3(11, 0, 6));
			temp_elems.push_back(glm::vec3(0, 1, 6));

			temp_elems.push_back(glm::vec3(6, 1, 10));
			temp_elems.push_back(glm::vec3(9, 0, 11));
			temp_elems.push_back(glm::vec3(9, 11, 2));
			temp_elems.push_back(glm::vec3(9, 2, 5));
			temp_elems.push_back(glm::vec3(7, 2, 11));
		}

		for (int i = 0; i < recursion_level; i++) {
			std::vector<glm::vec3> temp_elems2;
			for (glm::vec3 face : temp_elems) {
				glm::vec3 v1 = icosphere_vertices[face.x].position;
				glm::vec3 v2 = icosphere_vertices[face.y].position;
				glm::vec3 v3 = icosphere_vertices[face.z].position;
				int a = lookup(v1,v2);
				int b = lookup(v2,v3);
				int c = lookup(v3,v1);
				temp_elems2.push_back(glm::vec3(face.x, a, c));
				temp_elems2.push_back(glm::vec3(face.y, b, a));
				temp_elems2.push_back(glm::vec3(face.z, c, b));
				temp_elems2.push_back(glm::vec3(a, b, c));
			}
			temp_elems = temp_elems2;
		}
		for (glm::vec3 elems : temp_elems) {
			icosphere_vertices[elems.x].set_normal(icosphere_vertices[elems.y].position, icosphere_vertices[elems.z].position);
			icosphere_vertices[elems.y].set_normal(icosphere_vertices[elems.z].position, icosphere_vertices[elems.x].position);
			icosphere_vertices[elems.z].set_normal(icosphere_vertices[elems.x].position, icosphere_vertices[elems.y].position);
			icosphere_triangle_elements.push_back(elems.x);
			icosphere_triangle_elements.push_back(elems.y);
			icosphere_triangle_elements.push_back(elems.z);
		}
	}

	int add_vertex(glm::vec3 p) {
		Vertex to_insert;
		to_insert.position = glm::normalize(p);
		icosphere_vertices.push_back(to_insert);
		icosphere_vertices_lookup_table.insert({ glm::normalize(p), index });
		return index++;
	}

	int lookup(glm::vec3 p1, glm::vec3 p2) {
		// O(1), using unordered_map (hash table) - bigger space tho
		long long i1 = icosphere_vertices_lookup_table[p1];
		long long i2 = icosphere_vertices_lookup_table[p2];
		long long l = (i1 < i2 ? i1 : i2);
		long long r = (i1 < i2 ? i2 : i1);
		long long key = (l << 32) + r;

		if (cache.find(key) != cache.end()) {
			return cache[key];
		}

		glm::vec3 mid = glm::vec3((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f, (p1.z + p2.z) / 2.0f);
		int i = add_vertex(mid);
		cache.insert({ key,i });
		return i;
		//
	}

};

const GLfloat lbs = 0.5f;

glm::vec3 LightBox[] = {
	glm::vec3(-lbs, -lbs, -lbs),
	glm::vec3( lbs, -lbs, -lbs),
	glm::vec3( lbs,  lbs, -lbs),
	glm::vec3( lbs,  lbs, -lbs),
	glm::vec3(-lbs,  lbs, -lbs),
	glm::vec3(-lbs, -lbs, -lbs),

	glm::vec3(-lbs, -lbs,  lbs),
	glm::vec3(lbs, -lbs,  lbs),
	glm::vec3(lbs,  lbs,  lbs),
	glm::vec3(lbs,  lbs,  lbs),
	glm::vec3(-lbs,  lbs,  lbs),
	glm::vec3(-lbs, -lbs,  lbs),

	glm::vec3(-lbs,  lbs,  lbs),
	glm::vec3(-lbs,  lbs, -lbs),
	glm::vec3(-lbs, -lbs, -lbs),
	glm::vec3(-lbs, -lbs, -lbs),
	glm::vec3(-lbs, -lbs,  lbs),
	glm::vec3(-lbs,  lbs,  lbs),

	glm::vec3(lbs,  lbs,  lbs),
	glm::vec3(lbs,  lbs, -lbs),
	glm::vec3(lbs, -lbs, -lbs),
	glm::vec3(lbs, -lbs, -lbs),
	glm::vec3(lbs, -lbs,  lbs),
	glm::vec3(lbs,  lbs,  lbs),

	glm::vec3(-lbs, -lbs, -lbs),
	glm::vec3(lbs, -lbs, -lbs),
	glm::vec3(lbs, -lbs,  lbs),
	glm::vec3(lbs, -lbs,  lbs),
	glm::vec3(-lbs, -lbs,  lbs),
	glm::vec3(-lbs, -lbs, -lbs),

	glm::vec3(-lbs,  lbs, -lbs),
	glm::vec3(lbs,  lbs, -lbs),
	glm::vec3(lbs,  lbs,  lbs),
	glm::vec3(lbs,  lbs,  lbs),
	glm::vec3(-lbs,  lbs,  lbs),
	glm::vec3(-lbs,  lbs, -lbs)
};

// globals
// cpd https://learnopengl.com/Getting-started/Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = (float) SCR_WIDTH / 2.0;
float lastY = (float) SCR_HEIGHT / 2.0;
float fov = 45.0f;

//auto plane = genPlane(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(-.5, -.5, -1.f), 3);
//auto plane = genCube(.5f, 5, glm::vec3(0.f));

int main() {
	// glfw: initialize and configure
	// ------------------------------
	srand(time(NULL));
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CS177", NULL, NULL);
	if (window == NULL) {
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
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// gl stuff
	{
		stbi_set_flip_vertically_on_load(true);
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// glEnable(GL_BLEND);
		// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glfwSwapInterval(1);
	}

	GLuint program = loadProgram("simple.vsh", "simple.fsh");
	GLuint lightbox_shaders = loadProgram("lamp.vsh", "lamp.fsh");

	Icosphere temp(0.1f,4,glm::vec3(0,0,0));
	temp.generate_icosphere();
	auto vs = temp.icosphere_vertices;
	auto va = temp.icosphere_triangle_elements;

	// linking vertex attributes
	GLuint vao, vbo, ebo;
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vs.size(), vs.data(), GL_STATIC_DRAW);
		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * va.size(), va.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	}

	
	GLuint vao2, vbo2;
	{
		glGenVertexArrays(1, &vao2);
		glGenBuffers(1, &vbo2);
		glBindVertexArray(vao2);
		glBindBuffer(GL_ARRAY_BUFFER, vbo2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 36, &LightBox[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	}
	

	// vsh
	auto v_m = glGetUniformLocation(program, "m");
	auto v_mnormal = glGetUniformLocation(program, "mnormal");
	auto v_v = glGetUniformLocation(program, "v");
	auto v_p = glGetUniformLocation(program, "p");
	auto v_mvp = glGetUniformLocation(program, "mvp");
	// fsh
	auto f_lightPos = glGetUniformLocation(program, "lightPos");
	//auto f_lightD = glGetUniformLocation(program, "lightD");
	auto f_viewPos = glGetUniformLocation(program, "viewPos");
	auto f_lightColor = glGetUniformLocation(program, "lightColor");
	auto f_constant = glGetUniformLocation(program, "constant");
	auto f_linear = glGetUniformLocation(program, "linear");
	auto f_quadratic = glGetUniformLocation(program, "quadratic");
	//auto f_cutOff = glGetUniformLocation(program, "cutOff");
	//auto f_outerCutOff = glGetUniformLocation(program, "outerCutOff");
	// lamp
	auto vlbs_mvp = glGetUniformLocation(lightbox_shaders, "mvp");

	GLfloat obj_scales[NUM_OBJS] = {
		1.0f, 0.5f, 2.0f, 0.35f, 0.69f,
		0.420f, 1.0f, 0.86f, 1.5f, 1.0f
	};

	glm::vec3 obj_loc[NUM_OBJS] = {
		glm::vec3(0,0,0),
		glm::vec3(3,3,3),
		glm::vec3(9,9,9),
		glm::vec3(-3,-3,-3),
		glm::vec3(-9,-9,-9),
		glm::vec3(0,3,3),
		glm::vec3(0,9,9),
		glm::vec3(0,-3,-3),
		glm::vec3(0,-9,-9),
		glm::vec3(5,-5,0)
	};

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);
		glBindVertexArray(vao);

		// THIS DONT WORK PROPERLY

		glm::vec3 lightPos = glm::vec3(1.f, 1.f, 1.f);
		lightPos = glm::vec3(sin(glfwGetTime()), cos(glfwGetTime()), -sin(glfwGetTime())) * lightPos;
		//printf("light: %f, %f, %f\n", lightPos.x, lightPos.y, lightPos.z);
		glm::vec3 lightColor = glm::vec3(1.f, 1.f, 1.f);
		glUniform3fv(f_lightPos, 1, glm::value_ptr(lightPos));
		//glUniform3fv(f_lightD, 1, glm::value_ptr(cameraFront));
		//glUniform1f(f_cutOff, glm::cos(glm::radians(12.5f)));
		//glUniform1f(f_outerCutOff, glm::cos(glm::radians(17.5f)));
		

		glUniform3fv(f_lightColor, 1, glm::value_ptr(lightColor));
		
		glUniform1f(f_constant, 1.0f);
		glUniform1f(f_linear, 0.09f);
		glUniform1f(f_quadratic, 0.032f);
		


		glm::mat4 m, mn, v, p;
		m = glm::mat4(1);
		mn = glm::mat4(1);
		v = glm::mat4(1);
		p = glm::mat4(1);
		// ref: http://glslsandbox.com/e#53359.0
		// ref: http://glslsandbox.com/e#52629.0
		// ref: http://glslsandbox.com/e#51856.0
		// ref: http://glslsandbox.com/e#52629.0
		// ref: http://glslsandbox.com/e#51856.0
		// ref: http://glslsandbox.com/e#51718.0
		// ref: http://glslsandbox.com/e#32913.1
		// ref: http://glslsandbox.com/e#51487.0
		// ref: http://www.songho.ca/opengl/gl_sphere.html
		
		// Projection matrix
		p = glm::perspective(glm::radians(fov), (GLfloat)SCR_WIDTH / SCR_HEIGHT, .1f, 100.f);
		glUniformMatrix4fv(v_p, 1, GL_FALSE, glm::value_ptr(p));

		// View matrix
		/*GLfloat r = 3.f;
		GLfloat camX = sin(glfwGetTime())*r;
		GLfloat camY = sin(glfwGetTime())*r;
		GLfloat camZ = cos(glfwGetTime())*r;
		v = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		v = glm::translate(v, glm::vec3(0.0f, 0.0f, -3.f));*/
		GLfloat r = 3.f;
		GLfloat camX = sin(glfwGetTime())*r;
		GLfloat camY = sin(glfwGetTime())*r;
		GLfloat camZ = cos(glfwGetTime())*r;
		v = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(v_v, 1, GL_FALSE, glm::value_ptr(v));
		glm::vec3 camPos = glm::vec3(camX, r, camZ);
		glUniform3fv(f_viewPos, 1, glm::value_ptr(camPos));

		// Model matrix
		// m = glm::scale(m, glm::vec3(0.5, 0.5, 0.5));
		/*m = glm::rotate(m, glm::radians((GLfloat)glfwGetTime() * 10.f), glm::vec3(1, 1, 0));
		
		mn = glm::transpose(glm::inverse(m));
		glUniformMatrix4fv(v_m, 1, GL_FALSE, glm::value_ptr(m));
		glUniformMatrix4fv(v_mnormal, 1, GL_FALSE, glm::value_ptr(mn));*/

		/*glm::mat4 mvp = p * v*m;
		glUniformMatrix4fv(v_mvp, 1, GL_FALSE, glm::value_ptr(mvp));*/

		for (int objs = 0; objs < NUM_OBJS; objs++) {
			
			m = glm::mat4(1);
			mn = glm::mat4(1);
			m = glm::translate(m, obj_loc[objs]);
			m = glm::rotate(m, glm::radians((GLfloat)glfwGetTime() * 10.f), glm::vec3(1, 1, 0));
			m = glm::scale(m, glm::vec3(obj_scales[objs]));
			mn = glm::transpose(glm::inverse(m));
			glUniformMatrix4fv(v_m, 1, GL_FALSE, glm::value_ptr(m));
			glUniformMatrix4fv(v_mnormal, 1, GL_FALSE, glm::value_ptr(mn));
			glm::mat4 mvp = p * v*m;
			glUniformMatrix4fv(v_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
			for (int i = 0; i < va.size(); i += 3) {
				GLuint currentOffset = i * sizeof(GLuint);
				glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)currentOffset);
			}
		}
		
		glUseProgram(lightbox_shaders);
		glBindVertexArray(vao2);
		m = glm::mat4(1);
		m = glm::rotate(m, glm::radians((GLfloat)glfwGetTime() * 10.f), glm::vec3(1, 1, 0));
		m = glm::scale(m, glm::vec3(0.5f, 0.5f, 0.5f));
		m = glm::translate(m, glm::vec3(2.0f,2.0f,2.0f));
		glm::mat4 mvp = p * v*m;
		glUniformMatrix4fv(v_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// clean-up
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteProgram(lightbox_shaders);
	glDeleteVertexArrays(1, &vao2);
	glDeleteBuffers(1, &vbo2);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();

	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
	float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		blueAlpha = blueAlpha > 0.f ? blueAlpha - .03f : 0.f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		blueAlpha = blueAlpha < 1.f ? blueAlpha + .03f : 1.f;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// https://learnopengl.com/Getting-started/Camera
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

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}