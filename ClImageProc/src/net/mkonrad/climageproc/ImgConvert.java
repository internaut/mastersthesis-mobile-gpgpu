package net.mkonrad.climageproc;

import java.nio.ByteBuffer;

import android.util.Log;

public class ImgConvert {
	private static final String TAG = "ClImageProc::ImgConvert";
	
	public static ByteBuffer convertARGB2RGBA(ByteBuffer argb) {
		int px, px2;
		ByteBuffer rgba = ByteBuffer.allocateDirect(argb.capacity());
		while (argb.remaining() > 0) {
			px = argb.getInt();
			
			// Source is in format: 			 0xAARRGGBB
		    px2 = 	((px & 0xFF000000) >>> 24) | //RR______
		    		((px & 0x00FF0000) <<   8) | //__GG____
		    		((px & 0x0000FF00) <<   8) | //____BB__
		    		((px & 0x000000FF) <<   8);  //______AA;
		    // Return value is in format:  		 0xRRGGBBAA
		    
		    rgba.putInt(px2);
		}
		
		argb.rewind();
		rgba.rewind();
		
		return rgba;
	}
	
	public static ByteBuffer convertRGBA2ARGB(ByteBuffer rgba) {
		int px, px2;
		ByteBuffer argb = ByteBuffer.allocateDirect(rgba.capacity());
		while (rgba.remaining() > 0) {
			px = rgba.getInt();
			
			// Source is in format: 			 0xRRGGBBAA
		    px2 = 	((px & 0xFF000000) >>>  8) | //__RR____
		    		((px & 0x00FF0000) >>>  8) | //____GG__
		    		((px & 0x0000FF00) >>>  8) | //______BB
		    		((px & 0x000000FF) <<  24);  //AA______
		    // Return value is in format:  		 0xAARRGGBB
		    
		    argb.putInt(px2);
		}
		
		argb.rewind();
		rgba.rewind();
		
		return argb;
	}
}
