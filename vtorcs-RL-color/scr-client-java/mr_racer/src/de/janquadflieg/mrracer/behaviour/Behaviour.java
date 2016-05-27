/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.behaviour;

import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.telemetry.*;

import java.util.Properties;


/**
 * Interface for a behaviour. A behaviour is anything which is able to execute
 * an action concerning the control of the car. A controller can use one or more
 * behaviours to define its actions.
 *
 * @author Jan Quadflieg
 */
public interface Behaviour {    

    /**
     * Executes this behaviour.
     *
     * @param data The current sensor data.
     * @param action An action object, which gets modified by this behaviour.
     */
    public void execute(final SensorData data, ModifiableAction action);

    public void setParameters(Properties params, String prefix);
        
    public void getParameters(Properties params, String prefix);
    
    /**
     * The current situation of the car, as classified by an
     * appropriate classifier chosen by the controller which uses this behaviour.
     */
    public void setSituation(Situation s);

    public void reset();

    public void shutdown();
}
