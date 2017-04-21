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
point_light light;
frame_buffer frame;     
geometry screen_quad;   
effect gs;
effect mask;
bool gs_toggle;
bool mask_toggle;
texture alpha_map;

bool load_content() {

	     //For post-processing
		 // Create frame buffer - use screen width and height
	frame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// Create screen quad
	vector<vec3> positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f),
		vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };
	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	screen_quad.set_type(GL_TRIANGLE_STRIP);

	// Create plane mesh
	meshes["plane"] = mesh(geometry_builder::create_plane());

	//Create and light the wall
	meshes["wall"] = mesh(geometry("oldWall.obj"));
	meshes["wall"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["wall"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["wall"].get_material().set_shininess(25.0f);
	meshes["wall"].get_material().set_emissive(vec4(0.25f, 0.25f, 0.25f, 1.0f));

	//Create and light the Ball
	meshes["ball"] = mesh(geometry_builder::create_sphere(16, 16, vec3(2, 2, 2)));
	meshes["ball"].get_transform().position =vec3(10.0f, 2.0f, 0.0f);
	meshes["ball"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["ball"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["ball"].get_material().set_shininess(25.0f);
	meshes["ball"].get_material().set_emissive(vec4(0.25f, 0.25f, 0.25f, 1.0f));

	// Load textures
	planetex = texture("textures/dirt.jpg");
	walltex = texture("textures/wall1.jpg");
	balltex = texture("textures/ball.jpg");

		//Masking texture
		alpha_map = texture("textures/check_1.png");

	//Properties of the light
	light.set_position(vec3(40, 40, 40));
	light.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	light.set_range(2000);

	// Load in shaders
	eff.add_shader("shaders/basic.vert", GL_VERTEX_SHADER);
	eff.add_shader("shaders/basic.frag", GL_FRAGMENT_SHADER);
	
	
	//For greyscale
	gs.add_shader("shaders/greyscale.vert", GL_VERTEX_SHADER);
	gs.add_shader("shaders/greyscale.frag", GL_FRAGMENT_SHADER);
	
	//For greyscale
	mask.add_shader("shaders/masking.vert", GL_VERTEX_SHADER);
	mask.add_shader("shaders/masking.frag", GL_FRAGMENT_SHADER);
	
	// Build effect
	eff.build();
	gs.build();
	mask.build();
	
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
	if (glfwGetKey(renderer::get_window(), 'Y')) {
		translation.z += 5.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'H')) {
		translation.z -= 5.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'G')) {
		translation.x -= 5.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'J')) {
		translation.x += 5.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'U')) {
		translation.y += 5.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'T')) {
		translation.y -= 5.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'M')) {
			gs_toggle = true;
	}
	if (glfwGetKey(renderer::get_window(), 'N')) {
		gs_toggle = false;
	}
	if (glfwGetKey(renderer::get_window(), 'L')) {
		mask_toggle = true;
	}
	if (glfwGetKey(renderer::get_window(), 'K')) {
		mask_toggle = false;
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

	//For greyscale
	if (gs_toggle  == true) {
		// Set render target to frame buffer
		renderer::set_render_target(frame);
		// Clear frame
		renderer::clear();
	}
	//For masking
	if (mask_toggle == true) {
		// Set render target to frame buffer
		renderer::set_render_target(frame);
		// Clear frame
		renderer::clear();
	}


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
		// Set M matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		// Set N matrix uniform
		glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
		// Bind material
		renderer::bind(m.get_material(), "mat");
		// Bind light
		renderer::bind(light, "point");

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

	     //Greyscale
	if (gs_toggle == true) {
		// Set render target back to the screen
		renderer::set_render_target();
		// Bind Tex effect
		renderer::bind(gs);
		// MVP is now the identity matrix
		auto MVP = glm::mat4();
		// Set MVP matrix uniform
		glUniformMatrix4fv(gs.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind texture from frame buffer
		renderer::bind(frame.get_frame(), 0);
		// Set the tex uniform
		glUniform1i(gs.get_uniform_location("tex"), 0);
		// Render the screen quad
		renderer::render(screen_quad);
	}

	if (mask_toggle == true) {
		// Set render target back to the screen
		renderer::set_render_target();
		// Bind Tex effect
		renderer::bind(mask);
		// MVP is now the identity matrix
		auto MVP = glm::mat4();
		// Set MVP matrix uniform
		glUniformMatrix4fv(mask.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind texture from frame buffer to TU 0
		renderer::bind(frame.get_frame(), 0);
		// Set the tex uniform, 0
		glUniform1i(mask.get_uniform_location("tex"), 0);
		// Bind alpha texture to TU, 1
		renderer::bind(alpha_map, 1);
		// Set the tex uniform, 1
		glUniform1i(mask.get_uniform_location("tex"), 1);
		// Render the screen quad
		renderer::render(screen_quad);
	}

	return true;
}

void main() {
	// Create application
	app application("The wall");
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}