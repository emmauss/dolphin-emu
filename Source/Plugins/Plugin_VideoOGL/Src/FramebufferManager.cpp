// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.

#include "Globals.h"
#include "FramebufferManager.h"
#include "VertexShaderGen.h"

#include "TextureConverter.h"
#include "Render.h"
#include "HW/Memmap.h"

namespace OGL
{

int FramebufferManager::m_targetWidth;
int FramebufferManager::m_targetHeight;
int FramebufferManager::m_msaaSamples;
int FramebufferManager::m_msaaCoverageSamples;

GLuint FramebufferManager::m_efbFramebuffer;
GLuint FramebufferManager::m_efbColor; // Renderbuffer in MSAA mode; Texture otherwise
GLuint FramebufferManager::m_efbDepth; // Renderbuffer in MSAA mode; Texture otherwise

// Only used in MSAA mode.
GLuint FramebufferManager::m_resolvedFramebuffer;
GLuint FramebufferManager::m_resolvedColorTexture;
GLuint FramebufferManager::m_resolvedDepthTexture;

GLuint FramebufferManager::m_xfbFramebuffer;

FramebufferManager::FramebufferManager(int targetWidth, int targetHeight, int msaaSamples, int msaaCoverageSamples)
{
	m_efbFramebuffer = 0;
	m_efbColor = 0;
	m_efbDepth = 0;
	m_resolvedFramebuffer = 0;
	m_resolvedColorTexture = 0;
	m_resolvedDepthTexture = 0;
	m_xfbFramebuffer = 0;
	
	m_targetWidth = targetWidth;
	m_targetHeight = targetHeight;

	m_msaaSamples = msaaSamples;
	m_msaaCoverageSamples = msaaCoverageSamples;

	// The EFB can be set to different pixel formats by the game through the
	// BPMEM_ZCOMPARE register (which should probably have a different name).
	// They are:
	// - 24-bit RGB (8-bit components) with 24-bit Z
	// - 24-bit RGBA (6-bit components) with 24-bit Z
	// - Multisampled 16-bit RGB (5-6-5 format) with 16-bit Z
	// We only use one EFB format here: 32-bit ARGB with 24-bit Z.
	// Multisampling depends on user settings.
	// The distinction becomes important for certain operations, i.e. the
	// alpha channel should be ignored if the EFB does not have one.

	// Create EFB target.

	glGenFramebuffers(1, &m_efbFramebuffer);

	if (m_msaaSamples <= 1)
	{
		// EFB targets will be textures in non-MSAA mode.

		GLuint glObj[2];
		glGenTextures(2, glObj);
		m_efbColor = glObj[0];
		m_efbDepth = glObj[1];

		glBindTexture(GL_TEXTURE_RECTANGLE, m_efbColor);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA8, m_targetWidth, m_targetHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_RECTANGLE, m_efbDepth);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT24, m_targetWidth, m_targetHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

		glBindTexture(GL_TEXTURE_RECTANGLE, 0);

		// Bind target textures to the EFB framebuffer.

		glBindFramebuffer(GL_FRAMEBUFFER, m_efbFramebuffer);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, m_efbColor, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, m_efbDepth, 0);

		GL_REPORT_FBO_ERROR();
	}
#ifndef USE_GLES3
	else
	{
		// EFB targets will be renderbuffers in MSAA mode (required by OpenGL).
		// Resolve targets will be created to transfer EFB to RAM textures.
		// XFB framebuffer will be created to transfer EFB to XFB texture.

		// Create EFB target renderbuffers.

		GLuint glObj[2];
		glGenRenderbuffers(2, glObj);
		m_efbColor = glObj[0];
		m_efbDepth = glObj[1];

		glBindRenderbuffer(GL_RENDERBUFFER, m_efbColor);
		if (m_msaaCoverageSamples)
			glRenderbufferStorageMultisampleCoverageNV(GL_RENDERBUFFER, m_msaaCoverageSamples, m_msaaSamples, GL_RGBA8, m_targetWidth, m_targetHeight);
		else
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_msaaSamples, GL_RGBA8, m_targetWidth, m_targetHeight);

		glBindRenderbuffer(GL_RENDERBUFFER, m_efbDepth);
		if (m_msaaCoverageSamples)
			glRenderbufferStorageMultisampleCoverageNV(GL_RENDERBUFFER, m_msaaCoverageSamples, m_msaaSamples, GL_DEPTH_COMPONENT24, m_targetWidth, m_targetHeight);
		else
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_msaaSamples, GL_DEPTH_COMPONENT24, m_targetWidth, m_targetHeight);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// Bind target renderbuffers to EFB framebuffer.

		glBindFramebuffer(GL_FRAMEBUFFER, m_efbFramebuffer);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_efbColor);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_efbDepth);

		GL_REPORT_FBO_ERROR();

		// Create resolved targets for transferring multisampled EFB to texture.

		glGenFramebuffers(1, &m_resolvedFramebuffer);

		glGenTextures(2, glObj);
		m_resolvedColorTexture = glObj[0];
		m_resolvedDepthTexture = glObj[1];

		glBindTexture(GL_TEXTURE_RECTANGLE, m_resolvedColorTexture);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA8, m_targetWidth, m_targetHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_RECTANGLE, m_resolvedDepthTexture);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT24, m_targetWidth, m_targetHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_RECTANGLE, 0);

		// Bind resolved textures to resolved framebuffer.

		glBindFramebuffer(GL_FRAMEBUFFER, m_resolvedFramebuffer);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, m_resolvedColorTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, m_resolvedDepthTexture, 0);

		GL_REPORT_FBO_ERROR();

		// Return to EFB framebuffer.

		glBindFramebuffer(GL_FRAMEBUFFER, m_efbFramebuffer);
	}
#endif
	// Create XFB framebuffer; targets will be created elsewhere.

	glGenFramebuffers(1, &m_xfbFramebuffer);
	
	// EFB framebuffer is currently bound, make sure to clear its alpha value to 1.f
	glViewport(0, 0, m_targetWidth, m_targetHeight);
	glScissor(0, 0, m_targetWidth, m_targetHeight);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

FramebufferManager::~FramebufferManager()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint glObj[3];

	// Note: OpenGL deletion functions silently ignore parameters of "0".

	glObj[0] = m_efbFramebuffer;
	glObj[1] = m_resolvedFramebuffer;
	glObj[2] = m_xfbFramebuffer;
	glDeleteFramebuffers(3, glObj);
	m_efbFramebuffer = 0;
	m_xfbFramebuffer = 0;

	glObj[0] = m_resolvedColorTexture;
	glObj[1] = m_resolvedDepthTexture;
	glDeleteTextures(2, glObj);
	m_resolvedColorTexture = 0;
	m_resolvedDepthTexture = 0;

	glObj[0] = m_efbColor;
	glObj[1] = m_efbDepth;
	if (m_msaaSamples <= 1)
		glDeleteTextures(2, glObj);
	else
		glDeleteRenderbuffers(2, glObj);
	m_efbColor = 0;
	m_efbDepth = 0;
}

GLuint FramebufferManager::GetEFBColorTexture(const EFBRectangle& sourceRc)
{
	if (m_msaaSamples <= 1)
	{
		return m_efbColor;
	}
	else
	{
		// Transfer the EFB to a resolved texture. EXT_framebuffer_blit is
		// required.

		TargetRectangle targetRc = g_renderer->ConvertEFBRectangle(sourceRc);
		targetRc.ClampLL(0, 0, m_targetWidth, m_targetHeight);

		// Resolve.
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_efbFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolvedFramebuffer);
		glBlitFramebuffer(
			targetRc.left, targetRc.top, targetRc.right, targetRc.bottom,
			targetRc.left, targetRc.top, targetRc.right, targetRc.bottom,
			GL_COLOR_BUFFER_BIT, GL_NEAREST
			);

		// Return to EFB.
		glBindFramebuffer(GL_FRAMEBUFFER, m_efbFramebuffer);

		return m_resolvedColorTexture;
	}
}

GLuint FramebufferManager::GetEFBDepthTexture(const EFBRectangle& sourceRc)
{
	if (m_msaaSamples <= 1)
	{
		return m_efbDepth;
	}
	else
	{
		// Transfer the EFB to a resolved texture. EXT_framebuffer_blit is
		// required.

		TargetRectangle targetRc = g_renderer->ConvertEFBRectangle(sourceRc);
		targetRc.ClampLL(0, 0, m_targetWidth, m_targetHeight);

		// Resolve.
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_efbFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolvedFramebuffer);
		glBlitFramebuffer(
			targetRc.left, targetRc.top, targetRc.right, targetRc.bottom,
			targetRc.left, targetRc.top, targetRc.right, targetRc.bottom,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST
			);

		// Return to EFB.
		glBindFramebuffer(GL_FRAMEBUFFER, m_efbFramebuffer);

		return m_resolvedDepthTexture;
	}
}

void FramebufferManager::CopyToRealXFB(u32 xfbAddr, u32 fbWidth, u32 fbHeight, const EFBRectangle& sourceRc,float Gamma)
{
	u8* xfb_in_ram = Memory::GetPointer(xfbAddr);
	if (!xfb_in_ram)
	{
		WARN_LOG(VIDEO, "Tried to copy to invalid XFB address");
		return;
	}

	TargetRectangle targetRc = g_renderer->ConvertEFBRectangle(sourceRc);
	TextureConverter::EncodeToRamYUYV(ResolveAndGetRenderTarget(sourceRc), targetRc, xfb_in_ram, fbWidth, fbHeight);
}

void FramebufferManager::SetFramebuffer(GLuint fb)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fb != 0 ? fb : GetEFBFramebuffer());
}

// Apply AA if enabled
GLuint FramebufferManager::ResolveAndGetRenderTarget(const EFBRectangle &source_rect)
{
	return GetEFBColorTexture(source_rect);
}

GLuint FramebufferManager::ResolveAndGetDepthTarget(const EFBRectangle &source_rect)
{
	return GetEFBDepthTexture(source_rect);
}

XFBSource::~XFBSource()
{
	glDeleteRenderbuffers(1, &renderbuf);
}


void XFBSource::Draw(const MathUtil::Rectangle<float> &sourcerc,
		const MathUtil::Rectangle<float> &drawrc, int width, int height) const
{
	// Texture map xfbSource->texture onto the main buffer
	glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuf);
	glBlitFramebuffer(sourcerc.left, sourcerc.bottom, sourcerc.right, sourcerc.top,
		drawrc.left, drawrc.bottom, drawrc.right, drawrc.top,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);

	GL_REPORT_ERRORD();
}

void XFBSource::DecodeToTexture(u32 xfbAddr, u32 fbWidth, u32 fbHeight)
{
	TextureConverter::DecodeToTexture(xfbAddr, fbWidth, fbHeight, renderbuf);
}

void XFBSource::CopyEFB(float Gamma)
{
	// Copy EFB data to XFB and restore render target again
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FramebufferManager::GetEFBFramebuffer());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FramebufferManager::GetXFBFramebuffer());

	// Bind texture.
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuf);
	GL_REPORT_FBO_ERROR();

	glBlitFramebuffer(
		0, 0, texWidth, texHeight,
		0, 0, texWidth, texHeight,
		GL_COLOR_BUFFER_BIT, GL_NEAREST
	);

	// Return to EFB.
	FramebufferManager::SetFramebuffer(0);

}

XFBSourceBase* FramebufferManager::CreateXFBSource(unsigned int target_width, unsigned int target_height)
{
	GLuint renderbuf;

	glGenRenderbuffers(1, &renderbuf);
	
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, target_width, target_height);

	return new XFBSource(renderbuf);
}

void FramebufferManager::GetTargetSize(unsigned int *width, unsigned int *height, const EFBRectangle& sourceRc)
{
	*width = m_targetWidth;
	*height = m_targetHeight;
}

}  // namespace OGL
