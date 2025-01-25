#version 330 core
layout(location = 0) out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 dragonColor;
uniform vec3 viewPos;
uniform vec3 ditheringColor;
uniform vec3 edgeColor;

uniform int nbColors;
uniform float edgeThreshold;
uniform int dithering;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    float specularStrength = 0.8;
    
    vec3 norm = normalize(Normal);
    
    vec3 lightDir = normalize(lightPos - FragPos);

    //Luminance
    float diff = max(dot(norm, lightDir), 0.0);
    //Seuillage de l'intensité lumineuse (un nombre de couleurs sur l'objet égal à nbColors)
    diff = round(diff * nbColors)/nbColors;
    vec3 diffuse = diff * lightColor;

    //Calcul des reflets de la lumière par rapport au point de vue de la caméra
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    vec3 specular = specularStrength * spec * lightColor;  
    vec3 result = (ambient + diffuse + specular) * dragonColor;

    //Calcul du tramage: on vérifie tous les y pixels (ici 'dithering') que 
    //la coordonnée x (puis y) du pixel courant est multiple de 'dithering'.
    //tramage contient donc une valeur entre 0 et dithering - 1 (entre 0 et y avec y le modulo)
    int tramage = int(mod(floor(gl_FragCoord.x), float(dithering))) + int(mod(floor(gl_FragCoord.y), float(dithering)));

    //Dérivées des normales
    vec3 dNdx = dFdx(norm);
    vec3 dNdy = dFdy(norm);

    float edgeStrength = length(dNdx) + length(dNdy);
    
    //Déterminer la discontinuité des normales à chaque pixel (effet de crayon)
    float discontinuity = smoothstep(edgeThreshold, edgeThreshold + 0.2, edgeStrength);

    //Si la luminance du fragment est inférieure à un seuil 
    //et que le fragment est un multiple de 'dithering' sur les x et y, 
    //on le met de la couleur ditheringColor (points de couleur)
    if(diff < 0.7 && tramage == 0.0)
        FragColor = vec4(ditheringColor, 1.0);
    else
    {
        vec4 resultColor = vec4(result, 1.0);
        //Couleur de la discontinuité des normales
        vec4 borderColor = vec4(edgeColor, 1.0);
        FragColor = mix(resultColor, borderColor, discontinuity);
    }
}