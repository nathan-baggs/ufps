#pragma once

#include <GL/gl.h>
#include <windows.h>

#include "third_party/opengl/glext.h"
#include "third_party/opengl/wglext.h"

#define FOR_OPENGL_FUNCTIONS(DO)                                                                                       \
    DO(::PFNGLCREATESHADERPROC, glCreateShader)                                                                        \
    DO(::PFNGLDELETESHADERPROC, glDeleteShader)                                                                        \
    DO(::PFNGLSHADERSOURCEPROC, glShaderSource)                                                                        \
    DO(::PFNGLCOMPILESHADERPROC, glCompileShader)                                                                      \
    DO(::PFNGLGETSHADERIVPROC, glGetShaderiv)                                                                          \
    DO(::PFNGLGETPROGRAMIVPROC, glGetProgramiv)                                                                        \
    DO(::PFNGLCREATEPROGRAMPROC, glCreateProgram)                                                                      \
    DO(::PFNGLDELETEPROGRAMPROC, glDeleteProgram)                                                                      \
    DO(::PFNGLATTACHSHADERPROC, glAttachShader)                                                                        \
    DO(::PFNGLLINKPROGRAMPROC, glLinkProgram)                                                                          \
    DO(::PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays)                                                                  \
    DO(::PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays)                                                            \
    DO(::PFNGLGENBUFFERSPROC, glGenBuffers)                                                                            \
    DO(::PFNGLDELETEBUFFERSPROC, glDeleteBuffers)                                                                      \
    DO(::PFNGLBINDVERTEXARRAYPROC, glBindVertexArray)                                                                  \
    DO(::PFNGLBINDBUFFERPROC, glBindBuffer)                                                                            \
    DO(::PFNGLBUFFERDATAPROC, glBufferData)                                                                            \
    DO(::PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer)                                                          \
    DO(::PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray)                                                  \
    DO(::PFNGLDEBUGMESSAGECALLBACKPROC, glDebugMessageCallback)                                                        \
    DO(::PFNGLUSEPROGRAMPROC, glUseProgram)                                                                            \
    DO(::PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation)                                                            \
    DO(::PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv)                                                                \
    DO(::PFNGLUNIFORM3FVPROC, glUniform3fv)                                                                            \
    DO(::PFNGLUNIFORM1IPROC, glUniform1i)                                                                              \
    DO(::PFNGLUNIFORM1FPROC, glUniform1f)                                                                              \
    DO(::PFNGLCREATEBUFFERSPROC, glCreateBuffers)                                                                      \
    DO(::PFNGLNAMEDBUFFERSTORAGEPROC, glNamedBufferStorage)                                                            \
    DO(::PFNGLCREATEVERTEXARRAYSPROC, glCreateVertexArrays)                                                            \
    DO(::PFNGLVERTEXARRAYBINDVERTEXBUFFEREXTPROC, glVertexArrayVertexBuffer)                                           \
    DO(::PFNGLENABLEVERTEXARRAYATTRIBPROC, glEnableVertexArrayAttrib)                                                  \
    DO(::PFNGLVERTEXARRAYATTRIBFORMATPROC, glVertexArrayAttribFormat)                                                  \
    DO(::PFNGLVERTEXARRAYATTRIBBINDINGPROC, glVertexArrayAttribBinding)                                                \
    DO(::PFNGLNAMEDBUFFERSUBDATAPROC, glNamedBufferSubData)                                                            \
    DO(::PFNGLVERTEXARRAYELEMENTBUFFERPROC, glVertexArrayElementBuffer)                                                \
    DO(::PFNGLBINDBUFFERBASEPROC, glBindBufferBase)                                                                    \
    DO(::PFNGLBINDBUFFERRANGEPROC, glBindBufferRange)                                                                  \
    DO(::PFNGLCREATETEXTURESPROC, glCreateTextures)                                                                    \
    DO(::PFNGLGETTEXTURESAMPLERHANDLEARBPROC, glGetTextureSamplerHandleARB)                                            \
    DO(::PFNGLMAKETEXTUREHANDLERESIDENTARBPROC, glMakeTextureHandleResidentARB)                                        \
    DO(::PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC, glProgramUniformHandleui64ARB)                                          \
    DO(::PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC, glMakeTextureHandleNonResidentARB)                                  \
    DO(::PFNGLTEXTURESTORAGE2DPROC, glTextureStorage2D)                                                                \
    DO(::PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC, glTextureStorage2DMultisample)                                          \
    DO(::PFNGLTEXTURESUBIMAGE2DPROC, glTextureSubImage2D)                                                              \
    DO(::PFNGLTEXTURESUBIMAGE3DPROC, glTextureSubImage3D)                                                              \
    DO(::PFNGLCREATESAMPLERSPROC, glCreateSamplers)                                                                    \
    DO(::PFNGLDELETESAMPLERSPROC, glDeleteSamplers)                                                                    \
    DO(::PFNGLBINDTEXTUREUNITPROC, glBindTextureUnit)                                                                  \
    DO(::PFNGLBINDSAMPLERPROC, glBindSampler)                                                                          \
    DO(::PFNGLSAMPLERPARAMETERIPROC, glSamplerParameteri)                                                              \
    DO(::PFNGLSAMPLERPARAMETERFPROC, glSamplerParameterf)                                                              \
    DO(::PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog)                                                                \
    DO(::PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog)                                                              \
    DO(::PFNGLGETACTIVEUNIFORMPROC, glGetActiveUniform)                                                                \
    DO(::PFNGLCREATEFRAMEBUFFERSPROC, glCreateFramebuffers)                                                            \
    DO(::PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC, glCheckNamedFramebufferStatus)                                          \
    DO(::PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers)                                                            \
    DO(::PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer)                                                                  \
    DO(::PFNGLNAMEDFRAMEBUFFERTEXTUREPROC, glNamedFramebufferTexture)                                                  \
    DO(::PFNGLBLITNAMEDFRAMEBUFFERPROC, glBlitNamedFramebuffer)                                                        \
    DO(::PFNGLCREATERENDERBUFFERSPROC, glCreateRenderbuffers)                                                          \
    DO(::PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC, glNamedFramebufferRenderbuffer)                                        \
    DO(::PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC, glNamedFramebufferDrawBuffers)                                          \
    DO(::PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC, glNamedRenderbufferStorageMultisample)                          \
    DO(::PFNGLGENERATETEXTUREMIPMAPPROC, glGenerateTextureMipmap)                                                      \
    DO(::PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC, glNamedFramebufferReadBuffer)                                            \
    DO(::PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC, glNamedFramebufferDrawBuffer)                                            \
    DO(::PFNGLOBJECTLABELPROC, glObjectLabel)                                                                          \
    DO(::PFNGLVALIDATEPROGRAMPROC, glValidateProgram)                                                                  \
    DO(::PFNGLMULTIDRAWARRAYSINDIRECTPROC, glMultiDrawArraysIndirect)                                                  \
    DO(::PFNGLMULTIDRAWELEMENTSINDIRECTPROC, glMultiDrawElementsIndirect)                                              \
    DO(::PFNGLMAPNAMEDBUFFERRANGEPROC, glMapNamedBufferRange)                                                          \
    DO(::PFNGLUNMAPNAMEDBUFFERPROC, glUnmapNamedBuffer)

#define DO_DEFINE(TYPE, NAME) inline TYPE NAME;
FOR_OPENGL_FUNCTIONS(DO_DEFINE)
