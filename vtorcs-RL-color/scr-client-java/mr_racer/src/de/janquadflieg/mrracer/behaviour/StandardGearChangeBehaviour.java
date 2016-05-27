/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.behaviour;

import de.janquadflieg.mrracer.telemetry.*;

import java.util.ArrayList;
import java.util.HashMap;

/**
 *
 * @author quad
 */
public class StandardGearChangeBehaviour
    implements Behaviour{

    private int counter = 0;

    private String log = "";

    private static final int WAIT_TIME = 1 * 50;

    private int gear = 1;   
    
    private static final double UPSHIFT = 9200.0;
    private static final double DOWNSHIFT_2ND = 3500.0;
    private static final double DOWNSHIFT = 6000.0;    

    @Override
    public void setParameters(java.util.Properties params, String prefix){
    }

    @Override
    public void getParameters(java.util.Properties params, String prefix){
    }

    @Override
    public void execute(SensorData data, ModifiableAction action){
        if(data.getGear() != gear){
            log = "gear != data.gear, warte";

        } else if (data.getRPM() > UPSHIFT && data.getGear() < 6 && counter < 1) {
            gear = data.getGear() + 1;
            counter = WAIT_TIME;
            log = "Hochschalten "+data.getGear();

        } else if (data.getRPM() < DOWNSHIFT_2ND && data.getGear() == 2 && counter < 1) {
            gear = data.getGear() - 1;
            counter = WAIT_TIME;
            log = "Runterschalten "+data.getGear();

        } else if (data.getRPM() < DOWNSHIFT && data.getGear() > 2 && counter < 1) {
            gear = data.getGear() - 1;
            counter = WAIT_TIME;
            log = "Runterschalten "+data.getGear();

        } else if (data.getGear() < 1) {
            gear = 1;
            counter = WAIT_TIME;
            log = "Gear 1";

        } else {
            //action.setGear(data.getGear());
            log = "else "+data.getGear()+" "+counter;
            --counter;
        }

        action.setGear(gear);
        action.setClutch(0.0);
        //System.out.println(data.getCurrentLapTimeS()+" "+data.getRPMS()+" "+data.getGearS()+" "+action.getGearS()+" "+log);
    }

    public String getLog(){
        return log;
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
        counter = 0;
        gear = 1;
    }

    @Override
    public void shutdown(){

    }
}