
#version 430 core

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

// Nombre pseudo-aléatoire entre 0 et 1
float rand01()
{
    return fract(
        sin(dot(
            vec2(time * 100.0, float(gl_GlobalInvocationID.x)),
            vec2(12.9898, 78.233)
        )) * 43758.5453
    );
}

void main()
{
    uint i = gl_GlobalInvocationID.x;

    if (i >= particleCount)
        return;

    Particle p = dataIn.particles[i];

    if (p.timeToLive <= 0.0)
    {
        // Initialisation de la particule
        float randomValue = rand01();

        p.position = emitterPosition;

        p.zOrientation = randomValue * 6.28318530718;

        p.velocity = normalize(emitterDirection) * 0.3
                   + vec3(0.0, 0.2, 0.0);

        p.color = vec4(0.61, 0.52, 0.38, 0.0);

        p.size = vec2(1.2);

        p.maxTimeToLive = mix(2.5, 4.0, randomValue);
        p.timeToLive = p.maxTimeToLive;
    }
    else
    {
        // Mise à jour du temps de vie
        p.timeToLive = max(0.0, p.timeToLive - deltaTime);

        // Méthode d'Euler
        p.position += p.velocity * deltaTime;

        // Rotation constante
        p.zOrientation += 0.5 * deltaTime;

        float progress = 1.0
                       - p.timeToLive / p.maxTimeToLive;

        // Dégradé de couleur
        p.color.rgb = mix(
            vec3(0.61, 0.52, 0.38),
            vec3(1.0),
            progress
        );

        // Transparence avec fade in / fade out
        p.color.a = 0.2
                  * smoothstep(0.0, 0.2, progress)
                  * (1.0 - smoothstep(0.8, 1.0, progress));

        // Augmentation de la taille
        p.size = vec2(mix(1.2, 2.5, progress));
    }

    dataOut.particles[i] = p;
}
