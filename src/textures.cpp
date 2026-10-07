#include "textures.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

Texture2D::Texture2D()
    : m_id(0)
{
}

void Texture2D::load(const char *path)
{
    int width, height, nChannels;
    stbi_set_flip_vertically_on_load(true);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    unsigned char *data = stbi_load(path, &width, &height, &nChannels, 0);
    if (data == NULL)
    {
        std::cout << "Error loading texture \"" << path << "\": " << stbi_failure_reason() << std::endl;
        return;
    }

    glGenTextures(1, &m_id);            // Référence de la texture (son ID)
    glBindTexture(GL_TEXTURE_2D, m_id); // L'objet est créé et bindé en mémoire

    GLenum format = (nChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data);
    stbi_image_free(data);
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &m_id); // Libère les resources de textures
}

void Texture2D::setFiltering(GLenum filteringMode)
{
    glBindTexture(GL_TEXTURE_2D, m_id);                                   // FACULTATIF, mais fait en sorte que la méthode marche peu importe l'ordre des textures liées
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filteringMode); // Configure le MIN ET LE MAG en fonction
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filteringMode); // en soit nearest, soit linear
}

void Texture2D::setWrap(GLenum wrapMode)
{
    glBindTexture(GL_TEXTURE_2D, m_id); // FACULTATIF, mais fait en sorte que la méthode marche peu importe l'ordre des textures liées

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode); // Configure les 2 wrap modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
}

void Texture2D::enableMipmap()
{
    glBindTexture(GL_TEXTURE_2D, m_id); // FACULTATIF, mais fait en sorte que la méthode marche peu importe l'ordre des textures liées

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Utilisé pour le rendu lisse
}

void Texture2D::use()
{
    glActiveTexture(GL_TEXTURE0); // Met la texture active pour être utilisée
    glBindTexture(GL_TEXTURE_2D, m_id);
}

//
// Cubemap
//

TextureCubeMap::TextureCubeMap()
    : m_id(0)
{
}

void TextureCubeMap::load(const char **pathes)
{
    const size_t N_TEXTURES = 6;
    unsigned char *datas[N_TEXTURES];
    int widths[N_TEXTURES];
    int heights[N_TEXTURES];
    int nChannels[N_TEXTURES];
    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < 6; i++)
    {
        datas[i] = stbi_load(pathes[i], &widths[i], &heights[i], &nChannels[i], 0);
        if (datas[i] == NULL)
        {
            std::cout << "Error loading texture \"" << pathes[i] << "\": " << stbi_failure_reason() << std::endl;
            return;
        }
    }
    glGenTextures(1, &m_id);                  // Référence de la texture (son ID)
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id); // L'objet est créé et bindé en mémoire

    for (unsigned int i = 0; i < 6; i++)
    {
        GLenum format = (nChannels[i] == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, format, widths[i], heights[i], 0, format, GL_UNSIGNED_BYTE, datas[i]); // Chargement de la texture
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // toutes les configuraitons d'un coup
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (unsigned int i = 0; i < 6; i++)
    {
        stbi_image_free(datas[i]);
    }
}

TextureCubeMap::~TextureCubeMap()
{
    glDeleteTextures(1, &m_id); // Libère les resources de textures
}

void TextureCubeMap::use()
{
    glActiveTexture(GL_TEXTURE0);             // Met la texture active pour être utilisée
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id); // Bind le cubeMap pour qu'il soit utilisé
}
