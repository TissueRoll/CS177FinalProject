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
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

GLfloat rot_angle = 1.5; // in degrees, rotation per frame
GLfloat blueAlpha = .5f;

struct Vertex {
	glm::vec3 position;
	bool operator==(const Vertex& other) const {
		return position == other.position;
	}
};

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

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// gl stuff
	{
		stbi_set_flip_vertically_on_load(true);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glfwSwapInterval(1);
	}

	GLuint program = loadProgram("simple.vsh", "simple.fsh");

	Icosphere temp(1.f,1,glm::vec3(0,0,0));
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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	}

	/*
	GLuint vao2, vbo2;
	{
		glGenVertexArrays(1, &vao2);
		glGenBuffers(1, &vbo2);
		glBindVertexArray(vao2);
		glBindBuffer(GL_ARRAY_BUFFER, vbo2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * obj.size(), obj.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	}
	*/

	auto v_time = glGetUniformLocation(program, "time");
	auto v_m = glGetUniformLocation(program, "m");
	auto v_v = glGetUniformLocation(program, "v");
	auto v_p = glGetUniformLocation(program, "p");
	auto v_mvp = glGetUniformLocation(program, "mvp");

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);
		glBindVertexArray(vao);

		glm::mat4 m, v, p;
		m = glm::mat4(1);
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
		

		// Model matrix
		m = glm::rotate(m, glm::radians((GLfloat)glfwGetTime() * 10.f), glm::vec3(1, 1, 0));
		glUniformMatrix4fv(v_m, 1, GL_FALSE, glm::value_ptr(m));

		// View matrix
		GLfloat r = 3.f;
		GLfloat camX = sin(45.f)*r;
		GLfloat camZ = cos(45.f)*r;
		//v = glm::lookAt(glm::vec3(camX, 3.f, camZ), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(v_v, 1, GL_FALSE, glm::value_ptr(v));

		// Projection matrix
		//p = glm::perspective(90.f, (GLfloat)SCR_WIDTH / SCR_HEIGHT, .1f, 100.f);
		glUniformMatrix4fv(v_p, 1, GL_FALSE, glm::value_ptr(p));

		glm::mat4 mvp = p * v*m;
		glUniformMatrix4fv(v_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

		//glDrawArrays(GL_TRIANGLE_STRIP, 0, plane.size());
		for (int i = 0; i < va.size(); i += 3) {
			glUniform1f(v_time, cos(glfwGetTime()/200)/2);
			GLuint currentOffset = i * sizeof(GLuint);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)currentOffset);
		}
		//glBindVertexArray(vao2);
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, obj.size());

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

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();

	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		blueAlpha = blueAlpha > 0.f ? blueAlpha - .03f : 0.f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		blueAlpha = blueAlpha < 1.f ? blueAlpha + .03f : 1.f;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}