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
public class ClutchConstant
implements Behaviour{    

    @Override
    public void execute(final SensorData data, ModifiableAction action){
        action.setClutch(0.0);        
    }

    @Override
    public void setParameters(Properties params, String prefix){       

    }

    @Override
    public void getParameters(Properties params, String prefix){        
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
