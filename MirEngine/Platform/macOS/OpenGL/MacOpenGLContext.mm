#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>

#include "MacOpenGLContext.h"

namespace MirEngine {
namespace Platform {
namespace macOS {

struct MacOpenGLContext::Impl {
    NSOpenGLContext* context = nil;
    NSView*          view    = nil;
    Rendering::Size2D size{1, 1};
};

MacOpenGLContext::MacOpenGLContext() {
    m_impl = new Impl();
}

MacOpenGLContext::~MacOpenGLContext() {
    if (m_impl->context) {
        [m_impl->context clearDrawable];
        [m_impl->context release];
    }
    delete m_impl;
}

bool MacOpenGLContext::initialize(Rendering::NativeWindowHandle window,
                                  const Rendering::Size2D& size)
{
    m_impl->view = (__bridge NSView*)window;
    m_impl->size = size;

    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFAColorSize,     24,
        NSOpenGLPFAAlphaSize,     8,
        NSOpenGLPFADepthSize,     24,
        NSOpenGLPFAStencilSize,   8,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFAMultisample,
        NSOpenGLPFASampleBuffers, 1,
        NSOpenGLPFASamples,        4,
        0
    };

    NSOpenGLPixelFormat* format =
        [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if (!format) {
        // Some GPUs / virtual machines do not offer MSAA 4x. Fall back to a
        // plain core-profile format so the viewport still works.
        NSOpenGLPixelFormatAttribute basicAttrs[] = {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
            NSOpenGLPFAColorSize,     24,
            NSOpenGLPFAAlphaSize,     8,
            NSOpenGLPFADepthSize,     24,
            NSOpenGLPFAStencilSize,   8,
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAAccelerated,
            0
        };
        format = [[NSOpenGLPixelFormat alloc] initWithAttributes:basicAttrs];
        if (!format) return false;
    }

    m_impl->context =
        [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
    [format release];

    if (!m_impl->context) return false;

    // OpenGL is deprecated on macOS 10.14+, but remains the active renderer
    // backend of MirEngine (Metal is a future backend). Suppress the
    // deprecation diagnostics for the intentional legacy API usage below.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    setView(window);
    [m_impl->context makeCurrentContext];

    GLint swap = 1;
    [m_impl->context setValues:&swap forParameter:NSOpenGLCPSwapInterval];
#pragma clang diagnostic pop

    glViewport(0, 0,
               static_cast<GLsizei>(size.width),
               static_cast<GLsizei>(size.height));

    return true;
}

void MacOpenGLContext::makeCurrent() {
    if (m_impl->context)
        [m_impl->context makeCurrentContext];
}

void MacOpenGLContext::setView(Rendering::NativeWindowHandle window) {
    if (!m_impl->context)
        return;

    m_impl->view = (__bridge NSView*)window;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (m_impl->view) {
        [m_impl->context setView:m_impl->view];
        [m_impl->context update];
    } else {
        // Отвязываем контекст от уничтоженного view (remount NSView),
        // не уничтожая сам контекст — он переиспользуется при следующем mount.
        [m_impl->context clearDrawable];
    }
#pragma clang diagnostic pop
}

void MacOpenGLContext::swapBuffers() {
    if (m_impl->context)
        [m_impl->context flushBuffer];
}

void MacOpenGLContext::resize(const Rendering::Size2D& size) {
    m_impl->size = size;
    makeCurrent();
    glViewport(0, 0,
               static_cast<GLsizei>(size.width),
               static_cast<GLsizei>(size.height));
    [m_impl->context update];
}

Rendering::Size2D MacOpenGLContext::size() const {
    return m_impl->size;
}

} // namespace macOS
} // namespace Platform
} // namespace MirEngine