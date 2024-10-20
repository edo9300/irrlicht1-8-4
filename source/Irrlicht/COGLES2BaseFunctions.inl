GL_FUNC(glActiveTexture, void, GLenum texture)
GL_FUNC(glAttachShader, void, GLuint program, GLuint shader)
GL_FUNC(glBindAttribLocation, void, GLuint program, GLuint index, const GLchar* name)
GL_FUNC(glBindBuffer, void, GLenum target, GLuint buffer)
GL_FUNC(glBindFramebuffer, void, GLenum target, GLuint framebuffer)
GL_FUNC(glBindTexture, void, GLenum target, GLuint texture)
GL_FUNC(glBlendEquation, void, GLenum mode)
GL_FUNC(glBlendFunc, void, GLenum sfactor, GLenum dfactor)
GL_FUNC(glBlendFuncSeparate, void, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
GL_FUNC(glBufferData, void, GLenum target, GLsizeiptr size, const void* data, GLenum usage)
GL_FUNC(glBufferSubData, void, GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
GL_FUNC(glCheckFramebufferStatus, GLenum, GLenum target)
GL_FUNC(glClear, void, GLbitfield mask)
GL_FUNC(glClearColor, void, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
GL_FUNC(glClearDepthf, void, GLfloat d)
GL_FUNC(glClearStencil, void, GLint s)
GL_FUNC(glColorMask, void, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
GL_FUNC(glCompileShader, void, GLuint shader)
GL_FUNC(glCompressedTexImage2D, void, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
GL_FUNC(glCompressedTexSubImage2D, void, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
GL_FUNC(glCreateProgram, GLuint, void)
GL_FUNC(glCreateShader, GLuint, GLenum type)
GL_FUNC(glCullFace, void, GLenum mode)
GL_FUNC(glDeleteBuffers, void, GLsizei n, const GLuint* buffers)
GL_FUNC(glDeleteFramebuffers, void, GLsizei n, const GLuint* framebuffers)
GL_FUNC(glDeleteProgram, void, GLuint program)
GL_FUNC(glDeleteShader, void, GLuint shader)
GL_FUNC(glDeleteTextures, void, GLsizei n, const GLuint* textures)
GL_FUNC(glDepthFunc, void, GLenum func)
GL_FUNC(glDepthMask, void, GLboolean flag)
GL_FUNC(glDisable, void, GLenum cap)
GL_FUNC(glDisableVertexAttribArray, void, GLuint index)
GL_FUNC(glDrawArrays, void, GLenum mode, GLint first, GLsizei count)
GL_FUNC(glDrawElements, void, GLenum mode, GLsizei count, GLenum type, const void* indices)
GL_FUNC(glEnable, void, GLenum cap)
GL_FUNC(glEnableVertexAttribArray, void, GLuint index)
GL_FUNC(glFlush, void)
GL_FUNC(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
GL_FUNC(glFrontFace, void, GLenum mode)
GL_FUNC(glGenBuffers, void, GLsizei n, GLuint* buffers)
GL_FUNC(glGenFramebuffers, void, GLsizei n, GLuint* framebuffers)
GL_FUNC(glGenTextures, void, GLsizei n, GLuint* textures)
GL_FUNC(glGenerateMipmap, void, GLenum target)
GL_FUNC(glGetActiveUniform, void, GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
GL_FUNC(glGetAttachedShaders, void, GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)
GL_FUNC(glGetError, GLenum)
GL_FUNC(glGetFloatv, void, GLenum pname, GLfloat* data)
GL_FUNC(glGetIntegerv, void, GLenum pname, GLint* data)
GL_FUNC(glGetProgramInfoLog, void, GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
GL_FUNC(glGetProgramiv, void, GLuint program, GLenum pname, GLint* params)
GL_FUNC(glGetShaderInfoLog, void, GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
GL_FUNC(glGetShaderiv, void, GLuint shader, GLenum pname, GLint* params)
GL_FUNC(glGetString, const GLubyte*, GLenum name)
GL_FUNC(glGetUniformLocation, GLint, GLuint program, const GLchar* name)
GL_FUNC(glHint, void, GLenum target, GLenum mode)
GL_FUNC(glLineWidth, void, GLfloat width)
GL_FUNC(glLinkProgram, void, GLuint program)
GL_FUNC(glPixelStorei, void, GLenum pname, GLint param)
GL_FUNC(glReadPixels, void, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
GL_FUNC(glScissor, void, GLint x, GLint y, GLsizei width, GLsizei height)
#ifdef _IRR_ANDROID_PLATFORM_
GL_FUNC(glShaderSource, void, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
#else
GL_FUNC(glShaderSource, void, GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
#endif
GL_FUNC(glStencilFunc, void, GLenum func, GLint ref, GLuint mask)
GL_FUNC(glStencilMask, void, GLuint mask)
GL_FUNC(glStencilOp, void, GLenum fail, GLenum zfail, GLenum zpass)
GL_FUNC(glTexImage2D, void, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
GL_FUNC(glTexParameteri, void, GLenum target, GLenum pname, GLint param)
GL_FUNC(glTexSubImage2D, void, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
GL_FUNC(glUniform1fv, void, GLint location, GLsizei count, const GLfloat* value)
GL_FUNC(glUniform1iv, void, GLint location, GLsizei count, const GLint* value)
GL_FUNC(glUniform2fv, void, GLint location, GLsizei count, const GLfloat* value)
GL_FUNC(glUniform2iv, void, GLint location, GLsizei count, const GLint* value)
GL_FUNC(glUniform3fv, void, GLint location, GLsizei count, const GLfloat* value)
GL_FUNC(glUniform3iv, void, GLint location, GLsizei count, const GLint* value)
GL_FUNC(glUniform4fv, void, GLint location, GLsizei count, const GLfloat* value)
GL_FUNC(glUniform4iv, void, GLint location, GLsizei count, const GLint* value)
GL_FUNC(glUniformMatrix2fv, void, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_FUNC(glUniformMatrix3fv, void, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_FUNC(glUniformMatrix4fv, void, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
GL_FUNC(glUseProgram, void, GLuint program)
GL_FUNC(glVertexAttribPointer, void, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
GL_FUNC(glViewport, void, GLint x, GLint y, GLsizei width, GLsizei height)