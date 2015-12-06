// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#ifdef __linux__

#elif _WIN32
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include "GLFW\glfw3.h"
// Include GLM
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#elif __APPLE__
#include <glew.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

GLFWwindow* window;

using namespace glm;

#include "shader.hpp"
#include "objloader.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "vboindexer.hpp"



int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);


	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 08 - Basic Shading", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

	// Load the texture
	GLuint Texture = loadBMP_custom("bricks.bmp");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> verticesMesh;
	std::vector<glm::vec2> uvsMesh;
	std::vector<glm::vec3> normalsMesh;

	/* Insert obj file here */
	/*********************************************/

	bool res = loadOBJ("mesh.obj", verticesMesh, uvsMesh, normalsMesh);

	/*********************************************/

	// Load it into a VBO

	GLuint vertexbufferMesh;
	glGenBuffers(1, &vertexbufferMesh);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferMesh);
	glBufferData(GL_ARRAY_BUFFER, verticesMesh.size() * sizeof(glm::vec3), &verticesMesh[0], GL_STATIC_DRAW);

	GLuint uvbufferMesh;
	glGenBuffers(1, &uvbufferMesh);
	glBindBuffer(GL_ARRAY_BUFFER, uvbufferMesh);
	glBufferData(GL_ARRAY_BUFFER, uvsMesh.size() * sizeof(glm::vec2), &uvsMesh[0], GL_STATIC_DRAW);

	GLuint normalbufferMesh;
	glGenBuffers(1, &normalbufferMesh);
	glBindBuffer(GL_ARRAY_BUFFER, normalbufferMesh);
	glBufferData(GL_ARRAY_BUFFER, normalsMesh.size() * sizeof(glm::vec3), &normalsMesh[0], GL_STATIC_DRAW);
    
    std::vector<glm::vec3> verticesPin;
    std::vector<glm::vec2> uvsPin;
    std::vector<glm::vec3> normalsPin;

    res = loadOBJ("bowlpin.obj", verticesPin, uvsPin, normalsPin);
    
    /*********************************************/
    
    // Load it into a VBO
    
    GLuint vertexbufferPin;
    glGenBuffers(1, &vertexbufferPin);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbufferPin);
    glBufferData(GL_ARRAY_BUFFER, verticesPin.size() * sizeof(glm::vec3), &verticesPin[0], GL_STATIC_DRAW);
    
    GLuint uvbufferPin;
    glGenBuffers(1, &uvbufferPin);
    glBindBuffer(GL_ARRAY_BUFFER, uvbufferPin);
    glBufferData(GL_ARRAY_BUFFER, uvsPin.size() * sizeof(glm::vec2), &uvsPin[0], GL_STATIC_DRAW);
    
    GLuint normalbufferPin;
    glGenBuffers(1, &normalbufferPin);
    glBindBuffer(GL_ARRAY_BUFFER, normalbufferPin);
    glBufferData(GL_ARRAY_BUFFER, normalsPin.size() * sizeof(glm::vec3), &normalsPin[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(vertexPosition_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferMesh);
		glVertexAttribPointer(
			vertexPosition_modelspaceID,  // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(vertexUVID);
		glBindBuffer(GL_ARRAY_BUFFER, uvbufferMesh);
		glVertexAttribPointer(
			vertexUVID,                   // The attribute we want to configure
			2,                            // size : U+V => 2
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(vertexNormal_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, normalbufferMesh);
		glVertexAttribPointer(
			vertexNormal_modelspaceID,    // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, verticesMesh.size() );
        
        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs();
        ProjectionMatrix = getProjectionMatrix();
        ViewMatrix = getViewMatrix();
        ModelMatrix = glm::mat4(1.0);
        
        ModelMatrix = glm::translate(ModelMatrix, vec3(-4,0,0));
        
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
        
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to user Texture Unit 0
        glUniform1i(TextureID, 0);
        
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(vertexPosition_modelspaceID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferPin);
        glVertexAttribPointer(
                              vertexPosition_modelspaceID,  // The attribute we want to configure
                              3,                            // size
                              GL_FLOAT,                     // type
                              GL_FALSE,                     // normalized?
                              0,                            // stride
                              (void*)0                      // array buffer offset
                              );
        
        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(vertexUVID);
        glBindBuffer(GL_ARRAY_BUFFER, uvbufferPin);
        glVertexAttribPointer(
                              vertexUVID,                   // The attribute we want to configure
                              2,                            // size : U+V => 2
                              GL_FLOAT,                     // type
                              GL_FALSE,                     // normalized?
                              0,                            // stride
                              (void*)0                      // array buffer offset
                              );
        
        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(vertexNormal_modelspaceID);
        glBindBuffer(GL_ARRAY_BUFFER, normalbufferPin);
        glVertexAttribPointer(
                              vertexNormal_modelspaceID,    // The attribute we want to configure
                              3,                            // size
                              GL_FLOAT,                     // type
                              GL_FALSE,                     // normalized?
                              0,                            // stride
                              (void*)0                      // array buffer offset
                              );
        
        // Draw the triangles !
        glDrawArrays(GL_TRIANGLES, 0, verticesPin.size() );

		glDisableVertexAttribArray(vertexPosition_modelspaceID);
		glDisableVertexAttribArray(vertexUVID);
		glDisableVertexAttribArray(vertexNormal_modelspaceID);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbufferMesh);
	glDeleteBuffers(1, &uvbufferMesh);
	glDeleteBuffers(1, &normalbufferMesh);
    glDeleteBuffers(1, &vertexbufferPin);
    glDeleteBuffers(1, &uvbufferPin);
    glDeleteBuffers(1, &normalbufferPin);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

