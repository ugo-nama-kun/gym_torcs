/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.behaviour;

import de.janquadflieg.mrracer.telemetry.ModifiableAction;
import de.janquadflieg.mrracer.telemetry.SensorData;

import java.util.Properties;

/**
 *
 * @author quad
 */
public class ClutchMulti
implements Behaviour{

    private Clutch gear1 = new Clutch(0.0, 60.0);

    private Clutch gear2 = new Clutch(40.0, 130.0);

    public static final String GEAR1 = "-ClutchMulti.1st-";

    public static final String GEAR2 = "-ClutchMulti.2st-";

    private static final double MIN_SPEED = 0.0;

    private static final double MAX_SPEED = 130.0;

    @Override
    public void execute(final SensorData data, ModifiableAction action){
        action.setClutch(0.0);

        if(data.getSpeed() < MIN_SPEED || data.getSpeed() > MAX_SPEED){
            return;
        }

        if(data.getGear() == 1){
            gear1.execute(data, action);

        } else if(data.getGear() == 2){
            gear2.execute(data, action);
        }        
    }

    @Override
    public void setParameters(Properties params, String prefix){
        gear1.setParameters(params, prefix+GEAR1);
        gear2.setParameters(params, prefix+GEAR2);
    }

    @Override
    public void getParameters(Properties params, String prefix){
        gear1.getParameters(params, prefix+GEAR1);
        gear2.getParameters(params, prefix+GEAR2);
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
