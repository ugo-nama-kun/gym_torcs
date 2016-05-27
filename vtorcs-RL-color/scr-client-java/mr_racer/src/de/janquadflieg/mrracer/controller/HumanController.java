/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.controller;

import de.janquadflieg.mrracer.classification.*;
import de.janquadflieg.mrracer.telemetry.*;
import de.janquadflieg.mrracer.track.*;

import champ2011client.Controller;
import champ2011client.SensorModel;

import java.awt.AWTEvent;
import java.awt.event.KeyEvent;

/**
 *
 * @author Jan Quadflieg
 */
public class HumanController
        extends Controller
        implements java.awt.event.AWTEventListener {

    private AngleBasedClassifier c = new AngleBasedClassifier(true);
    private boolean up = false;
    private boolean down = false;
    private boolean left = false;
    private boolean right = false;
    private boolean reverse = false;
    private boolean meta = false;
    private double distance = 0.0;
    private int reverseCounter = 0;
    private boolean[] keyData = new boolean[128];
    private int brakeCounter = 0;
    private int accCounter = 0;
    private int leftCounter = 0;
    private int rightCounter = 0;
    Telemetry t;
    private TrackModel trackModel = new TrackModel();
    private TrackDB trackDB = null;
    private boolean firstPacket = true;
    /** Timestamp of the last packet. */
    private long lastPacket = -1;
    /** Timestamp of this packet. */
    private long thisPacket;

    public HumanController(Telemetry t) {
        this.t = t;
        trackDB = TrackDB.create();
    }

    public champ2011client.Action control(SensorModel m) {
        thisPacket = System.nanoTime();
        SensorData data = new SensorData(m);
        ModifiableAction action = new ModifiableAction();
        if (firstPacket) {
            double[] sensors = data.getTrackEdgeSensors();
            double width = sensors[0] + sensors[18];

            System.out.println("Asking database about " + data.getRacePositionS() +
                    " " + data.getDistanceFromStartLineS() + " " + width);

            TrackModel tm = null;//trackDB.get(data.getRacePosition(),
                    //data.getDistanceFromStartLine(), width);

            if (tm != null) {
                System.out.println("WOOOOOOT - die strecke kenne ich");
                System.out.println(tm.getName());
                this.trackModel = tm;

            } else {
                System.out.println("Unknown track :-(");
            }

            firstPacket = false;
        }

        trackModel.append(data, c.classify(data));

        // check for a state change
        if (reverse) {
            if (up) {
                reverse = false;
                reverseCounter = 0;
            }
        } else {
            if (down && data.getSpeed() <= 0) {
                ++reverseCounter;
            }

            if (reverseCounter > 50) {
                reverse = true;
            }
        }

        if (reverse) {
            if (up) {
                action.setBrake(1.0);
            }

            if (down) {
                action.setAcceleration(1.0);
            }
            brakeCounter = 0;
            accCounter = 0;

        } else {
            if (up) {
                action.setAcceleration(0.1 + (accCounter * 0.05));
                brakeCounter = 0;
                ++accCounter;
            }

            if (down) {
                action.setBrake(0.1 + (brakeCounter * 0.05));
                accCounter = 0;
                ++brakeCounter;
            }
        }

        if (left) {
            double speedFactor = 1;//(300.0-data.getSpeed())/300.0;

            action.setSteering((0.08 * speedFactor) + (leftCounter * 0.05 * speedFactor));
            rightCounter = 0;
            ++leftCounter;

        } else if (right) {
            double speedFactor = 1;//(300.0-data.getSpeed())/300.0;

            action.setSteering((-0.08 * speedFactor) + (rightCounter * -0.05 * speedFactor));
            leftCounter = 0;
            ++rightCounter;

        } else {
            rightCounter = 0;
            leftCounter = 0;
            action.setSteering(0);
        }

        gearChange(data, action);


        action.limitValues();

        String log = c.classify(data).toString();

        if (trackModel.complete()) {

            int index = trackModel.getIndex(data.getDistanceFromStartLine());

            log = "Segment #" + index + " ";
            int[] indices = trackModel.getIndices(data, 250.0);
            for (int i = 0; i < indices.length; ++i) {
                log += indices[i] + "-";
            }
        }

        log = this.handleOpponents(data, new ModifiableAction(), false);


        lastPacket = thisPacket;

        t.log(data, action, log);

        if(meta){
            distance = data.getDistanceFromStartLine();
        }
        System.out.println(data.getDistanceFromStartLine()-distance+";"+data.getSpeed());

        return action.getRaceClientAction();
    }

    public void eventDispatched(AWTEvent event) {
        //System.out.println(event);

        if (event instanceof java.awt.event.KeyEvent) {
            KeyEvent e = (KeyEvent) event;

            int id = e.getKeyCode();

            if (e.getID() == KeyEvent.KEY_PRESSED && id > 0 && id < keyData.length) {
                keyData[id] = true;

            } else if (e.getID() == KeyEvent.KEY_RELEASED && id > 0 && id < keyData.length) {
                keyData[id] = false;
            }

            // 38 = oben, 40 = unten, 37 = links, 39 = rechts
            up = keyData[38];
            down = keyData[40];
            right = keyData[39];
            left = keyData[37];
            meta = keyData[10];

            e.consume();
        }
    }

    private void gearChange(SensorData data, ModifiableAction action) {
        if (reverse) {
            action.setGear(-1);

        } else if (data.getRPM() > 9200 && data.getGear() < 6) {
            action.setGear(data.getGear() + 1);            

        } else if (data.getRPM() < 3500 && data.getGear() == 2) {
            action.setGear(data.getGear() - 1);

        } else if (data.getRPM() < 6000 && data.getGear() > 2) {
            action.setGear(data.getGear() - 1);

        } else if (data.getGear() < 1) {
            action.setGear(1);

        } else {
            action.setGear(data.getGear());
        }
    }
    private int lastOppIdx = -1;
    private double lastOppDistance = -1; // last distance to the closest opponent in front
    private int lastOppCtr = 0;

    private String handleOpponents(SensorData data, ModifiableAction action,
            boolean steerCorrectionAllowed) {
        if (!data.onTrack()) {
            return "egal";
        }

        final double CW = 2.6; // car width ca 2.6m
        final double CL = 4.8; // car length ca 4.8m
        final double MDX = 0.5; // minimum distance x
        final double MDY = 5.0; // minimum distance y

        double[] sensors = data.getOpponentSensors();
        double[] x = new double[sensors.length];
        double[] y = new double[sensors.length];

        double smallestValue = 150;
        double smallestIndex = -1;        

        boolean canSteerLeft = (trackModel.getWidth() - (data.getTrackPosition() * trackModel.getWidth())) > 3;
        boolean canSteerRight = (trackModel.getWidth() + (data.getTrackPosition() * trackModel.getWidth())) > 3;
        boolean canDriveForward = true;

        for (int i = 0; i < sensors.length; ++i) {
            double angleR = Math.toRadians(i * 10.0);
            x[i] = -Math.sin(angleR) * sensors[i];
            y[i] = -Math.cos(angleR) * sensors[i];

            // autos direkt neben mir etwa auf gleicher höhe
            // -> verhindern u.U., dass ich nach links oder rechts ausweichen kann
            if (Math.abs(y[i]) < (CL + (MDY / 2.0)) && Math.abs(x[i]) < (2 * CW)) {
                if (x[i] < 0) {
                    canSteerLeft = false;
                }
                if (x[i] > 0) {
                    canSteerRight = false;
                }
            }

            // autos direkt vor mir?
            if (Math.abs(x[i]) < ((CW * 5.0) + MDX) && y[i] > 0 && y[i] < 80.0) {
                canDriveForward = false;

                if (y[i] < smallestValue) {
                    smallestValue = y[i];
                    smallestIndex = i;
                }
            }
        }

        // when i'm not allowed to avoid other cars, enforce a slowdown
        canSteerRight &= steerCorrectionAllowed;
        canSteerLeft &= steerCorrectionAllowed;

        if (canDriveForward) {
            lastOppDistance = -1;
            lastOppIdx = -1;

            if (lastOppCtr > 0) {
                if (steerCorrectionAllowed) {
                    action.setSteering(action.getSteering() * 0.1);
                }
                --lastOppCtr;
            }

            return " ok";
        }

        this.lastOppCtr = 100;

        if ((canSteerRight || canSteerLeft) && smallestValue > 15.0) {
            if (canSteerRight) {
                action.setSteering(action.getSteering() + (-2.0 / 45.0));
            } else {
                action.setSteering(action.getSteering() + (2.0 / 45.0));
            }

        } else {
            String result = "";

            if (lastOppDistance != -1 && lastPacket != -1) {
                double timeDiff = ((double)(thisPacket-lastPacket))/1000000.0; // in ms
                double distance = lastOppDistance - smallestValue;  // in m
                double speedDiff = (distance/timeDiff)*3600.0; // in km / h

                result = " oh oh " + String.format(" %4.1f", smallestValue) +                        
                        String.format(" %4.1f", speedDiff);
            }

            lastOppDistance = smallestValue;

            return result;
        }


        //java.awt.Toolkit.getDefaultToolkit().beep();

        lastOppDistance = -1;


        return String.format(" %4.1f", smallestValue) + " " +
                canSteerLeft + " " + canSteerRight + " " + canDriveForward;
    }

    public void reset() {
    }

    public void shutdown() {
        trackModel.print();
        try {
            trackModel.save("c:\\model");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
