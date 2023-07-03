#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

const GLchar* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 normal;
    out vec3 FragPos;
    out vec3 Normal;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main()
    {
        FragPos = vec3(model * vec4(position, 1.0));
        Normal = mat3(transpose(inverse(model))) * normal;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

const GLchar* fragmentShaderSource = R"(
    #version 330 core
    in vec3 FragPos;
    in vec3 Normal;
    out vec4 FragColor;
    uniform vec3 lightPos;
    uniform vec3 cameraPos;
    uniform vec3 hairColor;
    const int NUM_HAIRS = 100;
    uniform vec3 hairPositions[NUM_HAIRS];

    void main()
    {
        vec3 lightDir = normalize(lightPos - FragPos);
        float diffuseStrength = max(dot(Normal, lightDir), 0.0);
        float shadowIntensity = 0.0;

        for (int i = 0; i < NUM_HAIRS; i++)
        {
            vec3 hairToLight = lightPos - hairPositions[i];
            if (dot(hairToLight, hairToLight) < dot(lightDir, lightDir))
            {
                shadowIntensity += 1.0;
            }
        }

        shadowIntensity /= float(NUM_HAIRS);
        float lightIntensity = diffuseStrength * (1.0 - shadowIntensity);
        vec3 finalColor = lightIntensity * hairColor;
        FragColor = vec4(finalColor, 1.0);
    }
)";

int main()
{
    // Инициализация GLFW и создание окна
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hair Shadows", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    glewInit();

    // Компиляция вершинного шейдера
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Проверка ошибок компиляции вершинного шейдера
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    // Компиляция фрагментного шейдера
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Проверка ошибок компиляции фрагментного шейдера
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    // Создание и привязка шейдерной программы
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Проверка ошибок связывания шейдерной программы
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    // Удаление шейдерных объектов
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Определение данных входного вершинного буфера
    GLfloat vertices[] = {
        // Позиции           // Нормали
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
    };

    // Создание и привязка вершинного буфера и вершинного массива
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Основной цикл отрисовки
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Очистка экрана
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Использование шейдерной программы
        glUseProgram(shaderProgram);

        // Установка uniform переменных
        GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        GLint cameraPosLoc = glGetUniformLocation(shaderProgram, "cameraPos");
        GLint hairColorLoc = glGetUniformLocation(shaderProgram, "hairColor");
        glUniform3f(lightPosLoc, 1.0f, 1.0f, 1.0f);
        glUniform3f(cameraPosLoc, 0.0f, 0.0f, 0.0f);
        glUniform3f(hairColorLoc, 0.8f, 0.4f, 0.1f);

        // Рисование объекта
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // Обновление экрана
        glfwSwapBuffers(window);
    }

    // Очистка ресурсов
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // Завершение работы GLFW
    glfwTerminate();

    return 0;
}
