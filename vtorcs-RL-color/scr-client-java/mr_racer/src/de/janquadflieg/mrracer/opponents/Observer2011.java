/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.opponents;

import champ2011client.Controller.Stage;


import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.classification.Situation;
import static de.janquadflieg.mrracer.data.CarConstants.*;
import de.janquadflieg.mrracer.gui.GraphicDebugable;
import de.janquadflieg.mrracer.plan.Plan2011;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.track.*;

import java.awt.*;
import java.awt.geom.*;
import java.util.*;
import javax.swing.JPanel;

/**
 *
 * @author quad
 */
public class Observer2011
        implements OpponentObserver, GraphicDebugable {

    /** Maximum distance of sensors accepted by the noise handler. */
    public static final double MAX_DISTANCE_NOISY = 175.0;
    /** Minimum lookahead for the critical zone. */
    private static final double MIN_CRITICAL_LOOKAHEAD = 10.0 + CAR_LENGTH;
    /** Maximum lookahead for the critical zone. */
    private static final double CRITICAL_LOOKAHEAD_INCREASE = MAX_DISTANCE_NOISY - MIN_CRITICAL_LOOKAHEAD;
    /** Minimum width for the critical zone. */
    private static final double MIN_CRITICAL_WIDTH = CAR_WIDTH + 2.0;
    /** Increase width for the critical zone. */
    private static final double CRITICAL_WIDTH_INCREASE = 4;
    /** Minimal distance to switch the position on the track by 1 meter. */
    private static final double MIN_SWITCH_DISTANCE = 5.0;
    /** Factor to increase the minimum switch distance according to the current speed. */
    private static final double SWITCH_INCREASE_MAX_FACTOR = 7.0;
    /** Minimum absolute distance in meter to other cars. */
    public static final Point2D MIN_DISTANCE = new Point2D.Double(3.0, 10.0);
    /** Graphical debugging? */
    private boolean GRAPHICAL_DEBUG = true;
    /** Output debug text messages? */
    private static final boolean TEXT_DEBUG = false;
    /** Debug painter. */
    private DebugPainter debugPainter;
    /** TrackModel. */
    private TrackModel model;
    /** Current stage. */
    private Stage stage = Stage.RACE;
    /** Recommended point. */
    private Point2D point = OpponentObserver.NO_RECOMMENDED_POINT;
    /** Recommmended speed. */
    private double speed = OpponentObserver.NO_RECOMMENDED_SPEED;
    /** Number of active cars. */
    private int lastNumCars = -1;
    /** Timestamp of the last packet. */
    private Opponent[] opponents = new Opponent[36];

    public Observer2011() {
        if (GRAPHICAL_DEBUG) {
            debugPainter = new DebugPainter();
            debugPainter.setName("Observer2011");
        }
        for (int i = 0; i < opponents.length; ++i) {
            opponents[i] = new Opponent(i);
        }
        /*if (System.getProperties().containsKey("ObserverDebug")) {
            System.out.println("ObserverDebug");
            TEXT_DEBUG = true;
        }*/
    }

    @Override
    public javax.swing.JComponent[] getComponent() {
        if (GRAPHICAL_DEBUG) {
            return new javax.swing.JComponent[]{debugPainter};

        } else {
            return new javax.swing.JComponent[0];
        }
    }

    @Override
    public void setStage(Stage s) {
        this.stage = s;
    }

    @Override
    public void update(SensorData data, Situation s) {
        if (stage != Stage.RACE) {
            reset();
            return;
        }

        if (!data.onTrack()) {
            reset();
            return;
        }

        if (!model.complete()) {
            reset();
            return;
        }

        doAvoid(data, s);

        if (GRAPHICAL_DEBUG) {
            debugPainter.repaint();
        }
    }

    public String getInfo() {
        return "";
    }

    private void doAvoid(SensorData data, Situation situation) {
        int currentIndex = model.getIndex(data.getDistanceFromStartLine());
        TrackSegment current = model.getSegment(currentIndex);
        TrackSegment nextCorner = null;

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

            if (next.isCorner()) {
                nextCorner = next;
            }
        }

        double CRITICAL_WIDTH = MIN_CRITICAL_WIDTH;

        if (remainingStraight > 0 && remainingStraight < 100.0) {
            double v = Math.abs(nextCorner.getApexes()[0].value);
            CRITICAL_WIDTH += (remainingStraight / 100.0) * Math.max(0, Math.min(1, v / 100.0)) * CRITICAL_WIDTH_INCREASE;

        } else if (current.isCorner()) {
            int apexIndex = -1;
            TrackSegment.Apex[] apexes = current.getApexes();

            for (int i = 0; i < apexes.length && apexIndex == -1; ++i) {
                if (data.getDistanceFromStartLine() < apexes[i].position) {
                    apexIndex = i;
                }
            }

            if (apexIndex >= 0 && apexIndex < apexes.length) {
                double v = Math.abs(apexes[apexIndex].value);
                CRITICAL_WIDTH += Math.max(0, Math.min(1, v / 100.0)) * CRITICAL_WIDTH_INCREASE;
            }
        }

        Rectangle2D criticalZone = new Rectangle2D.Double(
                -(CRITICAL_WIDTH * 0.5), 0.0, CRITICAL_WIDTH,
                MIN_CRITICAL_LOOKAHEAD + (Math.max(0, Math.min(1, data.getSpeed() / Plan2011.MAX_SPEED)) * CRITICAL_LOOKAHEAD_INCREASE));

        double[] sensors = data.getOpponentSensors();
        double[] track = data.getTrackEdgeSensors();
        //double[] x = new double[sensors.length];
        //double[] y = new double[sensors.length];

        /*double smallestValue = 150;
        int smallestIndex = -1;

        boolean canSteerLeft = (model.getWidth() - (data.getTrackPosition() * model.getWidth())) > 3;
        boolean canSteerRight = (model.getWidth() + (data.getTrackPosition() * model.getWidth())) > 3;
        boolean canDriveForward = true;
        int numCars = 0;*/

        ArrayList<Opponent> active = new ArrayList<Opponent>();
        ArrayList<Opponent> critical = new ArrayList<Opponent>();

        for (int i = 0; i < sensors.length; ++i) {
            if(i >= 9 && i <= 27){
                if(track[i-9] < sensors[i]){
                    sensors[i] = 200.0;
                }
            }

            opponents[i].setSensorValue(sensors[i]);

            if (opponents[i].isActive()) {
                active.add(opponents[i]);
                /*if(TEXT_DEBUG){
                System.out.println(criticalZone.toString());
                System.out.println(opponents[i].getRectangle());
                }*/
                if (criticalZone.intersects(opponents[i].getRectangle())) {
                    critical.add(opponents[i]);
                }
            }


            /*            

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
            }*/
        }

        if (active.isEmpty()) {
            point = NO_RECOMMENDED_POINT;
            speed = NO_RECOMMENDED_SPEED;
            lastNumCars = -1;
            return;
        }

        lastNumCars = active.size();

        Collections.sort(active, new TimeToMinDistanceComparator());
        Collections.sort(critical, new TimeToMinDistanceComparator());

        if (TEXT_DEBUG && !active.isEmpty()) {
            System.out.println("---------[" + active.size() + "/" + critical.size() + "]----------------");
            for (int i = 0; i < active.size(); ++i) {
                Point2D p = active.get(i).getPosition();
                Point2D s = active.get(i).getSpeedDiffVec();
                double time = active.get(i).getTimeToCrash();
                System.out.println("Active[" + i + "] "
                        + Utils.dTS(s.getY() * 3.6) + "km/h, "
                        + Utils.dTS(p.getY()) + "m, "
                        + Utils.dTS(time) + "s, "
                        + Utils.dTS(active.get(i).getTimeToMinDistance()) + "s");
            }
            System.out.println("");
        }

        if (TEXT_DEBUG && !critical.isEmpty()) {
            System.out.println("---------[" + active.size() + "/" + critical.size() + "]----------------");
            for (int i = 0; i < critical.size(); ++i) {
                Point2D p = critical.get(i).getPosition();
                Point2D s = critical.get(i).getSpeedDiffVec();
                double time = critical.get(i).getTimeToCrash();
                System.out.println("Critical[" + i + "] "
                        + Utils.dTS(s.getY() * 3.6) + "km/h, "
                        + Utils.dTS(p.getY()) + "m, "
                        + Utils.dTS(time) + "s, "
                        + Utils.dTS(critical.get(i).getTimeToMinDistance()) + "s");
            }
        }

        if (!critical.isEmpty() && critical.get(0).getTimeToMinDistance() <= 20.0) {
            double speedOther = data.getSpeed() + (critical.get(0).getSpeedDiffVec().getY() * 3.6);
            double allowedSpeed;

            if (critical.get(0).getTimeToMinDistance() >= 0) {
                allowedSpeed = speedOther + ((critical.get(0).getTimeToMinDistance() / 20.0) * 20.0);

            } else {
                //allowedSpeed = speedOther + ((critical.get(0).getTimeToMinDistance() / 20.0) * 20.0);
                allowedSpeed = data.getSpeed() - 5.0;
            }

            speed = Math.min(data.getSpeed(), allowedSpeed);

            if (TEXT_DEBUG) {
                System.out.println(data.getSpeedS() + "km/h, other: " + Utils.dTS(speedOther) + ", allowed: " + Utils.dTS(allowedSpeed));
                System.out.println("Recommended Speed: " + Utils.dTS(speed) + "km/h");
                System.out.println("");
            }

        } else {
            speed = NO_RECOMMENDED_SPEED;
        }

        if (remainingStraight > 100.0) {
            point = NO_RECOMMENDED_POINT;
            Collections.sort(active, new RectangleXPositionComparator());
            double myPosition = data.calcAbsoluteTrackPosition(model.getWidth());

            if (TEXT_DEBUG) {
                System.out.println(Utils.dTS(myPosition) + " Calculate overtaking point:");
                for (int i = 0; i < active.size(); ++i) {
                    System.out.println("Active[" + i + "] "
                            + Utils.dTS(active.get(i).getRectangle().getMinX()));
                }
            }

            ArrayList<Point2D> gaps = new ArrayList<Point2D>();
            if ((active.get(0).getRectangle().getMinX() + myPosition) > CAR_WIDTH + 2) {
                gaps.add(new Point2D.Double(0.0, (active.get(0).getRectangle().getMinX() + myPosition)));
            }
            for (int i = 1; i < active.size(); ++i) {
                double width = (active.get(i).getRectangle().getMinX() + myPosition) -
                        (active.get(i-1).getRectangle().getMinX() + myPosition + CAR_WIDTH);
                if (width > CAR_WIDTH + 2) {
                    gaps.add(new Point2D.Double((active.get(i-1).getRectangle().getMinX() + myPosition + CAR_WIDTH),
                            width));
                }
            }
            double width = model.getWidth()-
                    (active.get(active.size()-1).getRectangle().getMinX() + myPosition + CAR_WIDTH);
            if (width > CAR_WIDTH + 2) {
                gaps.add(new Point2D.Double((active.get(active.size()-1).getRectangle().getMinX() + myPosition + CAR_WIDTH),
                            width));
            }

            if (TEXT_DEBUG && !gaps.isEmpty()) {
                for (int i = 0; i < gaps.size(); ++i) {
                    System.out.println("Gap[" + i + "] "+
                            Utils.dTS(gaps.get(i).getX())+", width="+
                            Utils.dTS(gaps.get(i).getY()));
                }
            }

            if(!gaps.isEmpty()){
                Point2D gap;
                if(nextCorner.isRight()){
                    gap = gaps.get(0);
                    if(TEXT_DEBUG){
                        System.out.print("Choosing left gap");
                    }
                } else {
                    gap = gaps.get(gaps.size()-1);
                    if(TEXT_DEBUG){
                        System.out.print("Choosing right gap");
                    }
                }
                double xPos = SensorData.calcRelativeTrackPosition(gap.getX()+(gap.getY() *0.5), model.getWidth());
                double deltaPosition = Math.abs(myPosition-(gap.getX()+(gap.getY() *0.5)));
                double yPos = data.getDistanceRaced()+MIN_SWITCH_DISTANCE;
                yPos += deltaPosition * Math.max(0, Math.min(1, data.getSpeed()/300.0)) * SWITCH_INCREASE_MAX_FACTOR * MIN_SWITCH_DISTANCE;

                point = new Point2D.Double(xPos, yPos);
                if(TEXT_DEBUG){
                    System.out.println(" "+Utils.dTS(xPos)+", "+Utils.dTS(yPos));
                    System.out.println("");
                }
            }

        } else {
            point = NO_RECOMMENDED_POINT;
        }


        /*if (lastNumCars != numCars) {
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

        /*  info = String.valueOf(numCars) + Utils.dTS(smallestValue) + " "
        + canSteerLeft + " " + canSteerRight + " " + canDriveForward;

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

        info += " oh oh, distance:" + Utils.dTS(smallestValue)
        + String.format(", SpeedDiff: %+5.1f", speedDiff)
        + String.format(", TtC: %4.1fs", timeToCrash);
        if (speed == NO_RECOMMENDED_SPEED) {
        info += ", no speed ";
        } else {
        info += ", Speed: " + Utils.dTS(speed);
        }
        }

        lastOppDistance = smallestValue;
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
        for (Opponent o : opponents) {
            o.reset();
        }

        point = OpponentObserver.NO_RECOMMENDED_POINT;
        speed = OpponentObserver.NO_RECOMMENDED_SPEED;
        lastNumCars = -1;
        /*lastOppDistance = -1;
        lastOppCtr = 0;
        lastPacket = -1;*/
    }

    private class DebugPainter
            extends JPanel {

        @Override
        public void paintComponent(Graphics graphics) {
            Graphics2D g = (Graphics2D) graphics;
            try {
                paintComponent(g);
            } catch (Exception e) {
                e.printStackTrace(System.out);
            }
        }

        public void paintComponent(Graphics2D g) {
            Dimension size = getSize();

            g.setColor(Color.WHITE);
            g.fillRect(0, 0, getWidth(), getHeight());

            g.setColor(Color.BLACK);

            //g.drawString(info, 10, 10);
        }
    }
}
