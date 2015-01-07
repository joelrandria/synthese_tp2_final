#include "MyModelFactory.h"

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

MyModel* MyModelFactory::createModel(gk::Mesh* mesh)
{
  uint i;

  MyModel* model;
  int modelVertexCount;

  std::vector<gk::Vec3> modelVertices;

  modelVertexCount = 0;

  for (i = 0; i < mesh->positions.size(); ++i)
  {
    ++modelVertexCount;

    modelVertices.push_back(mesh->positions[i]);
    modelVertices.push_back(mesh->normals[i]);
    modelVertices.push_back(mesh->texcoords[i]);
  }

  model = new MyModel();
  model->_name = mesh->filename;
  model->_indexCount = mesh->indices.size();
  model->_indexOffset = _totalIndexCount;
  model->_vertexOffset = _totalVertexCount;

  bindSharedBuffers(true);

  glBufferSubData(GL_ARRAY_BUFFER,
		  _totalVertexCount * VERTEX_BYTESIZE,
		  modelVertexCount * VERTEX_BYTESIZE,
		  &modelVertices.front());

  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
		  _totalIndexCount * sizeof(GLuint),
		  mesh->indices.size() * sizeof(GLuint),
		  &mesh->indices.front());

  bindSharedBuffers(false);

  _totalVertexCount +=  mesh->positions.size();
  _totalIndexCount += mesh->indices.size();

  return model;
}
MyModel* MyModelFactory::createModel(const std::string& filename)
{
  MyModel* model;
  gk::Mesh* mesh;

  mesh = gk::MeshIO::readOBJ(filename);
  if (mesh == 0)
  {
    fprintf(stderr, "MyModelFactory::createModel(): Impossible de charger le mesh '%s'\r\n", filename.c_str());
    exit(-1);
  }

  model = createModel(mesh);

  delete mesh;

  return model;
}
