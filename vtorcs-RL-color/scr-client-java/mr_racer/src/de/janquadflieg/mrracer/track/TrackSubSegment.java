/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.track;

import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.classification.Situations;

/**
 * A TrackSubSegment represents a number of data points, which have been
 * consistently classified as the same type.
 *
 * @author Jan Quadflieg
 */
public class TrackSubSegment
implements java.io.Serializable{


    static final long serialVersionUID = -1200266135754954778L;

    /** Type of this segment. */
    private int type;

    /** Start of this segment. */
    private double start;

    /** Length of this segment. */
    private double length;

    /** Filter factor. */
    private double filterFactor = 1.0;

    /** Maximum measurement. */
    private double max = Double.NEGATIVE_INFINITY;

    /** Position of the maximum. */
    private double maxPos = 0.0;

    public TrackSubSegment(double start, double end, Situation s){
        this.start = start;
        this.length = Math.max(end - start, Double.MIN_NORMAL);
        this.type = s.getType();
        this.maxPos = (start+end)/2.0;
        this.max = Math.abs(s.getMeasure());
        if (type == Situations.TYPE_STRAIGHT_AC) {
            type = Situations.TYPE_STRAIGHT;
        }
    }

    public TrackSubSegment(double start, double end, int type, double measure){
        this.start = start;
        this.length = Math.max(end - start, Double.MIN_NORMAL);
        this.type = type;
        this.maxPos = (start+end)/2.0;
        this.max = measure;
    }

    public boolean contains(double d) {
        return d >= this.start && d < this.getEnd();
    }

    protected void setEnd(double end){
        this.length = end - start;        
    }

    protected void setEnd(double end, Situation s){
        this.length = end - start;
        if(Math.abs(s.getMeasure()) > max){
            max = Math.abs(s.getMeasure());
            maxPos = end;
        }
    }

    protected void setStart(double newStart){
        double oldEnd = start+length;
        this.start = newStart;
        this.length = oldEnd-start;        
    }

    protected double getFilterFactor(){
        return filterFactor;
    }

    protected void setFilterFactor(double d){
        this.filterFactor = d;
    }

    public double getMaximum(){
        return max;
    }

    public double getMaxPosition(){
        return maxPos;
    }

    protected void setMaximum(double max){
        this.max = max;
    }

    protected void setMaxPosition(double pos){
        this.maxPos = pos;
    }

    public double getEnd(){
        return start+length;
    }

    public double getLength(){
        return length;
    }

    public double getStart(){
        return start;
    }

    public int getType(){
        return type;
    }

    protected void setType(int t){
        this.type = t;
    }

    public boolean isFull(){
        return type == Situations.TYPE_FULL;
    }

    public boolean isStraight(){
        return type == Situations.TYPE_STRAIGHT;
    }

    public boolean isMedium(){
        return type == Situations.TYPE_MEDIUM;
    }

    public boolean isSlow(){
        return type == Situations.TYPE_SLOW;
    }

    public boolean isHairpin(){
        return type == Situations.TYPE_HAIRPIN;
    }
}