#version 400 core

// TODO: À remplir
layout(vertices = 3) out;

uniform mat4 modelView;

void main()
{
    // TODO: Définir le niveau de tessellation une fois par patch.
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0)
    {
    
        vec4 v0 = gl_in[0].gl_Position; // (1, 0, 0)
        vec4 v1 = gl_in[1].gl_Position; // (0, 1, 0)
        vec4 v2 = gl_in[2].gl_Position; // (0, 0, 1)
        
        const float MIN_TESS = 2.0;
        const float MAX_TESS = 32.0;

        const float MIN_DIST = 10.0f;
        const float MAX_DIST = 40.0f;

        // Points milieux des arêtes opposées aux sommets 0, 1 et 2 en espace vue
        vec3 mid0 = (modelView * ((v1 + v2) * 0.5)).xyz;
        vec3 mid1 = (modelView * ((v2 + v0) * 0.5)).xyz;
        vec3 mid2 = (modelView * ((v0 + v1) * 0.5)).xyz;

        float d0 = length(mid0);
        float d1 = length(mid1);
        float d2 = length(mid2);

        // Utiliser la distance du point milieu de l'arête traité par rapport à la caméra
        // comme facteur pour déterminer le niveau de tessellation.

        float f0 = clamp((d0 - MIN_DIST) / (MAX_DIST - MIN_DIST), 0.0, 1.0);
        float f1 = clamp((d1 - MIN_DIST) / (MAX_DIST - MIN_DIST), 0.0, 1.0);
        float f2 = clamp((d2 - MIN_DIST) / (MAX_DIST - MIN_DIST), 0.0, 1.0);
    
        // Une fonction par palié donnera un meilleur résultat pour déterminer le niveau de tessellation
        // selon la distance (variation moins fréquente).

        float level0 = exp2(floor(mix(log2(MAX_TESS), log2(MIN_TESS), f0) + 0.5));
        float level1 = exp2(floor(mix(log2(MAX_TESS), log2(MIN_TESS), f1) + 0.5));
        float level2 = exp2(floor(mix(log2(MAX_TESS), log2(MIN_TESS), f2) + 0.5));

        gl_TessLevelOuter[0] = level0;
        gl_TessLevelOuter[1] = level1;
        gl_TessLevelOuter[2] = level2;
        
        // La tessellation de l'intérieur est le maximum des tessellations extérieurs
        gl_TessLevelInner[0] = max(max(level0, level1), level2);
    }
}
