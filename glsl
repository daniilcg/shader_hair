#version 330 core

in vec3 FragPos; // Положение фрагмента
in vec3 Normal; // Нормаль фрагмента

out vec4 FragColor;

uniform vec3 lightPos; // Положение источника света
uniform vec3 cameraPos; // Положение камеры

void main()
{
    // Вычисляем направление света
    vec3 lightDir = normalize(lightPos - FragPos);

    // Вычисляем интенсивность диффузного отражения
    float diffuseStrength = max(dot(Normal, lightDir), 0.0);

    // Вычисляем интенсивность тени
    float shadowIntensity = 0.0;
    
    // Проходим по каждому волоску и проверяем, есть ли он на пути света
    for (int i = 0; i < NUM_HAIRS; i++)
    {
        // Вычисляем направление от текущего волоска до источника света
        vec3 hairToLight = lightPos - hairPositions[i];
        
        // Проверяем, находится ли волосок между источником света и текущим фрагментом
        if (dot(hairToLight, hairToLight) < dot(lightDir, lightDir))
        {
            // Если да, то увеличиваем интенсивность тени
            shadowIntensity += 1.0;
        }
    }
    
    // Нормализуем интенсивность тени
    shadowIntensity /= float(NUM_HAIRS);

    // Конечная интенсивность света
    float lightIntensity = diffuseStrength * (1.0 - shadowIntensity);

    // Вычисляем конечный цвет
    vec3 finalColor = lightIntensity * hairColor;

    FragColor = vec4(finalColor, 1.0);
}
