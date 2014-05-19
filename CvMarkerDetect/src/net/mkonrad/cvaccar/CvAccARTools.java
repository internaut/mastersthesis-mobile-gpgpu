package net.mkonrad.cvaccar;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.content.Context;
import android.util.Log;
import android.util.Xml;

public class CvAccARTools {
	final private static String TAG = "CvAccARTools";
	static public Context ctx;
	
	static public Mat getCamIntrinsicsFromFile(String file) throws IOException, XmlPullParserException {
		Mat camIntr = parseCamXMLFile(file);
		if (camIntr != null) {
			return camIntr;
		} else {
			throw new IOException("Could not parse XML file " + file);
		}
	}

	static private Mat parseCamXMLFile(String file) throws XmlPullParserException,
			IOException {
		XmlPullParser parser = Xml.newPullParser();
		parser.setFeature(XmlPullParser.FEATURE_PROCESS_NAMESPACES, false);

		InputStream stream = ctx.getResources().getAssets().open(file);
		parser.setInput(stream, null);
		parser.nextTag();

		parser.require(XmlPullParser.START_TAG, null, "opencv_storage");

		while (parser.next() != XmlPullParser.END_TAG) {
			if (parser.getEventType() != XmlPullParser.START_TAG) {
				continue;
			}

			if (parser.getName().equals("camera_matrix")) {
				// Log.i(TAG, "found camera_matrix");
				parser.require(XmlPullParser.START_TAG, null, "camera_matrix");

				while (parser.next() != XmlPullParser.END_TAG) {
					if (parser.getEventType() != XmlPullParser.START_TAG) {
						continue;
					}

					// Log.i(TAG, "name: " + parser.getName());
					if (parser.getName().equals("data")) {
						parser.require(XmlPullParser.START_TAG, null, "data");

						if (parser.next() == XmlPullParser.TEXT) {
							return mat33FromText(parser.getText());
						}
					} else {
						skipXml(parser);
					}
				}
			} else {
				skipXml(parser);
			}
		}

		return null;
	}

	private static void skipXml(XmlPullParser parser)
			throws XmlPullParserException, IOException {
		if (parser.getEventType() != XmlPullParser.START_TAG) {
			throw new IllegalStateException();
		}
		int depth = 1;
		while (depth != 0) {
			switch (parser.next()) {
			case XmlPullParser.END_TAG:
				depth--;
				break;
			case XmlPullParser.START_TAG:
				depth++;
				break;
			}
		}
	}

	private static Mat mat33FromText(String text) {
		// Log.i(TAG, "text to parse: " + text);

		Mat m = new Mat(3, 3, CvType.CV_32F);

		String[] elems = text.split(" ");
		int i = 0;
		float val = 0.0f;
		for (String elem : elems) {
			if (elem.trim().length() <= 0)
				continue;

			try {
				val = (float) Double.parseDouble(elem);
			} catch (NumberFormatException nfe) {
				continue;
			}

			m.put(i / 3, i % 3, val);

			// Log.i(TAG, "cam[" + (i / 3) + "][" + (i % 3) + "]=" + val);

			i++;
		}

		return m;
	}

	public static String loadStringsFromFile(String file) {
		StringBuilder strBuilder = new StringBuilder();
		try {
			// read the vertex shader source from file
			InputStream inputStream = ctx.getResources().getAssets().open(file);
			BufferedReader in = new BufferedReader(new InputStreamReader(inputStream));
			String line = in.readLine();
			while (line != null) {
				strBuilder.append(line).append("\n");
				line = in.readLine();
			}
			in.close();
		} catch (IOException e) {
			Log.e(TAG, "Error reading text file: " + e.getLocalizedMessage());
		}

		return strBuilder.toString();
	}
}
