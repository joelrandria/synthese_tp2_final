#include "MyModelFactory.h"

#include "Image.h"
#include "ImageIO.h"
#include "Mesh.h"
#include "MeshIO.h"

#include "MyModel.h"

#include <stdio.h>

#define VERTEX_BYTESIZE			(9 * sizeof(GLfloat))
#define SHARED_VERTEX_BUFFER_BYTESIZE	16 * 1024 * 1024
#define SHARED_INDEX_BUFFER_BYTESIZE	64 * 1024 * 1024

int MyModelFactory::_totalVertexCount = 0;
GLuint MyModelFactory::_sharedVertexBuffer = 0;

int MyModelFactory::_totalIndexCount = 0;
GLuint MyModelFactory::_sharedIndexBuffer = 0;

std::map<std::string, MyMeshGpuLocation> MyModelFactory::_meshGpuLocations;
std::map<std::string, GLuint> MyModelFactory::_textures;

void MyModelFactory::bindSharedBuffers(bool bind)
{
  if (MyModel::_sharedVertexArray == 0)
  {
    glGenVertexArrays(1, &MyModel::_sharedVertexArray);
    glBindVertexArray(MyModel::_sharedVertexArray);

    glGenBuffers(1, &_sharedVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _sharedVertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, SHARED_VERTEX_BUFFER_BYTESIZE, 0, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTESIZE, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTESIZE, (GLvoid*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTESIZE, (GLvoid*)(6 * sizeof(GLfloat)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &_sharedIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _sharedIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, SHARED_INDEX_BUFFER_BYTESIZE, 0, GL_STATIC_DRAW);

    glBindVertexArray(0);
  }

  glBindBuffer(GL_ARRAY_BUFFER, bind ? _sharedVertexBuffer : 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bind ? _sharedIndexBuffer : 0);
}

MyModel* MyModelFactory::createModel(const std::string& meshFilename, const std::string& diffuseTextureFilename)
{
  MyModel* model;

  model = new MyModel();
  model->_name = meshFilename;
  model->_meshGpuLocation = getMeshGpuLocation(meshFilename);
  model->_diffuseTexture = getTexture(diffuseTextureFilename);

  return model;
}

MyMeshGpuLocation MyModelFactory::getMeshGpuLocation(const std::string& filename)
{
  gk::Mesh* mesh;

  MeshGpuLocationMap::iterator it;
  MyMeshGpuLocation meshGpuLocation;

  it = _meshGpuLocations.find(filename);
  if (it != _meshGpuLocations.end())
    return (it->second);

  mesh = gk::MeshIO::readOBJ(filename);
  if (mesh == 0)
  {
    fprintf(stderr, "MyModelFactory::getMeshGpuLocation(): Impossible de charger le mesh '%s'\r\n", filename.c_str());
    exit(-1);
  }

  meshGpuLocation = getMeshGpuLocation(mesh);
  _meshGpuLocations[filename] = meshGpuLocation;

  delete mesh;

  return meshGpuLocation;
}
MyMeshGpuLocation MyModelFactory::getMeshGpuLocation(gk::Mesh* mesh)
{
  uint i;

  int meshVertexCount;
  std::vector<gk::Vec3> meshVertices;

  MyMeshGpuLocation meshGpuLocation;

  meshVertexCount = 0;

  for (i = 0; i < mesh->positions.size(); ++i)
  {
    ++meshVertexCount;

    meshVertices.push_back(mesh->positions[i]);
    meshVertices.push_back(mesh->normals[i]);
    meshVertices.push_back(mesh->texcoords[i]);
  }

  meshGpuLocation.indexCount = mesh->indices.size();
  meshGpuLocation.indexOffset = _totalIndexCount;
  meshGpuLocation.vertexOffset = _totalVertexCount;

  bindSharedBuffers(true);

  glBufferSubData(GL_ARRAY_BUFFER, _totalVertexCount * VERTEX_BYTESIZE, meshVertexCount * VERTEX_BYTESIZE, &meshVertices.front());
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, _totalIndexCount * sizeof(GLuint), mesh->indices.size() * sizeof(GLuint), &mesh->indices.front());

  bindSharedBuffers(false);

  _totalVertexCount +=  mesh->positions.size();
  _totalIndexCount += mesh->indices.size();

  return meshGpuLocation;
}

GLuint MyModelFactory::getTexture(const std::string& filename)
{
  GLuint texture;
  TextureMap::iterator it;

  texture = 0;

  it = _textures.find(filename);
  if (it != _textures.end())
    return it->second;

  gk::Image* image = gk::ImageIO::readImage(filename);
  if (image == 0)
  {
    fprintf(stderr, "MyModelFactory::getTexture(): Impossible de charger la texture '%s'\r\n", filename.c_str());
    exit(-1);
  }

  texture = getTexture(image);
  _textures[filename] = texture;

  delete image;

  return texture;
}
GLuint MyModelFactory::getTexture(gk::Image* image)
{
  GLuint texture;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  return texture;
}
