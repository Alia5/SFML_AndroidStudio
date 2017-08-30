package de.flatspotsoftware.testapp.activity;

import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.widget.PopupWindowCompat;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;
import android.widget.Toast;

import de.flatspotsoftware.testapp.CPPCallIns;
import de.flatspotsoftware.testapp.CPPCallbacks;
import sfml.com.sfml_example.R;

/**
 * Created by Alia5 on 8/30/2017.
 */

public class TestNativeActivity extends NativeActivity implements CPPCallbacks {


    /* Immersive sticky mode doesn't work properly with SFML... so we just write the stick part up ourselves....
        Additionally we can now hide the nav-bar...
     */
    Handler stickyHandler = new Handler();

    Runnable stickRunnable = new Runnable() {
        @Override
        public void run() {
            getWindow().getDecorView()
                    .setSystemUiVisibility(
                            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide nav bar
                                    | View.SYSTEM_UI_FLAG_FULLSCREEN // hide status bar
                                    | View.SYSTEM_UI_FLAG_IMMERSIVE);

            stickyHandler.postDelayed(this, 3000);
        }
    };


    private View androidOverlayView;


    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d("TEST", "Hello from JAVA!!!");
        super.onCreate(savedInstanceState);

        showOverlay();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d("onResume", "onResume");
        stickyHandler.post(stickRunnable);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stickyHandler.removeCallbacks(stickRunnable);
    }

    @Override
    public void testCall(String arg) {
        System.out.print(arg);

        if (arg.equals("MouseDidEnter"))
        {

            Log.d("Java_ACtivity", "onResumeCalled");

            return;
        }

        Log.wtf("INTERFASCE!!!",arg);

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(TestNativeActivity.this);

                builder.setMessage(CPPCallIns.getInstance().testCall("abc"));

                builder.create().show();
            }
        });

    }

    //ideally you would run this from a callback from native code... but for demo purposes
    void showOverlay()
    {
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                TestNativeActivity.this.runOnUiThread(new Runnable()  {
                    @Override
                    public void run()  {

                        final PopupWindow _popupWindow;

                        LayoutInflater layoutInflater
                                = (LayoutInflater)getBaseContext()
                                .getSystemService(LAYOUT_INFLATER_SERVICE);
                        View popupView = layoutInflater.inflate(R.layout.overlay_layout, null);
                        popupView.findViewById(R.id.testButton).setOnClickListener(new View.OnClickListener() {
                            @Override
                            public void onClick(View v) {
                                Toast.makeText(TestNativeActivity.this, "click!", Toast.LENGTH_SHORT).show();
                            }
                        });
                        _popupWindow = new PopupWindow(
                                popupView,
                                WindowManager.LayoutParams.MATCH_PARENT,
                                WindowManager.LayoutParams.MATCH_PARENT);


                        _popupWindow.setTouchable(false); //if false, touch events go through, else, the overlay is getting touch events


                        LinearLayout mainLayout = new LinearLayout(TestNativeActivity.this);
                        ViewGroup.MarginLayoutParams params = new ViewGroup.MarginLayoutParams(WindowManager.LayoutParams.MATCH_PARENT, WindowManager.LayoutParams.MATCH_PARENT);
                        params.setMargins(0, 0, 0, 0);
                        TestNativeActivity.this.setContentView(mainLayout, params);

                        // Show our UI over NativeActivity window
                        _popupWindow.showAtLocation(mainLayout, Gravity.TOP | Gravity.START, 0, 0);
                        _popupWindow.update();


                    }});
            }
        }, 2000);
    }

}
