/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.telemetry;

import de.janquadflieg.mrracer.Utils;

import champ2011client.SensorModel;

import java.awt.geom.Point2D;
import java.util.StringTokenizer;

/**
 *
 * @author Jan Quadflieg
 */
public class SensorData {

    /** 19 Track sensors. */
    protected double[] trackSensors = new double[19];
    /** 18 Opponent sensors. */
    protected double[] opponentSensors = new double[36];
    /** Lap time. */
    protected double lapTime = Utils.NO_DATA_D;
    /** Last lap time. */
    protected double lastLapTime = Utils.NO_DATA_D;
    /** Speed. */
    protected double speed = Utils.NO_DATA_D;
    /** Lateral speed. */
    protected double latSpeed = Utils.NO_DATA_D;
    /** Distance from start line in the current lap. */
    protected double lapDistance = Utils.NO_DATA_D;
    /** Overall distance raced. */
    protected double raceDistance = Utils.NO_DATA_D;
    /** Engine revs. */
    protected double rpm = Utils.NO_DATA_D;
    /** Track position. */
    protected double trackpos = Utils.NO_DATA_D;
    /** Angle to track axis. */
    protected double trackangle = Utils.NO_DATA_D;
    /** Race position. */
    protected int racePos = Utils.NO_DATA_I;
    /** Gear. */
    protected int gear = Utils.NO_DATA_I;
    /** Damage of the car. */
    protected double damage = Utils.NO_DATA_D;
    /** Fuel level. */
    protected double fuel = Utils.NO_DATA_D;
    /** Wheelspin. */
    protected double[] wheelspin = new double[4];
    /** Time stamp of this readout. */
    protected long timestamp = Utils.NO_DATA_L;
    /** Focus sensors. */
    protected double[] focusSensors = new double[5];
    /** Height of the track. */
    protected double trackHeight = Utils.NO_DATA_D;
    /** Speed in z direction. */
    protected double zSpeed = Utils.NO_DATA_D;

    public SensorData() {
        for (int i = 0; i < trackSensors.length; ++i) {
            trackSensors[i] = Utils.NO_DATA_D;
        }

        for (int i = 0; i < opponentSensors.length; ++i) {
            opponentSensors[i] = Utils.NO_DATA_D;
        }

        for (int i = 0; i < wheelspin.length; ++i) {
            wheelspin[i] = Utils.NO_DATA_D;
        }
        java.util.Arrays.fill(focusSensors, Utils.NO_DATA_D);
    }

    public SensorData(SensorModel m) {
        this.speed = m.getSpeed();
        this.latSpeed = m.getLateralSpeed();
        this.lapDistance = Math.max(m.getDistanceFromStartLine(), 0.0);
        this.lapTime = m.getCurrentLapTime();
        this.lastLapTime = m.getLastLapTime();
        this.raceDistance = m.getDistanceRaced();
        this.rpm = m.getRPM();
        this.trackpos = m.getTrackPosition();
        this.trackangle = m.getAngleToTrackAxis();
        this.racePos = m.getRacePosition();
        this.gear = m.getGear();
        this.timestamp = System.nanoTime();
        this.damage = m.getDamage();
        this.fuel = m.getFuelLevel();
        this.trackHeight = m.getZ();
        this.zSpeed = m.getZSpeed();
        System.arraycopy(m.getTrackEdgeSensors(), 0, trackSensors, 0, trackSensors.length);
        System.arraycopy(m.getOpponentSensors(), 0, opponentSensors, 0, opponentSensors.length);
        System.arraycopy(m.getWheelSpinVelocity(), 0, wheelspin, 0, wheelspin.length);
        System.arraycopy(m.getFocusSensors(), 0, focusSensors, 0, focusSensors.length);
        filterWrong();
    }

    public SensorData(String s, final String DELIMITER)
            throws Exception {
        StringTokenizer t = new StringTokenizer(s, DELIMITER);

        // race / lap / time info
        raceDistance = Double.parseDouble(t.nextToken());
        lapDistance = Double.parseDouble(t.nextToken());
        lapTime = Double.parseDouble(t.nextToken());
        racePos = Integer.parseInt(t.nextToken());

        // engine: rpm / gear / speed / lateral speed
        rpm = Double.parseDouble(t.nextToken());
        gear = Integer.parseInt(t.nextToken());
        speed = Double.parseDouble(t.nextToken());
        latSpeed = Double.parseDouble(t.nextToken());

        // track position: track position / angle to track axis / track sensor
        trackpos = Double.parseDouble(t.nextToken());
        trackangle = Double.parseDouble(t.nextToken());
        for (int i = 0; i < trackSensors.length; ++i) {
            trackSensors[i] = Double.parseDouble(t.nextToken());
        }

        // car: damage / fuel / wheelspin sensors
        damage = Double.parseDouble(t.nextToken());
        fuel = Double.parseDouble(t.nextToken());
        for (int i = 0; i < wheelspin.length; ++i) {
            wheelspin[i] = Double.parseDouble(t.nextToken());
        }

        // opponent sensor
        for (int i = 0; i < opponentSensors.length; ++i) {
            if (i >= 18 && !t.hasMoreTokens()) {
                opponentSensors[i] = 100.0;
            } else {
                opponentSensors[i] = Double.parseDouble(t.nextToken());
            }
        }

        // focus sensor
        for(int i = 0; i < focusSensors.length; ++i){
            if(t.hasMoreElements()){
                focusSensors[i] = Double.parseDouble(t.nextToken());
            } else{
                focusSensors[i] = -1.0;
            }            
        }

        if(t.hasMoreElements()){
            this.trackHeight = Double.parseDouble(t.nextToken());
        } else {
            this.trackHeight = 0.0;
        }

        if(t.hasMoreElements()){
            this.zSpeed = Double.parseDouble(t.nextToken());
        } else {
            this.zSpeed = 0.0;
        }
    }

    public void filterWrongNoisy(){
        if (Math.toDegrees(trackangle) < -25.0 || Math.toDegrees(trackangle) > 25.0) {
            return;
        }

        //StringBuilder log = new StringBuilder(Utils.dTS(lapDistance)+"/"+getTrackPositionS()+"-Warning, buggy sensor:");
//        boolean replaced = false;
        for (int i = 0; i < 2; ++i) {
            if (trackSensors[i] > 100.0) {
                double replacement = -1.0;
//                int replaceIndex = 0;
                for (int k = i + 1; k < 5 && replacement == -1.0; ++k) {
                    if (trackSensors[k] < 100.0) {
                        replacement = trackSensors[k];
//                        replaceIndex = k;
                    }
                }
                trackSensors[i] = replacement;
//                log.append(" [");
//                log.append(i);
//                log.append("] -> ");
//                log.append(replaceIndex);
//                log.append("/");
//                log.append(Utils.dTS(replacement));
//                replaced = true;
            }
        }

        for (int i = trackSensors.length - 1; i > 16; --i) {
            if (trackSensors[i] > 100.0) {
                double replacement = -1.0;
//                int replaceIndex = 0;
                for (int k = i - 1; k > 13 && replacement == -1.0; --k) {
                    if (trackSensors[k] < 100.0) {
                        replacement = trackSensors[k];
//                        replaceIndex = k;
                    }
                }
                trackSensors[i] = replacement;
//                log.append(" [");
//                log.append(i);
//                log.append("] -> ");
//                log.append(replaceIndex);
//                log.append("/");
//                log.append(Utils.dTS(replacement));
//                replaced = true;
            }
        }

//        if (replaced) {
//            System.out.println(log.toString());
//        }
    }

    private void filterWrong() {
        if (Math.toDegrees(trackangle) < -25.0 || Math.toDegrees(trackangle) > 25.0) {
            return;
        }

//        StringBuilder log = new StringBuilder(Utils.dTS(lapDistance)+"/"+getTrackPositionS()+"-Warning, buggy sensor:");
//        boolean replaced = false;
        for (int i = 0; i < 4; ++i) {
            if (trackSensors[i] == 200.0) {
                double replacement = -1.0;
//                int replaceIndex = 0;
                for (int k = i + 1; k < 5 && replacement == -1.0; ++k) {
                    if (trackSensors[k] != 200.0) {
                        replacement = trackSensors[k];
//                        replaceIndex = k;
                    }
                }
                trackSensors[i] = replacement;
//                log.append(" [");
//                log.append(i);
//                log.append("] -> ");
//                log.append(replaceIndex);
//                log.append("/");
//                log.append(Utils.dTS(replacement));
//                replaced = true;
            }
        }

        for (int i = trackSensors.length - 1; i > 14; --i) {
            if (trackSensors[i] == 200.0) {
                double replacement = -1.0;
//                int replaceIndex = 0;
                for (int k = i - 1; k > 13 && replacement == -1.0; --k) {
                    if (trackSensors[k] != 200.0) {
                        replacement = trackSensors[k];
//                        replaceIndex = k;
                    }
                }
                trackSensors[i] = replacement;
//                log.append(" [");
//                log.append(i);
//                log.append("] -> ");
//                log.append(replaceIndex);
//                log.append("/");
//                log.append(Utils.dTS(replacement));
//                replaced = true;
            }
        }

//        if (replaced) {
//            System.out.println(log.toString());
//        }
    }

    public boolean focusAvailable(){
        boolean result = true;

        for(int i=0; i < focusSensors.length && result; ++i){
            result &= focusSensors[i] != -1;
        }

        return result;
    }

    public double getAngleToTrackAxis() {
        return this.trackangle;
    }

    public String getAngleToTrackAxisS() {
        return Utils.dTS(Math.toDegrees(trackangle));
    }

    public double getCurrentLapTime() {
        return this.lapTime;
    }

    public String getCurrentLapTimeS() {
        return Utils.dTS(lapTime);
    }

    public double getLastLapTime(){
        return this.lastLapTime;
    }

    public double getDamage() {
        return damage;
    }

    public String getDamageS() {
        return Utils.dTS(damage);
    }

    public double getDistanceFromStartLine() {
        return this.lapDistance;
    }

    public String getDistanceFromStartLineS() {
        return Utils.dTS(lapDistance);
    }

    public double getDistanceRaced() {
        return this.raceDistance;
    }

    public String getDistanceRacedS() {
        return Utils.dTS(raceDistance);
    }

    public double[] getFocusSensors(){
        return this.focusSensors;
    }

    public double getFuelLevel() {
        return fuel;
    }

    public String getFuelLevelS() {
        return Utils.dTS(fuel);
    }

    public int getGear() {
        return gear;
    }

    public String getGearS() {
        return Utils.iTS(gear);
    }

    public double getLateralSpeed() {
        return this.latSpeed;
    }

    public String getLateralSpeedS() {
        return Utils.dTS(latSpeed);
    }

    public double[] getOpponentSensors() {
        return opponentSensors;
    }

    public int getRacePosition() {
        return this.racePos;
    }

    public String getRacePositionS() {
        return Utils.iTS(racePos);
    }

    public double getRPM() {
        return rpm;
    }

    public String getRPMS() {
        return Utils.dTS(rpm);
    }

    public SensorModel getSensorModel(){
        StringBuilder sb = new StringBuilder();

        sb.append("(angle ").append(trackangle).append(")");
        sb.append("(curLapTime ").append(lapTime).append(")");
        sb.append("(damage ").append(damage).append(")");
        sb.append("(distFromStart ").append(lapDistance).append(")");
        sb.append("(distRaced ").append(raceDistance).append(")");
        sb.append("(fuel ").append(fuel).append(")");
        sb.append("(gear ").append(gear).append(")");
        if(lastLapTime == Utils.NO_DATA_D){
            sb.append("(lastLapTime 0)");
        } else {
            sb.append("(lastLapTime ").append(lastLapTime).append(")");
        }
        sb.append("(opponents");
        for(int i=0; i < opponentSensors.length; ++i){
            sb.append(" ").append(opponentSensors[i]);
        }
        sb.append(")");
        sb.append("(racePos ").append(racePos).append(")");
        sb.append("(rpm ").append(rpm).append(")");
        sb.append("(speedX ").append(speed).append(")");
        sb.append("(speedY ").append(latSpeed).append(")");
        sb.append("(speedZ ").append(zSpeed).append(")");
        sb.append("(track");
        for(int i=0; i < trackSensors.length; ++i){
            sb.append(" ").append(trackSensors[i]);
        }
        sb.append(")");
        sb.append("(trackPos ").append(trackpos).append(")");
        sb.append("(wheelSpinVel");
        for(int i=0; i < wheelspin.length; ++i){
            sb.append(" ").append(wheelspin[i]);
        }
        sb.append(")");
        sb.append("(z ").append(trackHeight).append(")");
        sb.append("(focus");
        for(int i=0; i < focusSensors.length; ++i){
            sb.append(" ").append(focusSensors[i]);
        }
        sb.append(")");

        //System.out.println(sb.toString());

        return new champ2011client.MessageBasedSensorModel(sb.toString());       
    }

    public double getSpeed() {
        return this.speed;
    }

    public String getSpeedS() {
        return Utils.dTS(speed);
    }

    public long getTimeStamp() {
        return this.timestamp;
    }

    public String getTimeStampS() {
        return String.valueOf(this.timestamp);
    }

    public double[] getTrackEdgeSensors() {
        return this.trackSensors;
    }

    public double getTrackHeight(){
        return this.trackHeight;
    }

    public String getTrackHeightS(){
        return Utils.dTS(trackHeight);
    }

    public double getTrackPosition() {
        return this.trackpos;
    }

    public String getTrackPositionS() {
        return Utils.dTS(this.trackpos);
    }

    public double[] getWheelSpinVelocity() {
        return wheelspin;
    }

    public double getZSpeed(){
        return zSpeed;
    }

    public String getZSpeedS(){
        return Utils.dTS(this.zSpeed);
    }

    public static Point2D[] calculateTrackPoints(SensorData data) {
        Point2D[] result = new Point2D[data.trackSensors.length];

        //double carAngleD = Math.toDegrees(data.getAngleToTrackAxis());

        for (int i = 0; i < result.length; ++i) {
            double sensorAngleD = (i * 10.0);// + carAngleD;
            double x = -Math.cos(Math.toRadians(sensorAngleD)) * data.trackSensors[i];
            double y = Math.sin(Math.toRadians(sensorAngleD)) * data.trackSensors[i];

            result[i] = new Point2D.Double(x, y);
        }

        return result;
    }

    public static Point2D[] calculateTrackPointsWithCarDir(SensorData data, float[] angles) {
        Point2D[] result = new Point2D[data.trackSensors.length];

        double carAngleD = Math.toDegrees(data.getAngleToTrackAxis());

        for (int i = 0; i < result.length; ++i) {
            double sensorAngleD = angles[i] - carAngleD;
            double x = Math.sin(Math.toRadians(sensorAngleD)) * data.trackSensors[i];
            double y = Math.cos(Math.toRadians(sensorAngleD)) * data.trackSensors[i];

            result[i] = new Point2D.Double(x, y);
        }

        return result;
    }

    public static Point2D[] calculateTrackPoints(SensorData data, float[] angles) {
        Point2D[] result = new Point2D[data.trackSensors.length];

        //double carAngleD = Math.toDegrees(data.getAngleToTrackAxis());

        for (int i = 0; i < result.length; ++i) {
            double sensorAngleD = angles[i];// + carAngleD;
            double x = Math.sin(Math.toRadians(sensorAngleD)) * data.trackSensors[i];
            double y = Math.cos(Math.toRadians(sensorAngleD)) * data.trackSensors[i];

            result[i] = new Point2D.Double(x, y);
        }

        return result;
    }

    /**
     * Returns the index of the track sensor with the highest value, starting
     * the search with the leftmost sensor.
     *
     * @param data A SensorData object
     * @return
     */
    public static int maxTrackIndexLeft(SensorData data) {
        double max = Double.NEGATIVE_INFINITY;
        int result = -1;
        for (int i = 0; i < data.trackSensors.length; ++i) {
            if (data.trackSensors[i] > max) {
                max = data.trackSensors[i];
                result = i;
            }
        }

        return result;
    }

    /**
     * Calculates the relative position of the car on the track according to
     * the championship sensormodel, from the given absolute track position
     * in meter, with
     * 0m at the left track edge and trackwidth m at the right track edge.
     * @param trackWidth
     * @return
     */
    public static double calcRelativeTrackPosition(double position, double trackWidth){
        //return  position = (trackPos-1.0) * -(trackWidth/2.0);
        //return  position / -(trackWidth/2.0) = trackPos-1.0;
        //return  (position / -(trackWidth/2.0)) + 1.0 = trackPos;

        return (position / -(trackWidth/2.0)) + 1.0;
    }

    /**
     * Calculates the absolute position of the car in the track in meter, with
     * 0m at the left track edge and trackwidth m at the right track edge.
     * @param trackWidth
     * @return
     */
    public double calcAbsoluteTrackPosition(double trackWidth){
        return calcAbsoluteTrackPosition(trackpos, trackWidth);
    }

    /**
     * Calculates the absolute position of the car in the track in meter, with
     * 0m at the left track edge and trackwidth m at the right track edge.
     * @param trackWidth
     * @return
     */
    public static double calcAbsoluteTrackPosition(double trackPos, double trackWidth){
        return (trackPos-1.0) * -(trackWidth/2.0);
    }

    public boolean onTrack() {
        return (trackpos >= -1 && trackpos <= 1.0);
    }

    /**
     * Returns the index of the track sensor with the highest value, starting
     * the search with the rightmost sensor.
     *
     * @param data A SensorData object
     * @return
     */
    public static int maxTrackIndexRight(SensorData data) {
        double max = Double.NEGATIVE_INFINITY;
        int result = -1;
        for (int i = data.trackSensors.length - 1; i >= 0; --i) {
            if (data.trackSensors[i] > max) {
                max = data.trackSensors[i];
                result = i;
            }
        }

        return result;
    }

    public void write(java.io.OutputStreamWriter w)
            throws Exception {
        write(w, " ");
    }

    public void write(java.io.OutputStreamWriter w, final String DELIMITER)
            throws Exception {
        // race / lap / time info
        w.write(String.valueOf(getDistanceRaced()) + DELIMITER);
        w.write(String.valueOf(getDistanceFromStartLine()) + DELIMITER);
        w.write(String.valueOf(getCurrentLapTime()) + DELIMITER);
        w.write(String.valueOf(getRacePosition()) + DELIMITER);

        // engine: rpm / gear / speed / lateral speed
        w.write(String.valueOf(getRPM()) + DELIMITER);
        w.write(String.valueOf(getGear()) + DELIMITER);
        w.write(String.valueOf(getSpeed()) + DELIMITER);
        w.write(String.valueOf(getLateralSpeed()) + DELIMITER);

        // track position: track position / angle to track axis / track sensor
        w.write(String.valueOf(getTrackPosition()) + DELIMITER);
        w.write(String.valueOf(getAngleToTrackAxis()) + DELIMITER);
        for (int i = 0; i < trackSensors.length; ++i) {
            w.write(String.valueOf(trackSensors[i]) + DELIMITER);
        }

        // car: damage / fuel / wheelspin sensors
        w.write(String.valueOf(getDamage()) + DELIMITER);
        w.write(String.valueOf(getFuelLevel()) + DELIMITER);
        for (int i = 0; i < wheelspin.length; ++i) {
            w.write(String.valueOf(wheelspin[i]) + DELIMITER);
        }

        // opponent sensor
        for (int i = 0; i < opponentSensors.length; ++i) {
            w.write(String.valueOf(opponentSensors[i]) + DELIMITER);
        }

        // focus sensor
        for(int i=0; i < focusSensors.length; ++i){
            w.write(String.valueOf(focusSensors[i]) + DELIMITER);
        }

        // track height / z speed
        w.write(String.valueOf(trackHeight) + DELIMITER);
        w.write(String.valueOf(zSpeed));
    }

    public static void writeHeader(java.io.OutputStreamWriter w)
            throws Exception {
        SensorData.writeHeader(w, " ");
    }

    public static void writeHeader(java.io.OutputStreamWriter w, final String DELIMITER)
            throws Exception {
        w.write("RaceD" + DELIMITER + "DfSL" + DELIMITER + "CLT" + DELIMITER + "RP" +
                DELIMITER + "RPM" + DELIMITER + "Gear" + DELIMITER + "Speed" + DELIMITER +
                "LSpeed" + DELIMITER);
        w.write("TrackP" + DELIMITER + "AtoTA" + DELIMITER);
        for (int i = 0; i < 19; ++i) {
            w.write("tes" + i + DELIMITER);
        }
        w.write("Dam" + DELIMITER + "Fuel" + DELIMITER);
        for (int i = 0; i < 4; ++i) {
            w.write("ws" + i + DELIMITER);
        }

        for (int i = 0; i < 36; ++i) {
            w.write("os" + i);
            w.write(DELIMITER);
        }

        for (int i = 0; i < 5; ++i) {
            w.write("fs" + i);
            w.write(DELIMITER);
        }

        w.write("z" + DELIMITER + "zSpeed");
    }

    public boolean read(java.io.OutputStreamWriter w) {
        boolean result = false;

        return result;
    }

    public static void main(String[] args){
        for(double i = 1.0; i >= -1.0; i=i-0.1){
            System.out.println(i+" = "+SensorData.calcAbsoluteTrackPosition(i, 12.0));
        }
    }
}
