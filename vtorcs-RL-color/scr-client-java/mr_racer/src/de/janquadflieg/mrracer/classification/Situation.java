/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.classification;

import de.janquadflieg.mrracer.Utils;

/**
 * This class represents a situation on the track. It combines one of the
 * possible situation defined in Situations.java with some additional data.
 *
 * @author Jan Quadflieg
 */
public class Situation {

    /** The situation. */
    private int situation;
    /** The measure on which the classification was based. */
    private double measure;
    /** The approximated distance to the next corner. */
    private double dtc;

    public Situation(int i, double d, double dtc) {
        this.situation = i;
        this.measure = d;
        this.dtc = dtc;
    }

    public int getSituation() {
        return this.situation;
    }

    public int getType(){
        return this.situation & Situations.TYPE_MASK;
    }

    public int getDirection(){
        return this.situation & Situations.DIRECTION_MASK;
    }

    public int getError(){
        return this.situation & Situations.ERROR_MASK;
    }

    public boolean isCorner(){
        return getDirection() != Situations.DIRECTION_FORWARD && getType() != Situations.TYPE_OUTSIDE;
    }

    public boolean hasError(){
        return (situation & Situations.ERROR_MASK) != Situations.ERROR_NONE;
    }

    public boolean isForward() {
        return (situation & Situations.DIRECTION_MASK) == Situations.DIRECTION_FORWARD;
    }

    public boolean isLeft() {
        return (situation & Situations.DIRECTION_MASK) == Situations.DIRECTION_LEFT;
    }

    public boolean isRight() {
        return (situation & Situations.DIRECTION_MASK) == Situations.DIRECTION_RIGHT;
    }

    public boolean isFull(){
        return (situation & Situations.TYPE_MASK) == Situations.TYPE_FULL;
    }

    public boolean isMedium(){
        return (situation & Situations.TYPE_MASK) == Situations.TYPE_MEDIUM;
    }

    public boolean isSlow(){
        return (situation & Situations.TYPE_MASK) == Situations.TYPE_SLOW;
    }

    public boolean isHairpin(){
        return (situation & Situations.TYPE_MASK) == Situations.TYPE_HAIRPIN;
    }

    public boolean isOutside(){
        return (situation & Situations.TYPE_MASK) == Situations.TYPE_OUTSIDE;
    }

    public boolean isStraight(){
        return (situation & Situations.TYPE_MASK) == Situations.TYPE_STRAIGHT;
    }

    public boolean isStraightAC(){
        return (situation & Situations.TYPE_MASK) == Situations.TYPE_STRAIGHT_AC;
    }

    /**
     * Returns the measure the classifier calculated to classify this situation.
     *
     * @return The measure.
     */
    public double getMeasure() {
        return this.measure;
    }

    /**
     *
     * @return
     */
    public double getDistanceToCorner(){
       return this.dtc;
    }

//    protected void setDistanceToCorner(double d){
//        this.dtc = d;
//    }

    @Override
    public String toString() {
        if(hasError()){
            return "Error: "+Situations.toString(getError());

        } else if (isForward()) {
            if (isStraightAC()) {
                return "Straight, corner in " + Utils.dTS(dtc) + "m";

            } else {
                return "Straight, no corner in sight";
            }

        } else if(isOutside()){
            String side = Situations.toString(situation & Situations.DIRECTION_MASK);

            return "Outside the track, "+side.toLowerCase();
            
        } else {
            String dir = Situations.toString(situation & Situations.DIRECTION_MASK);            
            String corner = Situations.toString(situation & Situations.TYPE_MASK);

            return corner+" "+dir.toLowerCase()+" corner, angle "+Utils.dTS(measure);
        }
    }    
}
