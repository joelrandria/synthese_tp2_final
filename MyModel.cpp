#include "MyModel.h"

#include "MeshIO.h"

#include "GL/GLBuffer.h"
#include "GL/GLVertexArray.h"

#include <stdio.h>

#define VERTEX_BYTESIZE	(9 * sizeof(GLfloat))

std::vector<gk::GLVertexArray*> MyModel::_vaos;

void MyModel::print() const
{
  printf("Modèle '%s': index count = %d, index offset = %d, vertex offset = %d\r\n",
	 _name.c_str(),
	 _indexCount,
	 _indexOffset,
	 _vertexOffset);
}

void MyModel::setPosition(const gk::Point& position)
{
  _position = position;

  updateTransforms();
}
void MyModel::updateTransforms()
{
  _modelToWorldTransform = gk::Transform(gk::Matrix4x4(1, 0, 0, _position.x,
						       0, 1, 0, _position.y,
						       0, 0, 1, _position.z,
						       0, 0, 0, 1));
}

std::vector<MyModel*> MyModel::loadSharedVertexArrayModels(const std::vector<gk::Mesh*>& meshes)
{
  uint i;
  uint j;
  int vertexCount;

  gk::Mesh* mesh;

  MyModel* model;
  std::vector<MyModel*> models;

  std::vector<unsigned int> indices;
  std::vector<gk::Vec3> vertexAttributes;

  gk::GLVertexArray* vao;

  vertexCount = 0;

  vao = gk::createVertexArray();
  _vaos.push_back(vao);

  for (i = 0; i < meshes.size(); ++i)
  {
    mesh = meshes[i];

    for (j = 0; j < mesh->positions.size(); ++j)
    {
      ++vertexCount;

      vertexAttributes.push_back(mesh->positions[j]);
      vertexAttributes.push_back(mesh->normals[j]);
      vertexAttributes.push_back(mesh->texcoords[j]);
    }

    for (j = 0; j < mesh->indices.size(); ++j)
      indices.push_back(mesh->indices[j]);

    model = new MyModel();
    model->_vao = vao;
    model->_name = mesh->filename;
    model->_indexCount = mesh->indices.size();
    model->_indexOffset = indices.size() - mesh->indices.size();
    model->_vertexOffset = vertexCount - mesh->positions.size();

    models.push_back(model);
  }

  gk::GLBuffer* vertexAttributeBuffer = gk::createBuffer(GL_ARRAY_BUFFER, vertexAttributes);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTESIZE, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTESIZE, (GLvoid*)(3 * sizeof(GLfloat)));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTESIZE, (GLvoid*)(6 * sizeof(GLfloat)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  gk::GLBuffer* indexBuffer = gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

  delete vertexAttributeBuffer;
  delete indexBuffer;

  return models;
}
std::vector<MyModel*> MyModel::loadSharedVertexArrayModels(const std::vector<std::string>& filenames)
{
  uint i;
  gk::Mesh* mesh;
  std::vector<MyModel*> models;
  std::vector<gk::Mesh*> meshes;

  for (i = 0; i < filenames.size(); ++i)
  {
    mesh = gk::MeshIO::readOBJ(filenames[i]);
    if (mesh == 0)
      continue;

    meshes.push_back(mesh);
  }

  models = loadSharedVertexArrayModels(meshes);

  for (i = 0; i < meshes.size(); ++i)
    delete meshes[i];

  return models;
}
