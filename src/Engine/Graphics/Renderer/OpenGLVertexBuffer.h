#pragma once

#include <span>
#include <utility>

#include <glad/gl.h> // NOLINT: not a C system header.

#include "Library/Geometry/Vec.h"
#include "Library/Color/Colorf.h"

namespace detail {
template<typename T>
struct VertexAttribTraits;
} // namespace detail

template<typename T>
inline constexpr GLint vertex_size_v = detail::VertexAttribTraits<T>::size;

template<typename T>
inline constexpr GLenum vertex_type_v = detail::VertexAttribTraits<T>::type;

#define MM_REGISTER_OPENGL_VERTEX_TYPE(T, SIZE, TYPE)                           \
template<>                                                                      \
struct detail::VertexAttribTraits<T> {                                          \
    static constexpr GLint size = SIZE;                                         \
    static constexpr GLenum type = TYPE;                                        \
};

MM_REGISTER_OPENGL_VERTEX_TYPE(Vec2f, 2, GL_FLOAT)
MM_REGISTER_OPENGL_VERTEX_TYPE(Vec3f, 3, GL_FLOAT)
MM_REGISTER_OPENGL_VERTEX_TYPE(Colorf, 4, GL_FLOAT)
MM_REGISTER_OPENGL_VERTEX_TYPE(float, 1, GL_FLOAT)

/**
 * RAII wrapper for OpenGL VAO/VBO pair with type-safe attribute setup.
 *
 * Usage:
 * ```
 *   std::vector<MyVertex> verts;
 *   verts.emplace_back(...);
 *
 *   OpenGLVertexBuffer<MyVertex> buffer;
 *   buffer.setup(GL_DYNAMIC_DRAW, &MyVertex::pos, &MyVertex::texuv, &MyVertex::color);
 *   buffer.upload(verts);
 *   buffer.bind();
 *   glDrawArrays(GL_TRIANGLES, 0, verts.size());
 * ```
 */
template<typename Vertex>
class OpenGLVertexBuffer {
 public:
    OpenGLVertexBuffer() = default;

    ~OpenGLVertexBuffer() {
        reset();
    }

    // Non-copyable
    OpenGLVertexBuffer(const OpenGLVertexBuffer &) = delete;
    OpenGLVertexBuffer &operator=(const OpenGLVertexBuffer &) = delete;

    // Movable
    OpenGLVertexBuffer(OpenGLVertexBuffer &&other) noexcept {
        swap(other);
    }

    OpenGLVertexBuffer &operator=(OpenGLVertexBuffer &&other) noexcept {
        swap(other);
        return *this;
    }

    void swap(OpenGLVertexBuffer &other) noexcept {
        std::swap(_vao, other._vao);
        std::swap(_vbo, other._vbo);
        std::swap(_usage, other._usage);
    }

    /**
     * Initialize the VAO/VBO with specified attribute layout.
     *
     * @param usage                     `GL_STATIC_DRAW`, `GL_DYNAMIC_DRAW`, etc.
     * @param attrs                     Pointer-to-member for each vertex attribute (in shader location order).
     */
    template<typename... Attrs>
    void reset(GLenum usage, Attrs... attrs) {
        static_assert(sizeof...(Attrs) > 0, "At least one attribute required");

        if (_vao != 0)
            reset();

        _usage = usage;

        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);

        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);

        // Setup attributes using fold expression
        GLuint location = 0;
        (setupAttribute(location++, attrs), ...);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    /**
     * Release OpenGL resources.
     */
    void reset() {
        if (_vbo != 0) {
            glDeleteBuffers(1, &_vbo);
            _vbo = 0;
        }
        if (_vao != 0) {
            glDeleteVertexArrays(1, &_vao);
            _vao = 0;
        }
    }

    /**
     * Upload vertex data to the buffer.
     */
    void update(std::span<const Vertex> data) {
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * data.size(), data.data(), _usage);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    /**
     * Bind the VAO for drawing.
     */
    void bind() const {
        glBindVertexArray(_vao);
    }

    /**
     * Unbind any VAO. Static since it doesn't depend on instance state.
     */
    static void unbind() {
        glBindVertexArray(0);
    }

    /**
     * Check if the buffer has been initialized.
     */
    [[nodiscard]] operator bool() const {
        return _vao != 0;
    }

    [[nodiscard]] bool operator!() const {
        return !static_cast<bool>(*this);
    }

 private:
    template<typename MemberType>
    void setupAttribute(GLuint location, MemberType Vertex::*ptr) {
        // Calculate offset using pointer-to-member
        const Vertex *null = nullptr;
        const void *offset = static_cast<const void *>(&(null->*ptr));

        glVertexAttribPointer(
            location,
            vertex_size_v<MemberType>,
            vertex_type_v<MemberType>,
            GL_FALSE,
            sizeof(Vertex),
            offset
        );
        glEnableVertexAttribArray(location);
    }

    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLenum _usage = GL_DYNAMIC_DRAW;
};
