/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.behaviour;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.telemetry.ModifiableAction;
import de.janquadflieg.mrracer.telemetry.SensorData;

import java.util.Properties;

/**
 *
 * @author quad
 */
public class ClutchAutopia
implements Behaviour{

    private double clutch = 0.0;

    @Override
    public void execute(final SensorData data, ModifiableAction action){
        final double clutchMax = 0.5;
        final double clutchDeltaTime = 0.02;
        final double clutchDeltaRaced = 10;
        final double clutchDelta = 0.05;
        final double clutchMaxModifier = 1.3;
        final double clutchMaxTime = 1.5;
        final double clutchDec = 0.01;

        double maxClutch = clutchMax;

        // Check if the current situation is the race start
        if (data.getCurrentLapTime() < clutchDeltaTime && data.getDistanceRaced() < clutchDeltaRaced){
            clutch = maxClutch;
        }

        // Adjust the current value of the clutch
        if(clutch > 0){
            double delta = clutchDelta;

            if(data.getGear() < 2){
                // Apply a stronger clutch output when the gear is one and the race is just started
                delta /= 2;
                maxClutch *= clutchMaxModifier;
                if (data.getCurrentLapTime() < clutchMaxTime){
                    clutch = maxClutch;
                }
            }

            // check clutch is not bigger than maximum values
            clutch = Math.min(maxClutch, clutch);

            // if clutch is not at max value decrease it quite quickly
            if (clutch != maxClutch){
                clutch -= delta;
                clutch = Math.max(0.0, clutch);

            } else {
                // if clutch is at max value decrease it very slowly
		clutch -= clutchDec;
            }
        }
        action.setClutch(clutch);

        /*if(clutch > 0.0){
            System.out.println(data.getSpeed()+" "+clutch);
        }*/
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
        clutch = 0.0;
    }

    @Override
    public void shutdown(){
        
    }    
}
