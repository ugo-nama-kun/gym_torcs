/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.behaviour;

import de.janquadflieg.mrracer.functions.GeneralisedLogisticFunction;
import de.janquadflieg.mrracer.telemetry.ModifiableAction;
import de.janquadflieg.mrracer.telemetry.SensorData;

import java.util.Properties;

/**
 *
 * @author quad
 */
public class Clutch
implements Behaviour{

    private GeneralisedLogisticFunction f = new GeneralisedLogisticFunction();

    public static final String F = "-Clutch.f-";

    public static final String MS = "-Clutch.maxSpeed-";

    private double MIN_SPEED = 0.0;

    private double MAX_SPEED = 84.0;

    public Clutch(){        
    }

    public Clutch(double lb, double ub){
        MIN_SPEED = Math.min(lb, ub);
        MAX_SPEED = Math.max(lb, ub);
    }

    @Override
    public void execute(final SensorData data, ModifiableAction action){
        action.setClutch(0.0);

        if(data.getSpeed() < MIN_SPEED || data.getSpeed() > MAX_SPEED){
            return;
        }

        double value = (data.getSpeed()-MIN_SPEED) / (MAX_SPEED - MIN_SPEED);
        double result = this.f.getMirroredValue(value);

        action.setClutch(result);
    }

    @Override
    public void setParameters(Properties params, String prefix){
        f.setParameters(params, prefix+F);
        MAX_SPEED = Double.parseDouble(params.getProperty(prefix+MS, String.valueOf(MAX_SPEED)));
    }

    @Override
    public void getParameters(Properties params, String prefix){
        f.getParameters(params, prefix+F);
        params.setProperty(prefix+MS, String.valueOf(MAX_SPEED));
    }

    /**
     * The current situation of the car, as classified by an
     * appropriate classifier chosen by the controller which uses this behaviour.
     */
    @Override
    public void setSituation(de.janquadflieg.mrracer.classification.Situation s){        
    }

    @Override
    public void reset(){
    }

    @Override
    public void shutdown(){
        
    }    
}
