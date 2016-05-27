/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * VisualizationFrame.java
 *
 * Created on 06.12.2008, 13:01:17
 */
package de.janquadflieg.mrracer.gui;

import java.awt.*;
import java.awt.event.AWTEventListener;
import java.util.*;
import javax.swing.*;

import de.janquadflieg.mrracer.Configuration;
import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.network.*;
import de.janquadflieg.mrracer.telemetry.*;
import de.janquadflieg.mrracer.track.TrackDB;

/**
 * Main window for the data visualization toolkit.
 *
 * @author Jan Quadflieg
 */
public class VisualizationFrame
        extends javax.swing.JFrame
        implements ConnectionListener, TelemetryListener {

    private Configuration config = new Configuration();
    /** The component which visalizes the track sensors. */
    private TrackSensorDisplay trackSensorDisplay;
    /** The component which visalizes the opponent sensors. */
    private OpponentSensorDisplay opponentSensorDisplay;
    /** The component which visalizes the telemetry data. */
    private TelemetryDisplay telemetryDisplay;
    /** The current connection. */
    private Connection connection = null;
    /** The current controller. */
    private champ2011client.Controller controller = null;
    /** Telemetry data object. */
    private Telemetry telemetry = new Telemetry();
    // a hack - oder so, naja nicht so ganz - EIN BUFFER!
    private ModifiableSensorData data = new ModifiableSensorData();
    private ModifiableAction action = new ModifiableAction();
    private String controllerlog = "NO DATA";
    /** Filechooser. */
    private JFileChooser fileChooser = new JFileChooser();

    /** Creates new form VisualizationFrame */
    public VisualizationFrame() {
        super();

        config.init();

        setLookAndFeel();
        initComponents();
        initCustomComponents();
        initDisplay();

        // fill the list of controllers
        Iterator<Configuration.ControllerInfo> it = config.getControllers();
        Configuration.ControllerInfo toSelect = null;
        while (it.hasNext()) {
            Configuration.ControllerInfo ci = it.next();
            jcbController.addItem(ci);

            if (ci.toString().equalsIgnoreCase("MrRacer2011")) {
                toSelect = ci;
            }
        }
        for (int i = 0; i < champ2011client.Controller.Stage.values().length; ++i) {
            this.jcbStage.addItem(champ2011client.Controller.Stage.values()[i]);
        }

        // fill the list of track models
        TrackDB trackDB = TrackDB.create();
        ArrayList<String> tracks = new ArrayList<String>();
        for(int i=0; i < trackDB.size(); ++i){
            tracks.add(trackDB.get(i).getName());
        }
        java.util.Collections.sort(tracks);
        for(int i=0; i < tracks.size(); ++i){
            this.jcbTrack.addItem(tracks.get(i));
        }


        telemetry.addListener(this);

        doLayout();
        setExtendedState(JFrame.MAXIMIZED_BOTH);

        if (config.getComputer() == Configuration.DESKTOP) {
            setTitle(getTitle() + " - Desktop");

        } else if (config.getComputer() == Configuration.HIWI_POOL) {
            setTitle(getTitle() + " - Hiwi Pool");

        } else if (config.getComputer() == Configuration.THINKPAD) {
            setTitle(getTitle() + " - Thinkpad");
        }

        jtfServer.setText(config.getServerIP());

        if (toSelect != null) {
            jcbController.setSelectedItem(toSelect);
        }
        jcbStage.setSelectedItem(champ2011client.Controller.Stage.RACE);

        setVisible(true);
    }

    private void initDisplay() {
        if (config.getComputer() == Configuration.DESKTOP) {
            GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
            GraphicsDevice[] gs = ge.getScreenDevices();

            if (gs.length > 1) {
                setBounds(gs[1].getDefaultConfiguration().getBounds());
            }

//            for (int j = 0; j < gs.length; j++) {
//                System.out.println("Screen Device #"+j+" "+gs[j].toString());
//
//                GraphicsDevice gd = gs[j];
//
//                System.out.println("Graphics Device #"+j+" "+gd.toString());
//
//
//                GraphicsConfiguration gc = gd.getDefaultConfiguration();
//                System.out.println("Default Graphics Configuration "+gc);
//                System.out.println("Bounds "+gc.getBounds());
//            }

            //this.setBounds();
        }
    }

    public void setSensorData(SensorData m, de.janquadflieg.mrracer.telemetry.Action a, String l) {
        int lastFocusAngle = 0;
        if (m.focusAvailable()) {
            lastFocusAngle = action.getFocusAngle();
        }
        this.data.setData(m);
        this.trackSensorDisplay.setSensorData(data, lastFocusAngle);
        this.opponentSensorDisplay.setSensorData(data);

        this.action.setData(a);

        this.controllerlog = l;

        SwingUtilities.invokeLater(new Runnable() {

            @Override
            public void run() {
                updateSensorData();
            }
        });
    }

    private void updateSensorData() {
        jtfLatSpeed.setText(data.getLateralSpeedS());
        jtfSpeed.setText(data.getSpeedS());
        jtfLapDistance.setText(data.getDistanceFromStartLineS());
        jtfLapTime.setText(data.getCurrentLapTimeS());
        jtfLastLap.setText(Utils.dTS(telemetry.getLastLapTime()));
        jtfBestLap.setText(Utils.dTS(telemetry.getBestLapTime()));
        jtfRaceDistance.setText(data.getDistanceRacedS());
        jtfRPM.setText(data.getRPMS());
        jtfTrackPosition.setText(data.getTrackPositionS());
        jtfTrackAngle.setText(data.getAngleToTrackAxisS());
        jtfRacePos.setText(data.getRacePositionS());
        jtfGear.setText(data.getGearS());
        jtfDamage.setText(data.getDamageS());
        jtfFuel.setText(data.getFuelLevelS());
        jtfZPosition.setText(data.getTrackHeightS());
        double[] wheelSpin = data.getWheelSpinVelocity();
        jtfWheelSpinFL.setText(Utils.dTS(wheelSpin[0]));
        jtfWheelSpinFR.setText(Utils.dTS(wheelSpin[1]));
        jtfWheelSpinRL.setText(Utils.dTS(wheelSpin[2]));
        jtfWheelSpinRR.setText(Utils.dTS(wheelSpin[3]));
        jtfZSpeed.setText(data.getZSpeedS());

        jtfSteering.setText(action.getSteeringS());
        jtfAcceleration.setText(action.getAccelerationS());
        jtfBrake.setText(action.getBrakeS());
        jtfClutch.setText(action.getClutchS());
        jtfFocusAngle.setText(action.getFocusAngleS());

        jtfControllerLog.setText(controllerlog);
    }

    private void updateConnectionStatistics(final ConnectionStatistics conStats) {
        this.jtfConPPS.setText(conStats.getPPSS());
        this.jtfConAvgLat.setText(conStats.getAvgLatencyS());
        this.jtfConMinLat.setText(conStats.getMinLatencyS());
        this.jtfConMaxLat.setText(conStats.getMaxLatencyS());
    }

    private void setLookAndFeel() {
        try {
            javax.swing.UIManager.setLookAndFeel(
                    javax.swing.UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void initCustomComponents() {
        trackSensorDisplay = new TrackSensorDisplay();
        GridBagConstraints tsdCon = new GridBagConstraints();
        tsdCon.gridx = 0;
        tsdCon.gridy = 0;
        tsdCon.fill = GridBagConstraints.BOTH;
        tsdCon.weightx = 1.0;
        tsdCon.weighty = 1.0;
        jpTrackSensor.add(trackSensorDisplay, tsdCon);

        opponentSensorDisplay = new OpponentSensorDisplay();
        GridBagConstraints osdCon = new GridBagConstraints();
        osdCon.gridx = 0;
        osdCon.gridy = 0;
        osdCon.fill = GridBagConstraints.BOTH;
        osdCon.weightx = 1.0;
        osdCon.weighty = 1.0;
        jpOpponentSensor.add(opponentSensorDisplay, osdCon);

        telemetryDisplay = new TelemetryDisplay(telemetry);
        GridBagConstraints teleCon = new GridBagConstraints();
        teleCon.gridx = 0;
        teleCon.gridy = 0;
        teleCon.fill = GridBagConstraints.BOTH;
        teleCon.weightx = 1.0;
        teleCon.weighty = 1.0;
        jpTelemetry.add(this.telemetryDisplay, teleCon);

        try {
            fileChooser.setCurrentDirectory(new java.io.File(config.getDataPath()));

        } catch (Exception e) {
            System.out.println("Warning, unable to set default save directory: " + config.getDataPath());
        }
    }

    private void disableGui() {
        jcbRecordTelemetry.setEnabled(false);
        
    }

    private void enableGui() {
        jcbRecordTelemetry.setEnabled(true);
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        jtpMainTabPane = new javax.swing.JTabbedPane();
        jpSensors = new javax.swing.JPanel();
        jpTrackSensor = new javax.swing.JPanel();
        jpOpponentSensor = new javax.swing.JPanel();
        jpTelemetry = new javax.swing.JPanel();
        jpOtherSensors = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        jLabel10 = new javax.swing.JLabel();
        jtfTrackPosition = new javax.swing.JTextField();
        jtfTrackAngle = new javax.swing.JTextField();
        jLabel5 = new javax.swing.JLabel();
        jtfZPosition = new javax.swing.JTextField();
        jPanel2 = new javax.swing.JPanel();
        jLabel4 = new javax.swing.JLabel();
        jtfLapTime = new javax.swing.JTextField();
        jLabel7 = new javax.swing.JLabel();
        jLabel3 = new javax.swing.JLabel();
        jtfLapDistance = new javax.swing.JTextField();
        jtfRaceDistance = new javax.swing.JTextField();
        jtfLastLap = new javax.swing.JTextField();
        jtfBestLap = new javax.swing.JTextField();
        jtfRacePos = new javax.swing.JTextField();
        jPanel3 = new javax.swing.JPanel();
        jLabel6 = new javax.swing.JLabel();
        jtfRPM = new javax.swing.JTextField();
        jLabel1 = new javax.swing.JLabel();
        jtfSpeed = new javax.swing.JTextField();
        jtfLatSpeed = new javax.swing.JTextField();
        jtfGear = new javax.swing.JTextField();
        jLabel11 = new javax.swing.JLabel();
        jtfZSpeed = new javax.swing.JTextField();
        jPanel5 = new javax.swing.JPanel();
        jLabel17 = new javax.swing.JLabel();
        jtfDamage = new javax.swing.JTextField();
        jtfFuel = new javax.swing.JTextField();
        jLabel8 = new javax.swing.JLabel();
        jtfWheelSpinFL = new javax.swing.JTextField();
        jtfWheelSpinFR = new javax.swing.JTextField();
        jLabel9 = new javax.swing.JLabel();
        jtfWheelSpinRL = new javax.swing.JTextField();
        jtfWheelSpinRR = new javax.swing.JTextField();
        jPanel4 = new javax.swing.JPanel();
        jLabel13 = new javax.swing.JLabel();
        jtfConPPS = new javax.swing.JTextField();
        jtfConAvgLat = new javax.swing.JTextField();
        jLabel15 = new javax.swing.JLabel();
        jtfConMinLat = new javax.swing.JTextField();
        jtfConMaxLat = new javax.swing.JTextField();
        jbControllerStart = new javax.swing.JButton();
        jbControllerStop = new javax.swing.JButton();
        jLabel22 = new javax.swing.JLabel();
        jcbController = new javax.swing.JComboBox();
        jLabel23 = new javax.swing.JLabel();
        jtfServer = new javax.swing.JTextField();
        jLabel24 = new javax.swing.JLabel();
        jcbStage = new javax.swing.JComboBox();
        jLabel2 = new javax.swing.JLabel();
        jcbRecordTelemetry = new javax.swing.JCheckBox();
        jcbTrack = new javax.swing.JComboBox();
        jPanel6 = new javax.swing.JPanel();
        jLabel19 = new javax.swing.JLabel();
        jtfSteering = new javax.swing.JTextField();
        jLabel20 = new javax.swing.JLabel();
        jtfAcceleration = new javax.swing.JTextField();
        jLabel21 = new javax.swing.JLabel();
        jtfBrake = new javax.swing.JTextField();
        jtfControllerLog = new javax.swing.JTextField();
        jLabel12 = new javax.swing.JLabel();
        jtfClutch = new javax.swing.JTextField();
        jLabel14 = new javax.swing.JLabel();
        jtfFocusAngle = new javax.swing.JTextField();
        jMenuBar1 = new javax.swing.JMenuBar();
        jMenu1 = new javax.swing.JMenu();
        jMenuItem1 = new javax.swing.JMenuItem();
        jMenuItem2 = new javax.swing.JMenuItem();
        jMenuItem3 = new javax.swing.JMenuItem();
        jMenu2 = new javax.swing.JMenu();
        jcbmiTSCoordinates = new javax.swing.JCheckBoxMenuItem();
        jcbmiTSSensors = new javax.swing.JCheckBoxMenuItem();
        jcbmiTSOutline = new javax.swing.JCheckBoxMenuItem();
        jcbmiTSCapOff = new javax.swing.JCheckBoxMenuItem();
        jcbmiTSFocus = new javax.swing.JCheckBoxMenuItem();
        jcbmiTSFiltered = new javax.swing.JCheckBoxMenuItem();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("MrRacer");
        getContentPane().setLayout(new java.awt.GridBagLayout());

        jpSensors.setName("Sensordata"); // NOI18N
        jpSensors.setLayout(new java.awt.GridBagLayout());

        jpTrackSensor.setBorder(javax.swing.BorderFactory.createTitledBorder("Track Sensor"));
        jpTrackSensor.setLayout(new java.awt.GridBagLayout());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.4;
        gridBagConstraints.weighty = 1.0;
        jpSensors.add(jpTrackSensor, gridBagConstraints);

        jpOpponentSensor.setBorder(javax.swing.BorderFactory.createTitledBorder("Opponent Sensor"));
        jpOpponentSensor.setLayout(new java.awt.GridBagLayout());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.4;
        gridBagConstraints.weighty = 1.0;
        jpSensors.add(jpOpponentSensor, gridBagConstraints);

        jtpMainTabPane.addTab("Sensordata", jpSensors);

        jpTelemetry.setLayout(new java.awt.GridBagLayout());
        jtpMainTabPane.addTab("Telemetry", jpTelemetry);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        getContentPane().add(jtpMainTabPane, gridBagConstraints);

        jpOtherSensors.setBorder(javax.swing.BorderFactory.createTitledBorder("Other Sensors"));
        jpOtherSensors.setLayout(new java.awt.GridBagLayout());

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder("Road Position"));
        jPanel1.setLayout(new java.awt.GridBagLayout());

        jLabel10.setText("Position / Angle");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel1.add(jLabel10, gridBagConstraints);

        jtfTrackPosition.setColumns(10);
        jtfTrackPosition.setEditable(false);
        jtfTrackPosition.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfTrackPosition.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfTrackPosition.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel1.add(jtfTrackPosition, gridBagConstraints);

        jtfTrackAngle.setColumns(10);
        jtfTrackAngle.setEditable(false);
        jtfTrackAngle.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfTrackAngle.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfTrackAngle.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel1.add(jtfTrackAngle, gridBagConstraints);

        jLabel5.setText("Z-Position");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel1.add(jLabel5, gridBagConstraints);

        jtfZPosition.setColumns(10);
        jtfZPosition.setEditable(false);
        jtfZPosition.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfZPosition.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfZPosition.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel1.add(jtfZPosition, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        jpOtherSensors.add(jPanel1, gridBagConstraints);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder("Race"));
        jPanel2.setLayout(new java.awt.GridBagLayout());

        jLabel4.setText("Position / Time");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel2.add(jLabel4, gridBagConstraints);

        jtfLapTime.setColumns(10);
        jtfLapTime.setEditable(false);
        jtfLapTime.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfLapTime.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfLapTime.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel2.add(jtfLapTime, gridBagConstraints);

        jLabel7.setText("Last / Best time");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel2.add(jLabel7, gridBagConstraints);

        jLabel3.setText("Start / Race distance");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel2.add(jLabel3, gridBagConstraints);

        jtfLapDistance.setColumns(10);
        jtfLapDistance.setEditable(false);
        jtfLapDistance.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfLapDistance.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfLapDistance.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel2.add(jtfLapDistance, gridBagConstraints);

        jtfRaceDistance.setColumns(10);
        jtfRaceDistance.setEditable(false);
        jtfRaceDistance.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfRaceDistance.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfRaceDistance.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel2.add(jtfRaceDistance, gridBagConstraints);

        jtfLastLap.setColumns(10);
        jtfLastLap.setEditable(false);
        jtfLastLap.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfLastLap.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfLastLap.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel2.add(jtfLastLap, gridBagConstraints);

        jtfBestLap.setColumns(10);
        jtfBestLap.setEditable(false);
        jtfBestLap.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfBestLap.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfBestLap.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel2.add(jtfBestLap, gridBagConstraints);

        jtfRacePos.setColumns(10);
        jtfRacePos.setEditable(false);
        jtfRacePos.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfRacePos.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfRacePos.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel2.add(jtfRacePos, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        jpOtherSensors.add(jPanel2, gridBagConstraints);

        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder("Engine"));
        jPanel3.setLayout(new java.awt.GridBagLayout());

        jLabel6.setText("RPM / Gear");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel3.add(jLabel6, gridBagConstraints);

        jtfRPM.setColumns(10);
        jtfRPM.setEditable(false);
        jtfRPM.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfRPM.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfRPM.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel3.add(jtfRPM, gridBagConstraints);

        jLabel1.setText("Speed / Lateral");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel3.add(jLabel1, gridBagConstraints);

        jtfSpeed.setColumns(10);
        jtfSpeed.setEditable(false);
        jtfSpeed.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfSpeed.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfSpeed.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel3.add(jtfSpeed, gridBagConstraints);

        jtfLatSpeed.setColumns(10);
        jtfLatSpeed.setEditable(false);
        jtfLatSpeed.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfLatSpeed.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfLatSpeed.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel3.add(jtfLatSpeed, gridBagConstraints);

        jtfGear.setColumns(10);
        jtfGear.setEditable(false);
        jtfGear.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfGear.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfGear.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel3.add(jtfGear, gridBagConstraints);

        jLabel11.setText("Z-Speed");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel3.add(jLabel11, gridBagConstraints);

        jtfZSpeed.setColumns(10);
        jtfZSpeed.setEditable(false);
        jtfZSpeed.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfZSpeed.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfZSpeed.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel3.add(jtfZSpeed, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        jpOtherSensors.add(jPanel3, gridBagConstraints);

        jPanel5.setBorder(javax.swing.BorderFactory.createTitledBorder("Car"));
        jPanel5.setLayout(new java.awt.GridBagLayout());

        jLabel17.setText("Damage / Fuel");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel5.add(jLabel17, gridBagConstraints);

        jtfDamage.setColumns(10);
        jtfDamage.setEditable(false);
        jtfDamage.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfDamage.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfDamage.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel5.add(jtfDamage, gridBagConstraints);

        jtfFuel.setColumns(10);
        jtfFuel.setEditable(false);
        jtfFuel.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfFuel.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfFuel.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel5.add(jtfFuel, gridBagConstraints);

        jLabel8.setText("Wheel Spin FL / FR");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel5.add(jLabel8, gridBagConstraints);

        jtfWheelSpinFL.setColumns(10);
        jtfWheelSpinFL.setEditable(false);
        jtfWheelSpinFL.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfWheelSpinFL.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfWheelSpinFL.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel5.add(jtfWheelSpinFL, gridBagConstraints);

        jtfWheelSpinFR.setColumns(10);
        jtfWheelSpinFR.setEditable(false);
        jtfWheelSpinFR.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfWheelSpinFR.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfWheelSpinFR.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel5.add(jtfWheelSpinFR, gridBagConstraints);

        jLabel9.setText("Wheel Spin RL / RR");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel5.add(jLabel9, gridBagConstraints);

        jtfWheelSpinRL.setColumns(10);
        jtfWheelSpinRL.setEditable(false);
        jtfWheelSpinRL.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfWheelSpinRL.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfWheelSpinRL.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel5.add(jtfWheelSpinRL, gridBagConstraints);

        jtfWheelSpinRR.setColumns(10);
        jtfWheelSpinRR.setEditable(false);
        jtfWheelSpinRR.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfWheelSpinRR.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfWheelSpinRR.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel5.add(jtfWheelSpinRR, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        jpOtherSensors.add(jPanel5, gridBagConstraints);

        jPanel4.setBorder(javax.swing.BorderFactory.createTitledBorder("Connection"));
        jPanel4.setLayout(new java.awt.GridBagLayout());

        jLabel13.setText("Pps / Avg Latency");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel4.add(jLabel13, gridBagConstraints);

        jtfConPPS.setColumns(10);
        jtfConPPS.setEditable(false);
        jtfConPPS.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfConPPS.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfConPPS.setText("NO DATA");
        jtfConPPS.setMinimumSize(new java.awt.Dimension(80, 20));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.EAST;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel4.add(jtfConPPS, gridBagConstraints);

        jtfConAvgLat.setColumns(10);
        jtfConAvgLat.setEditable(false);
        jtfConAvgLat.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfConAvgLat.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfConAvgLat.setText("NO DATA");
        jtfConAvgLat.setMinimumSize(new java.awt.Dimension(80, 20));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.EAST;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel4.add(jtfConAvgLat, gridBagConstraints);

        jLabel15.setText("Min / Max Latency");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel4.add(jLabel15, gridBagConstraints);

        jtfConMinLat.setColumns(10);
        jtfConMinLat.setEditable(false);
        jtfConMinLat.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfConMinLat.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfConMinLat.setText("NO DATA");
        jtfConMinLat.setMinimumSize(new java.awt.Dimension(80, 20));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.EAST;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel4.add(jtfConMinLat, gridBagConstraints);

        jtfConMaxLat.setColumns(10);
        jtfConMaxLat.setEditable(false);
        jtfConMaxLat.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfConMaxLat.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfConMaxLat.setText("NO DATA");
        jtfConMaxLat.setMinimumSize(new java.awt.Dimension(80, 20));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.EAST;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel4.add(jtfConMaxLat, gridBagConstraints);

        jbControllerStart.setLabel("Start");
        jbControllerStart.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jbControllerStartActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 9;
        jPanel4.add(jbControllerStart, gridBagConstraints);

        jbControllerStop.setEnabled(false);
        jbControllerStop.setLabel("Stop");
        jbControllerStop.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jbControllerStopActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 9;
        jPanel4.add(jbControllerStop, gridBagConstraints);

        jLabel22.setText("Controller");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel4.add(jLabel22, gridBagConstraints);

        jcbController.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                jcbControllerItemStateChanged(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel4.add(jcbController, gridBagConstraints);

        jLabel23.setText("Server");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel4.add(jLabel23, gridBagConstraints);

        jtfServer.setColumns(20);
        jtfServer.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfServer.setHorizontalAlignment(javax.swing.JTextField.RIGHT);
        jtfServer.setText("IP:PORT");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel4.add(jtfServer, gridBagConstraints);

        jLabel24.setText("Stage");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 6;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel4.add(jLabel24, gridBagConstraints);

        jcbStage.setEnabled(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 6;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel4.add(jcbStage, gridBagConstraints);

        jLabel2.setText("Track");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 7;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel4.add(jLabel2, gridBagConstraints);

        jcbRecordTelemetry.setSelected(true);
        jcbRecordTelemetry.setText("Record Telemetry");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 8;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 5);
        jPanel4.add(jcbRecordTelemetry, gridBagConstraints);

        jcbTrack.setEditable(true);
        jcbTrack.setFont(new java.awt.Font("Courier New", 0, 11));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 7;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel4.add(jcbTrack, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        jpOtherSensors.add(jPanel4, gridBagConstraints);

        jPanel6.setBorder(javax.swing.BorderFactory.createTitledBorder("Action"));
        jPanel6.setLayout(new java.awt.GridBagLayout());

        jLabel19.setText("Steering");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel6.add(jLabel19, gridBagConstraints);

        jtfSteering.setColumns(10);
        jtfSteering.setEditable(false);
        jtfSteering.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfSteering.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfSteering.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel6.add(jtfSteering, gridBagConstraints);

        jLabel20.setText("Acceleration");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel6.add(jLabel20, gridBagConstraints);

        jtfAcceleration.setColumns(10);
        jtfAcceleration.setEditable(false);
        jtfAcceleration.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfAcceleration.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfAcceleration.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel6.add(jtfAcceleration, gridBagConstraints);

        jLabel21.setText("Brake");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel6.add(jLabel21, gridBagConstraints);

        jtfBrake.setColumns(10);
        jtfBrake.setEditable(false);
        jtfBrake.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfBrake.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfBrake.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel6.add(jtfBrake, gridBagConstraints);

        jtfControllerLog.setEditable(false);
        jtfControllerLog.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfControllerLog.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 5);
        jPanel6.add(jtfControllerLog, gridBagConstraints);

        jLabel12.setText("Clutch");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel6.add(jLabel12, gridBagConstraints);

        jtfClutch.setColumns(10);
        jtfClutch.setEditable(false);
        jtfClutch.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfClutch.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfClutch.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel6.add(jtfClutch, gridBagConstraints);

        jLabel14.setText("Focus angle");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 2);
        jPanel6.add(jLabel14, gridBagConstraints);

        jtfFocusAngle.setColumns(10);
        jtfFocusAngle.setEditable(false);
        jtfFocusAngle.setFont(new java.awt.Font("Courier New", 0, 11));
        jtfFocusAngle.setHorizontalAlignment(javax.swing.JTextField.TRAILING);
        jtfFocusAngle.setText("NO DATA");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 5);
        jPanel6.add(jtfFocusAngle, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        jpOtherSensors.add(jPanel6, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 1.0;
        getContentPane().add(jpOtherSensors, gridBagConstraints);

        jMenu1.setText("Telemetry");

        jMenuItem1.setText("Load");
        jMenuItem1.setEnabled(false);
        jMenu1.add(jMenuItem1);

        jMenuItem2.setText("Save");
        jMenuItem2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveTelemetry(evt);
            }
        });
        jMenu1.add(jMenuItem2);

        jMenuItem3.setText("Clear");
        jMenuItem3.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                clearTelemetry(evt);
            }
        });
        jMenu1.add(jMenuItem3);

        jMenuBar1.add(jMenu1);

        jMenu2.setText("Track Sensor");

        jcbmiTSCoordinates.setSelected(true);
        jcbmiTSCoordinates.setText("Coordinates");
        jcbmiTSCoordinates.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                jcbmiTSCoordinatesItemStateChanged(evt);
            }
        });
        jMenu2.add(jcbmiTSCoordinates);

        jcbmiTSSensors.setSelected(true);
        jcbmiTSSensors.setText("Sensors");
        jcbmiTSSensors.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                jcbmiTSSensorsItemStateChanged(evt);
            }
        });
        jMenu2.add(jcbmiTSSensors);

        jcbmiTSOutline.setSelected(true);
        jcbmiTSOutline.setText("Outline");
        jcbmiTSOutline.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                trackSensorOutlineStateChanged(evt);
            }
        });
        jMenu2.add(jcbmiTSOutline);

        jcbmiTSCapOff.setSelected(true);
        jcbmiTSCapOff.setText("Cap Off");
        jcbmiTSCapOff.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                jcbmiTSCapOffItemStateChanged(evt);
            }
        });
        jMenu2.add(jcbmiTSCapOff);

        jcbmiTSFocus.setSelected(true);
        jcbmiTSFocus.setLabel("Focus Sensor");
        jcbmiTSFocus.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                jcbmiTSFocusItemStateChanged(evt);
            }
        });
        jMenu2.add(jcbmiTSFocus);

        jcbmiTSFiltered.setSelected(true);
        jcbmiTSFiltered.setLabel("Filtered sensors");
        jcbmiTSFiltered.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                jcbmiTSFilteredItemStateChanged(evt);
            }
        });
        jMenu2.add(jcbmiTSFiltered);

        jMenuBar1.add(jMenu2);

        setJMenuBar(jMenuBar1);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void saveTelemetry(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveTelemetry
        // TODO add your handling code here:
        fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
        fileChooser.setAcceptAllFileFilterUsed(true);
        fileChooser.setMultiSelectionEnabled(false);


        int result = fileChooser.showSaveDialog(this);

        if (result != JFileChooser.APPROVE_OPTION) {
            return;
        }

        // test
        telemetry.save(fileChooser.getSelectedFile());
    }//GEN-LAST:event_saveTelemetry

    private void jbControllerStopActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jbControllerStopActionPerformed
        // TODO add your handling code here:
        if (connection != null) {
            connection.stop();
            if (controller instanceof AWTEventListener) {
                Toolkit.getDefaultToolkit().removeAWTEventListener((AWTEventListener) controller);
            }
            if (controller instanceof GraphicDebugable) {
                JComponent[] components = ((GraphicDebugable) controller).getComponent();

                for (JComponent comp : components) {
                    try {
                        jtpMainTabPane.remove(comp);
                    } catch (ArrayIndexOutOfBoundsException e) {
                    }
                }
            }
            enableGui();
        }
}//GEN-LAST:event_jbControllerStopActionPerformed

    private void jbControllerStartActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jbControllerStartActionPerformed
        // TODO add your handling code here:
        try {
            disableGui();
            telemetry.clear();
            telemetry.setMode(Telemetry.Mode.MODE_RECORDING);

            String name = ((Configuration.ControllerInfo) jcbController.getSelectedItem()).name;
            Class<?> c = Class.forName("de.janquadflieg.mrracer.controller." + name);

            

            if(jcbRecordTelemetry.isSelected()){
                java.lang.reflect.Constructor<?> constructor = c.getConstructor(Class.forName("de.janquadflieg.mrracer.telemetry.Telemetry"));
                controller = (champ2011client.Controller) constructor.newInstance(telemetry);

            } else {
                java.lang.reflect.Constructor<?> constructor = c.getConstructor();
                controller = (champ2011client.Controller) constructor.newInstance();
            }
            controller.setStage((champ2011client.Controller.Stage) jcbStage.getSelectedItem());
            System.out.println(jcbTrack.getSelectedItem().toString());
            controller.setTrackName(jcbTrack.getSelectedItem().toString());
            if(controller instanceof de.janquadflieg.mrracer.controller.BaseController){
                ((de.janquadflieg.mrracer.controller.BaseController)controller).resetFull();

            } else {
                controller.reset();
            }

            trackSensorDisplay.setAngles(controller.initAngles());

            if (controller instanceof GraphicDebugable) {
                JComponent[] components = ((GraphicDebugable) controller).getComponent();
                for (int i = 0; i < components.length; ++i) {
                    if (components[i].getName() == null) {
                        components[i].setName("Debug Tab " + (i + 1));
                    }
                    this.jtpMainTabPane.add(components[i]);
                }
                //jpDebug.doLayout();
            }

            String ip = jtfServer.getText().substring(0, jtfServer.getText().indexOf(':'));
            int port = Integer.parseInt(jtfServer.getText().substring(jtfServer.getText().indexOf(':') + 1, jtfServer.getText().length()));

            connection = new UDPConnection(ip, port, controller);

            connection.addConnectionListener(this);
            this.jbControllerStop.setEnabled(true);
            this.jbControllerStart.setEnabled(false);

            if (controller instanceof AWTEventListener) {
                Toolkit.getDefaultToolkit().addAWTEventListener((AWTEventListener) controller, AWTEvent.KEY_EVENT_MASK);
            }

        } catch (Exception e) {
            e.printStackTrace();
            enableGui();
        }
}//GEN-LAST:event_jbControllerStartActionPerformed

    private void clearTelemetry(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_clearTelemetry
        telemetry.clear();
    }//GEN-LAST:event_clearTelemetry

    private void trackSensorOutlineStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_trackSensorOutlineStateChanged
        // TODO add your handling code here:
        this.trackSensorDisplay.setDrawOutline(this.jcbmiTSOutline.isSelected());
    }//GEN-LAST:event_trackSensorOutlineStateChanged

    private void jcbmiTSCoordinatesItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_jcbmiTSCoordinatesItemStateChanged
        // TODO add your handling code here:
        this.trackSensorDisplay.setDrawCoordinates(this.jcbmiTSCoordinates.isSelected());
}//GEN-LAST:event_jcbmiTSCoordinatesItemStateChanged

    private void jcbmiTSSensorsItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_jcbmiTSSensorsItemStateChanged
        // TODO add your handling code here:
        this.trackSensorDisplay.setDrawSensors(this.jcbmiTSSensors.isSelected());
    }//GEN-LAST:event_jcbmiTSSensorsItemStateChanged

    private void jcbmiTSCapOffItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_jcbmiTSCapOffItemStateChanged
        // TODO add your handling code here:
        this.trackSensorDisplay.setDrawCapOff(this.jcbmiTSCapOff.isSelected());
    }//GEN-LAST:event_jcbmiTSCapOffItemStateChanged

    private void jcbControllerItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_jcbControllerItemStateChanged
        // TODO add your handling code here:
        Configuration.ControllerInfo ci = (Configuration.ControllerInfo) this.jcbController.getSelectedItem();
        this.jcbStage.setEnabled(ci.i2010);
    }//GEN-LAST:event_jcbControllerItemStateChanged

    private void jcbmiTSFocusItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_jcbmiTSFocusItemStateChanged
        trackSensorDisplay.setDrawFocus(jcbmiTSFocus.isSelected());
    }//GEN-LAST:event_jcbmiTSFocusItemStateChanged

    private void jcbmiTSFilteredItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_jcbmiTSFilteredItemStateChanged
        this.trackSensorDisplay.setDrawFiltered(jcbmiTSFiltered.isSelected());
    }//GEN-LAST:event_jcbmiTSFilteredItemStateChanged

    @Override
    public void stopped(boolean requested) {
        this.telemetry.setMode(Telemetry.Mode.MODE_PAUSED);
        this.controller.shutdown();
        this.connection.removeConnectionListener(this);
        this.jbControllerStop.setEnabled(false);
        this.jbControllerStart.setEnabled(true);
        if (controller instanceof AWTEventListener) {
            Toolkit.getDefaultToolkit().removeAWTEventListener((AWTEventListener) controller);
        }
        if (controller instanceof GraphicDebugable) {
            JComponent[] components = ((GraphicDebugable) controller).getComponent();

            for (JComponent comp : components) {
                try {
                    jtpMainTabPane.remove(comp);
                } catch (ArrayIndexOutOfBoundsException e) {
                }
            }
        }
        this.enableGui();
        //System.out.println("Telemetry entries: "+telemetry.getSize());
    }

    @Override
    public void newStatistics(final ConnectionStatistics data) {
        SwingUtilities.invokeLater(new Runnable() {

            @Override
            public void run() {
                updateConnectionStatistics(data);
            }
        });
    }

    /**
     * Called, when a new lap has started.
     */
    @Override
    public void newLap() {
        System.out.println("new lap");
    }

    @Override
    public void cleared() {
        SensorData emptyData = new SensorData();
        de.janquadflieg.mrracer.telemetry.Action a = new de.janquadflieg.mrracer.telemetry.Action();
        setSensorData(emptyData, a, "");
    }

    /**
     * Called, when new data has been added.
     * @param mode Indicates, wether the underlying telemetry object is in
     * recording or playback mode.
     */
    @Override
    public void newData(SensorData data, de.janquadflieg.mrracer.telemetry.Action a, String l, Telemetry.Mode mode) {
        this.setSensorData(data, a, l);
    }

    @Override
    public void modeChanged(int newMode) {
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        //System.getProperties().list(System.out);
        //System.out.println(System.getProperty("user.dir"));
        java.awt.EventQueue.invokeLater(new Runnable() {

            @Override
            public void run() {
                new VisualizationFrame();
            }
        });
    }
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel12;
    private javax.swing.JLabel jLabel13;
    private javax.swing.JLabel jLabel14;
    private javax.swing.JLabel jLabel15;
    private javax.swing.JLabel jLabel17;
    private javax.swing.JLabel jLabel19;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel20;
    private javax.swing.JLabel jLabel21;
    private javax.swing.JLabel jLabel22;
    private javax.swing.JLabel jLabel23;
    private javax.swing.JLabel jLabel24;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JMenu jMenu1;
    private javax.swing.JMenu jMenu2;
    private javax.swing.JMenuBar jMenuBar1;
    private javax.swing.JMenuItem jMenuItem1;
    private javax.swing.JMenuItem jMenuItem2;
    private javax.swing.JMenuItem jMenuItem3;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JPanel jPanel6;
    private javax.swing.JButton jbControllerStart;
    private javax.swing.JButton jbControllerStop;
    private javax.swing.JComboBox jcbController;
    private javax.swing.JCheckBox jcbRecordTelemetry;
    private javax.swing.JComboBox jcbStage;
    private javax.swing.JComboBox jcbTrack;
    private javax.swing.JCheckBoxMenuItem jcbmiTSCapOff;
    private javax.swing.JCheckBoxMenuItem jcbmiTSCoordinates;
    private javax.swing.JCheckBoxMenuItem jcbmiTSFiltered;
    private javax.swing.JCheckBoxMenuItem jcbmiTSFocus;
    private javax.swing.JCheckBoxMenuItem jcbmiTSOutline;
    private javax.swing.JCheckBoxMenuItem jcbmiTSSensors;
    private javax.swing.JPanel jpOpponentSensor;
    private javax.swing.JPanel jpOtherSensors;
    private javax.swing.JPanel jpSensors;
    private javax.swing.JPanel jpTelemetry;
    private javax.swing.JPanel jpTrackSensor;
    private javax.swing.JTextField jtfAcceleration;
    private javax.swing.JTextField jtfBestLap;
    private javax.swing.JTextField jtfBrake;
    private javax.swing.JTextField jtfClutch;
    private javax.swing.JTextField jtfConAvgLat;
    private javax.swing.JTextField jtfConMaxLat;
    private javax.swing.JTextField jtfConMinLat;
    private javax.swing.JTextField jtfConPPS;
    private javax.swing.JTextField jtfControllerLog;
    private javax.swing.JTextField jtfDamage;
    private javax.swing.JTextField jtfFocusAngle;
    private javax.swing.JTextField jtfFuel;
    private javax.swing.JTextField jtfGear;
    private javax.swing.JTextField jtfLapDistance;
    private javax.swing.JTextField jtfLapTime;
    private javax.swing.JTextField jtfLastLap;
    private javax.swing.JTextField jtfLatSpeed;
    private javax.swing.JTextField jtfRPM;
    private javax.swing.JTextField jtfRaceDistance;
    private javax.swing.JTextField jtfRacePos;
    private javax.swing.JTextField jtfServer;
    private javax.swing.JTextField jtfSpeed;
    private javax.swing.JTextField jtfSteering;
    private javax.swing.JTextField jtfTrackAngle;
    private javax.swing.JTextField jtfTrackPosition;
    private javax.swing.JTextField jtfWheelSpinFL;
    private javax.swing.JTextField jtfWheelSpinFR;
    private javax.swing.JTextField jtfWheelSpinRL;
    private javax.swing.JTextField jtfWheelSpinRR;
    private javax.swing.JTextField jtfZPosition;
    private javax.swing.JTextField jtfZSpeed;
    private javax.swing.JTabbedPane jtpMainTabPane;
    // End of variables declaration//GEN-END:variables
}
