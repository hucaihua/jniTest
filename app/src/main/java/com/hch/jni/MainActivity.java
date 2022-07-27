package com.hch.jni;

import android.os.Bundle;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.hch.jni.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'jni' library on application startup.
    static {
        System.loadLibrary("jni");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(check());

        setAntiBiBCallback(new IAntiDebugCallback() {
            @Override
            public void beInjectedDebug() {

            }
        });
        boolean result = init();
        if (!result){
            Toast.makeText(this , "盗版软件",Toast.LENGTH_LONG).show();
        }
    }



    /**
     * A native method that is implemented by the 'jni' native library,
     * which is packaged with this application.
     */
    public static native String check();

    public native void setAntiBiBCallback(IAntiDebugCallback callback);

    interface IAntiDebugCallback {
        void beInjectedDebug();
    }

    public static native boolean init();
}