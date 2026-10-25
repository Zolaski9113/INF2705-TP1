#pragma once
#include "shader_program.hpp"

#include <glm/glm.hpp>

class EdgeEffect : public ShaderProgram
{
public:
    GLuint mvpULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};

class Sky : public ShaderProgram
{
public:
    GLuint mvpULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};

class PhongShading : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint viewULoc;
    GLuint modelViewULoc;
    GLuint normalULoc;

    GLuint nSpotLightsULoc;

    GLuint globalAmbientULoc;

public:
    void setMatrices(glm::mat4 &mvp, glm::mat4 &view, glm::mat4 &model);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
    virtual void assignAllUniformBlockIndexes() override;
};
