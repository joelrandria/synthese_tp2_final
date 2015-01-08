#include "MyModelFactory.h"

#include "Image.h"
#include "ImageIO.h"
#include "Mesh.h"
#include "MeshIO.h"

#include "MyModel.h"

#include <stdio.h>

#define VERTEX_BYTESIZE			9 * sizeof(GLfloat)
#define SHARED_VERTEX_BUFFER_BYTESIZE	16 * 1024 * 1024
#define SHARED_INDEX_BUFFER_BYTESIZE	64 * 1024 * 1024

int MyModelFactory::_totalVertexCount = 0;
GLuint MyModelFactory::_sharedVertexBuffer = 0;

int MyModelFactory::_totalIndexCount = 0;
GLuint MyModelFactory::_sharedIndexBuffer = 0;

std::map<std::string, MyMeshInfo> MyModelFactory::_meshInfos;
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

  setMesh(model, meshFilename);
  setTexture(model, diffuseTextureFilename);

  return model;
}

void MyModelFactory::setMesh(MyModel* model, gk::Mesh* mesh)
{
  uint i;

  int meshVertexCount;
  std::vector<gk::Vec3> meshVertices;

  MyMeshInfo meshInfo;

  meshVertexCount = 0;

  for (i = 0; i < mesh->positions.size(); ++i)
  {
    ++meshVertexCount;

    meshVertices.push_back(mesh->positions[i]);
    meshVertices.push_back(mesh->normals[i]);
    meshVertices.push_back(mesh->texcoords[i]);
  }

  meshInfo.gpuIndexCount = mesh->indices.size();
  meshInfo.gpuIndexOffset = _totalIndexCount;
  meshInfo.gpuVertexOffset = _totalVertexCount;

  bindSharedBuffers(true);

  glBufferSubData(GL_ARRAY_BUFFER, _totalVertexCount * VERTEX_BYTESIZE, meshVertexCount * VERTEX_BYTESIZE, &meshVertices.front());
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, _totalIndexCount * sizeof(GLuint), mesh->indices.size() * sizeof(GLuint), &mesh->indices.front());

  bindSharedBuffers(false);

  _totalVertexCount +=  mesh->positions.size();
  _totalIndexCount += mesh->indices.size();

  if (!mesh->filename.empty())
    _meshInfos[mesh->filename] = meshInfo;

  model->_meshInfo = meshInfo;
}
void MyModelFactory::setMesh(MyModel* model, const std::string& filename)
{
  gk::Mesh* mesh;

  MeshInfoMap::iterator it;
  MyMeshInfo meshInfo;

  it = _meshInfos.find(filename);
  if (it != _meshInfos.end())
  {
    model->_meshInfo = it->second;
  }
  else
  {
    mesh = gk::MeshIO::readOBJ(filename);
    if (mesh == 0)
    {
      fprintf(stderr, "MyModelFactory::getMeshInfo(): Impossible de charger le mesh '%s'\r\n", filename.c_str());
      exit(-1);
    }

    setMesh(model, mesh);

    delete mesh;
  }
}

void MyModelFactory::setTexture(MyModel* model, gk::Image* image)
{
  glGenTextures(1, (GLuint*)&model->_diffuseTexture);
  glBindTexture(GL_TEXTURE_2D, (GLuint)model->_diffuseTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}
void MyModelFactory::setTexture(MyModel* model, const std::string& filename)
{
  gk::Image* image;
  TextureMap::iterator it;

  it = _textures.find(filename);
  if (it != _textures.end())
  {
    model->_diffuseTexture = it->second;
  }
  else
  {
    image = gk::ImageIO::readImage(filename);
    if (image == 0)
    {
      fprintf(stderr, "MyModelFactory::getTexture(): Impossible de charger la texture '%s'\r\n", filename.c_str());
      exit(-1);
    }

    setTexture(model, image);

    _textures[filename] = model->_diffuseTexture;

    delete image;
  }
}
