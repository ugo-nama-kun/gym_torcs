/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.classification.cig2011;

import flanagan.analysis.Regression;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.classification.AngleBasedClassifier;
import de.janquadflieg.mrracer.classification.AngleClassifierWithQuadraticRegression;
import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.telemetry.ModifiableSensorData;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.telemetry.Telemetry;

import java.awt.geom.Point2D;
import java.io.*;
import java.util.*;

/**
 *
 * @author quad
 */
public class Tools {

    public static AngleClassifierWithQuadraticRegression regClassifier = new AngleClassifierWithQuadraticRegression();
    public static AngleBasedClassifier classifier = new AngleBasedClassifier(true);
    public static final String BASE_PATH = "F:\\Quad\\Experiments\\CIG-2011\\Data\\work";
    public static final String[] RAW_INPUT_FILES = {
        "raw_wheel2_flyinglaps.txt",
        "raw_wheel2_slow_-0.8.txt",
        "raw_wheel2_slow_0.0.txt",
        "raw_wheel2_slow_0.8.txt",
        "raw_wheel2_slow_sinus_100.txt",
        "raw_wheel2_slow_sinus_77.txt"
    };
    public static final String[] MS_INPUT_FILES = {
        "m_wheel2_flyinglaps.txt",
        "m_wheel2_slow_-0.8.txt",
        "m_wheel2_slow_0.0.txt",
        "m_wheel2_slow_0.8.txt",
        "m_wheel2_slow_sinus_100.txt",
        "m_wheel2_slow_sinus_77.txt",
        "s_wheel2_flyinglaps.txt",
        "s_wheel2_slow_-0.8.txt",
        "s_wheel2_slow_0.0.txt",
        "s_wheel2_slow_0.8.txt",
        "s_wheel2_slow_sinus_100.txt",
        "s_wheel2_slow_sinus_77.txt"
    };
    //public static final int[] NOISE_LEVELS = {3, 5, 10, 15, 20};

    public static final int[] NOISE_LEVELS = {0};

    private static void copyTogether(String[] input)
            throws Exception {
        for (int i = 0; i < NOISE_LEVELS.length; ++i) {
            System.out.println("Noise level " + NOISE_LEVELS[i]);
            String out = BASE_PATH + File.separator + "n" + String.valueOf(NOISE_LEVELS[i]) + File.separator + "noisy_" + NOISE_LEVELS[i] + ".txt";
            System.out.println("Output: " + out);

            OutputStreamWriter writer = new OutputStreamWriter(new FileOutputStream(out), "UTF-8");
            StrippedData.writeHeader(writer);

            int ctr = 1;

            for (int k = 0; k < input.length; ++k) {
                String file = BASE_PATH + File.separator + "n" + String.valueOf(NOISE_LEVELS[i]) + File.separator + "n" + NOISE_LEVELS[i] + "_" + input[k];
                System.out.println("Input: " + file);
                BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(file), "UTF-8"));

                String line = reader.readLine();


                // ignore the header
                if (line != null && line.startsWith("R")) {
                    line = reader.readLine();
                }

                while (line != null && line.trim().length() != 0) {
                    writer.write(line);
                    writer.write('\n');
                    ++ctr;

                    line = reader.readLine();
                }
                writer.flush();
            }
            System.out.println(ctr + " entries");
            System.out.println("");
            writer.flush();
            writer.close();
        }
    }

    private static void makeNoisy(String[] input)
            throws Exception {
        Random r = new Random();

        for (int i = 0; i < input.length; ++i) {
            String file = BASE_PATH + File.separator + "mirrored" + File.separator + input[i];
            System.out.println(file);
            BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(file), "UTF-8"));

            OutputStreamWriter[] writer = new OutputStreamWriter[NOISE_LEVELS.length];
            for (int k = 0; k < NOISE_LEVELS.length; ++k) {
                String out = BASE_PATH + File.separator + "noisy" + File.separator + "n" + String.valueOf(NOISE_LEVELS[k]) + "_" + input[i];
                writer[k] = new OutputStreamWriter(new FileOutputStream(out), "UTF-8");
                StrippedData.writeHeader(writer[k]);
            }

            String line = reader.readLine();

            // ignore the header
            if (line != null && line.startsWith("R")) {
                line = reader.readLine();
            }

            while (line != null && line.trim().length() != 0) {
                StrippedData d = StrippedData.read(line);

                for (int k = 0; k < NOISE_LEVELS.length; ++k) {
                    StrippedData copy = new StrippedData(d);

                    for (int l = 0; l < copy.track.length; ++l) {
                        double random = r.nextGaussian() * (1.0 * NOISE_LEVELS[k]) * 0.01;
                        random += 1.0;
                        copy.track[l] *= random;
                    }

                    copy.measure = calcMeasure(copy);

                    copy.write(writer[k]);
                    writer[k].flush();
                }

                line = reader.readLine();
            }

            for (int k = 0; k < writer.length; ++k) {
                writer[k].flush();
                writer[k].close();
            }
        }
    }

    private static void mirrorAll(String[] input)
            throws Exception {
        for (int i = 0; i < input.length; ++i) {
            String file = BASE_PATH + File.separator + input[i];

            String s_file = BASE_PATH + File.separator + input[i].replace("raw_", "s_");
            String m_file = BASE_PATH + File.separator + input[i].replace("raw_", "m_");

            BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(file), "UTF-8"));
            OutputStreamWriter s_writer = new OutputStreamWriter(new FileOutputStream(s_file), "UTF-8");
            StrippedData.writeHeader(s_writer);
            OutputStreamWriter m_writer = new OutputStreamWriter(new FileOutputStream(m_file), "UTF-8");
            StrippedData.writeHeader(m_writer);

            String line = reader.readLine();

            // ignore the header
            if (line != null && line.startsWith("R")) {
                line = reader.readLine();
            }

            while (line != null) {
                line = line.substring(0, line.indexOf('§'));

                SensorData sdata = new SensorData(line, " ");
                Situation sit = classifier.classify(sdata);

                if (sdata.onTrack() && !sit.hasError()) {
                    StrippedData d = new StrippedData(sdata, 12.0);
                    StrippedData m = d.mirror();

                    d.correctMeasure = calcMeasure(d);
                    d.measure = d.correctMeasure;

                    m.correctMeasure = calcMeasure(m);
                    m.measure = m.correctMeasure;

                    d.write(s_writer);
                    s_writer.flush();
                    m.write(m_writer);
                    m_writer.flush();
                }

                line = reader.readLine();
            }
            s_writer.flush();
            s_writer.close();
            m_writer.flush();
            m_writer.close();
        }
    }

    private static void testRegression()
            throws Exception {
        //String file = BASE_PATH + File.separator + "n10" + File.separator + "n10_m_wheel2_flyinglaps.txt";
        String file = BASE_PATH + File.separator + "n3" + File.separator + "n3_m_wheel2_flyinglaps.txt";

        double minDev = Double.POSITIVE_INFINITY;
        double maxDev = Double.NEGATIVE_INFINITY;
        double avgDev = 0.0;
        int ctr = 0;

        //String file = BASE_PATH + File.separator + "mirrored" + File.separator + "m_wheel2_flyinglaps.txt";
        System.out.println("Input: " + file);
        BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(file), "UTF-8"));

        String line = reader.readLine();

        System.out.println("correctValue value");


        // ignore the header
        if (line != null && line.startsWith("R")) {
            line = reader.readLine();
        }

        while (line != null && line.trim().length() != 0) {
            StrippedData data = StrippedData.read(line);

            ModifiableSensorData sd = new ModifiableSensorData();
            sd.setTrackEdgeSensors(data.track);
            sd.setTrackPosition(data.position);
            sd.setAngleToTrackAxis(data.angle);

            try {
                double value = regClassifier.classify(sd).getMeasure();

                if (Math.abs(value) < 100.0) {
                    //System.out.println(Utils.dTS(value)+" "+Utils.dTS(data.correctMeasure));
                    System.out.println(data.correctMeasure + " " + value);

                    minDev = Math.min(Math.abs(data.correctMeasure - value), minDev);
                    maxDev = Math.max(Math.abs(data.correctMeasure - value), maxDev);
                    avgDev += Math.abs(data.correctMeasure - value);
                    ++ctr;
                }



            } catch (IllegalArgumentException e) {
                e.printStackTrace(System.out);
                System.out.println(line);
            }

            line = reader.readLine();
        }

        System.out.println("min abweichung: " + minDev);
        System.out.println("max abweichung: " + maxDev);
        System.out.println("durchschnitt: "+(avgDev/ctr));
    }

    private static double calcMeasure(SensorData d) {
        Situation s = classifier.classify(d);

        return s.getMeasure();
    }

    private static double calcMeasure(StrippedData d) {
        ModifiableSensorData data = new ModifiableSensorData();

        data.setTrackEdgeSensors(d.track);
        data.setTrackPosition(d.position);
        data.setAngleToTrackAxis(d.angle);

        Situation s = classifier.classify(data);

        return s.getMeasure();
    }

    public static void makeNoisyTelemetry()
    throws Exception{
        String file = "F:\\Quad\\svn\\Torcs-Paper\\TORCS\\CIG-2011\\experiments\\strecken\\nonoise.zip";

        Telemetry t = new Telemetry();
        t.shutdown();
        t.load(file);

        Telemetry noisy = makeNoisy(t);
        noisy.save(new File("F:\\Quad\\svn\\Torcs-Paper\\TORCS\\CIG-2011\\experiments\\strecken\\noisy.zip"));
    }

    public static Telemetry makeNoisy(Telemetry t){
        Telemetry result = new Telemetry();

        Random r = new Random(System.currentTimeMillis());

        for(int i=0; i < t.size(); ++i){
            ModifiableSensorData d = new ModifiableSensorData();
            d.setData(t.getSensorData(i));

            double[] sensors = new double[19];                   
            
            for(int k=0; k < sensors.length; ++k){
                sensors[k] = d.getTrackEdgeSensors()[k];

                double random = r.nextGaussian() * 0.1;
                random += 1.0;
                sensors[k] *= random;
            }

            d.setTrackEdgeSensors(sensors);

            result.log(d, t.getAction(i), t.getLog(i));
        }


        result.shutdown();
        return result;
    }

    public static void createRFile()
            throws Exception{
        String f1 = "F:\\Quad\\svn\\Torcs-Paper\\TORCS\\CIG-2011\\experiments\\strecken\\measure_no_noise.zip";
        //String f2 = "F:\\Quad\\svn\\Torcs-Paper\\TORCS\\CIG-2011\\experiments\\strecken\\measure_noise_naive.zip";
        //String f2 = "F:\\Quad\\svn\\Torcs-Paper\\TORCS\\CIG-2011\\experiments\\strecken\\measure_noise_final.zip";
        String f2 = "F:\\Quad\\svn\\Torcs-Paper\\TORCS\\CIG-2011\\experiments\\strecken\\measure_noise.zip";

        //String out = "F:\\Quad\\svn\\Torcs-Paper\\TORCS\\CIG-2011\\experiments\\strecken\\no_noise_naiv";
        String out = "F:\\Quad\\svn\\Torcs-Paper\\TORCS\\CIG-2011\\experiments\\strecken\\noise";

        Telemetry t1 = new Telemetry();
        t1.shutdown();
        t1.load(f1);

        Telemetry t2 = new Telemetry();
        t2.shutdown();
        t2.load(f2);

        FileWriter w = new FileWriter(out);
        w.write("correctValue Value"+"\n");

        System.out.println(Math.min(t1.size(), t2.size()));

        for(int i=0; i < Math.min(t1.size(), t2.size()); ++i){
            w.write(t1.getLog(i)+" "+t2.getLog(i)+"\n");
        }

        w.flush();
        w.close();
    }

    public static void main(String[] args) {
        try {
            //mirrorAll(RAW_INPUT_FILES);
            //makeNoisy(MS_INPUT_FILES);
            //copyTogether(MS_INPUT_FILES);
            //testRegression();
            //makeNoisyTelemetry();
            createRFile();

        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
    }

    private static class StrippedData {

        public double distanceRaced = 0.0;
        public double[] track = new double[19];
        public double angle = 0.0;
        public double position = 0.0;
        public double trackWidth = 0.0;
        public double correctMeasure = 0.0;
        public double measure = 0.0;

        public StrippedData() {
        }

        public StrippedData(StrippedData d) {
            this.distanceRaced = d.distanceRaced;
            System.arraycopy(d.track, 0, track, 0, track.length);
            this.angle = d.angle;
            this.position = d.position;
            this.trackWidth = d.trackWidth;
            this.correctMeasure = d.correctMeasure;
            this.measure = d.measure;
        }

        public StrippedData(SensorData data, double width) {
            this.distanceRaced = data.getDistanceRaced();
            System.arraycopy(data.getTrackEdgeSensors(), 0, track, 0, track.length);
            this.angle = data.getAngleToTrackAxis();
            this.position = data.getTrackPosition();
            this.trackWidth = width;
            this.correctMeasure = calcMeasure(data);
            this.measure = calcMeasure(data);
        }

        public StrippedData mirror() {
            StrippedData result = new StrippedData();

            for (int i = 0; i < result.track.length; ++i) {
                result.track[i] = track[(result.track.length - 1) - i];
            }
            result.distanceRaced = distanceRaced;
            result.angle = -angle;
            result.position = -position;
            result.trackWidth = trackWidth;
            result.correctMeasure = -correctMeasure;
            result.measure = -measure;

            return result;
        }

        public static boolean isHeader(String s) {
            return s.startsWith("R");
        }

        public static StrippedData read(String s)
                throws Exception {
            try {
                StrippedData result = new StrippedData();
                String line = s;
                StringTokenizer tokens = new StringTokenizer(line, " ");

                result.distanceRaced = Double.parseDouble(tokens.nextToken());
                result.angle = Double.parseDouble(tokens.nextToken());
                result.position = Double.parseDouble(tokens.nextToken());
                result.trackWidth = Double.parseDouble(tokens.nextToken());

                // track sensor
                for (int i = 0; i < result.track.length; ++i) {
                    result.track[i] = Double.parseDouble(tokens.nextToken());
                }
                result.correctMeasure = Double.parseDouble(tokens.nextToken());
                result.measure = Double.parseDouble(tokens.nextToken());

                return result;
            } catch (Exception e) {
                System.out.println(s);
                throw e;
            }
        }

        public void write(java.io.OutputStreamWriter w)
                throws Exception {
            write(w, " ");
        }

        public void write(java.io.OutputStreamWriter w, final String DELIMITER)
                throws Exception {
            // distance raced - angle - position - trackWidth
            w.write(String.valueOf(distanceRaced) + DELIMITER);
            w.write(String.valueOf(angle) + DELIMITER);
            w.write(String.valueOf(position) + DELIMITER);
            w.write(String.valueOf(trackWidth) + DELIMITER);

            // track sensor
            for (int i = 0; i < track.length; ++i) {
                w.write(String.valueOf(track[i]) + DELIMITER);
            }

            // correct measure and measure
            w.write(String.valueOf(correctMeasure) + DELIMITER);
            w.write(String.valueOf(measure));

            w.write("\n");
        }

        public static void writeHeader(java.io.OutputStreamWriter w)
                throws Exception {
            StrippedData.writeHeader(w, " ");
        }

        public static void writeHeader(java.io.OutputStreamWriter w, final String DELIMITER)
                throws Exception {
            w.write("RaceD" + DELIMITER + "angle" + DELIMITER + "position" + DELIMITER
                    + "trackWidth" + DELIMITER);
            for (int i = 0; i < 19; ++i) {
                w.write("tes" + i + DELIMITER);
            }
            w.write("correctMeasure" + DELIMITER);
            w.write("measure");
            w.write("\n");
        }
    }
}
