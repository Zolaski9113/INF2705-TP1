#version 430 core

// Une invocation par particule, 64 par groupe (le C++ lance ceil(nParticles / 64) groupes)
layout(local_size_x = 64) in;

struct Particle
{
    vec3 position;
    float zOrientation;
    vec3 velocity;
    vec4 color;
    vec2 size;
    float timeToLive;
    float maxTimeToLive;
};

layout(std140, binding = 0) readonly restrict buffer ParticlesInputBlock
{
    Particle particles[];
} dataIn;

layout(std140, binding = 1) writeonly restrict buffer ParticlesOutputBlock
{
    Particle particles[];
} dataOut;

uniform float time;
uniform float deltaTime;
uniform vec3 emitterPosition;
uniform vec3 emitterDirection;
uniform uint particleCount;

// Nombre pseudo-aléatoire entre 0 et 1, différent par particule et par trame
float rand01()
{
    return fract(sin(dot(vec2(time * 100.0, float(gl_GlobalInvocationID.x)), vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    uint i = gl_GlobalInvocationID.x;

    // Le dernier groupe peut dépasser le nombre de particules actives
    if (i >= particleCount)
        return;

    Particle p = dataIn.particles[i];

    if (p.timeToLive <= 0.0)
    {
        // (Ré)initialisation : particule neuve ou morte, repart de l'émetteur.
        // Une seule valeur aléatoire, utilisée pour l'orientation et la durée de vie.
        float randomValue = rand01();

        p.position = emitterPosition;
        p.zOrientation = randomValue * 6.28318530718;                         // [0, 2pi]
        p.velocity = normalize(emitterDirection) * 0.3 + vec3(0.0, 0.2, 0.0); // vers l'avant + montée
        p.color = vec4(0.61, 0.52, 0.38, 0.0);                                // invisible au départ (fondu)
        p.size = vec2(1.2);
        p.maxTimeToLive = mix(2.5, 4.0, randomValue);                         // [2.5, 4.0] s
        p.timeToLive = p.maxTimeToLive;
    }
    else
    {
        p.timeToLive = max(0.0, p.timeToLive - deltaTime);

        // Méthode d'Euler : position et orientation avancent de vitesse * dt
        p.position += p.velocity * deltaTime;
        p.zOrientation += 0.5 * deltaTime; // vitesse angulaire constante

        // Avancement dans la vie : 0 à la naissance, 1 à la mort
        float progress = 1.0 - p.timeToLive / p.maxTimeToLive;

        // Couleur et taille : interpolation linéaire selon l'âge
        p.color.rgb = mix(vec3(0.61, 0.52, 0.38), vec3(1.0), progress);
        p.size = vec2(mix(1.2, 2.5, progress));

        // Opacité 0.2, avec fondu d'apparition [0, 0.2] et de disparition [0.8, 1] (pas de pop in/out)
        p.color.a = 0.2 * smoothstep(0.0, 0.2, progress) * (1.0 - smoothstep(0.8, 1.0, progress));
    }

    dataOut.particles[i] = p;
}
