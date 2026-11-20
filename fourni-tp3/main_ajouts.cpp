
// TODO: Petit ajout à votre main.cpp

// Partie 1
Material bezierMat = 
{
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    1.0f
};

struct BezierCurve
{
    glm::vec3 p0;
    glm::vec3 c0;
    glm::vec3 c1;        
    glm::vec3 p1;
};


BezierCurve curves[5] = 
{
    {
        glm::vec3(-0.509, 2.403, 6.924),
        glm::vec3(2.412, 12.011, 20.396),
        glm::vec3(-23.731, 5.855, 25.199),
        glm::vec3(-25.732, 15.439, 13.561),
    },
    {
        glm::vec3(-25.732, 15.439, 13.561),
        glm::vec3(-27.069, 21.845, 5.781),
        glm::vec3(-4.807, 26.461, -10.716),
        glm::vec3(-7.574, 25.967, -12.861),
    },
    {
        glm::vec3(-7.574, 25.967, -12.861),
        glm::vec3(-21.299, 23.516, -23.495),
        glm::vec3(-18.956, 13.960, -36.813),
        glm::vec3(-3.324, 11.588, -33.743),
    },
    {
        glm::vec3(-3.324, 11.588, -33.743),
        glm::vec3(4.812, 10.354, -32.145),
        glm::vec3(8.408, 22.100, -7.555),
        glm::vec3(23.264, 6.067, -5.370),
    },
    {
        glm::vec3(23.264, 6.067, -5.370),
        glm::vec3(29.367, -0.518, -4.473),
        glm::vec3(2.412, 12.011, 20.396),
        glm::vec3(-0.088, 12.011, 17.896),
    },
};


// Partie 3
// Ne pas modifier
struct Particle
{
    glm::vec3 position;
    GLfloat zOrientation;
    glm::vec4 velocity; // vec3, but padded
    glm::vec4 color;
    glm::vec2 size; 
    GLfloat timeToLive;
    GLfloat maxTimeToLive;
};


App()
: totalTime(0.0)       // Initialiser à 0
, timerParticles_(0.0) // Initialiser à 0
, nParticles_(0)       // Initialiser à 0
// ...
{
}

void init() override
{
    // TODO: Création des nouveaux shaders
    
    // Partie 1-2
    // TODO: Initialisation des meshes (béziers, patches)


    glEnable(GL_PROGRAM_POINT_SIZE); // pour être en mesure de modifier gl_PointSize dans les shaders
    
    
    
    // Partie 3
    // TODO: Allocation des SSBO.
    //       Allouer suffisament d'espace pour le nombre maximal de particules.
    //       Seulement le buffer en entrée à besoin d'être initialisé à 0.
    //       Réfléchisser au type d'usage.
    
    // TODO: Créer un vao pour le dessin des particules et activer les attributs nécessaires.


    // TODO: Création des nouveaux shaders.
    
    
    // TODO: Initialisation de la nouvelle texture pour les particules.
    // "../textures/particle.png"
    
}


void drawFrame() override
{
    // TODO: Au besoin, ajouter la recharge de vos nouveaux shaders
    //       après if (ImGui::Button("Reload Shaders")).
}



// TODO: Ajouter ces attributs
unsigned int bezierNPoints = 3; // Nombre de points supplémentaires sur la courbe, 0 est une ligne droite
unsigned int oldBezierNPoints = 0;

int cameraMode = 0;
float cameraAnimation = 0.f;
bool isAnimatingCamera = false;


void sceneMain()
{
    // Partie 1-2

    ImGui::Begin("Scene Parameters");
    
    // TODO: À ajouter
    ImGui::SliderInt("Bezier Number Of Points", (int*)&bezierNPoints, 0, 16);
    if (ImGui::Button("Animate Camera"))
    {
        isAnimatingCamera = true;
        cameraMode = 1;
    }
    // Conserver les autres options du menu
    // ...
    
    ImGui::End();
    
    // ...
    
    if (isAnimatingCamera)
    {
        if (cameraAnimation < 5)
        {
            // TODO: Animation de la caméra
            // cameraPosition_ = ...
        
            cameraAnimation += deltaTime_ / 3.0;
        }
        else
        {
            // Remise à 0 de l'orientation
            glm::vec3 distance = glm::vec3(0, 2, -10) - cameraPosition_;               
            
            float horizontalDistance = sqrt(distance.x * distance.x + distance.z * distance.z);

            cameraOrientation_.y = atan2(-distance.x, -distance.z);
            cameraOrientation_.x = atan2(distance.y, horizontalDistance);
            
            cameraAnimation = 0.f;
            isAnimatingCamera = false;
            cameraMode = 0;
        }
    }
    
    // ...
    
    bool hasNumberOfSidesChanged = bezierNPoints != oldBezierNPoints;
    if (hasNumberOfSidesChanged)
    {
        oldBezierNPoints = bezierNPoints;
        
        // TODO: Calcul et mise à jour de la courbe
    }

    // TODO: Dessin de la courbe
    // glDraw...
    
    
    // TODO: Dessin du gazon
    // glDraw...
    
    
    // ...
    
    // Partie 3
    // TODO: Attention à l'endroit où vous faites votre dessin, la texture des particules est transparente.
    
    // Particles    
    totalTime += deltaTime_;
    timerParticles_ += deltaTime_;        
    const float particlesSpawnInterval = 0.2f;
    
    unsigned int particlesToAdd = timerParticles_ / particlesSpawnInterval;
    timerParticles_ -= particlesToAdd * particlesSpawnInterval;
    
    nParticles_ += particlesToAdd;
    if (nParticles_ > MAX_PARTICLES_)
        nParticles_ = MAX_PARTICLES_;

    // Particles update
    
    // TODO: Mise à jour des données à l'aide du compute shader
    //       Envoyer vos uniforms.
    
    // Utiliser la position de la meule pour calculer la nouvelle position et direction d'émission de particule.
    // glm::vec3 emitterPos = vec3(0.0, 0.0, 0.0);
    // glm::vec3 emitterDir = vec3(1, 0, 0);
    
    // TODO: Configurer les buffers d'entrée et de sortie.
    
    // TODO: Envois de la commande de calcul.
    //       Pas besoin d'optimiser le nombre de work group vs la taille local (dans le shader).
    
    
    // Particles draw
    
    // TODO: Dessin des particules. Utiliser le nombre de particules actuellement utilisées.
    //       Utiliser la texture et envoyer vos uniforms.
    //       Il sera nécessaire de spécifier les entrée en spécifiant le buffer d'entrée.
    //       Activer le blending et restaurer l'état du contexte modifié.
    
    // TODO: Interchanger les deux buffers, celui en entrée devient la sortie, et vice versa.
}


// Partie 1-2
// TODO: Ajouter les attributs de vbo, ebo, vao nécessaire



// Partie 3
// TODO: Attributs supplémentaires
GLuint vaoParticles_;

float totalTime;
float timerParticles_;

static const unsigned int MAX_PARTICLES_ = 64;
unsigned int nParticles_;    

// TODO: Ajouter vos shaders
// TODO: Ajouter la texture de particule

// Ssbo
ShaderStorageBuffer particles_[2];






