/*
 * Utility class that prepares a frame buffer object
 * 
 * (c) Seth Hall et al.
 * 
 * @see AndroidGPUCannyDemo.java
 */
package net.mkonrad.glimageproc;

import android.opengl.GLES20;

public class FrameBufferObject
{
   private int fboID;
   private int renderID;
   
   public FrameBufferObject()
   {
      fboID = 0;
   }

   public void init()
   {
      int[] idArray = new int[1];
      GLES20.glGenFramebuffers(1, idArray, 0);
      fboID = idArray[0];
   }

   public void attachNewRenderBuffer(int texW, int texH)
   {
      int[] tempIndices = new int[1];
      GLES20.glGenRenderbuffers(1, tempIndices, 0);
      renderID = tempIndices[0];
      GLES20.glBindRenderbuffer(GLES20.GL_RENDERBUFFER, renderID);
      GLES20.glRenderbufferStorage(GLES20.GL_RENDERBUFFER,
            GLES20.GL_DEPTH_COMPONENT16, texW, texH);
      GLES20.glFramebufferRenderbuffer(GLES20.GL_FRAMEBUFFER,
            GLES20.GL_DEPTH_ATTACHMENT, GLES20.GL_RENDERBUFFER, renderID);
   }

   public void bind()
   {
      GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, fboID);
   }

   public void unbind()
   {
      GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
   }

   public int getFBOID()
   {
      return fboID;
   }

   public void attachTextureID(int texID)
   {
      GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER,
            GLES20.GL_COLOR_ATTACHMENT0, GLES20.GL_TEXTURE_2D, texID, 0);
   }
}
