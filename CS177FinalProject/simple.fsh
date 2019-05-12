#version 330 core

in vec3 f_pos;
in vec3 f_normal;
in vec4 f_color;

out vec4 color;

// from: https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/2.2.basic_lighting_specular/2.2.basic_lighting.fs
uniform vec3 lightPos; 
// uniform vec3 lightD;
uniform vec3 viewPos; 
uniform vec3 lightColor;
// uniform float cutOff;
// uniform float outerCutOff;
uniform float constant;
uniform float linear;
uniform float quadratic;

void main(){
	// color = vec4(1.f, 1.f, 0.f, 1.f);
	// ambient
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor; // lightColor
  	
    // diffuse 
    vec3 norm = normalize(f_normal);
    vec3 lightDir = normalize(lightPos - f_pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor; // lightColor
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - f_pos);

	// blinn-phong
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

	// phong
    vec3 reflectDir = reflect(-lightDir, norm);  
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor; // lightColor

	// spotlight (soft edges)
    // float theta = dot(lightDir, normalize(-lightD)); 
    // float epsilon = (cutOff - outerCutOff);
    // float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
    // diffuse  *= intensity;
    // specular *= intensity;

	// attenuation
    float distance    = length(lightPos - f_pos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));    

    ambient  *= attenuation;  
    diffuse   *= attenuation;
    specular *= attenuation;  

	//color = vec4((ambient + diffuse + specular), 1.0f);
	color = vec4(f_color.rgba * vec4((ambient + diffuse + specular),1.0f));
	// color = f_color;

}
