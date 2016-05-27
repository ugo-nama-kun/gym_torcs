/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.track;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.classification.*;
import de.janquadflieg.mrracer.gui.GraphicDebugable;
import de.janquadflieg.mrracer.telemetry.ModifiableSensorData;
import de.janquadflieg.mrracer.telemetry.SensorData;

import java.io.*;
import java.util.*;

/**
 *
 * @author Jan Quadflieg
 */
public final class TrackModel
        implements Serializable {

    public static boolean TEXT_DEBUG = false;
    static final long serialVersionUID = 8633906798119644633L;
    /** Trackmodel file name extension. */
    public static final String TM_EXT = ".saved_model";
    /** String to identify the version information. */
    private static final String VERSION_IDENTIFIER = "VERSION";
    /** Name of the track. */
    private String name = "Unknown Track";
    /** Length of the whole track. */
    private double length = -1;
    /** Width of the track. */
    private double width = -1;
    /** The segments of the track. */
    private ArrayList<TrackSegment> segments = new ArrayList<TrackSegment>();
    /** Do we know the complete track? */
    private boolean complete = false;
    /** Has the model been initialized? */
    private boolean initialized = false;
    /** Has this model been filtered? */
    private boolean filtered = false;
    /** Helper object which builds the track model, null if the model is complete. */
    private transient TrackModelRecorder recorder = null;
    /** Positions of the places in the starting grid. */
    private double[] grid = new double[20];
    /** Raw data collected during the recording. */
    private double[] rawData = null;
    /** First derivation of raw data. */
    protected double[] rawDataDer1 = null;
    /** Second derivation of raw data. */
    protected double[] rawDataDer2 = null;

    public TrackModel() {
        recorder = new TrackModelRecorder();
        Arrays.fill(grid, -1);
    }

    public TrackModel(String name) {
        this.name = name;
        recorder = new TrackModelRecorder();
        Arrays.fill(grid, -1);
    }

    public double getRawData(double d) {
        return rawData[(int) Math.floor(d)];
    }

    public double[] getRawData() {
        return rawData;
    }

    public void adjustWidth(double d) {
        if (!complete && !initialized) {
            this.width = d;
        }
    }

    public void append(SensorData data, Situation s) {
        append(data, s, false);
    }

    public void append(SensorData data, Situation s, boolean noisy) {
        if (complete) {
            recorder = null;
            return;
        }

        recorder.append(data, s, noisy);
    }

    private void checkIntegrity() {
        double start = segments.get(0).getStart();
        double end = segments.get(0).getEnd();
        if (start != 0.0) {
            //System.out.println("WARNING: First segment not at 0.0m!");
        }
        for (int i = 1; i < segments.size(); ++i) {
            start = segments.get(i).getStart();
            if (start != end) {
                //System.out.println("Segment[" + (i - 1) + "] end: " + end +
                //        ", segment[" + i + "] start: " + start);
            }
            end = segments.get(i).getEnd();
        }
    }

    public boolean complete() {
        return complete;
    }

    public boolean initialized() {
        return this.initialized;
    }

    public double confidence() {
        if (complete) {
            return 1.0;

        } else {
            return recorder.known / length;
        }
    }

    public final int decIdx(int index) {
        return (index - 1 + segments.size()) % segments.size();
    }

    public final int decrementIndex(int index) {
        return decIdx(index);
    }

    public final int incIdx(int index) {
        return (index + 1 + segments.size()) % segments.size();
    }

    public final int incrementIndex(int index) {
        return incIdx(index);
    }

    private void filter(boolean noisy) {
        if (noisy) {
            filter(7.0);
        } else {
            filter(5.0);
        }
    }

    private void filter(final double MAX_VALUE) {
        if (TEXT_DEBUG) {
//            System.out.println("");
//            System.out.println("-----------TrackModel.filter(" + MAX_VALUE + ")-----------------");
//            System.out.println("Segments: " + segments.size());
        }

        for (int k = 0; k < segments.size(); ++k) {
            int prev = decIdx(k);
            int next = incIdx(k);

            boolean allKnown = !(segments.get(prev).isUnknown() || segments.get(k).isUnknown() || segments.get(next).isUnknown());
            double maxValue = 0.0;
            if (segments.get(k).isCorner()) {
                TrackSegment.Apex apexes[] = segments.get(k).getApexes();
                for (TrackSegment.Apex a : apexes) {
                    maxValue = Math.max(maxValue, Math.abs(a.value));
                }
            }


            // "vollgaskurven", die gar keine sind
            if (allKnown && segments.get(k).isFull() && maxValue < MAX_VALUE) {
                if (TEXT_DEBUG) {
                    System.out.println("Removing full speed segment with small maxValue" + (k) + ", maxValue=" + maxValue);
                }

                // außen gerade
                if (segments.get(prev).getDirection() == segments.get(next).getDirection()
                        && segments.get(prev).getDirection() == Situations.DIRECTION_FORWARD) {
                    if (TEXT_DEBUG) {
                        System.out.println("   Within straights");
                    }
                    if (k == 0) {
                        segments.get(next).extendFront(segments.get(k).getStart());
                        segments.remove(k);
                        k = 1;

                    } else if (k == segments.size() - 1) {
                        segments.get(prev).extendBack(segments.get(k).getEnd());
                        segments.remove(k);
                        k = 1;

                    } else {
                        segments.get(prev).join(segments.get(next));
                        segments.remove(k);
                        segments.remove(k);
                        k = 1;
                    }

                } else if (segments.get(prev).getDirection() == Situations.DIRECTION_FORWARD
                        && k > 0) {
                    if (TEXT_DEBUG) {
                        System.out.println("   Between straight and corner, extending straight");
                    }
                    segments.get(prev).extendBack(segments.get(k).getEnd());
                    segments.remove(k);
                    k = 1;


                } else if (segments.get(next).getDirection() == Situations.DIRECTION_FORWARD
                        && k < segments.size() - 1) {
                    if (TEXT_DEBUG) {
                        System.out.println("   Between corner and straight, extending straight");
                    }
                    segments.get(next).extendFront(segments.get(k).getStart());
                    segments.remove(k);
                    k = 1;


                } else {
                    if (TEXT_DEBUG) {
                        System.out.println("   Between corners, changing to straight");
                    }
                    segments.get(k).toStraight();
                    k = 0;
                }

                // kurze vollgaskurven, eingeschlossen von geraden
            } else if (allKnown && segments.get(prev).getDirection() == segments.get(next).getDirection()
                    && segments.get(prev).getDirection() == Situations.DIRECTION_FORWARD
                    && segments.get(k).getLength() < 2.0 * width && segments.get(k).getType() == Situations.TYPE_FULL
                    && k > 0 && k < segments.size() - 1) {
                if (TEXT_DEBUG) {
                    System.out.println("Removing short full speed segment " + k);
                }

                //System.out.println(i-1+" Vollgaskurve zwischen geraden, length "+Utils.dTS(segments.get(i - 1).getLength())+" < "+
                //        Utils.dTS(2.0*width));
                segments.get(prev).join(segments.get(next));
                segments.remove(k);
                segments.remove(k);
                k = 1;

                // ganz kurze kurven
            } else if (allKnown && segments.get(k).getLength() < 5.0
                    && segments.get(k).getDirection() != Situations.DIRECTION_FORWARD
                    && k > 0 && k < segments.size() - 1) {
                if (TEXT_DEBUG) {
                    System.out.println("Removing short corner: " + (k));
                }
                //System.out.println(i-1+" kurze Kurve");

                if (segments.get(prev).getDirection() != segments.get(next).getDirection()) {
                    double middle = segments.get(k).getStart() + (segments.get(k).getLength() / 2.0);
                    segments.get(prev).extendBack(middle);
                    segments.get(next).extendFront(middle);
                    segments.remove(k);
                    k = 1;

                } else {
                    segments.get(prev).join(segments.get(next));
                    segments.remove(k);
                    segments.remove(k);
                    k = 1;
                }
            }
        }

        for (int i = 2; complete && i < segments.size(); ++i) {
            boolean allKnown = !(segments.get(i - 2).isUnknown() || segments.get(i - 1).isUnknown() || segments.get(i).isUnknown());
            double maxValue = 0.0;
            if (segments.get(i - 1).isCorner()) {
                TrackSegment.Apex apexes[] = segments.get(i - 1).getApexes();
                for (TrackSegment.Apex a : apexes) {
                    maxValue = Math.max(maxValue, Math.abs(a.value));
                }
            }

            // kurze geraden, von kurven eingeschlossen
            if (allKnown && segments.get(i - 1).getDirection() == Situations.DIRECTION_FORWARD && segments.get(i - 1).getLength() < 3.0 * width) {
                if (TEXT_DEBUG) {
                    System.out.println("Removing short straight " + (i - 1));
                }
                //System.out.println(i-1+" Kurze Gerade");

                if (segments.get(i - 2).getDirection() != segments.get(i).getDirection()) {
                    double middle = segments.get(i - 1).getStart() + (segments.get(i - 1).getLength() / 2.0);
                    segments.get(i - 2).extendBack(middle);
                    segments.get(i).extendFront(middle);
                    segments.remove(i - 1);
                    i = 1;

                } else {
                    segments.get(i - 2).join(segments.get(i));
                    segments.remove(i - 1);
                    segments.remove(i - 1);
                    i = 1;
                }
            }
        }

        if (this.complete) {
            this.filtered = true;
        }
        if (TEXT_DEBUG) {
//            System.out.println("-----------TrackModel.filter end-----------------");
//            System.out.println("Segments: " + segments.size());
//            System.out.println("");
        }
    }

    public double getGridPosition(int i) {
        return grid[i - 1];
    }

    public int gridPositions() {
        return this.grid.length;
    }

    public double getLength() {
        return this.length;
    }

    public String getName() {
        return name;
    }

    public int getIndex(double distance) {
        if (!initialized()) {
            return -1;
        }

        if (distance >= length) {
            return segments.size() - 1;
        }

        for (int i = 0; i < segments.size(); ++i) {
            if (segments.get(i).getStart() <= distance && distance < segments.get(i).getEnd()) {
                return i;
            }
        }
        //System.out.println("Warning, index for "+distance+" not found!");
        //print();
        return -1;
    }

    public int[] getIndices(SensorData data, double distance) {
        ArrayList<Integer> list = new ArrayList<Integer>();

        int index = getIndex(data.getDistanceFromStartLine());

        if (index != -1) {
            list.add(index);
            distance -= segments.get(index).getStart() + segments.get(index).getLength() - data.getDistanceFromStartLine();

            while (distance > 0) {
                index = incrementIndex(index);
                list.add(index);
                distance -= segments.get(index).getLength();
            }
        }


        int[] result = new int[list.size()];
        for (int i = 0; i < result.length; ++i) {
            result[i] = list.get(i);
        }

        return result;
    }

    public int[] getIndices(SensorData data, double distance, int maxNum) {
        ArrayList<Integer> list = new ArrayList<Integer>();

        int index = getIndex(data.getDistanceFromStartLine());

        if (index != -1) {
            list.add(index);
            distance -= segments.get(index).getStart() + segments.get(index).getLength() - data.getDistanceFromStartLine();

            while (distance > 0 && list.size() < maxNum) {
                index = incrementIndex(index);
                list.add(index);
                distance -= segments.get(index).getLength();
            }
        }


        int[] result = new int[list.size()];
        for (int i = 0; i < result.length; ++i) {
            result[i] = list.get(i);
        }

        return result;
    }

    public TrackSegment getSegment(int index) {
        int i = (index + segments.size()) % segments.size();

        return segments.get(i);
    }

    public double getWidth() {
        return width;
    }

    public TrackSegment getSegment(double distance) {
        return getSegment(getIndex(distance));
    }

    public static TrackModel load(String file)
            throws Exception {
        TrackModel result = new TrackModel();

        int version = 1;

        BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(file), "UTF-8"));
        String line;
        StringTokenizer tokenizer;

        // name 
        line = reader.readLine();
        result.name = line;

        // version information, if there is any
        line = reader.readLine();
        if (line.startsWith(TrackModel.VERSION_IDENTIFIER)) {
            version = Integer.parseInt(line.substring(line.indexOf(" ") + 1));
            //System.out.println("Detected version: "+version);
            line = reader.readLine();
        }

        // length && width
        tokenizer = new StringTokenizer(line, " ");
        result.length = Double.parseDouble(tokenizer.nextToken());
        result.width = Double.parseDouble(tokenizer.nextToken());

        // complete? && firstLapComplete?
        line = reader.readLine();
        tokenizer = new StringTokenizer(line, " ");
        result.complete = Boolean.parseBoolean(tokenizer.nextToken());
        if (version >= 2) {
            result.filtered = Boolean.parseBoolean(tokenizer.nextToken());
            if (!result.filtered) {
                System.out.println("WARNING - NOT FILTERED");
            }
        }

        if (!result.complete) {
            result.recorder.lapStarted = true;
            result.recorder.firstPacket = false;
            result.recorder.lastOfTrack = true;
        }

        // grid positions
        line = reader.readLine();
        tokenizer = new StringTokenizer(line, " ");
        for (int i = 0; i < result.grid.length; ++i) {
            result.grid[i] = Double.parseDouble(tokenizer.nextToken());
        }

        // number of segments and segments
        line = reader.readLine();
        int numSegments = Integer.parseInt(line);
        result.initialized = (numSegments != 0);

        for (int i = 0; i < numSegments; ++i) {
            line = reader.readLine();
            TrackSegment segment = TrackSegment.read(line, version, result.complete);
            if (segment.isUnknown()) {
                result.recorder.unknownSegments++;
            } else {
                result.recorder.known += segment.getLength();
            }
            result.segments.add(segment);
        }

        if (version > 1) {
            line = reader.readLine();
            tokenizer = new StringTokenizer(line, " ");
            result.rawData = new double[(int) Math.ceil(result.length)];
            for (int i = 0; i < result.rawData.length; ++i) {
                result.rawData[i] = Double.parseDouble(tokenizer.nextToken());
            }

            result.rawDataDer1 = new double[result.rawData.length - 1];
            for (int i = 1; i < result.rawData.length; ++i) {
                result.rawDataDer1[i - 1] = result.rawData[i] - result.rawData[i - 1];
            }

            result.rawDataDer2 = new double[result.rawDataDer1.length - 1];
            for (int i = 1; i < result.rawDataDer1.length; ++i) {
                result.rawDataDer2[i - 1] = result.rawDataDer1[i] - result.rawDataDer1[i - 1];
            }

        }

        reader.close();

        if (result.complete) {
            for (TrackSegment s : result.segments) {
                s.setWidth(result.width);
            }


            if (version <= 1 || (version >= 2 && !result.filtered)) {
                result.checkIntegrity();
                for (TrackSegment s : result.segments) {
                    s.setWidth(result.width);
                    s.filter();
                }
                result.filter(false);
                for (int i = 0; i < result.segments.size(); ++i) {
                    result.segments.get(i).initTargetSpeeds();
                }
                result.filtered = true;
            }


            // HACK
            //System.out.println("Searching...");
            int start = -1;
            int end = -1;
            boolean finished = false;
            boolean restart = false;

            for (int i = 0; !finished; ++i) {

                int idx = i % result.rawData.length;

                // search the beginning of a corner
                if (start == -1 && result.rawData[idx] != 0.0) {
                    start = idx;

                } else if (start != -1 && (result.rawData[idx] == 0
                        || Math.signum(result.rawData[idx]) != Math.signum(result.rawData[start]))) {
                    end = idx;

                    double zeroMiddlePosition = start + ((end - start) / 2.0);
                    result.getSegment(zeroMiddlePosition).zeroMiddlePosition = zeroMiddlePosition;

                    double sum = 0.0;
                    for (int j = start; j <= end; ++j) {
                        int idx2 = j % result.rawData.length;
                        sum += Math.abs(result.rawData[idx2]);
                    }

                    double target = sum / 2.0;
                    double integralPosition = -1.0;
                    sum = 0.0;
                    for (int j = start; j <= end && integralPosition == -1.0; ++j) {
                        int idx2 = j % result.rawData.length;
                        sum += Math.abs(result.rawData[idx2]);
                        if (sum > target) {
                            integralPosition = idx2 * 1.0;
                        }
                    }
                    result.getSegment(integralPosition).integralPosition = integralPosition;
                    //System.out.println(start+" - "+end);
                    //System.out.println("int: "+integralPosition + ", zero:"+zeroMiddlePosition);
                    //System.out.println("");

                    if (result.rawData[idx] == 0.0) {
                        start = -1;
                        end = -1;

                    } else {
                        start = idx;
                    }

                    if (restart) {
                        finished = true;
                    }

                } else if (start != -1 && i == result.rawData.length - 1) {
                    restart = true;
                }

                if (i == result.rawData.length - 1) {
                    if (start == -1) {
                        finished = true;

                    } else {
                        restart = true;
                    }
                }
            }
        }

        for (int i = 0; i < result.size(); ++i) {
            TrackSegment s = result.getSegment(i);
            if (s.isCorner()) {
                //System.out.println(i);
                for (TrackSegment.Apex a : s.getApexes()) {
                    //System.out.println(a.toString());
                }
            }
        }

        return result;
    }

    public void print() {
        System.out.println(toString());
    }

    public void save(String file)
            throws Exception {
        FileOutputStream fo = new FileOutputStream(file);
        OutputStreamWriter w = new OutputStreamWriter(fo, "UTF-8");

        write(w);

        w.flush();
        fo.flush();
        w.close();
        fo.close();
    }

    public void setGridPosition(int position, double delta) {
        grid[position - 1] = delta;
    }

    public void setName(String s) {
        this.name = s;
    }

    public int size() {
        return segments.size();
    }

    @Override
    public String toString() {
        StringBuilder result = new StringBuilder();

        result.append("Trackmodel of ").append(name).append(", length: ").append(Utils.dTS(length)).append("m, width: ").append(Utils.dTS(width)).append("m\n");
        result.append("Complete: ").append(complete).append("\n");
        result.append("Filtered: ").append(filtered).append("\n");
        result.append("Grid: ");
        for (int i = 0; i < grid.length; ++i) {
            result.append(Utils.dTS(grid[i])).append(" ");
        }
        result.append("\n");
        result.append("Confidence: ").append(Utils.dTS(confidence() * 100)).append("%\n");
        if (!complete) {
            result.append("Unknown segments: ").append(recorder.unknownSegments).append("\n");
        }
        for (int i = 0; i < segments.size(); ++i) {
            result.append(String.format("%3d ", i)).append(segments.get(i).toString(true)).append("\n");
        }

        return result.toString();
    }

    public void write(java.io.OutputStreamWriter w)
            throws Exception {
        int version = 3;

        // name
        w.write(name);
        w.write("\n");
        // version
        w.write(TrackModel.VERSION_IDENTIFIER + " " + String.valueOf(version));
        w.write("\n");
        // length && width
        w.write(String.valueOf(length));
        w.write(" ");
        w.write(String.valueOf(width));
        w.write("\n");
        // complete?, filtered?
        w.write(String.valueOf(complete));
        w.write(" ");
        if (version >= 2) {
            w.write(String.valueOf(filtered));
        }
        w.write("\n");
        // grid positions
        for (int i = 0; i < grid.length; ++i) {
            w.write(String.valueOf(grid[i]) + " ");
        }
        w.write("\n");
        // number of segments
        w.write(String.valueOf(segments.size()));
        w.write("\n");
        // segments
        for (TrackSegment seg : segments) {
            seg.write(w, version);
            w.write("\n");
        }
        // rawdata
        if (version > 1) {
            for (int i = 0; i < rawData.length; ++i) {
                if (Math.abs(rawData[i]) < 0.1) {
                    rawData[i] = 0.0;
                }
                w.write(String.valueOf(rawData[i]));
                w.write(" ");
            }
            w.write("\n");
        }
    }

    private class TrackModelRecorder {

        /** How much of the track is known? .*/
        private double known = 0.0;
        private boolean lapStarted = false;
        private boolean firstPacket = true;
        private boolean lastOfTrack = false;
        private int unknownSegments = 0;
        private ModifiableSensorData lastData = new ModifiableSensorData();

        public TrackModelRecorder() {
        }

        public void append(SensorData data, Situation situation, boolean noisy) {
            if (firstPacket) {
                firstPacket = false;

                TrackModel.this.grid[data.getRacePosition() - 1] = data.getDistanceFromStartLine();

                double[] sensors = data.getTrackEdgeSensors();
                TrackModel.this.width = sensors[0] + sensors[18];

                lastData.setData(data);
                return;
            }

            // if we never crossed the start / finish line, check if we reached it
            if (!lapStarted && data.getDistanceFromStartLine() < lastData.getDistanceFromStartLine() && data.getDistanceRaced() > lastData.getDistanceRaced()) {

                lapStarted = true;

                TrackModel.this.length = lastData.getDistanceFromStartLine() + ((data.getDistanceRaced() - lastData.getDistanceRaced()) - data.getDistanceFromStartLine());

                TrackModel.this.rawData = new double[(int) Math.ceil(TrackModel.this.length)];

                if (situation.isOutside() || situation.hasError()) {
                    // add one unknown segment for the whole track
                    TrackSegment segment = new TrackSegment(0.0, TrackModel.this.length,
                            TrackModel.this.width);
                    TrackModel.this.segments.add(segment);

                    lastOfTrack = true;

                } else {
                    TrackSegment segment = new TrackSegment(0.0,
                            data.getDistanceFromStartLine(), situation,
                            TrackModel.this.width);
                    TrackModel.this.segments.add(segment);
                    known += data.getDistanceFromStartLine();

                    // add one unknown segment for the rest of the track
                    segment = new TrackSegment(data.getDistanceFromStartLine(),
                            TrackModel.this.length,
                            TrackModel.this.width);
                    TrackModel.this.segments.add(segment);

                    lastOfTrack = false;
                }

                TrackModel.this.initialized = true;
                unknownSegments = 1;
                lastData.setData(data);
                return;
            }

            if (!lapStarted) {
                lastData.setData(data);
                return;
            }

            if (!data.onTrack() || situation.hasError()) {
                lastOfTrack = true;
                lastData.setData(data);
                return;
            }

            if (lastOfTrack && data.onTrack() && !situation.hasError()) {
                lastOfTrack = false;
                lastData.setData(data);
                return;
            }

//            int lastUnknown = unknownSegments;
//            String old = TrackModel.this.toString();
//            int realUnknown = 0;
//            String log = "";
//            for(int i=0; i < TrackModel.this.segments.size(); ++i){
//                if(TrackModel.this.segments.get(i).isUnknown()){
//                    ++realUnknown;
//                    log += i+" ";
//                }
//            }

//            if(realUnknown != unknownSegments){
//                System.out.println(data.getDistanceFromStartLineS()+" "+log);
//            }

//            if(data.getDistanceFromStartLine() == 6355.48){
//                System.out.println("Here");
//            }

            if (data.getDistanceRaced() > lastData.getDistanceRaced()) {
                TrackModel.this.rawData[(int) Math.floor(data.getDistanceFromStartLine())] = situation.getMeasure();
                //TrackModel.this.rawData[(int) Math.floor(data.getDistanceFromStartLine())] = new AngleBasedClassifier(2).classify(data).getMeasure();
                extendModel(data, lastData, situation);

            } else {
                return;
            }

//            if (unknownSegments < 0) {
//                System.out.println("***************************************************************");
//                System.out.println(old);
//                System.out.println("***************************************************************");
//                System.out.println(TrackModel.this.toString());
//                System.out.println("***************************************************************");
//                System.out.println("lastUnknown: " + lastUnknown);
//                System.out.println("unknown: " + unknownSegments);
//                System.out.println("Appended: " + situation.toString());
//                System.out.println("@" + data.getDistanceFromStartLineS());
//                System.out.println("***************************************************************");
//                System.out.println("");
//            }

//            if (unknownSegments < 0) {
//                System.out.println("[WARNING] TrackModelRecorder.unknownSegments=" + unknownSegments + " @" + data.getDistanceFromStartLineS() + " / " + TrackModel.this.length);
//            }

            unknownSegments = Math.max(unknownSegments, 0);

            TrackModel.this.complete = (unknownSegments == 0);

            long start = System.nanoTime();

            for (int i = 0; i < TrackModel.this.segments.size(); ++i) {
                boolean prevKnown = true;
                boolean nextKnown = true;
                if (i != 0) {
                    prevKnown = !TrackModel.this.segments.get(i - 1).isUnknown();
                }
                if (i != TrackModel.this.segments.size() - 1) {
                    nextKnown = !TrackModel.this.segments.get(i + 1).isUnknown();
                }

                if (prevKnown && nextKnown && !TrackModel.this.segments.get(i).isUnknown()
                        && !TrackModel.this.segments.get(i).filtered()) {
                    TrackModel.this.segments.get(i).setWidth(TrackModel.this.width);
                    if (TEXT_DEBUG) {
                        System.out.println("Filtering segment [" + (i + 1) + "/" + segments.size()
                                + "," + TrackModel.this.segments.get(i).getStart() + "] " + Situations.toString(TrackModel.this.segments.get(i).getType())
                                + ", direction: " + Situations.toString(TrackModel.this.segments.get(i).getDirection()));
                    }
                    TrackModel.this.segments.get(i).filter(noisy);
                }

                TrackModel.this.filter(noisy);
            }

            if (TrackModel.this.complete) {
                if (TEXT_DEBUG) {
                    System.out.println("Model complete");
                }
                TrackModel.this.checkIntegrity();
                for (int i = 0; i < TrackModel.this.segments.size(); ++i) {
                    TrackModel.this.segments.get(i).setWidth(TrackModel.this.width);
                    TrackModel.this.segments.get(i).filter(noisy);
                }
                TrackModel.this.filter(noisy);
                for (int i = 0; i < TrackModel.this.segments.size(); ++i) {
                    TrackModel.this.segments.get(i).initTargetSpeeds();
                }
            }

            if (TEXT_DEBUG) {
                //long duration = System.nanoTime() - start;
                //System.out.println("Filtering took " + duration + "ns");
                //System.out.println("Filtering took " + (duration / 1000000) + "ms");
            }

            lastData.setData(data);
        }

        /**
         * Annahmen, wenn diese Methode aufgerufen wird:
         * - Beim letzten Paket und bei diesem Paket war das Auto auf der Strecke.
         * - Zwischen den Paketen ist es vorwärts gefahren.
         *
         * @param data
         * @param situation
         */
        public void extendModel(SensorData data, SensorData lastData, Situation situation) {
            double lastDistance = Math.min(lastData.getDistanceFromStartLine(),
                    TrackModel.this.length);
            double currentDistance = Math.min(data.getDistanceFromStartLine(),
                    TrackModel.this.length);
            int lastIndex = TrackModel.this.getIndex(lastDistance);
            TrackSegment lastSegment = TrackModel.this.getSegment(lastIndex);

            int currentIndex = TrackModel.this.getIndex(currentDistance);
            TrackSegment currentSegment = TrackModel.this.getSegment(currentIndex);

            int deltaIndex = (currentIndex - lastIndex + TrackModel.this.segments.size()) % TrackModel.this.segments.size();

            if (lastDistance == currentDistance) {
                //System.out.println("Last Distance == current");
                //throw new RuntimeException("Asertion failed, distance equal");
                return;
            }

            //System.out.println("deltaIndex: "+deltaIndex);

            if (deltaIndex == 0 && currentSegment.isUnknown()) {
                // esoterische situation, die nur beim start/ziel auftreten kann
                double start = lastDistance;
                double end = currentDistance;
                if (end < start) {
                    TrackModel.this.segments.clear();
                    if (end > 0) {
                        TrackModel.this.segments.add(new TrackSegment(0, end, situation, TrackModel.this.width));
                    }
                    TrackModel.this.segments.add(new TrackSegment(end, start, TrackModel.this.width));
                    TrackModel.this.segments.add(new TrackSegment(start, TrackModel.this.length, situation, TrackModel.this.width));

                    unknownSegments = 1;
                    known = end + (TrackModel.this.length - start);
                    return;

                } else {
                    // normalfall
                    insertWithin(lastDistance,
                            currentDistance, situation);
                }

            } else if (deltaIndex == 1 && (lastSegment.isUnknown() || currentSegment.isUnknown())) {
                insertData(lastDistance,
                        currentDistance,
                        situation);

            } else if (deltaIndex > 1) {
                double[] startPoints = new double[deltaIndex + 1];
                double[] endPoints = new double[deltaIndex + 1];

                int idx = lastIndex;
                startPoints[0] = lastDistance;
                endPoints[0] = lastSegment.getEnd();

                for (int i = 1; i < deltaIndex; ++i) {
                    idx = TrackModel.this.incrementIndex(idx);
                    startPoints[i] = TrackModel.this.getSegment(idx).getStart();
                    endPoints[i] = TrackModel.this.getSegment(idx).getEnd();
                }
                startPoints[deltaIndex] = currentSegment.getStart();
                endPoints[deltaIndex] = currentDistance;

//                for(int i=0; i < startPoints.length; ++i){
//                    System.out.println(i+" - SP: "+startPoints[i]+", EP: "+endPoints[i]);
//                }

                // schleife
                for (int i = 0; i < deltaIndex; ++i) {
                    insertData(startPoints[i], endPoints[i], situation);
                }

                insertWithin(startPoints[deltaIndex], endPoints[deltaIndex], situation);
            }
        }

        private void insertData(double start, double end, Situation s) {
            int preIndex = TrackModel.this.getIndex(start);
            int currIndex = TrackModel.this.getIndex(end);
            TrackSegment lastSegment = TrackModel.this.getSegment(preIndex);
            TrackSegment currentSegment = TrackModel.this.getSegment(currIndex);

            if (lastSegment.isUnknown() && currentSegment.isUnknown()) {
                // kann nur bei start / ziel auftreten
                insertFront(preIndex, start, s);
                insertBack(0, end, s);

            } else if (lastSegment.isUnknown()) {
                if (currIndex != 0 && currentSegment.getDirection() == s.getDirection()) {
                    extendFront(currIndex, preIndex, start, s);

                } else {
                    insertFront(preIndex, start, s);
                }

            } else if (currentSegment.isUnknown()) {
                if (preIndex != TrackModel.this.segments.size() - 1 && lastSegment.getDirection() == s.getDirection()) {
                    extendBack(preIndex, currIndex, end, s);

                } else {
                    insertBack(currIndex, end, s);
                }
            }
        }

        /**
         * Helper method to extend a known segment at the front.
         */
        private void extendFront(int knownIndex, int unknownIndex, double start, Situation s) {
            TrackSegment knownSegment = TrackModel.this.segments.get(knownIndex);
            TrackSegment unknownSegment = TrackModel.this.segments.get(unknownIndex);

            boolean removeUnknown;
            double newStart = start;

            if (unknownIndex != 0) {
                removeUnknown = start <= unknownSegment.getStart();

            } else {
                removeUnknown = start == unknownSegment.getStart() || start > unknownSegment.getEnd();
            }

            if (removeUnknown) {
                newStart = unknownSegment.getStart();
            }

            knownSegment.extendFront(newStart, s);
            known += unknownSegment.getEnd() - knownSegment.getStart();

            if (removeUnknown) {
                TrackModel.this.segments.remove(unknownIndex);
                --unknownSegments;

                // check if we can join this segment with the next one
                int preIndex = TrackModel.this.decrementIndex(unknownIndex);
                tryToJoin(preIndex, unknownIndex);

            } else {
                unknownSegment.shortenBack(start);
            }
        }

        /**
         * Helper method to extend a known segment at the end.
         */
        private void extendBack(int knownIndex, int unknownIndex, double end, Situation s) {
            TrackSegment knownSegment = TrackModel.this.segments.get(knownIndex);
            TrackSegment unknownSegment = TrackModel.this.segments.get(unknownIndex);

            boolean removeUnknown;
            double newKnownEnd = end;

            if (unknownIndex != TrackModel.this.segments.size() - 1) {
                removeUnknown = end >= unknownSegment.getEnd();

            } else {
                removeUnknown = end == unknownSegment.getEnd() || end < unknownSegment.getStart();
            }

            if (removeUnknown) {
                newKnownEnd = unknownSegment.getEnd();
            }

            knownSegment.extendBack(newKnownEnd, s);
            known += knownSegment.getEnd() - unknownSegment.getStart();

            if (removeUnknown) {
                TrackModel.this.segments.remove(unknownIndex);
                unknownSegments -= 1;

                // check if we can join this segment with the next one
                int succIndex = TrackModel.this.incrementIndex(knownIndex);
                tryToJoin(knownIndex, succIndex);
            } else {
                unknownSegment.shortenFront(newKnownEnd);
            }
        }

        /**
         * Helper method to insert a segment at the front of the known segment.
         */
        private void insertFront(int unknownIndex, double start, Situation s) {
            TrackSegment unknownSegment = TrackModel.this.segments.get(unknownIndex);

            boolean removeUnknown;
            double newStart = start;

            if (unknownIndex != 0) {
                removeUnknown = start <= unknownSegment.getStart();

            } else {
                removeUnknown = start == unknownSegment.getStart() || start > unknownSegment.getEnd();
            }

            if (removeUnknown) {
                newStart = unknownSegment.getStart();
            }

            TrackSegment newSegment = new TrackSegment(newStart, unknownSegment.getEnd(), s, TrackModel.this.width);
            if (newSegment.getLength() == 0) {
                return;
            }
            known += newSegment.getLength();

            if (removeUnknown) {
                --unknownSegments;
                TrackModel.this.segments.set(unknownIndex, newSegment);

                // check if we can join the new segment with the one before
                int preIndex = TrackModel.this.decrementIndex(unknownIndex);
                tryToJoin(preIndex, unknownIndex);

            } else {
                unknownSegment.shortenBack(start);
                if (unknownIndex == TrackModel.this.segments.size() - 1) {
                    TrackModel.this.segments.add(newSegment);
                } else {
                    TrackModel.this.segments.add(
                            TrackModel.this.incrementIndex(unknownIndex),
                            newSegment);
                }
            }
        }

        /**
         * Helper method to insert a segment at the back of the known segment.
         */
        private void insertBack(int unknownIndex, double end, Situation s) {
            TrackSegment unknownSegment = TrackModel.this.segments.get(unknownIndex);

            boolean removeUnknown;
            double newEnd = end;

            if (unknownIndex != TrackModel.this.segments.size() - 1) {
                removeUnknown = end >= unknownSegment.getEnd();

            } else {
                removeUnknown = end == unknownSegment.getEnd() || end < unknownSegment.getStart();
            }

            if (removeUnknown) {
                newEnd = unknownSegment.getEnd();
            }

            TrackSegment newSegment = new TrackSegment(unknownSegment.getStart(), newEnd, s, TrackModel.this.width);
            if (newSegment.getLength() == 0) {
                return;
            }
            known += newSegment.getLength();

            if (removeUnknown) {
                --unknownSegments;
                TrackModel.this.segments.set(unknownIndex, newSegment);

                // check if we can join the new segment with the next one
                int nextIndex = TrackModel.this.incrementIndex(unknownIndex);
                tryToJoin(unknownIndex, nextIndex);

            } else {
                unknownSegment.shortenFront(newEnd);
                TrackModel.this.segments.add(unknownIndex,
                        newSegment);
            }
        }

        private void insertWithin(double start, double end,
                Situation s) {
            int unknownIndex = TrackModel.this.getIndex(start);
            TrackSegment unknownSegment = TrackModel.this.getSegment(unknownIndex);

            if (!unknownSegment.isUnknown()) {
                return;
            }

            if (start == unknownSegment.getStart()) {
                if (unknownIndex != 0 && TrackModel.this.getSegment(unknownIndex - 1).getDirection() == s.getDirection()) {
                    extendBack(unknownIndex - 1, unknownIndex, end, s);

                } else {
                    insertBack(unknownIndex, end, s);
                }

            } else {
                TrackSegment newSegment = new TrackSegment(start, end, s, TrackModel.this.width);
                known += start - end;

                if (unknownIndex == TrackModel.this.segments.size() - 1 && end == TrackModel.this.length) {
                    unknownSegment.shortenBack(start);
                    TrackModel.this.segments.add(unknownIndex, newSegment);

                } else {
                    double oldStart = unknownSegment.getStart();
                    unknownSegment.shortenFront(end);
                    TrackModel.this.segments.add(unknownIndex, newSegment);
                    TrackModel.this.segments.add(unknownIndex, new TrackSegment(oldStart, start, TrackModel.this.width));
                    ++unknownSegments;
                }
            }
        }

        private void tryToJoin(int firstIndex, int secondIndex) {
            // don't join the segments at the start / finish line
            if ((firstIndex == TrackModel.this.segments.size() - 1 && secondIndex == 0) || (secondIndex == TrackModel.this.segments.size() - 1 && firstIndex == 0)) {
                return;
            }
            // don't join segments with different directions
            if (TrackModel.this.getSegment(firstIndex).getDirection() != TrackModel.this.getSegment(secondIndex).getDirection()) {
                return;
            }
            TrackModel.this.getSegment(firstIndex).join(TrackModel.this.getSegment(secondIndex));
            TrackModel.this.segments.remove(secondIndex);
        }
    }

    public static void testWData()
            throws Exception {
        String file = "D:\\Studium\\Diplomarbeit\\bug_spa.txt";
        ArrayList<SensorData> data = new ArrayList<SensorData>(); // FIX ME
        //de.janquadflieg.mrracer.telemetry.Tools.loadSensorData(file);

        TrackModel tm = new TrackModel();
        AngleBasedClassifier c = new AngleBasedClassifier(true);

        Iterator<SensorData> it = data.iterator();
        int i = 0;
        while (it.hasNext()) {
            SensorData d = it.next();
            Situation s = c.classify(d);
            ++i;

            System.out.println("");
            System.out.println("##############################################");
            //tm.print();
            System.out.println("----------------------------------------------");
            /*System.out.println("Append e=" + Utils.dTS(d.getDistanceRaced())
            + ", d=" + Utils.dTS(d.getDistanceFromStartLine()) +
            ", " + s);*/
            System.out.println("Append e=" + d.getDistanceRaced() + ", d=" + d.getDistanceFromStartLine() + ", " + s);

            tm.append(d, s);
            System.out.println("----------------------------------------------");
            /*if(i==5229){
            tm.save("D:\\Studium\\Diplomarbeit\\bug_spa_model.txt");
            }*/
            /*if(i > 5200 || d.getDistanceFromStartLine() == 2961.24){
            tm.print();
            }*/
            /*if(i > 1000 && tm.getSegment(tm.getLength()-1).getType() == Situations.TYPE_FULL){
            System.out.println("i "+i);
            System.exit(1);
            }*/
            System.out.println("##############################################");
            System.out.println("");
        }
        tm.print();
    }

    public static void test(String path)
            throws Exception {
        String line;
        StringTokenizer tokenizer;
        double eps1, eps2, delta1, delta2;
        int s1, s2;

        // beschreibung
        File f = new File(path + File.separator + "description.txt");
        BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(f), "UTF-8"));
        line = reader.readLine();
        while (line != null) {
            System.out.println(line);
            line = reader.readLine();
        }
        reader.close();

        // daten
        f = new File(path + File.separator + "data.txt");
        reader = new BufferedReader(new InputStreamReader(new FileInputStream(f), "UTF-8"));
        line = reader.readLine();
        tokenizer = new StringTokenizer(line, " ");
        eps1 = Double.parseDouble(tokenizer.nextToken());
        delta1 = Double.parseDouble(tokenizer.nextToken());
        s1 = Integer.parseInt(tokenizer.nextToken());
        line = reader.readLine();
        tokenizer = new StringTokenizer(line, " ");
        eps2 = Double.parseDouble(tokenizer.nextToken());
        delta2 = Double.parseDouble(tokenizer.nextToken());
        s2 = Integer.parseInt(tokenizer.nextToken());
        reader.close();

        TrackModel model = TrackModel.load(path + File.separator + "model.txt");
        System.out.println("");
        model.print();
        System.out.println("");

        ModifiableSensorData d1 = new ModifiableSensorData();
        d1.setDistanceFromStartline(delta1);
        d1.setRaceDistance(eps1);
        d1.setTrackPosition(0);
        Situation sit1 = new Situation(s1, 40.0, 0.0);
        System.out.println("Append e=" + eps1 + ", d=" + delta1 + ", " + sit1);
        model.append(d1, sit1);

        ModifiableSensorData d2 = new ModifiableSensorData();
        d2.setDistanceFromStartline(delta2);
        d2.setRaceDistance(eps2);
        d2.setTrackPosition(0);
        Situation sit2 = new Situation(s2, 40.0, 0.0);
        System.out.println("Append e=" + eps2 + ", d=" + delta2 + ", " + sit2);
        model.append(d2, sit2);

        System.out.println("");
        model.print();
    }

    public static void test()
            throws Exception {
        File f = new File("D:\\Studium\\Diplomarbeit\\Data\\Tracks\\tests");
        File[] directories = f.listFiles();
        for (int i = 0; i < directories.length; ++i) {
            if (directories[i].isDirectory() && !directories[i].isHidden()) {
                System.out.println("");
                System.out.println("*********************************************");
                System.out.println("Testing: " + directories[i].getAbsolutePath());
                test(directories[i].getAbsolutePath());
                System.out.println("*********************************************");
                System.out.println("");
            }
        }
    }

    public static void main(String[] args) {
        try {
            //TrackModel tm = TrackModel.load("/home/quad/Diplomarbeit/Data/Tracks/TrackModels/wheel_2");
            //TrackModel tm = TrackModel.load("D:\\Studium\\Diplomarbeit\\Data\\Tracks\\TrackModels\\brondehach");
            TrackModel tm = TrackModel.load("X:\\Test\\goldstone\\goldstone" + TrackModel.TM_EXT);
            tm.print();

            //test();
            //testWData();

            if (1 == 1) {
                return;
            }

            TrackSegment s = tm.getSegment(39);
            System.out.println(String.format("TrackSegment %10.2f %10.2f %10.2f %3s",
                    s.getStart(), s.getStart() + s.getLength(), s.getLength(), Situations.toShortString(s.getDirection())));

            double hp = 0;
            double slow = 0;
            double med = 0;
            double full = 0;

            for (int i = 0; i < s.size(); ++i) {
                TrackSubSegment sub = s.getSubSegment(i);
                double fraction = (sub.getLength() / s.getLength()) * 100.0;
                System.out.println(String.format("[%10.2f %10.2f %10.2f %5.1f%% %4s]",
                        sub.getStart(), sub.getStart() + sub.getLength(), sub.getLength(),
                        fraction, Situations.toShortString(sub.getType())));
                if (sub.getType() == Situations.TYPE_HAIRPIN) {
                    hp += sub.getLength();

                } else if (sub.getType() == Situations.TYPE_SLOW) {
                    slow += sub.getLength();

                } else if (sub.getType() == Situations.TYPE_MEDIUM) {
                    med += sub.getLength();

                } else {
                    full += sub.getLength();
                }
            }

            System.out.println(String.format("Hairpin: %10.2f %5.1f%%", hp,
                    (hp / s.getLength()) * 100));
            System.out.println(String.format("Slow:    %10.2f %5.1f%%", slow,
                    (slow / s.getLength()) * 100));
            System.out.println(String.format("Medium:  %10.2f %5.1f%%", med,
                    (med / s.getLength()) * 100));
            System.out.println(String.format("Full:    %10.2f %5.1f%%", full,
                    (full / s.getLength()) * 100));

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
