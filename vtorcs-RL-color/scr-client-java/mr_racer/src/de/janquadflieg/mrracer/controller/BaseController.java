/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.controller;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.behaviour.Behaviour;
import de.janquadflieg.mrracer.classification.AngleBasedClassifier;
import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.classification.Situations;
import de.janquadflieg.mrracer.telemetry.*;
import de.janquadflieg.mrracer.track.*;

import java.util.Properties;

/**
 *
 * @author Jan Quadflieg
 */
public abstract class BaseController
        extends champ2011client.Controller {

    /** Text debug messages? */
    private static final boolean TEXT_DEBUG = false;
    /** Extension for the parameter file. */
    public static final String PARAMETER_EXT = ".params";
    /** Telemetry object to log all data. */
    protected Telemetry telemetry = null;
    /** Model of the track we're racing on. */
    protected TrackModel trackModel = new TrackModel();
    /** Collection of track models. */
    protected TrackDB trackDB;
    /** StringBuilder to collect log data. */
    protected StringBuilder controllerLog = new StringBuilder(500);
    /** A controller to handle situations not handled by this controller. */
    protected Behaviour backupBehaviour;
    /** Coming back from the recovery behaviour? */
    protected boolean wasRecovering = false;
    /** Classifier. */
    protected AngleBasedClassifier classifier;
    /** Noise detector. */
    protected NoiseDetector noise;
    /** First packet? */
    protected boolean firstPacket = true;
    /** Precompile? */
    protected boolean precompile = false;
    private static final String PC_WARMUP_FIRST = "(angle 0)(curLapTime 10.21)(damage 0)(distFromStart 6201.46)(distRaced 0)(fuel 94)(gear 0)(lastLapTime 0)(opponents 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200)(racePos 1)(rpm 942.478)(speedX 0)(speedY 0)(speedZ 0.0196266)(track 4.00001 4.06171 4.25672 4.61881 5.22164 6.22291 8.00001 11.6952 23.0351 200 46.0701 23.3904 16 12.4458 10.4433 9.2376 8.51342 8.12341 7.99999)(trackPos 0.333332)(wheelSpinVel 0 0 0 0)(z 0.339955)(focus -1 -1 -1 -1 -1)";
    private static final String PC_WARMUP_SECOND = "(angle 0)(curLapTime 10.24)(damage 0)(distFromStart 6202.46)(distRaced 1)(fuel 94)(gear 0)(lastLapTime 0)(opponents 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200)(racePos 1)(rpm 942.478)(speedX 0)(speedY 0)(speedZ 0.0196266)(track 4.00001 4.06171 4.25672 4.61881 5.22164 6.22291 8.00001 11.6952 23.0351 200 46.0701 23.3904 16 12.4458 10.4433 9.2376 8.51342 8.12341 7.99999)(trackPos 0.333332)(wheelSpinVel 0 0 0 0)(z 0.339955)(focus -1 -1 -1 -1 -1)";
    private static final String PC_WARMUP_OFFTRACK = "(angle 0)(curLapTime 10.21)(damage 0)(distFromStart 6201.46)(distRaced 0)(fuel 94)(gear 0)(lastLapTime 0)(opponents 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200)(racePos 1)(rpm 942.478)(speedX 0)(speedY 0)(speedZ 0.0196266)(track 4.00001 4.06171 4.25672 4.61881 5.22164 6.22291 8.00001 11.6952 23.0351 200 46.0701 23.3904 16 12.4458 10.4433 9.2376 8.51342 8.12341 7.99999)(trackPos -1.333332)(wheelSpinVel 0 0 0 0)(z 0.339955)(focus -1 -1 -1 -1 -1)";
    private static final String PC_WARMUP_ONTRACK = "(angle -1.1)(curLapTime 10.21)(damage 0)(distFromStart 6201.46)(distRaced 0)(fuel 94)(gear 0)(lastLapTime 0)(opponents 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200 200)(racePos 1)(rpm 942.478)(speedX 0)(speedY 0)(speedZ 0.0196266)(track 4.00001 4.06171 4.25672 4.61881 5.22164 6.22291 8.00001 11.6952 23.0351 200 46.0701 23.3904 16 12.4458 10.4433 9.2376 8.51342 8.12341 7.99999)(trackPos 0.333332)(wheelSpinVel 0 0 0 0)(z 0.339955)(focus -1 -1 -1 -1 -1)";

    public BaseController(Telemetry t) {
        this.telemetry = t;
        trackDB = TrackDB.create();
    }

    public void setParameters(Properties params) {
    }

    public void getParameters(Properties params) {
    }

    @Override
    public final champ2011client.Action control(champ2011client.SensorModel model) {
        try {
            return unsaveControl(model);

        } catch (RuntimeException e) {
            if (TEXT_DEBUG) {
                System.out.println("Warning, calling control() caused an exception...");
                e.printStackTrace(System.out);
                if (e.getCause() != null) {
                    System.out.println("Cause:");
                    e.getCause().printStackTrace(System.out);
                }
                System.out.println("Trying to use the recovery behaviour");
            }

            functionalReset();
            wasRecovering = true;

            ModifiableAction action = new ModifiableAction();

            try {
                SensorData data = new SensorData(model);
                Situation sit = classifier.classify(data);
                if (noise.isNoisy()) {
                    sit = new Situation(Situations.ERROR_UNABLE_TO_CLASSIFY, 0, 0);
                }
                backupBehaviour.setSituation(sit);
                backupBehaviour.execute(data, action);

            } catch (RuntimeException e2) {
                if (TEXT_DEBUG) {
                    System.out.println("BackupBehaviour crashed too...");
                    e2.printStackTrace(System.out);
                    System.out.println("Returning a save action...");
                }
                action.setAcceleration(0.5);
                action.setBrake(0.0);
                action.setClutch(0.0);
                action.setSteering(0.0);
                action.setGear(model.getGear());
                action.setRestartRace(false);
                action.setFocusAngle(0);
            }

            return action.getRaceClientAction();
        }
    }

    public abstract champ2011client.Action unsaveControl(champ2011client.SensorModel model);

    protected void logSituation(Situation s) {
        controllerLog.append(Situations.toShortString(s.getType()));
        controllerLog.append(" ");
        controllerLog.append(Situations.toShortString(s.getDirection()));
        controllerLog.append(" ");
        controllerLog.append(Utils.dTS(s.getMeasure()));
    }

    protected void logTrackSegment(SensorData data, int index, TrackSegment current) {
        controllerLog.append("#");
        controllerLog.append(index);
        if (current.isUnknown()) {
            controllerLog.append("u");
        } else {
            controllerLog.append(Situations.toShortString(current.getType()));
            if (current.isCorner()) {
                int subIndex = current.getIndex(data.getDistanceFromStartLine());
                TrackSubSegment subSegment = current.getSubSegment(subIndex);
                controllerLog.append("[");
                controllerLog.append(subIndex);
                controllerLog.append("]");
                controllerLog.append(Situations.toShortString(subSegment.getType()));
            }
        }

        controllerLog.append("/");
    }

    /**
     * Method which executes two gameticks. This can be called in the constructor
     * of a derived class and will hopefully cause the jit compiler to do its job
     * at a time when it doesn't hurt.
     */
    protected void precompile() {
        //System.out.println("************** PRECOMPILE ***********************");
        precompile = true;
        //Stage oldStage = getStage();
        String oldTrackName = getTrackName();
        Telemetry tb = telemetry;
        telemetry = null;

        // precompile
        // warmup ontrack first packet
        setStage(Stage.WARMUP);
        setTrackName("precompile");
        resetFull();
        control(new champ2011client.MessageBasedSensorModel(PC_WARMUP_FIRST));
        // second packet
        setStage(Stage.WARMUP);
        resetFull();
        control(new champ2011client.MessageBasedSensorModel(PC_WARMUP_SECOND));

        // offtrack
        setStage(Stage.WARMUP);
        resetFull();
        control(new champ2011client.MessageBasedSensorModel(PC_WARMUP_OFFTRACK));
        resetFull();

        // ontrack
        setStage(Stage.WARMUP);
        resetFull();
        control(new champ2011client.MessageBasedSensorModel(PC_WARMUP_ONTRACK));
        resetFull();

        Telemetry data = Telemetry.createPrecompile();
        data.shutdown();
        setTrackName("precompile2");
        resetFull();
        for (int i = 0; i < data.size() / 2; ++i) {
            //System.out.println("Packet["+i+"]");
            champ2011client.Action a = control(data.getSensorData(i).getSensorModel());
        }
        setTrackName("precompile2");
        resetFull();
        java.util.Random r = new java.util.Random(System.currentTimeMillis());
        for (int i = 0; i < data.size() / 2; ++i) {
            SensorData wonoise = data.getSensorData(i);
            double[] track = wonoise.getTrackEdgeSensors();
            for (int l = 0; l < track.length; ++l) {
                double random = r.nextGaussian() * 0.1;
                random += 1.0;
                track[l] *= random;
            }
            ModifiableSensorData wnoise = new ModifiableSensorData();
            wnoise.setData(wonoise);
            wnoise.setTrackEdgeSensors(track);
            champ2011client.Action a = control(wnoise.getSensorModel());
        }

        resetFull();

        data = Telemetry.createPrecompile2();
        data.shutdown();
        setTrackName("Wheel 2");
        setStage(Stage.RACE);
        resetFull();
        for (int i = 0; i < data.size(); ++i) {
            //System.out.println("Packet["+i+"]");
            champ2011client.Action a = control(data.getSensorData(i).getSensorModel());
        }

        resetFull();
        precompile = false;

        telemetry = tb;
        //setStage(oldStage);  // doesn't make sense, since this hasn't been set
        setTrackName(oldTrackName);
        trackModel = new TrackModel();
        resetFull();
        //System.out.println("************** PRECOMPILE END *******************");
    }

    public int getParameterSetCount() {
        return 0;
    }

    public void selectParameterSet(int i) {
    }

    @Override
    public void reset() {
        firstPacket = true;
    }

    protected void functionalReset(){        
    }

    public void resetFull() {
        firstPacket = true;
    }

    @Override
    public void shutdown() {
    }
}
