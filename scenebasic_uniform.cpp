#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include "helper/texture.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

SceneBasic_Uniform::SceneBasic_Uniform() :
	tPrev(0.0f),
	angle(0),
	rockRing(2.5f * 2.0f, 0.4f * 2.0f, 50, 10), //(float MajorRadius, float MinorRadius, int numMajor, int numMinor)
	lavaPool(0.2* 2.0f, 0.5f* 2.0f, 50, 10),
	plane(50.0f, 50.0f, 1, 1), //(float xsize, float zsize, int xdivs, int zdivs)
	Sky(50.0f)
{
	mesh = ObjMesh::load("media/Skeleton/Skelly.obj", true); //load custom model here
}

void SceneBasic_Uniform::initScene()
{
	compile();
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	model = mat4(1.0f);
	view = glm::lookAt(
		vec3(3.0f, 3.0f, 4.0f), //camera position
		vec3(0.0f, 0.75f, 0.0f), //position of target
		vec3(0.0f, 1.0f, 0.0f)); //up vector
	projection = mat4(1.0f);
	GLuint cubeTex = Texture::loadHdrCubeMap("media/texture/cube/pisa-hdr/pisa");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
	glDepthMask(GL_TRUE);

	Ytranslation = -0.5f;

	GLuint Rock = Texture::loadTexture("media/texture/cement.jpg");
	GLuint Lava = Texture::loadTexture("media/texture/fire.png");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Rock);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Lava);	
	/**/

	//The Fog is coming
		prog.setUniform("Fog.MaxDist", 30.0f);
		prog.setUniform("Fog.MinDist", 1.0f);
		prog.setUniform("Fog.Color", vec3(0.3f, 0.2f, 0.2f)); //RGB higher is brighter

	//generate 3 lights
		float x, z;
		for (int i = 0; i < 3; i++) {  
			std::stringstream name;
			name << "lights[" << i << "].Position";
			x = 0.5f * cosf((glm::two_pi<float>() / 3) * i);
			z = 0.5f * sinf((glm::two_pi<float>() / 3) * i);
			prog.setUniform(name.str().c_str(), view * glm::vec4(x, 1.2f, z + 1.0f, 1.0f));
		}

		//intensity
		prog.setUniform("lights[0].L", vec3(0.8f,0.0f, 0.0f));
		prog.setUniform("lights[1].L", vec3(0.8f, 0.0f, 0.0f));
		prog.setUniform("lights[2].L", vec3(0.8f, 0.0f, 0.0f));

		//ambient RGB
		prog.setUniform("lights[0].La", vec3(0.2f, 0.0f, 0.2f));
		prog.setUniform("lights[1].La", vec3(0.2f, 0.2f, 0.0f));
		prog.setUniform("lights[2].La", vec3(0.1f, 0.0f, 0.0f));

		//spotlight
		prog.setUniform("Spot.L", vec3(0.8f));
		prog.setUniform("Spot.La", vec3(0.5f, 0.4f, 0.4f));
		prog.setUniform("Spot.Exponent", 100.0f);
		prog.setUniform("Spot.Cutoff", glm::radians(45.0f));

		
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
		
		skyProg.compileShader("shader/skybox.vert");
		skyProg.compileShader("shader/skybox.frag");
		skyProg.link();
		/**/
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
	//spin
	
	float deltaT = t - tPrev;
	if (tPrev == 0.0f) deltaT = 0.0f;
	tPrev = t;
	angle += 2.0f * deltaT;
	if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();
	/**/
	Ytranslation = (sin(0.8*t) * 0.2f);
}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
	// Render the skybox
		//skyProg.use(); //this breaks everything 
		model = mat4(1.0f);
		setMatrices(prog);
		Sky.render();
		/**/
	
	//Render Spotlight
		vec4 lightPos = vec4(1.0f * cos(angle), 10.0f, 1.0f * sin(angle), 1.0f);
		prog.setUniform("Spot.Position", vec3(view * lightPos));
		mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
		prog.setUniform("Spot.Direction", normalMatrix * vec3(-lightPos));
		/**/

	// Render the mesh
		prog.setUniform("Material.Kd", vec3(0.5f, 0.5f, 0.5f));
		prog.setUniform("Material.Ka", vec3(0.1f, 0.1f, 0.1f) * 0.1f);
		prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
		prog.setUniform("Material.Shininess", 100.0f);
		prog.setUniform("Material.TexDetail", 0);

		model = mat4(1.0f);
		model = glm::rotate(model, glm::radians(-45.0f), vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(20.0f), vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, vec3(0.0f, Ytranslation-5.0f, 0.0f));
		model = glm::scale(model, vec3(10.0f));
		setMatrices(prog);
		mesh->render();

	// Render the rock ring
		prog.setUniform("Material.Kd", vec3(0.9f, 0.9f, 0.9f));
		prog.setUniform("Material.Ka", vec3(0.5f, 0.5f, 0.5f));
		prog.setUniform("Material.Ks", vec3(0.1f, 0.1f, 0.1f));
		prog.setUniform("Material.Shininess", 010.0f);
		prog.setUniform("Material.TexDetail", 2);

		model = mat4(1.0f);
		model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
		setMatrices(prog);
		rockRing.render();

	// Render the lava pool
		prog.setUniform("Material.Kd", vec3(0.99f, 0.99f, 0.99f));
		prog.setUniform("Material.Ka", vec3(0.8f, 0.8f, 0.8f));
		prog.setUniform("Material.Ks", vec3(1.0f, 1.0f, 1.0f));
		prog.setUniform("Material.Shininess", 200.0f);
		prog.setUniform("Material.TexDetail", 3);

		model = mat4(1.0f);
		model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, vec3(4.0f, 4.0f, 0.5f));
		setMatrices(prog);
		lavaPool.render();

	// Render the plane
		prog.setUniform("Material.Kd", vec3(0.1f, 0.1f, 0.1f));
		prog.setUniform("Material.Ka", vec3(0.7f, 0.7f, 0.7f));
		prog.setUniform("Material.Ks", vec3(0.9f, 0.9f, 0.9f));
		prog.setUniform("Material.Shininess", 180.0f);
		prog.setUniform("Material.TexDetail", 2);

		model = mat4(1.0f);
		model = glm::translate(model, vec3(0.0f, -0.45f, 0.0f));
		setMatrices(prog);
		plane.render();

}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
	height = h;
	projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}

void SceneBasic_Uniform::setMatrices(GLSLProgram &p)
{
	mat4 mv = view * model;
	p.setUniform("ModelViewMatrix", mv);
	p.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	p.setUniform("MVP", projection * mv);
}