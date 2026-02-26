#include "TextureUtils.h"
#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <cmath>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


unsigned int loadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int generateRaindropTexture()
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width = 64;
    int height = 64;

    std::vector<unsigned char> data(width * height * 4);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float centerX = width / 2.0f;
            float centerY = height / 2.0f;
            float distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
            float maxDist = width / 2.0f;
            float alpha = 1.0f - (distance / maxDist);
            if (alpha < 0.0f) alpha = 0.0f;

            alpha = pow(alpha, 3.0f);

            int index = (y * width + x) * 4;
            data[index + 0] = 200;
            data[index + 1] = 200;
            data[index + 2] = 255;
            data[index + 3] = (unsigned char)(alpha * 255);
        }
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;

}