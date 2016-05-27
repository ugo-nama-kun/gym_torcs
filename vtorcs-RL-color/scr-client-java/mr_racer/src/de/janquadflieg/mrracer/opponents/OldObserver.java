/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.opponents;

import champ2011client.Controller.Stage;

import java.awt.geom.Point2D;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.gui.GraphicDebugable;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.*;

import java.awt.*;
import javax.swing.JPanel;

/**
 *
 * @author quad
 */
public class OldObserver
        implements OpponentObserver, GraphicDebugable {

    //private SensorData data;
    private TrackModel model;
    private Point2D point = OpponentObserver.NO_RECOMMENDED_POINT;
    private double speed = OpponentObserver.NO_RECOMMENDED_SPEED;
    private int lastNumCars = -1;
    private double lastOppDistance = -1; // last distance to the closest opponent in front
    private int lastOppCtr = 0;
    private String info = "";
    /** Timestamp of the last packet. */
    private long lastPacket = -1;

    private boolean DEBUG = true;

    /** Debug painter. */
    private DebugPainter debugPainter;

    public OldObserver(){
        if (DEBUG) {
            debugPainter = new DebugPainter();
            debugPainter.setName("OldObserver");
        }
    }

    @Override
    public javax.swing.JComponent[] getComponent() {
        if (DEBUG) {
            return new javax.swing.JComponent[]{debugPainter};
            
        } else {
            return new javax.swing.JComponent[0];
        }

    }

    public void setStage(Stage s){
        
    }

    @Override
    public void update(SensorData data, Situation s) {
        if (!data.onTrack()) {
            reset();
            return;
        }

        if (!model.complete()) {
            reset();
            return;
        }

        doAvoid(data);

        if(DEBUG){
            debugPainter.repaint();
        }

        lastPacket = data.getTimeStamp();
    }

    public String getInfo() {
        return info;
    }

    private void doAvoid(SensorData data) {

        int currentIndex = model.getIndex(data.getDistanceFromStartLine());
        TrackSegment current = model.getSegment(currentIndex);

        double remainingStraight = 0.0;
        if (current.isStraight()) {
            remainingStraight += current.getEnd() - data.getDistanceFromStartLine();

            int nextIndex = model.incrementIndex(currentIndex);
            TrackSegment next = model.getSegment(nextIndex);

            while (nextIndex != currentIndex && next.isStraight()) {
                remainingStraight += next.getLength();

                nextIndex = model.incrementIndex(nextIndex);
                next = model.getSegment(nextIndex);
            }
        }

        info = "";

        final double CW = 3.0; // car width ca 2.6m
        final double CL = 4.8; // car length ca 4.8m
        final double MDX = 3.0; // minimum distance x
        final double MDY = 5.0; // minimum distance y

        double[] sensors = data.getOpponentSensors();
        double[] x = new double[sensors.length];
        double[] y = new double[sensors.length];

        double smallestValue = 150;
        int smallestIndex = -1;

        boolean canSteerLeft = (model.getWidth() - (data.getTrackPosition() * model.getWidth())) > 3;
        boolean canSteerRight = (model.getWidth() + (data.getTrackPosition() * model.getWidth())) > 3;
        boolean canDriveForward = true;
        int numCars = 0;

        for (int i = 0; i < sensors.length; ++i) {
            if(sensors[i] == 200.0){
                continue;
            }

            double angleR = Math.toRadians(i * 10.0);
            x[i] = -Math.sin(angleR) * sensors[i];
            y[i] = -Math.cos(angleR) * sensors[i];

            if (current.isCorner()) {
                if (y[i] > 5.0 && y[i] < 99.0) {
                    canDriveForward = false;

                    if (sensors[i] < smallestValue) {
                        smallestValue = sensors[i];
                        smallestIndex = i;
                    }
                    ++numCars;
                }
                
            } else {
                // autos direkt neben mir etwa auf gleicher höhe
                // -> verhindern u.U., dass ich nach links oder rechts ausweichen kann
                if (Math.abs(y[i]) < (CL + (MDY / 2.0)) && Math.abs(x[i]) < (2 * CW)) {
                    if (x[i] < 0) {
                        canSteerLeft = false;
                    }
                    if (x[i] > 0) {
                        canSteerRight = false;
                    }
                    ++numCars;               
                }

                // autos direkt vor mir?
                if (Math.abs(x[i]) < ((CW * 1.0) + MDX) && y[i] > 0 && y[i] < 99.0) {
                    canDriveForward = false;

                    if (y[i] < smallestValue) {
                        smallestValue = y[i];
                        smallestIndex = i;
                    }
                    ++numCars;

                }
            }
        }

        if (numCars == 0) {
            reset();
            info = "nobody";
            return;
        }

        if (lastNumCars != numCars) {
            lastOppDistance = -1;
        }

        lastNumCars = numCars;

        // when i'm not allowed to avoid other cars, enforce a slowdown
        canSteerRight &= remainingStraight >= 50;
        canSteerLeft &= remainingStraight >= 50;

        if (canDriveForward) {
            lastOppDistance = -1;

            if (lastOppCtr > 0) {
                //if (steerCorrectionAllowed) {
                //    action.setSteering(action.getSteering() * 0.1);
                //}
                --lastOppCtr;
            }

            info = String.valueOf(numCars) + " ok";
            return;
        }

        this.lastOppCtr = 100;

        if ((canSteerRight || canSteerLeft) && smallestValue > 15.0) {
            if (lastOppDistance != -1 && lastPacket != -1) {
                double timeDiff = ((double) (data.getTimeStamp() - lastPacket)) / 1000000.0; // in ms
                double distance = lastOppDistance - smallestValue;  // in m
                double speedDiff = (distance / timeDiff) * 3600.0; // in km / h

                double timeToCrash = (distance / (speedDiff * 1000)) * 3600; //in seconds

                double offset = ((model.getWidth() - 4) / 2) / (model.getWidth() / 2);

                if (x[smallestIndex] < 0.0) {
                    offset *= -1.0;
                }

                if (point == OpponentObserver.NO_RECOMMENDED_POINT) {
                    double toTravel = ((timeToCrash / 3600.0) * speedDiff) * 1000.0;
                    //System.out.println("We would crash in "+Utils.dTS(timeToCrash)+"s, "+
                    //        Utils.dTS(toTravel)+"m");
                    point = new Point2D.Double(offset, data.getDistanceRaced() + toTravel);
                //System.out.println(point);
                }
            }


            lastOppDistance = smallestValue;
            /*if (canSteerRight) {
            action.setSteering(action.getSteering() + (-2.0 / 45.0));
            } else {
            action.setSteering(action.getSteering() + (2.0 / 45.0));
            }*/

            info = String.valueOf(numCars) + Utils.dTS(smallestValue) + " " +
                    canSteerLeft + " " + canSteerRight + " " + canDriveForward;

        } else {
            info = String.valueOf(numCars);

            if (lastOppDistance != -1 && lastPacket != -1) {
                double timeDiff = ((double) (data.getTimeStamp() - lastPacket)) / 1000000.0; // in ms
                double distance = lastOppDistance - smallestValue;  // in m
                double speedDiff = (distance / timeDiff) * 3600.0; // in km / h

                double timeToCrash = (distance / (speedDiff * 1000)) * 3600;

                if (speedDiff > 0) {
                    if (timeToCrash < 1.0) {
                        speed = data.getSpeed() - speedDiff;

                    } else if (timeToCrash < 5.0) {
                        speed = data.getSpeed() - (speedDiff * 0.5);
                    } else {
                        // plenty of time, don't do anything
                        speed = NO_RECOMMENDED_SPEED;
                    }
                } else {
                    speed = NO_RECOMMENDED_SPEED;
                }

                info += " oh oh, distance:" + Utils.dTS(smallestValue) +
                        String.format(", SpeedDiff: %+5.1f", speedDiff) +
                        String.format(", TtC: %4.1fs", timeToCrash);
                if(speed == NO_RECOMMENDED_SPEED){
                    info += ", no speed ";
                } else {
                    info += ", Speed: "+Utils.dTS(speed);
                }
            }

            lastOppDistance = smallestValue;
        }
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
    public java.awt.geom.Point2D getRecommendedPosition() {
        return point;
    }

    /**
     * Returns the recommended speed to avoid crashing into other cars.
     * Might return NO_RECOMMENDE_SPEED if there is no need to slow down.
     *
     * @return
     */
    @Override
    public double getRecommendedSpeed() {
        return speed;
    }

    @Override
    public boolean otherCars() {
        return lastNumCars != -1;

    }

    @Override
    public void setTrackModel(TrackModel trackModel) {
        this.model = trackModel;
    }

    @Override
    public void reset() {
        point = OpponentObserver.NO_RECOMMENDED_POINT;
        speed = OpponentObserver.NO_RECOMMENDED_SPEED;
        lastNumCars = -1;
        lastOppDistance = -1;
        lastOppCtr = 0;
        lastPacket = -1;
    }

    private class DebugPainter
            extends JPanel{
        
        @Override
        public void paintComponent(Graphics graphics) {
            Graphics2D g = (Graphics2D) graphics;
            try {
                paintComponent(g);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public void paintComponent(Graphics2D g) {
            Dimension size = getSize();

            g.setColor(Color.WHITE);
            g.fillRect(0, 0, getWidth(), getHeight());

            g.setColor(Color.BLACK);

            g.drawString(info, 10, 10);
        }
    }
}
