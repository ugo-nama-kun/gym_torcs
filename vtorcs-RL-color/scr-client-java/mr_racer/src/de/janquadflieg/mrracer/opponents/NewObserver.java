/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.opponents;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.gui.GraphicDebugable;
import de.janquadflieg.mrracer.telemetry.ModifiableSensorData;
import java.awt.geom.Point2D;

import champ2011client.Controller.Stage;

import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.TrackModel;
import de.janquadflieg.mrracer.track.TrackSegment;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import javax.swing.JComponent;

/**
 *
 * @author Nana
 */
public class NewObserver
implements OpponentObserver, GraphicDebugable{

    public static final int CAR_WIDTH = 2;
    public static final int CAR_LENGTH = 5;
    public static final int MAX_OPPONENTS = 19;
    public static final int AVOID = 150;

    private ModifiableSensorData m = new ModifiableSensorData();
    private boolean collision, collisionTest, isCorner, willBeCorner, isCornerExit = false;
    private boolean isLeft, isRight = false;
    private double resultSpeed = OpponentObserver.NO_RECOMMENDED_SPEED;
    private int opponentCount;
    //private int oldOpponentCount;
    private double lastOppDistance = -1;
    private long lastPacket = -1;
    private double distanceCorner = 150;
    private double trackWidth = 0;
    private double carAvoid = 2.5;
    double breakDistance = 50;

    private boolean debug = true;
    private DebugPainter debugPainter = new DebugPainter();
    private static final int PPM = 2;

    SensorData data;
    TrackModel model;
    Point2D point = OpponentObserver.NO_RECOMMENDED_POINT;

    //tests
    double[] xValOut = new double[MAX_OPPONENTS];
    double[] yValOut = new double[MAX_OPPONENTS];
    boolean trouble = false;
    //my meter-prosition
    double trackPosition = 0;
    private double trackEndMeters = trackWidth/2;
    private double trackPositionMeters = trackPosition * trackEndMeters;

    public void setStage(Stage s){
        
    }

    @Override
    public void update(SensorData data, Situation blub){
        if(this.data == null){
            lastPacket = data.getTimeStamp();
        }
        else{lastPacket = this.data.getTimeStamp();}
        this.data = data;

        /*if(model != null && model.initialized() && model.getLength()-data.getDistanceFromStartLine() < 100.0 &&
                point == OpponentObserver.NO_RECOMMENDED_POINT){

            double position = data.getDistanceRaced();
            position += model.getLength()-data.getDistanceFromStartLine()+50;

                point = new Point2D.Double(-0.75, position);


        } else if(model.initialized() && model.getLength()-data.getDistanceFromStartLine() >= 100.0 &&
                data.getDistanceFromStartLine() >= 50.0) {
            point = OpponentObserver.NO_RECOMMENDED_POINT;
        }*/
    }

    /**
     * Returns the recommended position on the track to avoid other cars. The
     * x-coordinate corresponds to the position on the track and the y-coordinate
     * to the race distance, at which the given x coordinate should be reached.
     *
     * Might return NO_RECOMMENDED_POINT if there is no recommendation.
     *
     * @return
     */
    @Override
    public java.awt.geom.Point2D getRecommendedPosition(){
        if(model != null && model.initialized() && otherCars()){
            //reset the collisionTest, willBeCorner, isLeft &isRight
            resultSpeed = OpponentObserver.NO_RECOMMENDED_SPEED;
            collisionTest = false;
            willBeCorner = false;
            isCornerExit = false;
            isLeft = false;
            isRight = false;
            trouble = false;
            //set tempDistanceFromStartLine
            double tempDFSL = data.getDistanceFromStartLine();
            //isCorner-test
            int currentIndex = model.getIndex(tempDFSL);
            TrackSegment current = model.getSegment(currentIndex);
            isCorner = current.isCorner();
            //find out if the next segment is a corner
            //get the track width
            trackWidth = model.getWidth();
            int nextIndex = model.incrementIndex(currentIndex);
            int lastIndex = model.decrementIndex(currentIndex);
            TrackSegment next = model.getSegment(nextIndex);
            TrackSegment last = model.getSegment(lastIndex);
            //get distance if the next segment is a corner
            if(next.isCorner()){
                distanceCorner = next.getStart()-tempDFSL;
                if(distanceCorner < 25){
                    willBeCorner = true;
                    isLeft = next.isLeft();
                    isRight = next.isRight();
                }
            }
            double[] sensors = data.getOpponentSensors();
            double[] trackSensors = new double[19];
            System.arraycopy(m.getTrackEdgeSensors(), 0, trackSensors, 0, 19);
            double[] xVal = new double[MAX_OPPONENTS];
            double[] yVal = new double[MAX_OPPONENTS];
            double smallestValue = 150;
            double smallestFrontValue = 150;
            opponentCount = 0;
            //check only the front sensors
            for (int i = 8; i < 27; i++){
                if(sensors[i] < 200.00){
                    if(sensors[i] < trackSensors[i-8]){
                        double angle = getAngle(i);
                        xVal[opponentCount] = xComponent(sensors[i], angle);
                        yVal[opponentCount] = yComponent(sensors[i], angle);
                        xValOut[opponentCount] = xComponent(sensors[i], angle);
                        //keep an eye on sensors 16-21
                        if(i > 13 && i < 23){
                            collisionTest = true;
                            if(sensors[i] < smallestFrontValue){
                                smallestFrontValue = sensors[i];
                            }
                        }
                        if (sensors[i] < smallestValue) {
                            smallestValue = sensors[i];
                        }
                        opponentCount++;
                    }
                }
            }
            if(last.isCorner()){
                double distanceLastCorner = tempDFSL - last.getEnd();
                if(distanceLastCorner < 10){
                    changeRecommendedSpeed(smallestValue);
                }
            }
            if(smallestValue < 70 && (isCorner || distanceCorner < 70)){
                trouble = true;
                changeRecommendedSpeed(smallestFrontValue);
            }
            //if the number of other cars changed: wait
            /*if(oldOpponentCount != opponentCount){
                oldOpponentCount = opponentCount;
                lastOppDistance = -1;
                return OpponentObserver.NO_RECOMMENDED_POINT;
            }*/
            //only if we cannot just keep on driving forward
            if(collisionTest){
                if(isCorner || willBeCorner){
                    changeRecommendedSpeed(smallestValue);
                    if(isCorner) return OpponentObserver.NO_RECOMMENDED_POINT;
                }
                else {
                    switch (opponentCount){
                        case 1 : {
                            return oneOpponentRecommendedPosition(xVal, current, smallestValue);}
                        default : return moreOpponentsRecommendedPosition(xVal, yVal, opponentCount, current, smallestValue);
                        }
                }
            }
            return OpponentObserver.NO_RECOMMENDED_POINT;
        }
        return OpponentObserver.NO_RECOMMENDED_POINT;
    }

    /**
     * Returns the recommended speed to avoid crashing into other cars.
     * Might return NO_RECOMMENDE_SPEED if there is no need to slow down.
     *
     * @return
     */
    @Override
    public double getRecommendedSpeed(){
        if(collision){
            collision = false;
            return resultSpeed;
        }
        return OpponentObserver.NO_RECOMMENDED_SPEED;
    }

    /**
     * This method changes resultSpeed
     */
    public void changeRecommendedSpeed(double smallestValue){
        double tempSpeed = getSpeed(smallestValue);
        double currentSpeed = m.getSpeed();
        double opponentSpeed = currentSpeed - tempSpeed;
        if(tempSpeed > 0){
            breakDistance = Math.pow(tempSpeed/10, 2);
            if(smallestValue <= breakDistance){
                //get the resultSpeed
                resultSpeed = opponentSpeed;
                //getRecommendedSpeed will slow the car down
                collision = true;
                lastOppDistance = smallestValue;
            }
        }
        else lastOppDistance = smallestValue;
    }

    @Override
    public boolean otherCars(){
         double[] sensors = data.getOpponentSensors();
         boolean allHundred = true;
         for (int i = 0; i < sensors.length; ++i) {
             allHundred &= (sensors[i] == 200.0);
         }
        return !allHundred;
    }

    /**
     * oneOpponentRecommand returns a recommended position, if there is only one opponet.
     * moreOpponentsRecommand deals with the problems of more then one opponent and
     * returns a recomanded position.
     * @param opponents
     * @return
     */
    public java.awt.geom.Point2D oneOpponentRecommendedPosition(double[] xVal, TrackSegment current, double smallestValue){
        //reset the collisionTest
        collisionTest = false;
            //if we are getting too close to the opponent
            if(smallestValue < 150 && (xVal[0] < 4 && xVal[0] > -4)){
                //position on the track 1 (left) to -1 (right)
                if(smallestValue < 70){
                    changeRecommendedSpeed(smallestValue);
                }
                trackPosition = data.getTrackPosition();
                //get the y-component
                double distanceFromStart = data.getDistanceFromStartLine();
                double returnPosition = trackPosition;
                trackEndMeters = (trackWidth/2)-1;
                trackPositionMeters = trackPosition * (trackWidth/2);
                double returnDistance = distanceFromStart + AVOID;
                double trackEnd = (trackEndMeters) /(trackWidth/2);
                //if next corner is left
                if(isLeft){
                    returnPosition = trackEnd;
                }
                //if next corner is right
                else if(isRight){
                    returnPosition = -trackEnd;
                }
                //if we do not know anything about the next corner
                else{
                    if(trackPosition != 0){
                        trackPositionMeters = trackPositionMeters - (carAvoid - xVal[0]);
                        trackPositionMeters = validTrackPosition(trackPositionMeters, trackEndMeters, xVal[0]);
                        returnPosition = trackPositionMeters/(trackWidth/2);
                    }
                    else{
                        //opponent is right in front of me
                        trackPositionMeters = trackPositionMeters - carAvoid;
                        trackPositionMeters = validTrackPosition(trackPositionMeters, trackEndMeters, xVal[0]);
                        returnPosition = trackPositionMeters/(trackWidth/2);
                    }
                }
                
                lastOppDistance = smallestValue;
                return new Point2D.Double(returnPosition, returnDistance);
            }
            lastOppDistance = smallestValue;
            return OpponentObserver.NO_RECOMMENDED_POINT;
    }

    public java.awt.geom.Point2D moreOpponentsRecommendedPosition(double[] xVal, double[] yVal, int count, TrackSegment current, double smallestValue){
        collisionTest = false;
        if(smallestValue < 150){
            trackPosition = data.getTrackPosition();
            if(smallestValue < 70){
                    changeRecommendedSpeed(smallestValue);
            }
                //get the y-component
                double distanceFromStart = data.getDistanceFromStartLine();
                double returnPosition = trackPosition;
                trackEndMeters = (trackWidth/2)-1;
                trackPositionMeters = trackPosition * (trackWidth/2);
                double returnDistance = distanceFromStart + AVOID;
                double trackEnd = (trackEndMeters) /(trackWidth/2);
                 //if next corner is left
                if(isLeft){
                    returnPosition = trackEnd;
                }
                //if next corner is right
                else if(isRight){
                    returnPosition = -trackEnd;
                }
                //if we do not know anything about our opponent
                else{
                    //possible points in an array
                    //opponentCount - 1 +2 = count +1
                    //[leftpos] [1.] [2.] ... [count.] [rightpos]
                    double[] pointArr = new double[count+1];
                    //dist between left end of track and left opponent
                    double distLeft = trackEndMeters - xVal[0];
                    if(distLeft > 3){
                        pointArr[0] =trackEndMeters;
                    }
                    else pointArr[0] = 100;
                    double distRight = -trackEndMeters - xVal[count-1];
                    if(-distRight >3){
                        pointArr[count] = -trackEndMeters;
                    }
                    else pointArr[count] = 100;
                    for(int i =1; i<count; i++){
                        double tempDist = getDistance(xVal[i-1], yVal[i-1], xVal[i], yVal[i]);
                        if(tempDist >3){
                            pointArr[i]= xVal[i]-carAvoid-0.5;
                        }
                        else pointArr[i] = 100;
                    }
                    double tempResult = 100;
                    int position = -1;
                    for (int j = 0; j <= count; j++){
                        if(pointArr[j] != 100){
                            double tempDiff = trackPositionMeters - pointArr[j];
                            if(tempDiff<0) tempDiff = -tempDiff;
                            if(tempResult > tempDiff){
                                tempResult = tempDiff;
                                position = j;
                            }
                        }
                    }
                    if(position != -1) {
                        returnPosition = pointArr[position] / (trackWidth/2);
                    }
                }
                lastOppDistance = smallestValue;
                return new Point2D.Double(returnPosition, returnDistance);
        }
        lastOppDistance = smallestValue;
        return OpponentObserver.NO_RECOMMENDED_POINT;
    }

    /**
     *
     */

    public double getDistance(double x1, double y1, double x2, double y2){
        double result = Math.pow(x1-x2, 2)+Math.pow(y1-y2, 2);
        result = Math.sqrt(result);
        if(result <0){return -result;}
        return result;
    }

    /**
     * method to check, if the trackPosition is valid. if not, change
     */

    public double validTrackPosition(double trackPosition, double trackEnd, double xVal){
        if(trackPosition < -trackEnd){
            trackPosition += xVal + carAvoid;
        }
        else if(trackPosition > trackEnd){
            trackPosition -= xVal + carAvoid;
        }
        return trackPosition;
    }


    /**
     * getAngle returns an angle
     * xComponent is the x-distance to an opponent, yCompinent returns the y-distance
     */

    public double getAngle(int i){
        i = i-18;
        //if(i<0){i = -i;}
        return Math.toRadians(-i*10);
    }

    public double xComponent(double dist, double angle){
        return dist * Math.sin(angle);
    }

    public double yComponent(double dist, double angle){
        return dist * Math.cos(angle);
    }

    public double roundResult(double notRound){
        return notRound;
    }

    //gets (newX, oldX, newY, oldY)
    public double getSpeed(double smallestValue){
        if (lastOppDistance != -1 && lastPacket != -1) {
            double timeDiff = ((double) (data.getTimeStamp() - lastPacket)) / 1000000.0; // in ms
            double distance = lastOppDistance - smallestValue;  // in m
            double speedDiff = (distance / timeDiff) * 3600.0; // in km / h
            return speedDiff;
        }
        return OpponentObserver.NO_RECOMMENDED_SPEED;
    }

    @Override
    public void setTrackModel(TrackModel trackModel){
        this.model = trackModel;
    }

    @Override
    public void reset(){
        collision = false;
        collisionTest = false;
        isCorner = false;
        isCornerExit = false;
        willBeCorner = false;
        isLeft = false;
        isRight = false;
        trackWidth = 0;
        distanceCorner = 0;
        opponentCount = 0;
        breakDistance = 50;
        //oldOpponentCount = 0;
        lastOppDistance = -1;
        lastPacket = -1;
        resultSpeed = OpponentObserver.NO_RECOMMENDED_SPEED;
        point = OpponentObserver.NO_RECOMMENDED_POINT;
    }

    @Override
    public JComponent[] getComponent() {
        if (debug) {
            return new javax.swing.JComponent[]{debugPainter};
        } else {
            return new javax.swing.JComponent[0];
        }
    }

    private class DebugPainter
            extends javax.swing.JPanel {

        public DebugPainter(){
            setName("Observer");
        }

        @Override
        public void paintComponent(Graphics graphics) {
            Graphics2D g = (Graphics2D) graphics;

            g.setColor(Color.WHITE);

            g.fillRect(0, 0, getWidth(), getHeight());

            setSensorData(data);

            if (m.getAngleToTrackAxis() != Utils.NO_DATA_D) {
                // my car
                drawCar(g);

                //double[] sensors = m.getOpponentSensors();

                if (!otherCars()) {
                    g.setColor(Color.BLACK);
                    g.drawString("No opponent in sight", 30, 30);
                } else {
                    // track sensors
                    if(opponentCount < 2){
                        drawOpponentSensors(g);
                        g.setColor(Color.BLACK);
                        g.drawString("count | collisionTest | collision",30,30);
                        g.drawString(opponentCount+ " | " + collisionTest + " | " + collision, 30, 50);
                        g.drawString("trouble | resultSpeed | break", 30, 70);
                        g.drawString(trouble +" | "+(int)resultSpeed +" | "+ (int)breakDistance, 30, 90);
                        g.drawString("" + xValOut[0], 30, 110);
                        g.drawString("point", 30, 150);
                        g.drawString("" + getRecommendedPosition(), 30, 170);

                    }
                    else{
                        drawOpponentSensors(g);
                        g.setColor(Color.BLACK);
                        g.drawString("count | collisionTest | collision",30,30);
                        g.drawString(opponentCount+ " | " + collisionTest + " | " + collision, 30, 50);
                        g.drawString("trouble | resultSpeed | break", 30, 70);
                        g.drawString(trouble +" | "+(int)resultSpeed +" | "+ (int)breakDistance, 30, 90);
                        g.drawString("point", 30, 150);
                        g.drawString("" + getRecommendedPosition(), 30, 170);
                    }

                }

            } else {
                g.setColor(Color.BLACK);
                g.drawString("No data available", 30, 30);
            }
        }

        private void drawCar(Graphics2D g) {
            AffineTransform backup = g.getTransform();

            g.translate(getWidth() / 2, (getHeight() - (CAR_LENGTH * PPM)) / 2);
            g.setColor(Color.BLACK);
            g.fillRect(-(CAR_WIDTH / 2), 0, CAR_WIDTH, CAR_LENGTH);

            g.setTransform(backup);
        }

        private void drawOpponentSensors(Graphics2D g) {
            AffineTransform backup = g.getTransform();

            g.translate(getWidth() / 2, (getHeight() - (CAR_LENGTH * PPM)) / 2);

            g.setColor(Color.RED);

            double[] sensors = m.getOpponentSensors();

            int last_x = 0;
            int last_y = 0;

            for (int i = 0; i < sensors.length; ++i) {
                if (sensors[i] < 200.0) {
                    double angle = Math.toRadians(-180.0) + Math.toRadians(i * 10.0);
                    int x_offset = (int) Math.round(Math.sin(angle) * sensors[i] * PPM);
                    int y_offset = (int) Math.round(-Math.cos(angle) * sensors[i] * PPM);


                    g.setColor(Color.RED);
                    g.drawLine(0, 0, x_offset, y_offset);
                    g.fillRect(x_offset - 1, y_offset - 1, 2, 2);
                    g.setColor(Color.BLUE);
                    g.drawString(Integer.toString(i)+"-"+String.format("%.1f", sensors[i]),
                            x_offset + 2, y_offset + 2);
                }
            }

            g.setTransform(backup);

        }
         public void setSensorData(SensorData model) {
         m.setData(model);
         repaint();
         }

    }
}
