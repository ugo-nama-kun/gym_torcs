/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.telemetry;

import de.janquadflieg.mrracer.Utils;

import java.util.StringTokenizer;

/**
 *
 * @author quad
 */
public class Action {

    protected double accelerate = Utils.NO_DATA_D;
    protected double brake = Utils.NO_DATA_D;
    protected int gear = Utils.NO_DATA_I;
    protected boolean restartRace = false;
    protected double steering = Utils.NO_DATA_D;
    protected double clutch = 0.0;
    protected int focusAngle = 0;

    public Action() {
    }

    public Action(String s, final String DELIMITER)
            throws Exception {
        StringTokenizer t = new StringTokenizer(s, DELIMITER);

        accelerate = Double.parseDouble(t.nextToken());
        brake = Double.parseDouble(t.nextToken());
        steering = Double.parseDouble(t.nextToken());
        gear = Integer.parseInt(t.nextToken());
        restartRace = Boolean.parseBoolean(t.nextToken());
        if(t.hasMoreTokens()){
            clutch = Double.parseDouble(t.nextToken());
        }
        if(t.hasMoreTokens()){
            focusAngle = Integer.parseInt(t.nextToken());
        }
    }

//    public Action(champ2009client.Action action){
//        this.accelerate = action.accelerate;
//        this.brake = action.brake;
//        this.steering = action.steering;
//        this.brake = action.brake;
//        this.restartRace = action.restartRace;
//    }
    public champ2011client.Action getRaceClientAction() {
        champ2011client.Action result = new champ2011client.Action();

        result.accelerate = this.accelerate;
        result.brake = this.brake;
        result.steering = this.steering;
        result.gear = this.gear;
        result.restartRace = this.restartRace;
        result.clutch = this.clutch;
        result.focus = this.focusAngle;

        result.limitValues();

        return result;
    }

    public double getAcceleration() {
        return accelerate;
    }

    public String getAccelerationS() {
        return Utils.dTS(accelerate);
    }

    public double getBrake() {
        return brake;
    }

    public String getBrakeS() {
        return Utils.dTS(brake);
    }

    public double getClutch(){
        return clutch;
    }

    public String getClutchS(){
        return Utils.dTS(clutch);
    }

    public int getFocusAngle(){
        return this.focusAngle;
    }

    public String getFocusAngleS(){
        return Utils.iTS(focusAngle);
    }

    public double getSteering() {
        return steering;
    }

    public String getSteeringS() {
        return Utils.dTS(steering);
    }

    public int getGear() {
        return gear;
    }

    public String getGearS() {
        return Utils.iTS(gear);
    }

    public boolean restartRace() {
        return restartRace;
    }

    public static void writeHeader(java.io.OutputStreamWriter w)
            throws Exception {
        Action.writeHeader(w, " ");
    }

    public static void writeHeader(java.io.OutputStreamWriter w, final String DELIMITER)
            throws Exception {
        w.write("Acc" + DELIMITER + "Brake" + DELIMITER);
        w.write("Steer" + DELIMITER + "Gear" + DELIMITER);
        w.write("Restart" + DELIMITER + "Clutch" + DELIMITER + "FocusAngle");
    }

    public void write(java.io.OutputStreamWriter w)
            throws Exception {
        write(w, " ");
    }

    public void write(java.io.OutputStreamWriter w, final String DELIMITER)
            throws Exception {
        w.write(String.valueOf(accelerate) + DELIMITER);
        w.write(String.valueOf(brake) + DELIMITER);
        w.write(String.valueOf(steering) + DELIMITER);
        w.write(String.valueOf(gear) + DELIMITER);
        w.write(String.valueOf(restartRace)+ DELIMITER);
        w.write(String.valueOf(clutch) + DELIMITER);
        w.write(String.valueOf(focusAngle));
    }

    @Override
    public String toString(){
        return "Action: a="+Utils.dTS(accelerate)+", b="+Utils.dTS(brake)+", s="+
                Utils.dTS(steering)+", g="+gear+", r="+this.restartRace+
                ", c="+Utils.dTS(clutch)+",  fA="+this.focusAngle;
    }
}
