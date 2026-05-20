
#include"TextureArray.h"

TextureArray::TextureArray(const std::vector<std::string>& imagePaths, int tileW, int tileH) {
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, tileW, tileH, (GLsizeiptr)imagePaths.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	stbi_set_flip_vertically_on_load(true);

    for (int i = 0; i < (int)imagePaths.size(); i++) {
        int widthImg, heightImg, colCh;
        unsigned char* data = stbi_load(imagePaths[i].c_str(), &widthImg, &heightImg, &colCh, 4);

        if (!data) {
            printf("TextureArray: failed to load %s\n", imagePaths[i].c_str());
            continue;
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, tileW, tileH, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);

    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
void TextureArray::TexUnit(Shader shader, const char* samplerName, GLuint unit) {
    GLuint texUni = glGetUniformLocation(shader.ID, samplerName);
    shader.Activate();
    glUniform1i(texUni, unit);
}
void TextureArray::Bind() {
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
}
void TextureArray::Unbind() {
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
void TextureArray::Delete() {
    glDeleteTextures(1, &ID);
}