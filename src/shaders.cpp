#include "shaders.hpp"

#include <glm/gtc/type_ptr.hpp>

void EdgeEffect::load()
{
    const char *VERTEX_SRC_PATH = "./shaders/edge.vs.glsl";
    const char *FRAGMENT_SRC_PATH = "./shaders/edge.fs.glsl";

    name_ = "EdgeEffect";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void EdgeEffect::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
}

void Sky::load()
{
    const char *VERTEX_SRC_PATH = "./shaders/sky.vs.glsl";
    const char *FRAGMENT_SRC_PATH = "./shaders/sky.fs.glsl";

    name_ = "Sky";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void Sky::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp"); // initialisation des valeurs de uniform en attributs
}

void Bezier::load()
{
    const char *VERTEX_SRC_PATH = "./shaders/bezier.vs.glsl";
    const char *FRAGMENT_SRC_PATH = "./shaders/bezier.fs.glsl";

    name_ = "Bezier";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void Bezier::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp"); // initialisation des valeurs de uniform en attributs
}

void PhongShading::load()
{
    const char *VERTEX_SRC_PATH = "./shaders/phong.vs.glsl";
    const char *FRAGMENT_SRC_PATH = "./shaders/phong.fs.glsl";

    name_ = "PhongShading";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void PhongShading::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
    viewULoc = glGetUniformLocation(id_, "view");
    modelViewULoc = glGetUniformLocation(id_, "modelView");
    normalULoc = glGetUniformLocation(id_, "normalMatrix");

    nSpotLightsULoc = glGetUniformLocation(id_, "nSpotLights");

    globalAmbientULoc = glGetUniformLocation(id_, "globalAmbient");
}

void PhongShading::assignAllUniformBlockIndexes()
{
    setUniformBlockBinding("MaterialBlock", 0);
    setUniformBlockBinding("LightingBlock", 1);
}

void PhongShading::setMatrices(glm::mat4 &mvp, glm::mat4 &view, glm::mat4 &model)
{
    glm::mat4 modelView = view * model;

    glUniformMatrix4fv(viewULoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(mvpULoc, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(modelViewULoc, 1, GL_FALSE, &modelView[0][0]);
    glUniformMatrix3fv(normalULoc, 1, GL_TRUE, glm::value_ptr(glm::inverse(glm::mat3(modelView))));
}

void Grass::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/grass.vs.glsl";
    const char* TESS_CTRL_SRC_PATH = "./shaders/grass.tcs.glsl";
    const char* TESS_EVAL_SRC_PATH = "./shaders/grass.tes.glsl";
    const char* GEOMETRY_SRC_PATH = "./shaders/grass.gs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/grass.fs.glsl";

    name_ = "Grass";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_TESS_CONTROL_SHADER, TESS_CTRL_SRC_PATH);
    loadShaderSource(GL_TESS_EVALUATION_SHADER, TESS_EVAL_SRC_PATH);
    loadShaderSource(GL_GEOMETRY_SHADER, GEOMETRY_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void Grass::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
    modelViewULoc = glGetUniformLocation(id_, "modelView");
}