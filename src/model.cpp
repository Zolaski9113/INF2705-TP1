#include "model.hpp"

#include "happly.h"

using namespace gl;

// Format entrelacé : chaque sommet regroupe sa position et sa couleur.
struct Position
{
    float x, y, z;
};
struct Couleur
{
    float r, g, b;
};

struct Vertex
{
    Position pos;
    Couleur couleur;
};

void Model::load(const char *path)
{
    // Chargement des données du fichier .ply.
    // Ne modifier pas cette partie.
    happly::PLYData plyIn(path);

    happly::Element &vertex = plyIn.getElement("vertex");
    std::vector<float> positionX = vertex.getProperty<float>("x");
    std::vector<float> positionY = vertex.getProperty<float>("y");
    std::vector<float> positionZ = vertex.getProperty<float>("z");

    std::vector<unsigned char> colorRed = vertex.getProperty<unsigned char>("red");
    std::vector<unsigned char> colorGreen = vertex.getProperty<unsigned char>("green");
    std::vector<unsigned char> colorBlue = vertex.getProperty<unsigned char>("blue");

    // Tableau de faces, une face est un tableau d'indices.
    // Les faces sont toutes des triangles dans nos modèles (donc 3 indices par face).
    std::vector<std::vector<unsigned int>> facesIndices = plyIn.getFaceIndices<unsigned int>();

    // Regroupement des propriétés du .ply en sommets entrelacés
    std::vector<Vertex> vertices(positionX.size());
    for (size_t i = 0; i < vertices.size(); i++)
    {
        vertices[i].pos = {positionX[i], positionY[i], positionZ[i]};
        vertices[i].couleur = {colorRed[i] / 255.0f,
                               colorGreen[i] / 255.0f,
                               colorBlue[i] / 255.0f};
    }

    // Aplatissement des faces en un seul tableau contigu d'indices
    std::vector<GLuint> indices;
    indices.reserve(facesIndices.size() * 3);
    for (const auto &face : facesIndices)
        for (unsigned int idx : face)
            indices.push_back(idx);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                 indices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void *)offsetof(Vertex, couleur));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    // Délier le vao en premier
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    count_ = static_cast<GLsizei>(indices.size());
}

Model::~Model()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
}

void Model::draw()
{
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
