///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(bFound);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


 /***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering.
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{

	bool bReturn = false;

	bReturn = CreateGLTexture("textures/grass.jpg", "floor");
	bReturn = CreateGLTexture("textures/brick-wall-v.jpg", "pyramid");
	bReturn = CreateGLTexture("textures/black-and-grey-metal-texture.jpg", "cylinder");
	bReturn = CreateGLTexture("textures/wood.jpg", "legs");
	bReturn = CreateGLTexture("textures/tiles.jpg", "box2");
	bReturn = CreateGLTexture("textures/water.jpg", "sphere");
	bReturn = CreateGLTexture("textures/wood desk.jpg", "desk");
	bReturn = CreateGLTexture("textures/wood desk 2.jpg", "desk2");
	bReturn = CreateGLTexture("textures/floor1.jpg", "floor1");
	bReturn = CreateGLTexture("textures/floor2.jpg", "floor2");
	bReturn = CreateGLTexture("textures/beige wall.jpg", "wall");
	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// load the textures for the 3D scene
	LoadSceneTextures();
	DefineObjectMaterials();
	SetupSceneLights();
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadTorusMesh();

	// Defines all scene objects for centralized rendering system.
	DefineSceneObjects();
}
/***********************************************************
* DefineObjectMaterials()
*
* This method is used for configuring the various material
* settings for all of the objects in the 3D scene. This method was given in the supporting materials reading.
***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL goldMaterial;
	goldMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.1f);
	goldMaterial.ambientStrength = 0.5f;
	goldMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
	goldMaterial.specularColor = glm::vec3(0.6f, 0.5f, 0.4f);
	goldMaterial.shininess = 22.0f;
	goldMaterial.tag = "gold";
	m_objectMaterials.push_back(goldMaterial);

	OBJECT_MATERIAL cementMaterial;
	cementMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.3f);
	cementMaterial.ambientStrength = 0.6f; // Increased
	cementMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	cementMaterial.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	cementMaterial.shininess = 0.5f;
	cementMaterial.tag = "cement";
	m_objectMaterials.push_back(cementMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.4f, 0.3f, 0.1f);
	woodMaterial.ambientStrength = 0.5f; // Increased
	woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	woodMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	woodMaterial.shininess = 0.3f;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL tileMaterial;
	tileMaterial.ambientColor = glm::vec3(0.2f, 0.3f, 0.4f);
	tileMaterial.ambientStrength = 0.5f; // Increased
	tileMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	tileMaterial.specularColor = glm::vec3(0.4f, 0.5f, 0.6f);
	tileMaterial.shininess = 25.0f;
	tileMaterial.tag = "tile";
	m_objectMaterials.push_back(tileMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	glassMaterial.ambientStrength = 0.5f; // Increased
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
	glassMaterial.shininess = 85.0f;
	glassMaterial.tag = "glass";
	m_objectMaterials.push_back(glassMaterial);

	OBJECT_MATERIAL clayMaterial;
	clayMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.3f);
	clayMaterial.ambientStrength = 0.5f; // Increased
	clayMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.5f);
	clayMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.4f);
	clayMaterial.shininess = 0.5f;
	clayMaterial.tag = "clay";
	m_objectMaterials.push_back(clayMaterial);

	OBJECT_MATERIAL lampMaterial;
	lampMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	lampMaterial.ambientStrength = 0.3f;
	lampMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	lampMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	lampMaterial.shininess = 10.0f;
	lampMaterial.tag = "lamp";
	m_objectMaterials.push_back(lampMaterial);

	OBJECT_MATERIAL wallMaterial;
	wallMaterial.ambientColor = glm::vec3(0.5f, 0.45f, 0.4f);//beige-ish
	wallMaterial.ambientStrength = 0.2f;
	wallMaterial.diffuseColor = glm::vec3(0.6f, 0.55f, 0.5f);
	wallMaterial.specularColor = glm::vec3(0.15f, 0.15f, 0.15f);
	wallMaterial.shininess = 4.0f;
	wallMaterial.tag = "wall";
	m_objectMaterials.push_back(wallMaterial);
}
/***********************************************************
* SetupSceneLights()
*
* This method is used for setting the position and color values on the light sources, with up to 4 sources possible.
* This method was given in the supporting materials reading.
***********************************************************/
void SceneManager::SetupSceneLights() {
	// light 1
	m_pShaderManager->setVec3Value("lightSources[0].position", glm::vec3(0.0f, 6.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", glm::vec3(0.05f));
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", glm::vec3(0.3f, 0.3f, 0.2f));
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", glm::vec3(0.2f));
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 12.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.2f);

	// light 2
	m_pShaderManager->setVec3Value("lightSources[1].position", glm::vec3(-5.0f, 2.0f, -8.0f));
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", glm::vec3(0.02f));
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", glm::vec3(0.2f));
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", glm::vec3(0.1f));
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 8.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.15f);

	// light 3
	m_pShaderManager->setVec3Value("lightSources[2].position", glm::vec3(7.0f, 1.0f, 3.0f));
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", glm::vec3(0.01f));
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", glm::vec3(0.1f, 0.1f, 0.2f));
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", glm::vec3(0.05f));
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 6.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.1f);

	// light 4
	m_pShaderManager->setVec3Value("lightSources[3].position", glm::vec3(4.0f, 10.0f, 5.0f));
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", glm::vec3(0.03f));
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", glm::vec3(0.25f));
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", glm::vec3(0.15f));
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 16.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.15f);


	// Enable lighting in the shader. Currently configured for 4 light sources,
	// because the shader expects all light slots to be initialized.
	m_pShaderManager->setBoolValue("bUseLighting", true);

}
/***********************************************************
 *  DrawFloorAndDesk()
 *
 *  This method is used for rendering the floor and desk
 *  components of the 3D scene.
 ***********************************************************/
void SceneManager::DrawFloorAndDesk()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// floor
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("floor2");
	SetShaderMaterial("clay");
	SetTextureUVScale(5.0f, 5.0f);
	m_basicMeshes->DrawPlaneMesh();

	// tabletop
	scaleXYZ = glm::vec3(10.0f, 0.3f, 4.0f);
	positionXYZ = glm::vec3(-3.0f, 2.65f, 1.8f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderColor(0.75f, 0.6f, 0.4f, 1.0f);
	SetShaderTexture("desk");
	SetShaderMaterial("wood");
	SetTextureUVScale(5.0f, 5.0f);
	m_basicMeshes->DrawBoxMesh();

	// legs
	scaleXYZ = glm::vec3(0.3f, 2.5f, 0.3f);
	SetShaderColor(0.5f, 0.4f, 0.3f, 1.0f);
	SetShaderTexture("legs");
	SetShaderMaterial("tile");

	positionXYZ = glm::vec3(-7.85f, 1.25f, 3.65f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	positionXYZ = glm::vec3(1.85f, 1.25f, 3.65f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	positionXYZ = glm::vec3(-7.85f, 1.25f, -0.05f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	positionXYZ = glm::vec3(1.85f, 1.25f, -0.05f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();
}
/***********************************************************
 *  DrawLamp()
 *
 *  This method is used for rendering the lamp object
 *  including its base, arms, joints, and head.
 *  This method is deprecated, and is being left in to demonstrate how the code changed during the enhancement process.
 ***********************************************************/
void SceneManager::DrawLamp()
{
	glm::vec3 positionXYZ = glm::vec3(-3.0f, 2.66f, 1.8f);
	SetShaderMaterial("lamp");

	// base
	SetTransformations(glm::vec3(0.8f, 0.1f, 0.8f), 0, 0, 0, glm::vec3(-6.0f, 2.8f, 1.8f));
	m_basicMeshes->DrawCylinderMesh();

	// arm bottom
	positionXYZ = glm::vec3(-6.0f, 2.85f, 1.8f);
	SetTransformations(glm::vec3(0.1f, 1.5f, 0.1f), -45.0f, 30.0f, 45.0f, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();

	// joint
	positionXYZ = glm::vec3(-6.9f, 4.0f, 1.4f);
	SetTransformations(glm::vec3(0.2f), 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawSphereMesh();

	// top arm
	SetTransformations(glm::vec3(0.1f, 1.0f, 0.1f), 45.0f, -30.0f, -45.0f, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();

	// head joint
	positionXYZ = glm::vec3(-6.3f, 4.3f, 2.2f);
	SetTransformations(glm::vec3(0.2f), 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawSphereMesh();

	// neck
	SetTransformations(glm::vec3(0.1f, 0.6f, 0.1f), 40.0f, 0.0f, -25.0f, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();

	// head
	positionXYZ = glm::vec3(-5.85f, 4.30f, 2.75f);
	SetTransformations(glm::vec3(0.6f), 100.0f, 130.0f, 230.0f, positionXYZ);
	m_basicMeshes->DrawConeMesh();

	// bulb
	positionXYZ = glm::vec3(-5.80f, 4.35f, 2.80f);
	SetShaderMaterial("");
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetTransformations(glm::vec3(0.2f, 0.25f, 0.2f), 100.0f, 130.0f, 230.0f, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();
}
/***********************************************************
 *  DrawWalls()
 *
 *  This method is used for rendering the walls of the scene.
 *  This method is deprecated, and is being left in to demonstrate how the code changed during the enhancement process.
 ***********************************************************/
void SceneManager::DrawWalls()
{
	glm::vec3 scale;
	glm::vec3 position;

	SetShaderTexture("wall");
	SetShaderMaterial("wall");
	SetTextureUVScale(3.0f, 3.0f);

	// back wall
	scale = glm::vec3(20.0f, 10.0f, 0.3f);
	position = glm::vec3(0.0f, 5.0f, -1.5f);
	SetTransformations(scale, 0.0f, 0.0f, 0.0f, position);
	m_basicMeshes->DrawBoxMesh();

	// right wall
	scale = glm::vec3(10.0f, 10.0f, 0.3f);
	position = glm::vec3(10.0f, 5.0f, 3.0f);
	SetTransformations(scale, 0.0f, 90.0f, 0.0f, position);
	m_basicMeshes->DrawBoxMesh();

	// left wall
	position = glm::vec3(-10.0f, 5.0f, 3.0f);
	SetTransformations(scale, 0.0f, 90.0f, 0.0f, position);
	m_basicMeshes->DrawBoxMesh();
}
/***********************************************************
 *  DrawBooks()
 *
 *  This method is used for rendering stacked books.
 *  This method is deprecated, and is being left in to demonstrate how the code changed during the enhancement process.
 ***********************************************************/
void SceneManager::DrawBooks()
{
	glm::vec3 base = glm::vec3(-5.9f, 2.8f, 3.05f);
	glm::vec3 size = glm::vec3(1.0f, 0.2f, 0.6f);

	SetShaderMaterial("wood");

	SetShaderColor(0.1f, 0.2f, 0.6f, 1.0f);
	SetTransformations(size, 0, 0, 0, base);
	m_basicMeshes->DrawBoxMesh();

	SetShaderColor(0.6f, 0.1f, 0.1f, 1.0f);
	SetTransformations(size, 0, 0, 0, base + glm::vec3(0.0f, 0.22f, 0.0f));
	m_basicMeshes->DrawBoxMesh();

	SetShaderColor(0.95f, 0.95f, 0.95f, 1.0f);
	SetTransformations(size, 0, 0, 0, base + glm::vec3(0.0f, 0.44f, 0.0f));
	m_basicMeshes->DrawBoxMesh();

	SetShaderColor(0.85f, 0.75f, 0.6f, 1.0f);
	SetTransformations(size, 0, 0, 0, base + glm::vec3(0.0f, 0.66f, 0.0f));
	m_basicMeshes->DrawBoxMesh();
}
/***********************************************************
 *  DrawPencilCupAndPencils()
 *
 *  This method is used for rendering the pencil holder
 *  and pencils.
 *  This method is deprecated, and is being left in to demonstrate how the code changed during the enhancement process.
 ***********************************************************/
void SceneManager::DrawPencilCupAndPencils()
{
	glm::vec3 cup = glm::vec3(-4.4f, 2.8f, 1.85f);

	SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f);
	SetTransformations(glm::vec3(0.15f, 0.3f, 0.15f), 0, 0, 0, cup);
	m_basicMeshes->DrawCylinderMesh();

	glm::vec3 color = glm::vec3(0.9f, 0.6f, 0.2f);
	float h = 0.4f;
	float tip = 0.1f;
	float s = 0.07f;

	SetShaderColor(color.r, color.g, color.b, 1.0f);

	SetTransformations(glm::vec3(0.03f, h, 0.03f), -10, 0, 0, cup + glm::vec3(-s, 0.3f, 0));
	m_basicMeshes->DrawCylinderMesh();
	SetTransformations(glm::vec3(0.03f, tip, 0.03f), -10, 0, 0, cup + glm::vec3(-s, 0.3f + h, -0.07f));
	m_basicMeshes->DrawConeMesh();

	SetTransformations(glm::vec3(0.03f, h, 0.03f), 0, 0, 0, cup + glm::vec3(0, 0.3f, 0));
	m_basicMeshes->DrawCylinderMesh();
	SetTransformations(glm::vec3(0.03f, tip, 0.03f), 0, 0, 0, cup + glm::vec3(0, 0.3f + h, 0));
	m_basicMeshes->DrawConeMesh();

	SetTransformations(glm::vec3(0.03f, h, 0.03f), 10, 0, 0, cup + glm::vec3(s, 0.3f, 0));
	m_basicMeshes->DrawCylinderMesh();
	SetTransformations(glm::vec3(0.03f, tip, 0.03f), 10, 0, 0, cup + glm::vec3(s, 0.3f + h, 0.07f));
	m_basicMeshes->DrawConeMesh();
}
/***********************************************************
 *  DrawLaptop()
 *
 *  This method is used for rendering the laptop and its
 *  components.
 *  This method is deprecated, and is being left in to demonstrate how the code changed during the enhancement process.
 ***********************************************************/
void SceneManager::DrawLaptop()
{
	glm::vec3 base = glm::vec3(-2.5f, 2.8f, 2.0f);

	SetShaderMaterial("glass");
	SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f);
	SetTransformations(glm::vec3(1.5f, 0.1f, 1.0f), 0, 0, 0, base);
	m_basicMeshes->DrawBoxMesh();

	glm::vec3 screen = base + glm::vec3(0.0f, 0.5f, -0.60f);
	SetTransformations(glm::vec3(1.5f, 1.0f, 0.05f), -10, 0, 0, screen);
	m_basicMeshes->DrawBoxMesh();

	SetTransformations(glm::vec3(1.4f, 0.9f, 0.01f), -10, 0, 0, screen + glm::vec3(0, 0, 0.03f));
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

	SetTransformations(glm::vec3(1.4f, 0.01f, 0.46f), 0, 0, 0, base + glm::vec3(0, 0.06f, -0.2f));
	SetShaderColor(0.15f, 0.15f, 0.15f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	SetTransformations(glm::vec3(0.5f, 0.01f, 0.3f), 0, 0, 0, base + glm::vec3(0, 0.06f, 0.3f));
	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
	m_basicMeshes->DrawBoxMesh();
}
/***********************************************************
 *  DrawMug()
 *
 *  This method is used for rendering the coffee mug.
 *  This method is deprecated, and is being left in to demonstrate how the code changed during the enhancement process.
 ***********************************************************/
void SceneManager::DrawMug()
{
	glm::vec3 base = glm::vec3(-4.4f, 2.8f, 2.65f);

	SetShaderMaterial("clay");
	SetShaderColor(0.9f, 0.9f, 0.9f, 1.0f);
	SetTransformations(glm::vec3(0.25f, 0.4f, 0.25f), 0, 0, 0, base);
	m_basicMeshes->DrawCylinderMesh();

	glm::vec3 handle = glm::vec3(-4.15f, 2.95f, 2.65f);
	SetTransformations(glm::vec3(0.1f), 0, 0, 90.0f, handle);
	m_basicMeshes->DrawTorusMesh();

	glm::vec3 top = glm::vec3(-4.4f, 3.185f, 2.65f);
	SetShaderColor(0.75f, 0.6f, 0.4f, 1.0f);
	SetTransformations(glm::vec3(0.24f, 0.02f, 0.24f), 0, 0, 0, top);
	m_basicMeshes->DrawCylinderMesh();
}
/***********************************************************
 *  DefineSceneObjects()
 *
 *  This method initializes all scene objects used in the centralized object management system
 *  and supersedes the old method of hardcoding each object before it gets created.
 ***********************************************************/
void SceneManager::DefineSceneObjects()
{
	m_sceneObjects.clear();

	SCENE_OBJECT obj;

	// floor
	obj.groupTag = "environment";
	obj.name = "floor";
	obj.meshType = "plane";
	obj.scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	obj.textureTag = "floor2";
	obj.materialTag = "clay";
	obj.colorValue = glm::vec4(1.0f);
	obj.useTexture = true;
	obj.useColor = false;
	obj.uScale = 5.0f;
	obj.vScale = 5.0f;
	m_sceneObjects.push_back(obj);

	// desk tabletop
	obj.groupTag = "furniture";
	obj.name = "desk";
	obj.meshType = "box";
	obj.scaleXYZ = glm::vec3(10.0f, 0.3f, 4.0f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-3.0f, 2.65f, 1.8f);
	obj.textureTag = "desk";
	obj.materialTag = "wood";
	obj.colorValue = glm::vec4(0.75f, 0.6f, 0.4f, 1.0f);
	obj.useTexture = true;
	obj.useColor = true;
	obj.uScale = 5.0f;
	obj.vScale = 5.0f;
	m_sceneObjects.push_back(obj);

	// desk legs
	obj.name = "desk_leg_front_left";
	obj.meshType = "box";
	obj.scaleXYZ = glm::vec3(0.3f, 2.5f, 0.3f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-7.85f, 1.25f, 3.65f);
	obj.textureTag = "legs";
	obj.materialTag = "tile";
	obj.colorValue = glm::vec4(0.5f, 0.4f, 0.3f, 1.0f);
	obj.useTexture = true;
	obj.useColor = true;
	obj.uScale = 1.0f;
	obj.vScale = 1.0f;
	m_sceneObjects.push_back(obj);

	obj.name = "desk_leg_front_right";
	obj.positionXYZ = glm::vec3(1.85f, 1.25f, 3.65f);
	m_sceneObjects.push_back(obj);

	obj.name = "desk_leg_back_left";
	obj.positionXYZ = glm::vec3(-7.85f, 1.25f, -0.05f);
	m_sceneObjects.push_back(obj);

	obj.name = "desk_leg_back_right";
	obj.positionXYZ = glm::vec3(1.85f, 1.25f, -0.05f);
	m_sceneObjects.push_back(obj);

	// lamp base
	obj.name = "lamp_base";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.8f, 0.1f, 0.8f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-6.0f, 2.8f, 1.8f);
	obj.textureTag = "";
	obj.materialTag = "lamp";
	obj.colorValue = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	obj.uScale = 1.0f;
	obj.vScale = 1.0f;
	m_sceneObjects.push_back(obj);

	// lamp arm bottom
	obj.name = "lamp_arm_bottom";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.1f, 1.5f, 0.1f);
	obj.xRotationDegrees = -45.0f;
	obj.yRotationDegrees = 30.0f;
	obj.zRotationDegrees = 45.0f;
	obj.positionXYZ = glm::vec3(-6.0f, 2.85f, 1.8f);
	m_sceneObjects.push_back(obj);

	// lamp joint
	obj.name = "lamp_joint_1";
	obj.meshType = "sphere";
	obj.scaleXYZ = glm::vec3(0.2f, 0.2f, 0.2f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-6.9f, 4.0f, 1.4f);
	m_sceneObjects.push_back(obj);

	// lamp top arm
	obj.name = "lamp_arm_top";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.1f, 1.0f, 0.1f);
	obj.xRotationDegrees = 45.0f;
	obj.yRotationDegrees = -30.0f;
	obj.zRotationDegrees = -45.0f;
	obj.positionXYZ = glm::vec3(-6.9f, 4.0f, 1.4f);
	m_sceneObjects.push_back(obj);

	// lamp head joint
	obj.name = "lamp_joint_2";
	obj.meshType = "sphere";
	obj.scaleXYZ = glm::vec3(0.2f, 0.2f, 0.2f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-6.3f, 4.3f, 2.2f);
	m_sceneObjects.push_back(obj);

	// lamp neck
	obj.name = "lamp_neck";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.1f, 0.6f, 0.1f);
	obj.xRotationDegrees = 40.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = -25.0f;
	obj.positionXYZ = glm::vec3(-6.3f, 4.3f, 2.2f);
	m_sceneObjects.push_back(obj);

	// lamp head
	obj.name = "lamp_head";
	obj.meshType = "cone";
	obj.scaleXYZ = glm::vec3(0.6f, 0.6f, 0.6f);
	obj.xRotationDegrees = 100.0f;
	obj.yRotationDegrees = 130.0f;
	obj.zRotationDegrees = 230.0f;
	obj.positionXYZ = glm::vec3(-5.85f, 4.30f, 2.75f);
	m_sceneObjects.push_back(obj);

	// lamp bulb
	obj.name = "lamp_bulb";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.2f, 0.25f, 0.2f);
	obj.xRotationDegrees = 100.0f;
	obj.yRotationDegrees = 130.0f;
	obj.zRotationDegrees = 230.0f;
	obj.positionXYZ = glm::vec3(-5.80f, 4.35f, 2.80f);
	obj.textureTag = "";
	obj.materialTag = "";
	obj.colorValue = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	obj.uScale = 1.0f;
	obj.vScale = 1.0f;
	m_sceneObjects.push_back(obj);

	// back wall
	obj.groupTag = "environment";
	obj.name = "back_wall";
	obj.meshType = "box";
	obj.scaleXYZ = glm::vec3(20.0f, 10.0f, 0.3f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(0.0f, 5.0f, -1.5f);
	obj.textureTag = "wall";
	obj.materialTag = "wall";
	obj.colorValue = glm::vec4(1.0f);
	obj.useTexture = true;
	obj.useColor = false;
	obj.uScale = 3.0f;
	obj.vScale = 3.0f;
	m_sceneObjects.push_back(obj);

	// right wall
	obj.name = "right_wall";
	obj.meshType = "box";
	obj.scaleXYZ = glm::vec3(10.0f, 10.0f, 0.3f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 90.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(10.0f, 5.0f, 3.0f);
	m_sceneObjects.push_back(obj);

	// left wall
	obj.name = "left_wall";
	obj.positionXYZ = glm::vec3(-10.0f, 5.0f, 3.0f);
	m_sceneObjects.push_back(obj);

	// books
	obj.groupTag = "furniture";
	obj.name = "book_1";
	obj.meshType = "box";
	obj.scaleXYZ = glm::vec3(1.0f, 0.2f, 0.6f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-5.9f, 2.8f, 3.05f);
	obj.textureTag = "";
	obj.materialTag = "wood";
	obj.colorValue = glm::vec4(0.1f, 0.2f, 0.6f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	obj.uScale = 1.0f;
	obj.vScale = 1.0f;
	m_sceneObjects.push_back(obj);

	obj.name = "book_2";
	obj.positionXYZ = glm::vec3(-5.9f, 3.02f, 3.05f);
	obj.colorValue = glm::vec4(0.6f, 0.1f, 0.1f, 1.0f);
	m_sceneObjects.push_back(obj);

	obj.name = "book_3";
	obj.positionXYZ = glm::vec3(-5.9f, 3.24f, 3.05f);
	obj.colorValue = glm::vec4(0.95f, 0.95f, 0.95f, 1.0f);
	m_sceneObjects.push_back(obj);

	obj.name = "book_4";
	obj.positionXYZ = glm::vec3(-5.9f, 3.46f, 3.05f);
	obj.colorValue = glm::vec4(0.85f, 0.75f, 0.6f, 1.0f);
	m_sceneObjects.push_back(obj);

	// mug body
	obj.groupTag = "props";
	obj.name = "mug_body";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.25f, 0.4f, 0.25f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-4.4f, 2.8f, 2.65f);
	obj.textureTag = "";
	obj.materialTag = "clay";
	obj.colorValue = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	obj.uScale = 1.0f;
	obj.vScale = 1.0f;
	m_sceneObjects.push_back(obj);

	// mug handle
	obj.name = "mug_handle";
	obj.meshType = "torus";
	obj.scaleXYZ = glm::vec3(0.1f, 0.1f, 0.1f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 90.0f;
	obj.positionXYZ = glm::vec3(-4.15f, 2.95f, 2.65f);
	obj.textureTag = "";
	obj.materialTag = "clay";
	obj.colorValue = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	m_sceneObjects.push_back(obj);

	// mug top / coffee
	obj.name = "mug_top";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.24f, 0.02f, 0.24f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-4.4f, 3.185f, 2.65f);
	obj.textureTag = "";
	obj.materialTag = "clay";
	obj.colorValue = glm::vec4(0.75f, 0.6f, 0.4f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	m_sceneObjects.push_back(obj);

	// pencil cup
	obj.name = "pencil_cup";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.15f, 0.3f, 0.15f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-4.4f, 2.8f, 1.85f);
	obj.textureTag = "";
	obj.materialTag = "clay";
	obj.colorValue = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	m_sceneObjects.push_back(obj);

	// pencil 1 body
	obj.name = "pencil_1_body";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.03f, 0.4f, 0.03f);
	obj.xRotationDegrees = -10.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-4.47f, 3.1f, 1.85f);
	obj.textureTag = "";
	obj.materialTag = "clay";
	obj.colorValue = glm::vec4(0.9f, 0.6f, 0.2f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	m_sceneObjects.push_back(obj);

	// pencil 1 tip
	obj.name = "pencil_1_tip";
	obj.meshType = "cone";
	obj.scaleXYZ = glm::vec3(0.03f, 0.1f, 0.03f);
	obj.xRotationDegrees = -10.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-4.47f, 3.5f, 1.78f);
	m_sceneObjects.push_back(obj);

	// pencil 2 body
	obj.name = "pencil_2_body";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.03f, 0.4f, 0.03f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-4.4f, 3.1f, 1.85f);
	m_sceneObjects.push_back(obj);

	// pencil 2 tip
	obj.name = "pencil_2_tip";
	obj.meshType = "cone";
	obj.scaleXYZ = glm::vec3(0.03f, 0.1f, 0.03f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-4.4f, 3.5f, 1.85f);
	m_sceneObjects.push_back(obj);

	// pencil 3 body
	obj.name = "pencil_3_body";
	obj.meshType = "cylinder";
	obj.scaleXYZ = glm::vec3(0.03f, 0.4f, 0.03f);
	obj.xRotationDegrees = 10.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-4.33f, 3.1f, 1.85f);
	m_sceneObjects.push_back(obj);

	// pencil 3 tip
	obj.name = "pencil_3_tip";
	obj.meshType = "cone";
	obj.scaleXYZ = glm::vec3(0.03f, 0.1f, 0.03f);
	obj.xRotationDegrees = 10.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-4.33f, 3.5f, 1.92f);
	m_sceneObjects.push_back(obj);

	// laptop base
	obj.name = "laptop_base";
	obj.meshType = "box";
	obj.scaleXYZ = glm::vec3(1.5f, 0.1f, 1.0f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-2.5f, 2.8f, 2.0f);
	obj.textureTag = "";
	obj.materialTag = "glass";
	obj.colorValue = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	m_sceneObjects.push_back(obj);

	// laptop screen
	obj.name = "laptop_screen";
	obj.meshType = "box";
	obj.scaleXYZ = glm::vec3(1.5f, 1.0f, 0.05f);
	obj.xRotationDegrees = -10.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-2.5f, 3.3f, 1.4f);
	obj.textureTag = "";
	obj.materialTag = "glass";
	obj.colorValue = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	m_sceneObjects.push_back(obj);

	// laptop screen lighting
	obj.name = "laptop_screen_light";
	obj.meshType = "box";
	obj.scaleXYZ = glm::vec3(1.4f, 0.9f, 0.01f);
	obj.xRotationDegrees = -10.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-2.5f, 3.3f, 1.43f);
	obj.textureTag = "";
	obj.materialTag = "glass";
	obj.colorValue = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	m_sceneObjects.push_back(obj);

	// keyboard
	obj.name = "keyboard";
	obj.meshType = "box";
	obj.scaleXYZ = glm::vec3(1.4f, 0.01f, 0.46f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-2.5f, 2.86f, 1.8f);
	obj.textureTag = "";
	obj.materialTag = "glass";
	obj.colorValue = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	m_sceneObjects.push_back(obj);

	// touchpad
	obj.name = "touchpad";
	obj.meshType = "box";
	obj.scaleXYZ = glm::vec3(0.5f, 0.01f, 0.3f);
	obj.xRotationDegrees = 0.0f;
	obj.yRotationDegrees = 0.0f;
	obj.zRotationDegrees = 0.0f;
	obj.positionXYZ = glm::vec3(-2.5f, 2.86f, 2.3f);
	obj.textureTag = "";
	obj.materialTag = "glass";
	obj.colorValue = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	obj.useTexture = false;
	obj.useColor = true;
	m_sceneObjects.push_back(obj);

	std::cout << "Environment objects: " << GetObjectCountByGroup("environment") << std::endl;
	std::cout << "Furniture objects: " << GetObjectCountByGroup("furniture") << std::endl;
	std::cout << "Props objects: " << GetObjectCountByGroup("props") << std::endl;
	BuildGroupedSceneObjects();
}
void SceneManager::BuildGroupedSceneObjects()
{
	m_groupedSceneObjects.clear();

	for (int i = 0; i < m_sceneObjects.size(); i++)
	{
		m_groupedSceneObjects[m_sceneObjects[i].groupTag].push_back(m_sceneObjects[i]);
	}
}
/***********************************************************
 *  RenderSingleObject()
 *  
 *  This method is used for rendering one scene object from
 *  the centralized object container.
 ***********************************************************/
void SceneManager::RenderSingleObject(const SCENE_OBJECT& sceneObject)
{
	SetTransformations(
		sceneObject.scaleXYZ,
		sceneObject.xRotationDegrees,
		sceneObject.yRotationDegrees,
		sceneObject.zRotationDegrees,
		sceneObject.positionXYZ);

	if (sceneObject.useColor)
	{
		SetShaderColor(
			sceneObject.colorValue.r,
			sceneObject.colorValue.g,
			sceneObject.colorValue.b,
			sceneObject.colorValue.a);
	}

	if (sceneObject.useTexture)
	{
		SetShaderTexture(sceneObject.textureTag);
	}

	SetShaderMaterial(sceneObject.materialTag);
	SetTextureUVScale(sceneObject.uScale, sceneObject.vScale);

	if (sceneObject.meshType == "plane")
	{
		m_basicMeshes->DrawPlaneMesh();
	}
	else if (sceneObject.meshType == "box")
	{
		m_basicMeshes->DrawBoxMesh();
	}
	else if (sceneObject.meshType == "cylinder")
	{
		m_basicMeshes->DrawCylinderMesh();
	}
	else if (sceneObject.meshType == "sphere")
	{
		m_basicMeshes->DrawSphereMesh();
	}
	else if (sceneObject.meshType == "cone")
	{
		m_basicMeshes->DrawConeMesh();
	}
	else if (sceneObject.meshType == "torus")
	{
		m_basicMeshes->DrawTorusMesh();
	}
}

/***********************************************************
 *  RenderSceneObjects()
 *  
 *  This method is used for rendering all scene objects that
 *  have been defined in the centralized object container.
 ***********************************************************/
void SceneManager::RenderSceneObjects()
{
	for (int i = 0; i < m_sceneObjects.size(); i++)
	{
		RenderSingleObject(m_sceneObjects[i]);
	}
}
int SceneManager::GetObjectCountByGroup(std::string groupTag)
{
	int count = 0;

	for (int i = 0; i < m_sceneObjects.size(); i++)
	{
		if (m_sceneObjects[i].groupTag == groupTag)
		{
			count++;
		}
	}

	return count;
}
void SceneManager::RenderSceneObjectsByGroup(std::string groupTag)
{
	if (m_groupedSceneObjects.find(groupTag) == m_groupedSceneObjects.end())
	{
		return;
	}

	for (int i = 0; i < m_groupedSceneObjects[groupTag].size(); i++)
	{
		RenderSingleObject(m_groupedSceneObjects[groupTag][i]);
	}
}
/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{

	// centralized object-based rendering system
	RenderSceneObjectsByGroup("environment");
	RenderSceneObjectsByGroup("furniture");
	RenderSceneObjectsByGroup("props");
}