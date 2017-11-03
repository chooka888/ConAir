package com.shoeboxfiles.aircon;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.NumberPicker;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.io.UnsupportedEncodingException;

public class ACController extends AppCompatActivity {

    //class variables
    private MqttAndroidClient client;
    private static final String TAG = "AIRCON";

    //Controls
    Spinner spAircon;
    Switch swOnOff;
    Spinner spMode;
    Spinner spProfile;
    NumberPicker npTemp;
    NumberPicker npFan;
    TextView tvMessage;

    //State variables
    boolean spModeInitialSelection = true;
    boolean spProfileInitialSelection = true;
    boolean activityLoaded = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_accontroller);

        //Get a reference to all the controls
        spAircon = (Spinner) findViewById(R.id.spAircon);
        swOnOff = (Switch) findViewById(R.id.swACOnOff);
        spMode = (Spinner) findViewById(R.id.spMode);
        spProfile = (Spinner) findViewById(R.id.spProfile);
        npTemp = (NumberPicker) findViewById(R.id.npTemp);
        npFan = (NumberPicker) findViewById(R.id.npFan);
        tvMessage = (TextView) findViewById(R.id.tvMessage);


        //TODO: later use last MQTT message to set the start values for now...
        //find controls and set defaults and attach handlers
// Create an ArrayAdapter using the string array and a default spinner layout
        ArrayAdapter<CharSequence> adapter3 = ArrayAdapter.createFromResource(this,R.array.air_conns, android.R.layout.simple_spinner_item);

        // Specify the layout to use when the list of choices appears
        adapter3.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        // Apply the adapter to the spinner
        spAircon.setAdapter(adapter3);
        spAircon.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                //Check initialised flag
                if (!spModeInitialSelection) {
                    changeEventDriver();
                }else{
                    spModeInitialSelection = false;
                }

            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });


        //On/Off switch
        swOnOff.setChecked(false);
        swOnOff.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                changeEventDriver();
            }
        });

        //Mode spinner
        // Create an ArrayAdapter using the string array and a default spinner layout
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,R.array.ac_mode, android.R.layout.simple_spinner_item);

        // Specify the layout to use when the list of choices appears
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        // Apply the adapter to the spinner
        spMode.setAdapter(adapter);
        spMode.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                //Check initialised flag
                if (!spModeInitialSelection) {
                    changeEventDriver();
                }else{
                    spModeInitialSelection = false;
                }

            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        //Profile spinner
        //TODO: investingate if reusing adapter is ok
        // Create an ArrayAdapter using the string array and a default spinner layout
        ArrayAdapter<CharSequence> adapter2 = ArrayAdapter.createFromResource(this,R.array.ac_profile, android.R.layout.simple_spinner_item);

        // Specify the layout to use when the list of choices appears
        adapter2.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        // Apply the adapter to the spinner
        spProfile.setAdapter(adapter2);
        spProfile.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                //Check initialised flag
                if (!spProfileInitialSelection) {
                    changeEventDriver();
                }else{
                    spProfileInitialSelection = false;
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        //Temp number picker
        npTemp.setMinValue(18);
        npTemp.setMaxValue(30);
;

        //Default to 18
        npTemp.setValue(20);
        //Set event listener
        npTemp.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                changeEventDriver();
            }
        });

        //Fan number Picker
        npFan.setMinValue(0);
        npFan.setMaxValue(4);

        //default to o which equals auto mode
        npFan.setValue(0);

        //Set the Fan event listener
        npFan.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                changeEventDriver();
            }
        });

        //Connect to the MQTT server
        mqttConnection(getString(R.string.MQTT_CONN));

        //Set flag to allow MQTT messages to be sent
        activityLoaded = true;

    }

    @Override
    protected void onDestroy() {
        //Disconnect mqtt server
        mqttDisconnect();
        super.onDestroy();
    }

    // fast way to call Toast
    private void msg(String s)
    {
        Toast.makeText(getApplicationContext(),s,Toast.LENGTH_LONG).show();
    }

    //Capture and send change from any control
    protected void changeEventDriver(){

        //Check if the activity is loaded to avoid sending message everytime the app opens
        if (activityLoaded) {

            //Read the current settings form the form
            ACSetting acSetting = captureCurrentSetting();

            //check for selected aircons
            String selectedAC = spAircon.getSelectedItem().toString();

            //if blank do not sent
            if (!selectedAC.isEmpty()) {

                //call send MQTT message
                mqttSendMessage(selectedAC, acSetting.getJSONString());
            }else{
                msg(getString(R.string.message_selectAC));
            }
        }
    }

    //method to construct this class on change of any field
    protected ACSetting captureCurrentSetting(){
        ACSetting acSetting = new ACSetting();
        acSetting.setPower(swOnOff.isChecked());
        acSetting.setAcMode(spMode.getSelectedItem().toString());
        acSetting.setAcProfile(spProfile.getSelectedItem().toString());
        acSetting.setTemp(npTemp.getValue());
        acSetting.setFan(npFan.getValue());

        return acSetting;
    }

    //TODO: Create method to subscribe to topics

    //TODO: Create method to reload last settings


    //MQTT Helper methods

    protected void mqttConnection(String connectionString){

        String clientId = MqttClient.generateClientId();
        client = new MqttAndroidClient(this.getApplicationContext(), connectionString, clientId);

        try {
            IMqttToken token = client.connect();
            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    // We are connected
                    Log.d(TAG, "onSuccess");

                    if (client.isConnected()) {
                        //send debug message that connectin succeeded
                        mqttSendMessage("debug", "Android connected");

                        //Subscribe to topics required
                        mqttSubscribe("yeahboy", 1); //Test subscription

                        //Create listener for MQTT messages.
                        mqttCallback();
                    }
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    // Something went wrong e.g. connection timeout or firewall problems
                    Log.d(TAG, "onFailure");
                }
            });


        } catch (MqttException e) {
            e.printStackTrace();
        }

    }

    protected void mqttSendMessage(String topic, String payload){
        byte[] encodedPayload = new byte[0];
        try {
            encodedPayload = payload.getBytes("UTF-8");
            MqttMessage message = new MqttMessage(encodedPayload);
            message.setRetained(true);
            message.setQos(1);
            client.publish(topic, message);
        } catch (UnsupportedEncodingException | MqttException e) {
            e.printStackTrace();
        }
    }

    protected void mqttSubscribe(String topic, int qos){
        try {
            IMqttToken subToken = client.subscribe(topic, qos);
            subToken.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    // The message was published
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken,
                                      Throwable exception) {
                    // The subscription could not be performed, maybe the user was not
                    // authorized to subscribe on the specified topic e.g. using wildcards

                }

            });
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    protected void mqttCallback() {
        client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {

            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                TextView tvMessage = (TextView) findViewById(R.id.tvMessage);
                tvMessage.setText(message.toString());
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {

            }
        });
    }
    protected void mqttDisconnect(){
        try {
            IMqttToken disconToken = client.disconnect();
            disconToken.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    // we are now successfully disconnected
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken,
                                      Throwable exception) {
                    // something went wrong, but probably we are disconnected anyway
                }
            });
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }
}
