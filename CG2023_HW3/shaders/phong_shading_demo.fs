#version 330 core

uniform vec3 fillColor;
//material data
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
//Light data
uniform vec3 pointLightPos;
uniform vec3 pointLightIntensity;
uniform vec3 ambientLight;
uniform vec3 dirLightDir;
uniform vec3 dirLightRadiance;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightIntensity;
uniform float spotLightCutoffStartInDegree;
uniform float spotLightTotalWidthInDegree;

in vec3 iNormalWorld;
in vec3 iPosWorld;
//Texture
in vec2 iTexCoord;
uniform sampler2D mapKd;
//-------------------------
out vec4 FragColor;

// to determined whether to use mapKd 
uniform int flag;

void main()
{
    // modified version, you need to modified Kd below to finalKd
    vec3 finalKd = Kd;
    if(flag == 1){
        finalKd = texture2D(mapKd,iTexCoord).rgb;
    }
    // original
    // vec3 texColor = texture2D(mapKd,iTexCoord).rgb;
    vec3 result = vec3(0.0f,0.0f,0.0f);


    //ambient
    vec3 ambient = ambientLight * Ka; 


    //diffuse
    vec3 N = normalize(iNormalWorld);
    vec3 lightDir =normalize(pointLightPos - iPosWorld);
    float diff = max(dot(lightDir,normalize(iNormalWorld)),0.0);
    vec3 diffuse = diff * finalKd * pointLightIntensity;
    
    //specular
    vec3 viewDir = normalize(vec3(0.0f, 1.0f, 5.0f) - iPosWorld);
    vec3 reflectDir = normalize(reflect(-lightDir, N));
    float spec = pow(max(dot(viewDir, reflectDir), 0.0),Ns);
    vec3 specular = spec * Ks *pointLightIntensity; 
    
    //Bling Phong
    lightDir = normalize(pointLightPos - iPosWorld);
    vec3 BlingPhong = normalize(viewDir + lightDir);

    //point light

    float distance  = length(pointLightPos - iPosWorld);
    float attenuation = 1.0 /  (distance * distance);
    vec3 pointlight = diffuse * attenuation + specular * attenuation;


    //directional light
    lightDir = normalize(-dirLightDir);
    diff = max(dot(N, lightDir), 0.0);
    reflectDir = reflect(-lightDir, N);
    spec = pow(max(dot(viewDir, reflectDir), 0.0),Ns);
    diffuse  = finalKd  * diff * dirLightRadiance;
    specular = Ks * spec * dirLightRadiance;
    vec3 directionallight = (ambient + diffuse + specular);


    //Spot light
    float theta = dot(normalize(spotLightPos - iPosWorld),normalize(-spotLightDir));
    vec3 spotlight;

    lightDir = normalize(spotLightPos - iPosWorld);
    BlingPhong = normalize(viewDir + lightDir);
    diff = max(dot(N,lightDir), 0.0);
    reflectDir = reflect(-lightDir,N);
    spec = pow(max(dot(viewDir, reflectDir), 0.0),Ns);
    distance  = length(spotLightPos - iPosWorld);
    attenuation = 1 /  (distance * distance);
    float cutoff =  cos(radians(spotLightCutoffStartInDegree));
    float totalwidth = cos(radians(spotLightTotalWidthInDegree));
    float epsilon = cutoff - totalwidth;
    float intensity = clamp((theta - totalwidth)/ epsilon,0.0,1.0);
    diffuse  = finalKd * diff * spotLightIntensity  * attenuation * intensity;
    specular = Ks * spec * spotLightIntensity  * attenuation * intensity;

    spotlight = (diffuse + specular);

    result = pointlight + spotlight + directionallight;
    FragColor = vec4(result,1.0);
}