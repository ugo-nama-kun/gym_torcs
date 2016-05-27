/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.telemetry;

import champ2011client.SensorModel;

/**
 *
 * @author Jan Quadflieg
 */
public class ModifiableSensorData
extends SensorData{

    public ModifiableSensorData(){
        super();
    }

    public ModifiableSensorData(SensorModel m){
        super(m);
    }

    public ModifiableSensorData(String s, final String DELIMITER)
            throws Exception {
        super(s, DELIMITER);
    }

    public void setData(SensorData m){
        this.speed = m.getSpeed();
        this.latSpeed = m.getLateralSpeed();
        this.lapDistance = m.getDistanceFromStartLine();        
        this.lapTime = m.getCurrentLapTime();
        this.lastLapTime = m.getLastLapTime();
        this.raceDistance = m.getDistanceRaced();
        this.rpm = m.getRPM();
        this.trackpos = m.getTrackPosition();
        this.trackangle = m.getAngleToTrackAxis();
        this.racePos = m.getRacePosition();
        this.gear = m.getGear();
        this.timestamp = m.timestamp;
        this.damage = m.getDamage();
        this.fuel = m.getFuelLevel();
        this.trackHeight = m.getTrackHeight();
        this.zSpeed = m.getZSpeed();
        System.arraycopy(m.getTrackEdgeSensors(), 0, trackSensors, 0, trackSensors.length);
        System.arraycopy(m.getOpponentSensors(), 0, opponentSensors, 0, opponentSensors.length);
        System.arraycopy(m.getWheelSpinVelocity(), 0, wheelspin, 0, wheelspin.length);
        System.arraycopy(m.getFocusSensors(), 0, focusSensors, 0, focusSensors.length);
    }

    public void setDistanceFromStartline(double d){
        this.lapDistance = d;
    }

    public void setLastLapTime(double d){
        this.lastLapTime = d;
    }

    public void setRaceDistance(double d){
        this.raceDistance = d;
    }

    public void setTrackPosition(double d){
        this.trackpos = d;
    }

    public void setAngleToTrackAxis(double d){
        this.trackangle = d;
    }

    public void setTrackEdgeSensors(double[] d){
        System.arraycopy(d, 0, this.trackSensors, 0, this.trackSensors.length);
    }

    public void setOpponentSensors(double[] d){
        System.arraycopy(d, 0, this.opponentSensors, 0, this.opponentSensors.length);
    }
}