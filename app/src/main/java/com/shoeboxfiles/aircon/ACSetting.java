package com.shoeboxfiles.aircon;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * Created by chook on 7/10/17.
 */

public class ACSetting {

    //TODO: add members, getters and setters, and constructor
    private boolean power;
    private String acMode;
    private String acProfile;
    private int temp;
    private int fan;

    public ACSetting(){}

    public ACSetting(boolean power, String acMode, String acProfile, int temp, int fan) {
        this.power = power;
        this.acMode = acMode;
        this.acProfile = acProfile;

        //Validate temp is between 18 & 30
        if (temp < 18){temp = 18;}
        else if(temp >30){temp = 30;}
        this.temp = temp;

        //Validate Fan is between 0 & 4
        if (fan < 0){fan = 0;}
        else if(fan >4){fan= 4;}
        this.fan = fan;
    }


    //Get string wrapper of get Json
    public String getJSONString(){
        return getJSON().toString();
    }

    public JSONObject getJSON(){

        //Check if settings object is valid
        if (!validateSetting()){
            //throw new exception();
        }

        //Create json object
        JSONObject settings = new JSONObject();

        //Populate json object
        try {
            settings.put("on",this.power);
            settings.put("mode",this.acMode);
            settings.put("profile",this.acProfile);
            settings.put("temp", this.temp);
            settings.put("fan", this.fan);

        } catch (JSONException e) {
            e.printStackTrace();
        }

        //Return json object
        return settings;

    }

    //Validates the current setting
    private boolean validateSetting(){
        boolean isValid = false;

        if (!acMode.isEmpty() && !acProfile.isEmpty() &&
                temp >=18 && temp <=30) {
            isValid = true;
        }

        return isValid;
    }


    //Getters & Setters
    public boolean isPower() {
        return power;
    }

    public void setPower(boolean power) {
        this.power = power;
    }

    public String getAcMode() {
        return acMode;
    }

    public void setAcMode(String acMode) {
        this.acMode = acMode;
    }

    public String getAcProfile() {
        return acProfile;
    }

    public void setAcProfile(String acProfile) {
        this.acProfile = acProfile;
    }

    public int getTemp() {
        return temp;
    }

    public void setTemp(int temp) {
        this.temp = temp;
    }

    public int getFan() {
        return fan;
    }

    public void setFan(int fan) {
        this.fan = fan;
    }

}
