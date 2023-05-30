package com.b1stable.threecrystals;

import java.io.File;
import java.io.FileFilter;
import java.util.Locale;
import java.nio.file.Path;
import java.nio.file.PathMatcher;
import java.nio.file.FileSystems;
import java.util.Vector;

import android.os.Bundle;
import android.os.Build;
import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Color;
import android.net.Uri;
import android.os.Vibrator;
import android.os.VibrationEffect;
import android.util.Log;
import android.view.Surface;
import android.view.Gravity;

import org.libsdl.app.SDLActivity; 

import com.b1stable.threecrystals.License_Viewer_Activity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

public class ThreeCrystals_Activity extends SDLActivity
{
	final static int LICENSE_REQUEST = 1;

	native static void resume_after_showing_license();

	// request codes we use when invoking an external activity
	private static final int RC_UNUSED = 5001;

	// This is so the screen is never cleared pure black, only shim::black (r:35, g:30, b:60)
	static boolean paused = false;

	 private static final String TAG = "3Crystals";

	@Override
	public void onCreate(Bundle savedInstance)
	{
		super.onCreate(savedInstance);
		// Create the client used to sign in to Google services.
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data)
	{
    super.onActivityResult(requestCode, resultCode, data);
		if (requestCode == LICENSE_REQUEST) {
			if (data != null) {
				if (resultCode == RESULT_OK && data.getExtras().getString("MESSAGE").equals("OK")) {
					show_license_result = 0;
				}
				else if (resultCode == RESULT_CANCELED && data.getExtras().getString("MESSAGE").equals("FAIL")) {
					show_license_result = 1;
				}
				else {
					show_license_result = 1;
				}
			}
			else {
				show_license_result = 1;
			}

			resume_after_showing_license();
		}
	}

	public void onStart() {
		super.onStart();
	}

	@Override
	public void onStop()
	{
		super.onStop();
	}
	
	@Override
	public void onRestart()
	{
		super.onRestart();
	}

	@Override
	public void onResume()
	{
		super.onResume();
	}

	@Override
	public void onPostResume()
	{
		super.onPostResume();
		paused = true;
	}

	public void logString(String s)
	{
		Log.d("3Crystals", s);
	}

	public String getAppdataDir()
	{
		return getFilesDir().getAbsolutePath();
	}
	
	public String getSDCardDir()
	{
		File f = getExternalFilesDir(null);
		if (f != null) {
			return f.getAbsolutePath();
		}
		else {
			return getFilesDir().getAbsolutePath();
		}
	}

	static int show_license_result;

	public void showLicense()
	{
		show_license_result = -1;
		Intent intent = new Intent(this, License_Viewer_Activity.class);
		startActivityForResult(intent, LICENSE_REQUEST);
	}

	public int getShowLicenseResult()
	{
		return show_license_result;
	}

	/*
	public void openURL(String url)
	{
		Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		startActivity(intent);
	}
	*/

	public void rumble(int milliseconds)
	{
		Vibrator v = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
		if (v != null && v.hasVibrator()) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
				v.vibrate(VibrationEffect.createOneShot(milliseconds, VibrationEffect.DEFAULT_AMPLITUDE));
			}
			else {
				v.vibrate(milliseconds);
			}
		}
	}

	public boolean has_touchscreen()
	{
		return getPackageManager().hasSystemFeature("android.hardware.touchscreen");
	}

	public boolean has_vibrator()
	{
		Vibrator v = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
		if (v != null) {
			return v.hasVibrator();
		}
		else {
			return false;
		}
	}

	public void start_draw()
	{
		if (paused) {
			paused = false;
		}
	}
	
	public String get_android_language()
	{
		return Locale.getDefault().getLanguage();
	}

	private static File[] list_dir_files = null;

	public void list_dir_start(String path)
	{
		try {
			int slash = path.lastIndexOf('/');
			final String glob = path.substring(slash+1).replace("*", ".*"); // +1 works even if not found (-1+1 == 0)
			String dir = path.substring(0, slash);
			File f = new File(dir);
			list_dir_files = f.listFiles(new FileFilter() {
				public boolean accept(File f)
				{
					try {
						if (f.getName().matches(glob)) {
							return true;
						}
						else {
							return false;
						}
					}
					catch (Exception e) {
						Log.d("3Crystals", "list_dir_start FileFilter throwing " + e.getMessage());
						return false;
					}
				}
			});
		}
		catch (Exception e) {
			list_dir_files = null;
			Log.d("3Crystals", "list_dir_start throwing " + e.getMessage());
		}
	}

	public String list_dir_next()
	{
		if (list_dir_files == null) {
			return "";
		}
		else if (list_dir_files.length == 0) {
			list_dir_files = null;
			return "";
		}
		else {
			File f = list_dir_files[0];
			String name = f.getName();
			if (list_dir_files.length == 1) {
				list_dir_files = null;
			}
			else {
				File[] new_list = new File[list_dir_files.length-1];
				for (int i = 1; i < list_dir_files.length; i++) {
					new_list[i-1] = list_dir_files[i];
				}
				list_dir_files = new_list;
			}
			return name;
		}
	}

	private static final String ARC_DEVICE_PATTERN = ".+_cheets|cheets_.+";

	public boolean is_chromebook()
	{
		// Google uses this, so should work?
		return Build.DEVICE != null && Build.DEVICE.matches(ARC_DEVICE_PATTERN);
	}
}
