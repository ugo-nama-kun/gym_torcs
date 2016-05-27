/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.opponents;

import static de.janquadflieg.mrracer.data.CarConstants.*;

import java.awt.geom.*;
import java.util.*;

/**
 *
 * @author nostromo
 */
public class Opponent {

    private int id = 0;
    private boolean active = false;
    /** Angle of the sensor belonging to this Opponent in radiants. */
    private double angleR = 0.0;
    /** List of last positions. */
    private ArrayList<Point2D> positions = new ArrayList<Point2D>();
    /** Number of positions used to calculate the relative speed vector. */
    private static final int NUM_POSITIONS = 4;
    /** Time in seconds between the positions. */
    private static final double DELTA_TIME = 0.02;
    /** Speed difference vector in m/s. */
    private Point2D speedDiffVec = new Point2D.Double();
    /** Position. */
    private Point2D position = new Point2D.Double();
    /** Time in seconds until we hit this car, thats the important information! */
    private double timeToCrash = 0.0;
    /** Time in seconds until we reach the minimum save distance, thats the important information! */
    private double timeToMinDistance = 0.0;
    /** Rectangle of this car. */
    private Rectangle2D rectangle = new Rectangle2D.Double();

    public Opponent(int id) {
        this.id = id;
        this.angleR = Math.toRadians(id * 10.0);
    }

    //public void deactivate(){
    //    active = false;
    //}

    public int getID() {
        return id;
    }

    public Point2D getPosition(){
        return position;
    }

    public Rectangle2D getRectangle(){
        return rectangle;
    }

    public Point2D getSpeedDiffVec(){
        return speedDiffVec;
    }

    public double getTimeToCrash(){
        return timeToCrash;
    }

    public double getTimeToMinDistance(){
        return timeToMinDistance;
    }

    public boolean isActive() {
        return active;
    }

    public void reset() {
        active = false;
        positions.clear();
        speedDiffVec.setLocation(0.0, 0.0);
        position.setLocation(0.0, 0.0);
        timeToCrash = 0.0;
        timeToMinDistance = 0.0;
    }

    public void setSensorValue(double d) {
        if (d == 200.0) {            
            reset();
            return;
        }
        Point2D pos = new Point2D.Double(-Math.sin(angleR) * d,
                -Math.cos(angleR) * d);

        positions.add(pos);

        if (positions.size() > NUM_POSITIONS) {
            positions.remove(0);
        }

        if (positions.size() < NUM_POSITIONS) {
            return;
        }

        active = true;

        Point2D avgLastPosition = new Point2D.Double();
        Point2D avgPosition = new Point2D.Double();
        /*for (int i = 0; i < NUM_POSITIONS / 2; ++i) {
            avgLastPosition.setLocation(
                    avgLastPosition.getX() + positions.get(i).getX(),
                    avgLastPosition.getY() + positions.get(i).getY());
        }
        avgLastPosition.setLocation(
                avgLastPosition.getX() / (NUM_POSITIONS / 2),
                avgLastPosition.getY() / (NUM_POSITIONS / 2));

        for (int i = NUM_POSITIONS / 2; i < positions.size(); ++i) {
            avgPosition.setLocation(
                    avgPosition.getX() + positions.get(i).getX(),
                    avgPosition.getY() + positions.get(i).getY());
        }
        avgPosition.setLocation(
                avgPosition.getX() / (NUM_POSITIONS / 2),
                avgPosition.getY() / (NUM_POSITIONS / 2));

        speedDiffVec.setLocation(
                (avgPosition.getX()-avgLastPosition.getX())/(DELTA_TIME*(NUM_POSITIONS / 2)),
                (avgPosition.getY()-avgLastPosition.getY())/(DELTA_TIME*(NUM_POSITIONS / 2)));*/

        for (int i = 0; i < NUM_POSITIONS - 1; ++i) {
            avgLastPosition.setLocation(
                    avgLastPosition.getX() + positions.get(i).getX(),
                    avgLastPosition.getY() + positions.get(i).getY());
        }
        avgLastPosition.setLocation(
                avgLastPosition.getX() / (NUM_POSITIONS - 1),
                avgLastPosition.getY() / (NUM_POSITIONS - 1));

        for (int i = 1; i < positions.size(); ++i) {
            avgPosition.setLocation(
                    avgPosition.getX() + positions.get(i).getX(),
                    avgPosition.getY() + positions.get(i).getY());
        }
        avgPosition.setLocation(
                avgPosition.getX() / (NUM_POSITIONS - 1),
                avgPosition.getY() / (NUM_POSITIONS - 1));

        speedDiffVec.setLocation(
                (avgPosition.getX()-avgLastPosition.getX())/(DELTA_TIME),
                (avgPosition.getY()-avgLastPosition.getY())/(DELTA_TIME));
        position.setLocation(avgPosition);

        // die aussortieren, die nicht wichtig sind
        // hinter mir und langsamer
        if(position.getY() < 0 && speedDiffVec.getY() < 0){
            active = false;
        }
        // vor mir und schneller
        if(position.getY() > 0 && speedDiffVec.getY() > 0){
            active = false;
        }
        // hinter mir
        if(position.getY() < 0){
            active = false;
        }

        rectangle.setRect(position.getX()-(CAR_WIDTH*0.5),
                position.getY()+(CAR_LENGTH*0.5),
                CAR_WIDTH, CAR_LENGTH);                

        timeToCrash = position.getY()/speedDiffVec.getY()*-1.0;
        timeToMinDistance = (position.getY()-(Observer2011.MIN_DISTANCE.getY()+CAR_LENGTH))/speedDiffVec.getY()*-1.0;
    }
}
