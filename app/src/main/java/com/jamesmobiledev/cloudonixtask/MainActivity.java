package com.jamesmobiledev.cloudonixtask;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.LinearLayout;

import com.jamesmobiledev.cloudonixtask.databinding.ActivityMainBinding;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }

    private ActivityMainBinding binding;
    private Handler handler = new Handler(Looper.getMainLooper());
    private Runnable showLoadingRunnable;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        binding.btnSubmit.setOnClickListener((view) -> {
            String ipAddress = getIPAddress();
            binding.resultLayout.setVisibility(View.GONE);

            showLoadingRunnable = this::showLoading;

            new Thread(() -> {
                try {
                    handler.postDelayed(showLoadingRunnable, 3000); // If the server takes more than 3 seconds to respond, the application will display a UI element showing a “loading animation” and the text “Please wait”.
                    String response = sendIpAddress(ipAddress);
                    handler.post(() -> handleResponse(response, ipAddress));
                } catch (Exception e) {
                    e.printStackTrace();
                    handler.post(() -> {
                        binding.resultText.setText("Connection Error!");
                        binding.resultLayout.setVisibility(View.VISIBLE);
                    });
                } finally {
                    handler.post(() -> {
                        handler.removeCallbacks(showLoadingRunnable); // Remove callback if API request finishes within 3 seconds
                        hideLoading();
                    });
                }
            }).start();
        });
    }

    private void showLoading() {
        binding.loadingLayout.setVisibility(LinearLayout.VISIBLE);
        binding.resultLayout.setVisibility(LinearLayout.GONE);
    }

    private void hideLoading() {
        binding.loadingLayout.setVisibility(LinearLayout.GONE);
    }

    public native String getIPAddress();

    private void handleResponse(String response, String ipAddress) {
        try {
            JSONObject jsonResponse = new JSONObject(response);
            boolean nat = jsonResponse.getBoolean("nat");

            binding.resultText.setText("IP Address: " + ipAddress);
            binding.resultIcon.setImageResource(nat ? R.drawable.ic_green_ok : R.drawable.ic_red_not_ok);
            binding.resultLayout.setVisibility(LinearLayout.VISIBLE);
        } catch (Exception e) {
            e.printStackTrace();
            binding.resultText.setText(response);
            binding.resultIcon.setImageResource(R.drawable.ic_red_not_ok);
            binding.resultLayout.setVisibility(LinearLayout.VISIBLE);
        }
    }

    private String sendIpAddress(String ipAddress) throws Exception {
        HttpURLConnection conn = getHttpURLConnection(ipAddress);

        int code = conn.getResponseCode();
        BufferedReader br;
        if (code == 200) {
            br = new BufferedReader(new InputStreamReader(conn.getInputStream(), "utf-8"));
        } else {
            br = new BufferedReader(new InputStreamReader(conn.getErrorStream(), "utf-8"));
        }

        StringBuilder response = new StringBuilder();
        String responseLine;
        while ((responseLine = br.readLine()) != null) {
            response.append(responseLine.trim());
        }

        return response.toString();
    }

    @NonNull
    private static HttpURLConnection getHttpURLConnection(String ipAddress) throws IOException, JSONException {
        String urlString = "https://s7om3fdgbt7lcvqdnxitjmtiim0uczux.lambda-url.us-east-2.on.aws/";
        URL url = new URL(urlString);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("POST");
        conn.setRequestProperty("Content-Type", "application/json; utf-8");
        conn.setRequestProperty("Accept", "application/json");
        conn.setDoOutput(true);
        conn.setConnectTimeout(10000);
        conn.setReadTimeout(10000);

        JSONObject jsonParam = new JSONObject();
        jsonParam.put("address", ipAddress);

        try (OutputStream os = conn.getOutputStream()) {
            byte[] input = jsonParam.toString().getBytes("utf-8");
            os.write(input, 0, input.length);
        }
        return conn;
    }
}
