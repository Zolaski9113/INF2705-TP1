#include <cstddef>
#include <cstdint>

#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include "happly.h"
#include <imgui/imgui.h>

#include <inf2705/OpenGLApplication.hpp>

#include "model.hpp"
#include "windmill.hpp"

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

struct Position
{
    float x;
    float y;
};
struct Couleur
{
    vec3 coul;
};
struct Sommet
{
    Position pos;
    Couleur couleur;
};

struct App : public OpenGLApplication
{
    App()
        : nSide_(5), oldNSide_(0), cameraPosition_(0.f, 0.f, 0.f), cameraOrientation_(0.f, 0.f), currentScene_(0), isMouseMotionEnabled_(false)
    {
    }

    void init() override
    {
        // Le message expliquant les touches de clavier.
        setKeybindMessage(
            "ESC : quitter l'application."
            "\n"
            "T : changer de scène."
            "\n"
            "W : déplacer la caméra vers l'avant."
            "\n"
            "S : déplacer la caméra vers l'arrière."
            "\n"
            "A : déplacer la caméra vers la gauche."
            "\n"
            "D : déplacer la caméra vers la droite."
            "\n"
            "Q : déplacer la caméra vers le bas."
            "\n"
            "E : déplacer la caméra vers le haut."
            "\n"
            "Flèches : tourner la caméra."
            "\n"
            "Souris : tourner la caméra"
            "\n"
            "Espace : activer/désactiver la souris."
            "\n");

        // Config de base.

        // Initialisation de la couleur de fond.
        glClearColor(0.05f, 0.1f, 0.1f, 1.0f);
        // TODO: Partie 2: Activez le test de profondeur (GL_DEPTH_TEST) et
        //       l'élimination des faces arrières (GL_CULL_FACE).

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        loadShaderPrograms();

        // Partie 1
        initShapeData();

        // Partie 2
        loadModels();

        // TODO: Insérez les initialisations supplémentaires ici au besoin.
    }

    void checkShaderCompilingError(const char *name, GLuint id)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            glDeleteShader(id);
            std::cout << "Shader \"" << name << "\" compile error: " << infoLog << std::endl;
        }
    }

    void checkProgramLinkingError(const char *name, GLuint id)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, 1024, NULL, infoLog);
            glDeleteProgram(id);
            std::cout << "Program \"" << name << "\" linking error: " << infoLog << std::endl;
        }
    }

    // Appelée à chaque trame. Le buffer swap est fait juste après.
    void drawFrame() override
    {
        // Nettoyage de la surface de dessin.
        glClear(GL_COLOR_BUFFER_BIT);
        // TODO: Partie 2: Ajoutez le nettoyage du tampon de profondeur.
        glClear(GL_DEPTH_BUFFER_BIT);

        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);
        ImGui::End();

        switch (currentScene_)
        {
        case 0:
            sceneShape();
            break;
        case 1:
            sceneModels();
            break;
        }
    }

    // Appelée lorsque la fenêtre se ferme.
    void onClose() override
    {
        // TODO: Libérez les ressources allouées (buffers, shaders, etc.).
    }

    // Appelée lors d'une touche de clavier.
    void onKeyPress(const sf::Event::KeyPressed &key) override
    {
        using enum sf::Keyboard::Key;
        switch (key.code)
        {
        case Escape:
            window_.close();
            break;
        case Space:
            isMouseMotionEnabled_ = !isMouseMotionEnabled_;
            if (isMouseMotionEnabled_)
            {
                window_.setMouseCursorGrabbed(true);
                window_.setMouseCursorVisible(false);
            }
            else
            {
                window_.setMouseCursorGrabbed(false);
                window_.setMouseCursorVisible(true);
            }
            break;
        case T:
            currentScene_ = ++currentScene_ < N_SCENE_NAMES ? currentScene_ : 0;
            break;
        default:
            break;
        }
    }

    void onResize(const sf::Event::Resized &event) override
    {
    }

    void onMouseMove(const sf::Event::MouseMoved &mouseDelta) override
    {
        if (!isMouseMotionEnabled_)
            return;

        const float MOUSE_SENSITIVITY = 0.1;
        float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
        float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;
    }

    void updateCameraInput()
    {
        if (!window_.hasFocus())
            return;

        if (isMouseMotionEnabled_)
        {
            sf::Vector2u windowSize = window_.getSize();
            sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
            sf::Mouse::setPosition(windowHalfSize, window_);
        }

        float cameraMouvementX = 0;
        float cameraMouvementY = 0;

        const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;

        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;

        // Keyboard input
        glm::vec3 positionOffset = glm::vec3(0.0);
        const float SPEED = 10.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            positionOffset.z -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            positionOffset.z += SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            positionOffset.x -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            positionOffset.x += SPEED;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            positionOffset.y -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
            positionOffset.y += SPEED;

        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
    }

    void loadModels()
    {
        windmill_.loadModels();
        grass_.load("../models/grass.ply");
    }

    GLuint loadShaderObject(GLenum type, const char *path)
    {
        // Créer les objets de shaders.
        // shaderProgram = glCreateProgram(); --> mettre dans une autre méthode
        GLuint shader = glCreateShader(type);

        // Lire et envoyer la source du nuanceur de sommets.
        std::string shaderSource = readFile(path);
        auto src = shaderSource.c_str();
        glShaderSource(shader, 1, &src, nullptr);

        // Compiler et attacher le nuanceur de sommets.
        glCompileShader(shader);
        // Vérifier qu'il n'y a pas d'erreurs de compilations
        checkShaderCompilingError(path, shader);
        //       Utilisez readFile pour lire le fichier.
        //       N'oubliez pas de vérifier les erreurs suite à la compilation
        //       avec la méthode App::checkShaderCompilingError.

        return shader;
    }

    void loadShaderPrograms()
    {
        // Partie 1
        const char *BASIC_VERTEX_SRC_PATH = "./shaders/basic.vs.glsl";
        const char *BASIC_FRAGMENT_SRC_PATH = "./shaders/basic.fs.glsl";

        // Partie 2
        const char *TRANSFORM_VERTEX_SRC_PATH = "./shaders/transform.vs.glsl";
        const char *TRANSFORM_FRAGMENT_SRC_PATH = "./shaders/transform.fs.glsl";

        basicSP_ = glCreateProgram();
        GLuint vs = loadShaderObject(GL_VERTEX_SHADER, BASIC_VERTEX_SRC_PATH);
        GLuint fs = loadShaderObject(GL_FRAGMENT_SHADER, BASIC_FRAGMENT_SRC_PATH);

        glAttachShader(basicSP_, vs);
        glAttachShader(basicSP_, fs);

        glLinkProgram(basicSP_);

        glDetachShader(basicSP_, vs);
        glDetachShader(basicSP_, fs);

        glDeleteShader(vs);
        glDeleteShader(fs);

        checkProgramLinkingError("basic", basicSP_);

        // TODO: Allez chercher les locations de vos variables uniform dans le shader
        //       pour initialiser mvpUniformLocation_ et windmill_.mvpUniformLocation.
    }

    void generateNgon()
    {
        const float RADIUS = 0.7f;
        float theta, r, g, b, x, y;
        vertices_[0] = Sommet{ {0.0f, 0.0f}, glm::vec3(1.00f, 1.00f, 1.00f) };

        for (int i = 0; i < nSide_; i++)
        {
            theta = glm::half_pi<float>() + glm::two_pi<float>() * i / nSide_;
            float t = (float)i / nSide_;

            x = RADIUS * std::cos(theta);
            y = RADIUS * std::sin(theta);
            r = 0.5f + 0.5f * std::cos(glm::two_pi<float>() * (t));
            g = 0.5f + 0.5f * std::cos(glm::two_pi<float>() * (t - 1.0f / 3));
            b = 0.5f + 0.5f * std::cos(glm::two_pi<float>() * (t - 2.0f / 3));

            vertices_[i + 1] = Sommet{{x, y}, glm::vec3(r, g, b)};

            elements_[3 * i] = 0;
            elements_[3 * i + 1] = i + 1;
            elements_[3 * i + 2] = (i + 1) % nSide_ + 1;
        }
    }

    void initShapeData()
    {
        // Allocation du vbo avec taille maximale, pas de données cuz dynamic draw
        glGenBuffers(1, &vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), nullptr, GL_DYNAMIC_DRAW);

        // Pareil ebo pour les indices
        glGenBuffers(1, &ebo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements_), nullptr, GL_DYNAMIC_DRAW);

        // liaison du eao
        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);

        // Format des données : les attributs sont entrelacés, donc le saut d'un
        // sommet au suivant vaut sizeof(Sommet) pour les deux.
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Sommet),
                              (const void *)offsetof(Sommet, pos));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Sommet),
                              (const void *)offsetof(Sommet, couleur));

        // La liaison du ebo fait partie de l'état du vao.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

        // Délier le vao en premier
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void sceneShape()
    {
        ImGui::Begin("Scene Parameters");
        ImGui::SliderInt("Sides", &nSide_, MIN_N_SIDES, MAX_N_SIDES);
        ImGui::End();

        bool hasNumberOfSidesChanged = nSide_ != oldNSide_;
        if (hasNumberOfSidesChanged)
        {
            oldNSide_ = nSide_;
            generateNgon();

            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            glBufferSubData(GL_ARRAY_BUFFER, 0, (nSide_ + 1) * sizeof(Sommet), vertices_);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 3 * nSide_ * sizeof(GLuint), elements_);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        glUseProgram(basicSP_);
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, 3 * nSide_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void drawGround(glm::mat4 &projView)
    {
        // TODO: Dessin du sol.
        //
        //       Ici, le modèle original est un carré de 1 unité.
        //
        //       Le gazon a une mise à l'échelle pour être long de 50
        //       unités et large de 50. Celui-ci doit aussi être légèrement
        //       baisé de 0.1.
    }

    glm::mat4 getViewMatrix()
    {
        // TODO: Calculer la matrice de vue.
        //
        //       Vous n'avez pas le droit d'utiliser de fonction lookAt ou
        //       d'inversion de matrice. À la place, procéder en inversant
        //       les opérations. N'oubliez pas que cette matrice est appliquée
        //       aux éléments de la scène. Au lieu de déplacer la caméra 10
        //       unités vers la gauche, on déplace le monde 10 unités vers la
        //       droite, ce qui donne le même résultat final.
        //
        //       La caméra est placée à la position cameraPosition et orientée
        //       par les angles cameraOrientation (en radian).

        return glm::mat4(1.0);
    }

    glm::mat4 getPerspectiveProjectionMatrix()
    {
        // TODO: Calculer la matrice de projection.
        //
        //       Celle-ci aura un fov de 70 degrés, un near à 0.1 et un far à 300.
        //

        // getWindowAspect();

        return glm::mat4(1.0);
    }

    void sceneModels()
    {
        ImGui::Begin("Scene Parameters");
        ImGui::SliderFloat("Wind Speed", &windmill_.windSpeed, 0.0f, 20.0f, "%.2f m/s");
        ImGui::SliderFloat("Wind Angle", &windmill_.windAngle, -M_PI, M_PI, "%.2f°");
        ImGui::End();

        updateCameraInput();
        windmill_.update(deltaTime_);

        // TODO: Dessin de la totalité de la scène graphique.
        //       On devrait voir le gazon et le moulin.
        //       Le moulin est contrôlable avec l'interface graphique.
    }

private:
    // Shaders
    GLuint basicSP_;
    GLuint transformSP_;
    GLuint mvpUniformLocation_;

    // Partie 1
    GLuint vbo_, ebo_, vao_;

    static constexpr unsigned int MIN_N_SIDES = 5;
    static constexpr unsigned int MAX_N_SIDES = 12;

    Sommet vertices_[MAX_N_SIDES + 1];
    GLuint elements_[MAX_N_SIDES * 3];

    int nSide_, oldNSide_;

    // Partie 2
    Model grass_;

    Windmill windmill_;

    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;

    // Imgui var
    const char *const SCENE_NAMES[2] = {
        "Introduction",
        "3D Model & transformation",
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;

    bool isMouseMotionEnabled_;
};

int main(int argc, char *argv[])
{
    WindowSettings settings = {};
    settings.fps = 60;
    settings.context.depthBits = 24;
    settings.context.stencilBits = 8;
    settings.context.antiAliasingLevel = 4;
    settings.context.majorVersion = 3;
    settings.context.minorVersion = 3;
    settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

    App app;
    app.run(argc, argv, "Tp1", settings);
}