/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.controller;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.telemetry.*;

/**
 *
 * @author Jan Quadflieg
 */
public class Evaluator
        extends champ2011client.Controller {

    private BaseController controller;
    /** Distance raced. */
    private double distanceRaced = 0;
    /** Number of gameTicks of the track. */
    private int offTrackCtr = 0;
    /** Damage of the car. */
    private double damage = 0;
    /** Number of gameticks after which a restart should be requested. */
    private int maxGameTicks = 0;
    private int gameTickCtr = 0;
    private int lapCtr = -1;
    public static final int NO_MAXIMUM = Integer.MIN_VALUE;
    private int maxLaps = NO_MAXIMUM;
    private SensorData lastData = null;
    private boolean damageCheck = true;
    private double fastestLap = 60 * 10;
    private double time = 0.0;
    private double sumTime = 0.0;
    private boolean aborted = false;    

    public Evaluator(BaseController c, int i) {
        this(c, i, true);
    }

    public Evaluator(BaseController c, int i, boolean b) {
        this.controller = c;
        this.maxGameTicks = i;
        this.damageCheck = b;
    }

    public boolean aborted(){
        return aborted;
    }

    public double getFastestLap() {
        return this.fastestLap;
    }

    public double getOverallTime() {
        return this.time;
    }

    public void setMaxLaps(int d) {
        this.maxLaps = d;
    }

    public int getGameTickCtr() {
        return gameTickCtr;
    }

    public int getLapCtr() {
        return lapCtr;
    }

    public double getDistanceRaced() {
        return this.distanceRaced;
    }

    public int getOffTrackCtr() {
        return this.offTrackCtr;
    }

    public double getDamage() {
        return this.damage;
    }

    public champ2011client.Action control(champ2011client.SensorModel m) {
        if(aborted){
            champ2011client.Action action = new champ2011client.Action();
            action.restartRace = true;
            return action;
        }

        SensorData data = new SensorData(m);
        ++gameTickCtr;
        if (lastData != null && data.getDistanceFromStartLine() < lastData.getDistanceFromStartLine()
                && data.getDistanceRaced() > lastData.getDistanceRaced()) {
            ++lapCtr;
            if (lapCtr > 0) {
                fastestLap = Math.min(fastestLap, data.getLastLapTime());
                sumTime += data.getLastLapTime();
                //System.out.println(sumTime);
                //System.out.println(fastestLap);
            }
        }

        //if(aborted){
        //    System.out.println("control called after the evaluation has been aborted!");
        //}

        time = sumTime + data.getCurrentLapTime();

        if (!data.onTrack()) {
            ++offTrackCtr;
        }

        distanceRaced = data.getDistanceRaced();
        damage = data.getDamage();

        champ2011client.Action action = controller.control(m);

        // end of evaluation
        if (maxTicksReached() || maxDamageReached() || maxLapsReached()){
            action.restartRace = true;

            if ((maxTicksReached() || maxDamageReached()) && !maxLapsReached()) {
                System.out.println("Abort Laps[" + lapCtr + "/" + maxLaps + "] "
                        + "Ticks["+gameTickCtr+"/"+maxGameTicks+"] "
                        + "Damage: " + data.getDamage()+" "
                        + "Time raced: "+Utils.timeToExactString(time));
                double overallDist = this.maxLaps * controller.trackModel.getLength();
                System.out.print("Distance[" + Utils.dTS(data.getDistanceRaced()) + "/" + Utils.dTS(overallDist) + "] " + Utils.timeToExactString(time));
                double ratio = data.getDistanceRaced() / overallDist;
                time *= 1.0 / ratio;
                // damage multiplier
                time *= (1.0 + (2.0 * (data.getDamage()/5000.0)));
                System.out.println(" -> " + Utils.timeToExactString(time));
                aborted = true;
            }
        }

        lastData = data;

        return action;
    }

    private boolean maxTicksReached(){
        return gameTickCtr >= maxGameTicks;
    }

    public boolean maxDamageReached(){
        return damageCheck && damage > 5000.0;
    }

    private boolean maxLapsReached(){
        return maxLaps != NO_MAXIMUM && lapCtr == maxLaps;
    }

    public void resetFull() {
        controller.resetFull();
    }

    public void reset() {
        controller.reset();
    }

    public void shutdown() {
        controller.shutdown();
    }    
}
