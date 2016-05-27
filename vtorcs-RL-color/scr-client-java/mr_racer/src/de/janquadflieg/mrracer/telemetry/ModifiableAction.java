/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.telemetry;

/**
 *
 * @author Jan Quadflieg
 */
public class ModifiableAction
extends Action{
    

    public ModifiableAction(){
        super();
    }

    public void reset(){
        setAcceleration(0.0);
        setBrake(0.0);
        setClutch(0.0);
        setGear(0);
        setSteering(0.0);
        setFocusAngle(0);
        setRestartRace(false);
    }

    public void setData(Action action){
        this.accelerate = action.accelerate;
        this.brake = action.brake;
        this.gear = action.gear;
        this.restartRace = action.restartRace;
        this.steering = action.steering;
        this.clutch = action.clutch;
        this.focusAngle = action.focusAngle;
    }

    public void setData(champ2011client.Action a){
        this.accelerate = a.accelerate;
        this.brake = a.brake;
        this.gear = a.gear;
        this.restartRace = a.restartRace;
        this.steering = a.steering;
        this.clutch = a.clutch;
        this.focusAngle = a.focus;
    }

    public void limitValues(){
        accelerate = Math.max (0, Math.min (1, accelerate));
        brake = Math.max (0, Math.min (1, brake));
        steering = Math.max (-1, Math.min (1, steering));
        gear = Math.max (-1, Math.min (6, gear));
        clutch = Math.max (0, Math.min (1, clutch));
        focusAngle = Math.max (-90, Math.min (90, focusAngle));
    }

    public void setAcceleration(double d){
        this.accelerate = d;
    }

    public void setBrake(double d){
        this.brake = d;
    }

    public void setClutch(double d){
        this.clutch = d;
    }

    public void setFocusAngle(int i){
        this.focusAngle = i;
    }

    public void setGear(int i){
        this.gear = i;
    }

    public void setSteering(double d){
        this.steering = d;
    }

    public void setRestartRace(boolean b){
        this.restartRace = b;
    }    
}
