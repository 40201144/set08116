#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;
effect eff;
texture planetex;
texture walltex;
texture balltex;
free_camera cam;

bool load_content() {
	// Create plane mesh
	meshes["plane"] = mesh(geometry_builder::create_plane());

	//Create the wall
	meshes["wall"] = mesh(geometry("oldWall.obj"));

	//Create the Ball
	meshes["ball"] = mesh(geometry_builder::create_sphere(16, 16, vec3(2, 2, 2)));
	meshes["ball"].get_transform().position =vec3(10.0f, 2.0f, 0.0f);

	// Load textures
	planetex = texture("textures/dirt.jpg");
	walltex = texture("textures/wall1.jpg");
	balltex = texture("textures/ball.jpg");

	// Load in shaders
	eff.add_shader("shaders/basic.vert", GL_VERTEX_SHADER);
	eff.add_shader("shaders/basic.frag", GL_FRAGMENT_SHADER);
	// Build effect
	eff.build();

	// Set camera properties
	cam.set_position(vec3(0.0f, 5.0f, 10.0f));
	cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return true;
}

bool update(float delta_time) {
	//Rotation of ball
	meshes["ball"].get_transform().rotate(vec3(2.0f, half_pi<float>(), 0.0f)*delta_time);

	//Free camera
	// The ratio of pixels to rotation - remember the fov
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height =
		(quarter_pi<float>() * renderer::get_screen_aspect()) / static_cast<float>(renderer::get_screen_height());
	static double cursor_x = 0.0;
	static double cursor_y = 0.0;
	double current_x;
	double current_y;
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;
	// Multiply deltas by ratios - gets actual change in orientation
	delta_x *= ratio_width;
	delta_y *= ratio_height;
	// Rotate cameras by delta
	cam.rotate(delta_x, -delta_y);
	// Use keyboard to move the camera - WASDQE
	vec3 translation(0.0f, 0.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), 'W')) {
		translation.z += 15.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'S')) {
		translation.z -= 15.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'A')) {
		translation.x -= 15.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'D')) {
		translation.x += 15.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'E')) {
		translation.y += 15.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'Q')) {
		translation.y -= 15.0f * delta_time;
	}
	// Move camera
	cam.move(translation);
	// Update the camera
	cam.update(delta_time);
	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;
	return true;
}

bool render() {
	// Render meshes
	for (auto &e : meshes) {
		auto m = e.second;
		// Bind effect
		renderer::bind(eff);
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto V = cam.get_view();
		auto P = cam.get_projection();
		auto MVP = P * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

		// *********************************
		// Bind texture to renderer
		if (e.first == "plane")
		{
			renderer::bind(planetex, 0);
		}
		if (e.first == "ball")
		{
			renderer::bind(balltex, 0);
		}
		if (e.first == "wall")
		{
			renderer::bind(walltex, 0);
		}
		// Set the texture value for the shader here
		glUniform1i(eff.get_uniform_location("planetex"), 0);
		glUniform1i(eff.get_uniform_location("walltex"), 1);
		glUniform1i(eff.get_uniform_location("balltex"), 2);

		// *********************************
		// Render mesh
		renderer::render(m);
	}

	return true;
}

void main() {
	// Create application
	app application("35_Geometry_Builder");
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}