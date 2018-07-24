// Include C++ headers
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "Winmm.lib")

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>
#include <common/camera.h>
#include <common/ModelLoader.h>
#include <common/texture.h>
#include <common/planetPosition.h>
#include <common/drawPlanets.h>
#include <common/Sphere.h>
#include <common/RigidBody.h>
#include <common/Collision.h>


using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Apallaktikh - 3 Body problem - 228268"


// Global variables
GLFWwindow* window;
Camera* camera;
GLuint shaderProgram;
GLuint projectionMatrixLocation, viewMatrixLocation, modelMatrixLocation;
GLuint lightLocation;
GLuint diffuceColorSampler, specularColorSampler, difSpace, specSpace;
GLuint diffuseTextureEarth, specularTextureEarth, diffuseTextureSun, specularTextureSun,
		diffuseTextureBackground, specularTextureBackground, diffuseTextureMoon, specularTextureMoon;
GLuint diffuseTextureMercury, specularTextureMercury, diffuseTextureVenus, specularTextureVenus,
		diffuseTextureMars, specularTextureMars; 
GLuint diffuseTextureJupiter, specularTextureJupiter,
		diffuseTextureSaturn, specularTextureSaturn, diffuseTextureUranus, specularTextureUranus,
		diffuseTextureNeptune, specularTextureNeptune;
GLuint diffuseTextureComet, specularTextureComet;
GLuint diffuseTextureSpaceShip, specularTextureSpaceShip;
GLuint objVAO;
GLuint objVerticiesVBO, objUVVBO, objNormalsVBO;
GLuint TextureCaseLocation;
GLuint ShadowL, centerL, center2L;
GLuint drawS;
std::vector<vec3> objVertices, objNormals;
std::vector<vec2> objUVs;

float earthRad = 0.00004258744697;
float moonRad = 0.00001161111213;

struct PlanetsData {
	float PlanetRad;
	double PlanetMass;
	float TrajectoryRad;
	vec3 InitPosition;
	vec3 InitVelocity;
} MercuryS, VenusS, EarthS, MoonS, MarsS, JupiterS, 
SaturnS, UranusS, NeptuneS;

float RadiiFactor;
enum { planet = 0, sun, background };

Sphere* MercuryPhysics;
Sphere* VenusPhysics;
Sphere* EarthPhysics;
Sphere* MoonPhysics;
Sphere* MarsPhysics;
Sphere* JupiterPhysics;
Sphere* SaturnPhysics;
Sphere* UranusPhysics;
Sphere* NeptunePhysics;
Sphere* Comet;

void createContext()
{
    // Create and compile our GLSL program from the shaders
    shaderProgram = loadShaders(
        "StandardShading.vertexshader",
        "StandardShading.fragmentshader");

    // load obj
    loadOBJWithTiny("earth.obj", objVertices, objUVs, objNormals);

    //load diffuse texture maps
    diffuseTextureEarth = loadSOIL("earth_diffuse.jpg");
	specularTextureEarth = loadSOIL("earth_diffuse.jpg");
	diffuseTextureSun = loadSOIL("Map_of_the_full_sun.jpg");
	specularTextureSun = loadSOIL("Map_of_the_full_sun.jpg");
	diffuseTextureBackground = loadSOIL("Background.jpg");
	specularTextureBackground = loadSOIL("Background.jpg");
	diffuseTextureMoon = loadSOIL("moon.jpg");
	specularTextureMoon = loadSOIL("moon.jpg");
	diffuseTextureSpaceShip = loadSOIL("pit_inter1.jpg");
	specularTextureSpaceShip = loadSOIL("pit_inter1.jpg");
	diffuseTextureMercury = loadSOIL("mercury.jpg");
	specularTextureMercury = loadSOIL("mercury.jpg");
	diffuseTextureVenus = loadSOIL("venus.jpg");
	specularTextureVenus = loadSOIL("venus.jpg");
	diffuseTextureMars = loadSOIL("mars.jpg");
	specularTextureMars = loadSOIL("mars.jpg");
	diffuseTextureJupiter = loadSOIL("jupiter.jpg");
	specularTextureJupiter = loadSOIL("jupiter.jpg");
	diffuseTextureSaturn = loadSOIL("saturn.jpg");
	specularTextureSaturn = loadSOIL("saturn.jpg");
	diffuseTextureUranus = loadSOIL("uranus.jpg");
	specularTextureUranus = loadSOIL("uranus.jpg");
	diffuseTextureNeptune = loadSOIL("neptune.jpg");
	specularTextureNeptune = loadSOIL("neptune.jpg");
	diffuseTextureComet = loadSOIL("comet.jpg");
	specularTextureComet = loadSOIL("comet.jpg");

    //get a pointer to the texture samplers (diffuseColorSampler, specularColorSampler)
    diffuceColorSampler = glGetUniformLocation(shaderProgram, "diffuceColorSampler");
    specularColorSampler = glGetUniformLocation(shaderProgram, "specularColorSampler");
	difSpace = glGetUniformLocation(shaderProgram, "diffuceSpaceship");
	specSpace = glGetUniformLocation(shaderProgram, "specularSpaceship");

    // get pointers to the uniform variables
    projectionMatrixLocation = glGetUniformLocation(shaderProgram, "P");
    viewMatrixLocation = glGetUniformLocation(shaderProgram, "V");
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
	lightLocation = glGetUniformLocation(shaderProgram, "light_position_worldspace");
	TextureCaseLocation = glGetUniformLocation(shaderProgram, "textureCase");
	ShadowL = glGetUniformLocation(shaderProgram, "Shadow");
	centerL = glGetUniformLocation(shaderProgram, "center");
	center2L = glGetUniformLocation(shaderProgram, "center2");
	drawS = glGetUniformLocation(shaderProgram, "drawSpace");

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // obj
    // bind object vertex positions to attribute 0, UV coordinates
    // to attribute 1 and normals to attribute 2

    glGenVertexArrays(1, &objVAO);
    glBindVertexArray(objVAO);

    // vertex VBO
    glGenBuffers(1, &objVerticiesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, objVerticiesVBO);
    glBufferData(GL_ARRAY_BUFFER, objVertices.size() * sizeof(glm::vec3),
        &objVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
	
    glGenBuffers(1, &objNormalsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, objNormalsVBO);
    glBufferData(GL_ARRAY_BUFFER, objNormals.size() * sizeof(glm::vec3),
        &objNormals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
	
    // uvs VBO
    glGenBuffers(1, &objUVVBO);
    glBindBuffer(GL_ARRAY_BUFFER, objUVVBO);
    glBufferData(GL_ARRAY_BUFFER, objUVs.size() * sizeof(glm::vec2),
        &objUVs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);

	//Initial values for planets Struct
	// MoonS;
	MercuryS.PlanetRad = 0.0000162992;
	MercuryS.TrajectoryRad = 0.387098* 149597870700.0;
	MercuryS.InitPosition = vec3(1, 0, 0);
	MercuryS.InitVelocity = vec3(0, 0, -3.163782e-7* 149597870700.0);
	MercuryS.PlanetMass = 3.3011e+23;
	VenusS.PlanetRad = 0.000040426024;
	VenusS.TrajectoryRad = 0.723332* 149597870700.0;
	VenusS.InitPosition = vec3(1, 0, 0);
	VenusS.InitVelocity = vec3(0, 0, -2.339336e-7* 149597870700.0);
	VenusS.PlanetMass = 4.8675e+24;
	EarthS.PlanetRad = 0.00004258744697;
	EarthS.TrajectoryRad = 0.999315 * 149597870700.0;
	EarthS.InitPosition = vec3(1, 0, 0);
	EarthS.InitVelocity = vec3(0, 0, -29780.0); 
	EarthS.PlanetMass = 5.97237e+24;
	MarsS.PlanetRad = 0.00002264186;
	MarsS.TrajectoryRad = 227939200000.0;
	MarsS.InitPosition = vec3(1, 0, 0);
	MarsS.InitVelocity = vec3(0, 0, -24007.0);
	MarsS.PlanetMass = 6.4171e+23;
	JupiterS.PlanetRad = 0.0004670055;
	JupiterS.TrajectoryRad = 778570000000.0;
	JupiterS.InitPosition = vec3(1, 0, 0);
	JupiterS.InitVelocity = vec3(0, 0, -13070.0);
	JupiterS.PlanetMass = 1.8982e+27;
	SaturnS.PlanetRad = 0.0003889898;
	SaturnS.TrajectoryRad = 1433530000000.0;
	SaturnS.InitPosition = vec3(1, 0, 0);
	SaturnS.InitVelocity = vec3(0, 0, -9680);
	SaturnS.PlanetMass = 5.6834e+26;
	UranusS.PlanetRad = 0.0001694182;
	UranusS.TrajectoryRad = 2.877006e+12;
	UranusS.InitPosition = vec3(1, 0, 0);
	UranusS.InitVelocity = vec3(0, 0, -6800.0);
	UranusS.PlanetMass = 8.681e+25;
	NeptuneS.PlanetRad = 0.0001647422;
	NeptuneS.TrajectoryRad = 4.507543e+12;
	NeptuneS.InitPosition = vec3(1, 0, 0);
	NeptuneS.InitVelocity = vec3(0, 0, -5430.0);
	NeptuneS.PlanetMass = 1.0243e+26;
	MoonS.TrajectoryRad = 384399000.0;
	MoonS.InitVelocity = vec3(0, 0, -1022.0);
	MoonS.InitPosition = vec3(1, 0, 0);
	MoonS.PlanetRad = 1737100.0 / 149597870700.0;
	MoonS.PlanetMass = 7.342e+22;
}

void free()
{
    glDeleteBuffers(1, &objVerticiesVBO);
    glDeleteBuffers(1, &objUVVBO);
    glDeleteBuffers(1, &objNormalsVBO);
    glDeleteVertexArrays(1, &objVAO);

	glDeleteTextures(1, &diffuseTextureMercury);
	glDeleteTextures(1, &specularTextureMercury);
	glDeleteTextures(1, &diffuseTextureVenus);
	glDeleteTextures(1, &specularTextureVenus);
	glDeleteTextures(1, &diffuseTextureMoon);
	glDeleteTextures(1, &specularTextureMoon);
    glDeleteTextures(1, &diffuseTextureEarth);
	glDeleteTextures(1, &specularTextureEarth);
	glDeleteTextures(1, &diffuseTextureMars);
	glDeleteTextures(1, &specularTextureMars);
	glDeleteTextures(1, &diffuseTextureJupiter);
	glDeleteTextures(1, &specularTextureJupiter);
	glDeleteTextures(1, &diffuseTextureSaturn);
	glDeleteTextures(1, &specularTextureSaturn);
	glDeleteTextures(1, &diffuseTextureUranus);
	glDeleteTextures(1, &specularTextureUranus);
	glDeleteTextures(1, &diffuseTextureNeptune);
	glDeleteTextures(1, &specularTextureNeptune);
	glDeleteTextures(1, &diffuseTextureSun);
	glDeleteTextures(1, &specularTextureSun);
	glDeleteTextures(1, &diffuseTextureComet);
	glDeleteTextures(1, &specularTextureComet);
	glDeleteTextures(1, &diffuseTextureBackground);
	glDeleteTextures(1, &specularTextureBackground);
	glDeleteTextures(1, &diffuseTextureSpaceShip);
	glDeleteTextures(1, &specularTextureSpaceShip);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void mainLoop()
{
	//PlaySound("Song.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
	// light's position
    glm::vec3 lightPos = glm::vec3(0, 0, 0);
	enum { other = 0, moon, earth };
	enum { animation = 0, physicsSim, physicsSimSolar,collision};
	float time=0.0;
	float scalingFactor = 2.0;
	float drawSPACE = 0.0;
	float G = -6.674e-11;
	int Part = animation;
	int DisplayAll = animation;
	RadiiFactor = 500.0;
	vec3 earthP, moonP;
	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
		
		vec3 differ;
		static double lastTime = glfwGetTime();
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);
		time += deltaTime*scalingFactor;

		glUseProgram(shaderProgram);
		// camera
		camera->update();
		mat4 projectionMatrix = camera->projectionMatrix;
		mat4 viewMatrix = camera->viewMatrix;

		glUniform1f(drawS, drawSPACE);
		// Bodies position
		if (Part == animation) { 
			earthP = EarthPos((time - 2451545.0) / 365250.0);
			moonP = MoonPos(time) + earthP;
		};
		if ((Part == physicsSim)||(Part==collision)) {  
			// Physics
			MercuryPhysics->forcing = [&](float time, const vector<float>& y)->vector<float> {
				vector<float> f(6, 0.0f);
				f[0] = (ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, 1.98855e+30, vec3(0.0))).x +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).x +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).x +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).x +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).x +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).x +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).x +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).x +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).x;
				f[1] = (ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, 1.98855e+30, vec3(0.0))).y +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).y +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).y +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).y +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).y +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).y +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).y +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).y +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).y;
				f[2] = (ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, 1.98855e+30, vec3(0.0))).z +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).z +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).z +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).z +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).z +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).z +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).z +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).z +
					(ForceCalc(MercuryS.PlanetMass, MercuryPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).z;
				return f;
			};
			

			VenusPhysics->forcing = [&](float time, const vector<float>& y)->vector<float> {
				vector<float> f(6, 0.0f);
				f[0] = (ForceCalc(VenusS.PlanetMass, VenusPhysics->x, 1.98855e+30, vec3(0.0))).x +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).x +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).x +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).x +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).x +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).x +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).x +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).x +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).x;
				f[1] = (ForceCalc(VenusS.PlanetMass, VenusPhysics->x, 1.98855e+30, vec3(0.0))).y +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).y +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).y +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).y +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).y +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).y +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).y +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).y +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).y;
				f[2] = (ForceCalc(VenusS.PlanetMass, VenusPhysics->x, 1.98855e+30, vec3(0.0))).z +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).z +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).z +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).z +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).z +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).z +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).z +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).z +
					(ForceCalc(VenusS.PlanetMass, VenusPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).z;
				return f;
			};
			

			JupiterPhysics->forcing = [&](float time, const vector<float>& y)->vector<float> {
				vector<float> f(6, 0.0f);
				f[0] = (ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, 1.98855e+30, vec3(0.0))).x +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).x +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).x +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).x +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).x +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).x +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).x +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).x +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).x;
				f[1] = (ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, 1.98855e+30, vec3(0.0))).y +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).y +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).y +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).y +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).y +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).y +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).y +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).y +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).y;
				f[2] = (ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, 1.98855e+30, vec3(0.0))).z +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).z +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).z +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).z +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).z +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).z +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).z +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).z +
					(ForceCalc(JupiterS.PlanetMass, JupiterPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).z;
				return f;
			};
			

			earthP = (EarthPhysics->x) / vec3(149597871000.0);
			EarthPhysics->forcing = [&](float time, const vector<float>& y)->vector<float> {
				vector<float> f(6, 0.0f);
				
				f[0] = (ForceCalc(EarthS.PlanetMass, EarthPhysics->x, 1.98855e+30, vec3(0.0))).x+
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).x +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).x +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).x +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).x +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).x +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).x +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).x +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).x;
				f[1] = (ForceCalc(EarthS.PlanetMass, EarthPhysics->x, 1.98855e+30, vec3(0.0))).y+
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).y +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).y +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).y +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).y +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).y +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).y +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).y +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).y;
				f[2] = (ForceCalc(EarthS.PlanetMass, EarthPhysics->x, 1.98855e+30, vec3(0.0))).z +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).z +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).z +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).z +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).z +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).z +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).z +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).z +
					(ForceCalc(EarthS.PlanetMass, EarthPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).z;
				return f;
			};
			

			MarsPhysics->forcing = [&](float time, const vector<float>& y)->vector<float> {
				vector<float> f(6, 0.0f);

				f[0] = (ForceCalc(MarsS.PlanetMass, MarsPhysics->x, 1.98855e+30, vec3(0.0))).x +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).x +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).x +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).x +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).x +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).x +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).x +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).x +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).x;
				f[1] = (ForceCalc(MarsS.PlanetMass, MarsPhysics->x, 1.98855e+30, vec3(0.0))).y +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).y +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).y +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).y +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).y +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).y +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).y +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).y +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).y;
				f[2] = (ForceCalc(MarsS.PlanetMass, MarsPhysics->x, 1.98855e+30, vec3(0.0))).z +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).z +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).z +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).z +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).z +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).z +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).z +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).z +
					(ForceCalc(MarsS.PlanetMass, MarsPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).z;
				return f;
			};

			SaturnPhysics->forcing = [&](float time, const vector<float>& y)->vector<float> {
				vector<float> f(6, 0.0f);

				f[0] = (ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, 1.98855e+30, vec3(0.0))).x +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).x +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).x +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).x +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).x +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).x +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).x +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).x +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).x;
				f[1] = (ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, 1.98855e+30, vec3(0.0))).y +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).y +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).y +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).y +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).y +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).y +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).y +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).y +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).y;
				f[2] = (ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, 1.98855e+30, vec3(0.0))).z +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).z +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).z +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).z +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).z +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).z +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).z +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).z +
					(ForceCalc(SaturnS.PlanetMass, SaturnPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).z;
				return f;
			};

			UranusPhysics->forcing = [&](float time, const vector<float>& y)->vector<float> {
				vector<float> f(6, 0.0f);

				f[0] = (ForceCalc(UranusS.PlanetMass, SaturnPhysics->x, 1.98855e+30, vec3(0.0))).x +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).x +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).x +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).x +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).x +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).x +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).x +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).x +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).x;
				f[1] = (ForceCalc(UranusS.PlanetMass, SaturnPhysics->x, 1.98855e+30, vec3(0.0))).y +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).y +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).y +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).y +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).y +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).y +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).y +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).y +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).y;
				f[2] = (ForceCalc(UranusS.PlanetMass, SaturnPhysics->x, 1.98855e+30, vec3(0.0))).z +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).z +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).z +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, MoonS.PlanetMass, MoonPhysics->x)).z +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).z +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).z +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).z +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).z +
					(ForceCalc(UranusS.PlanetMass, UranusPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).z;
				return f;
			};

			NeptunePhysics->forcing = [&](float time, const vector<float>& y)->vector<float> {
				vector<float> f(6, 0.0f);

				f[0] = (ForceCalc(NeptuneS.PlanetMass, SaturnPhysics->x, 1.98855e+30, vec3(0.0))).x +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).x +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, VenusS.PlanetMass, VenusPhysics->x)).x +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, MoonS.PlanetMass, MoonPhysics->x)).x +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, EarthS.PlanetMass, EarthPhysics->x)).x +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).x +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, MarsS.PlanetMass, MarsPhysics->x)).x +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).x +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, UranusS.PlanetMass, UranusPhysics->x)).x;
				f[1] = (ForceCalc(NeptuneS.PlanetMass, SaturnPhysics->x, 1.98855e+30, vec3(0.0))).y +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).y +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, VenusS.PlanetMass, VenusPhysics->x)).y +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, MoonS.PlanetMass, MoonPhysics->x)).y +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, EarthS.PlanetMass, EarthPhysics->x)).y +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).y +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, MarsS.PlanetMass, MarsPhysics->x)).y +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).y +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, UranusS.PlanetMass, UranusPhysics->x)).y;
				f[2] = (ForceCalc(NeptuneS.PlanetMass, SaturnPhysics->x, 1.98855e+30, vec3(0.0))).z +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).z +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, VenusS.PlanetMass, VenusPhysics->x)).z +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, MoonS.PlanetMass, MoonPhysics->x)).z +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, EarthS.PlanetMass, EarthPhysics->x)).z +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).z +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, MarsS.PlanetMass, MarsPhysics->x)).z +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).z +
					(ForceCalc(NeptuneS.PlanetMass, NeptunePhysics->x, UranusS.PlanetMass, UranusPhysics->x)).z;
				return f;
			};

			MoonPhysics->forcing = [&](float time, const vector<float>& y)->vector<float> {
				vector<float> f(6, 0.0f);

				f[0] = (ForceCalc(MoonS.PlanetMass, MoonPhysics->x, 1.98855e+30, vec3(0.0))).x +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).x +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).x +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).x +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).x +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).x +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).x +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).x +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).x;
				f[1] = (ForceCalc(MoonS.PlanetMass, MoonPhysics->x, 1.98855e+30, vec3(0.0))).y +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).y +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).y +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).y +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).y +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).y +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).y +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).y +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).y;
				f[2] = (ForceCalc(MoonS.PlanetMass, MoonPhysics->x, 1.98855e+30, vec3(0.0))).z +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, MercuryS.PlanetMass, MercuryPhysics->x)).z +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, VenusS.PlanetMass, VenusPhysics->x)).z +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, NeptuneS.PlanetMass, NeptunePhysics->x)).z +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, EarthS.PlanetMass, EarthPhysics->x)).z +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, JupiterS.PlanetMass, JupiterPhysics->x)).z +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, MarsS.PlanetMass, MarsPhysics->x)).z +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, SaturnS.PlanetMass, SaturnPhysics->x)).z +
					(ForceCalc(MoonS.PlanetMass, MoonPhysics->x, UranusS.PlanetMass, UranusPhysics->x)).z;
				return f;
			};
			moonP = (MoonPhysics->x) / vec3(149597871000.0);
			differ = vec3(0.1) * normalize(moonP - earthP);
			//printf("%f %f %f\n", differ.x,differ.y,differ.z);
			moonP += vec3(0.1) * normalize(moonP - earthP);

			MarsPhysics->update(time, (86250.0)*deltaTime*scalingFactor);
			EarthPhysics->update(time, (86250.0)*deltaTime*scalingFactor);
			JupiterPhysics->update(time, (86250.0)*deltaTime*scalingFactor);
			VenusPhysics->update(time, (86250.0)*deltaTime*scalingFactor);
			MercuryPhysics->update(time, (86250.0)*deltaTime*scalingFactor);
			SaturnPhysics->update(time, (86250.0)*deltaTime*scalingFactor);
			UranusPhysics->update(time, (86250.0)*deltaTime*scalingFactor);
			NeptunePhysics->update(time, (86250.0)*deltaTime*scalingFactor);
			MoonPhysics->update(time, (86250.0)*deltaTime*scalingFactor);
		};


		// SUN //
		ComputeShadowPrism(earthP, moonP, other);
		DrawingPlanets(projectionMatrix, viewMatrix, vec3(0.0), 0.0f, 0.0f,
			RadiiFactor*0.0001f, lightPos, sun, diffuseTextureSun, specularTextureSun);

		
		// EARTH //
		ComputeShadowPrism(earthP, moonP, earth);
		DrawingPlanets(projectionMatrix, viewMatrix, earthP, time*3.14, radians(23.4f),
			RadiiFactor*EarthS.PlanetRad, lightPos, planet, diffuseTextureEarth, specularTextureEarth);

		if (Part == collision) {
			Comet->forcing = [&](float time, const vector<float>& y)->vector<float> {
				vector<float> f(6, 0.0f);
				f[0] = 0;
				f[1] = 0;
				f[2] = 0;
				return f;
			};
			Comet->update(time, (86250.0)*deltaTime*scalingFactor);
			handleSphereSphereCollision(*MoonPhysics, *Comet);
			DrawingPlanets(projectionMatrix, viewMatrix, (Comet->x) / (vec3(149597871000.0))+vec3(0.005,0.0,-0.1), time, radians(50.4f),
				RadiiFactor*(Comet->r), lightPos, planet, diffuseTextureComet, specularTextureComet);
		}
		// Moon //
		ComputeShadowPrism(earthP, moonP, moon);
		float moonRot = radians((27.3)*time) ;
		DrawingPlanets(projectionMatrix, viewMatrix, moonP, moonRot, radians(-5.0f),
			RadiiFactor*moonRad, lightPos, planet, diffuseTextureMoon, specularTextureMoon);
		
		// BACKGROUND //
		ComputeShadowPrism(earthP, moonP, other);
		DrawingPlanets(projectionMatrix, viewMatrix, vec3(0), 0.0f, 0.0f,
			40.0f, lightPos, background, diffuseTextureBackground, specularTextureBackground);
		
		lastTime = currentTime;

		if ((Part == physicsSim)&&(DisplayAll==physicsSimSolar)) {
			// Mercury
			DrawingPlanets(projectionMatrix, viewMatrix, (MercuryPhysics->x)/ (vec3(149597871000.0)), time*3.14/58.0, radians(2.04f),
				RadiiFactor*MercuryS.PlanetRad, lightPos, planet, diffuseTextureMercury, specularTextureMercury);
			// Venus
			DrawingPlanets(projectionMatrix, viewMatrix, (VenusPhysics->x) / (vec3(149597871000.0)), time*3.14/243.0, radians(2.64f),
				RadiiFactor*VenusS.PlanetRad, lightPos, planet, diffuseTextureVenus, specularTextureVenus);
			// Mars
			DrawingPlanets(projectionMatrix, viewMatrix, (MarsPhysics->x) / (vec3(149597871000.0)), time*3.14/1.025, radians(25.19f),
				RadiiFactor*MarsS.PlanetRad, lightPos, planet, diffuseTextureMars, specularTextureMars);
			// Jupiter
			DrawingPlanets(projectionMatrix, viewMatrix, (JupiterPhysics->x) / (vec3(149597871000.0)), time*3.14*24.0/9.925, radians(3.13f),
				RadiiFactor*JupiterS.PlanetRad, lightPos, planet, diffuseTextureJupiter, specularTextureJupiter);
			// Saturn
			DrawingPlanets(projectionMatrix, viewMatrix, (SaturnPhysics->x) / (vec3(149597871000.0)), time*24.0/10.55, radians(26.73f),
				RadiiFactor*SaturnS.PlanetRad, lightPos, planet, diffuseTextureSaturn, specularTextureSaturn);
			// Uranus
			DrawingPlanets(projectionMatrix, viewMatrix, (UranusPhysics->x) / (vec3(149597871000.0)), -time*3.14/ 0.718, radians(97.77f),
				RadiiFactor*UranusS.PlanetRad, lightPos, planet, diffuseTextureUranus, specularTextureUranus);
			// Neptune
			DrawingPlanets(projectionMatrix, viewMatrix, (NeptunePhysics->x) / (vec3(149597871000.0)), time*3.14/ 0.6713, radians(28.32f),
				RadiiFactor*NeptuneS.PlanetRad, lightPos, planet, diffuseTextureNeptune, specularTextureNeptune);
		}

		
		// Polling //
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			drawSPACE = 0.0;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			drawSPACE = 1.0;
		}
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			scalingFactor += scalingFactor / 100;
		}
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
			scalingFactor -= scalingFactor / 100;
		}
		if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
			RadiiFactor += 1.0;
		}
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
			RadiiFactor -= 1.0;
		}
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			Part = animation;
			DisplayAll = animation;
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			//Initialization of Physics
			MercuryPhysics = new Sphere(MercuryS.InitPosition*MercuryS.TrajectoryRad, MercuryS.InitVelocity, MercuryS.PlanetRad, MercuryS.PlanetMass);
			VenusPhysics = new Sphere(VenusS.InitPosition*VenusS.TrajectoryRad, VenusS.InitVelocity, VenusS.PlanetRad, VenusS.PlanetMass);
			EarthPhysics = new Sphere(EarthS.InitPosition*EarthS.TrajectoryRad, EarthS.InitVelocity, EarthS.PlanetRad, EarthS.PlanetMass);
			MoonPhysics = new Sphere(MoonS.InitPosition*MoonS.TrajectoryRad, MoonS.InitVelocity, MoonS.PlanetRad, MoonS.PlanetMass);
			MarsPhysics = new Sphere(MarsS.InitPosition*MarsS.TrajectoryRad, MarsS.InitVelocity, MarsS.PlanetRad, MarsS.PlanetMass);
			JupiterPhysics = new Sphere(JupiterS.InitPosition*JupiterS.TrajectoryRad, JupiterS.InitVelocity, JupiterS.PlanetRad, JupiterS.PlanetMass);
			SaturnPhysics = new Sphere(SaturnS.InitPosition*SaturnS.TrajectoryRad, SaturnS.InitVelocity, SaturnS.PlanetRad, SaturnS.PlanetMass);
			UranusPhysics = new Sphere(UranusS.InitPosition*UranusS.TrajectoryRad, UranusS.InitVelocity, UranusS.PlanetRad, UranusS.PlanetMass);
			NeptunePhysics = new Sphere(NeptuneS.InitPosition*NeptuneS.TrajectoryRad, NeptuneS.InitVelocity, NeptuneS.PlanetRad, NeptuneS.PlanetMass);
			MoonPhysics = new Sphere(MoonS.InitPosition*MoonS.TrajectoryRad + EarthS.InitPosition*EarthS.TrajectoryRad, 
				MoonS.InitVelocity + EarthS.InitVelocity, MoonS.PlanetRad, MoonS.PlanetMass);
			Part = physicsSim;
			DisplayAll = animation;
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
			//Initialization of Physics
			MercuryPhysics = new Sphere(MercuryS.InitPosition*MercuryS.TrajectoryRad, MercuryS.InitVelocity, MercuryS.PlanetRad, MercuryS.PlanetMass);
			VenusPhysics = new Sphere(VenusS.InitPosition*VenusS.TrajectoryRad, VenusS.InitVelocity, VenusS.PlanetRad, VenusS.PlanetMass);
			EarthPhysics = new Sphere(EarthS.InitPosition*EarthS.TrajectoryRad, EarthS.InitVelocity, EarthS.PlanetRad, EarthS.PlanetMass);
			MoonPhysics = new Sphere(MoonS.InitPosition*MoonS.TrajectoryRad, MoonS.InitVelocity, MoonS.PlanetRad, MoonS.PlanetMass);
			MarsPhysics = new Sphere(MarsS.InitPosition*MarsS.TrajectoryRad, MarsS.InitVelocity, MarsS.PlanetRad, MarsS.PlanetMass);
			JupiterPhysics = new Sphere(JupiterS.InitPosition*JupiterS.TrajectoryRad, JupiterS.InitVelocity, JupiterS.PlanetRad, JupiterS.PlanetMass);
			SaturnPhysics = new Sphere(SaturnS.InitPosition*SaturnS.TrajectoryRad, SaturnS.InitVelocity, SaturnS.PlanetRad, SaturnS.PlanetMass);
			UranusPhysics = new Sphere(UranusS.InitPosition*UranusS.TrajectoryRad, UranusS.InitVelocity, UranusS.PlanetRad, UranusS.PlanetMass);
			NeptunePhysics = new Sphere(NeptuneS.InitPosition*NeptuneS.TrajectoryRad, NeptuneS.InitVelocity, NeptuneS.PlanetRad, NeptuneS.PlanetMass);
			MoonPhysics = new Sphere(MoonS.InitPosition*MoonS.TrajectoryRad + EarthS.InitPosition*EarthS.TrajectoryRad, MoonS.InitVelocity + EarthS.InitVelocity, MoonS.PlanetRad, MoonS.PlanetMass);
			Part = physicsSim;
			DisplayAll = physicsSimSolar;
		}
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
			//Initialization of Physics
			MercuryPhysics = new Sphere(MercuryS.InitPosition*MercuryS.TrajectoryRad, MercuryS.InitVelocity, MercuryS.PlanetRad, MercuryS.PlanetMass);
			VenusPhysics = new Sphere(VenusS.InitPosition*VenusS.TrajectoryRad, VenusS.InitVelocity, VenusS.PlanetRad, VenusS.PlanetMass);
			EarthPhysics = new Sphere(EarthS.InitPosition*EarthS.TrajectoryRad, EarthS.InitVelocity, EarthS.PlanetRad, EarthS.PlanetMass);
			MoonPhysics = new Sphere(MoonS.InitPosition*MoonS.TrajectoryRad, MoonS.InitVelocity, MoonS.PlanetRad, MoonS.PlanetMass);
			MarsPhysics = new Sphere(MarsS.InitPosition*MarsS.TrajectoryRad, MarsS.InitVelocity, MarsS.PlanetRad, MarsS.PlanetMass);
			JupiterPhysics = new Sphere(JupiterS.InitPosition*JupiterS.TrajectoryRad, JupiterS.InitVelocity, JupiterS.PlanetRad, JupiterS.PlanetMass);
			SaturnPhysics = new Sphere(SaturnS.InitPosition*SaturnS.TrajectoryRad, SaturnS.InitVelocity, SaturnS.PlanetRad, SaturnS.PlanetMass);
			UranusPhysics = new Sphere(UranusS.InitPosition*UranusS.TrajectoryRad, UranusS.InitVelocity, UranusS.PlanetRad, UranusS.PlanetMass);
			NeptunePhysics = new Sphere(NeptuneS.InitPosition*NeptuneS.TrajectoryRad, NeptuneS.InitVelocity, NeptuneS.PlanetRad, NeptuneS.PlanetMass);
			MoonPhysics = new Sphere(MoonS.InitPosition*MoonS.TrajectoryRad + EarthS.InitPosition*EarthS.TrajectoryRad, MoonS.InitVelocity + EarthS.InitVelocity, MoonS.PlanetRad, MoonS.PlanetMass);
			Part = collision;
			DisplayAll = animation;
			vec3 startloc = vec3(1.0, 0.0, -0.2);
			Comet = new Sphere(startloc * vec3(149597870700.0), vec3(0.0,0.0,1.0)* vec3(20000.0), MoonS.PlanetRad / 2, MoonS.PlanetMass / 2);
		}

		if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
			PlaySound("Song.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
		}
		

		glfwSwapBuffers(window);

		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
			glfwWindowShouldClose(window) == 0);
	
}

void initialize()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        throw runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        throw runtime_error(string(string("Failed to open GLFW window.") +
            " If you have an Intel GPU, they are not 3.3 compatible." +
            "Try the 2.1 version.\n"));
    }
    glfwMakeContextCurrent(window);

    // Start GLEW extension handler
    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        throw runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, W_WIDTH / 2, W_HEIGHT / 2);

    // Gray background color
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // enable textures
    glEnable(GL_TEXTURE_2D);

    // Log
    logGLParameters();

    // Create camera
    camera = new Camera(window);
}

int main(void)
{
    try
    {
        initialize();
        createContext();
        mainLoop();
        free();
    }
    catch (exception& ex)
    {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }

    return 0;
}