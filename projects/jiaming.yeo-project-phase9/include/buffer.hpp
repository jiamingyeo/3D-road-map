#pragma once

#include <GL/glew.h>

class Buffer {
public:
    Buffer();
    ~Buffer();

    // NO COPY
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // YES MOVE
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    void BindVAO() const;
    void BindVBO() const;
    void BindEBO() const;

    void UnbindVAO() const;
    void UnbindVBO() const;

    void SetData(
        GLsizeiptr size,
        const void* vertices,
        GLenum usage
    ) const;

    void SetIndexData(
        GLsizeiptr size,
        const void* data,
        GLenum usage
    ) const;

    void SetAttribute(
        GLuint index,
        GLint size,
        GLenum type,
        GLboolean normalized,
        GLsizei stride,
        const void* offset
    ) const;

private:
    GLuint mVAO = 0;
    GLuint mVBO = 0;
    GLuint mEBO = 0;
};